#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <sched.h>

typedef struct {
    char ** data;		//Ensemble de données
    int sizeLines;		//Nombre de lignes de données
    int sizeColumns;	//Nombre de colonnes de données
    int missingLines;	//Nombre de lignes incomplètes
} data_lines;

int set_l2cap_mtu( int s , uint16_t mtu ) { //Fonction qui change le MTU d'un socket

	struct l2cap_options opts ;
	int optlen = sizeof(opts ) ;
	int status = getsockopt(s, SOL_L2CAP, L2CAP_OPTIONS, &opts, &optlen);
	if( status == 0) {
		opts.omtu = opts.imtu = mtu ;
		status = setsockopt( s , SOL_L2CAP , L2CAP_OPTIONS , &opts ,optlen ) ;
	}
	return status ;
}

data_lines DataConvert ( char * lien ){    //Fonction de conversion des fichiers txt en char **

    //Ouverture du fichier
	FILE * fichier = fopen(lien, "r");

    //Initialisations
    data_lines dataConverted;
	char ** data;
    dataConverted.data = NULL;
    dataConverted.sizeColumns, dataConverted.sizeLines, dataConverted.missingLines = 0;
    int sizeColumns, sizeLines = 0;
    char currentChar;    //Caractère lu actuellement
    int c1 = 1;  //Compteur de lignes
    int c2 = 1;  //Compteur de colonnes
    int c3 = 0;  //Compteur annexe

    //Allocation mémoire initiale
    data = (char **) malloc (c1 * sizeof(char *));
    data[c1 - 1] = (char *) malloc (c2 * sizeof(char));

    while ( ! feof(fichier)) {  //Tant qu'on est pas arrivés à la fin du fichier

        //Récupération du caractère lu
        currentChar = fgetc(fichier);
        int currentInt = currentChar;

        if (currentInt == 10) {    //Si on a un saut de ligne (car int "\n" = 10)

            c1++;

            if (c1==2){      
                c3 = c2;       //Stockage du nombre de colonnes "normal" du fichier
                dataConverted.sizeColumns = c3;
            } else {
                if (c2 == c3 + 1 || c2 == c3 + 2 || c2 == c3 + 3){    // Présence (ou non) des - dans les données
                    c3 = c2;     //Stockage du nouveau nombre "normal" de colonnes
                    dataConverted.sizeColumns = c3;
                }
                if (c2 != c3){
                    printf("Il manque %d caracteres dans la ligne %d du fichier %s\n", abs(c2-c3), c1-1, lien);
                    dataConverted.missingLines++;
                }
            }
            c2 = 1;      //Retour à la première colonne

            //Réallocation mémoire pour la nouvelle ligne
            data = (char **) realloc (data, c1 * sizeof(char *));
            data [c1 - 1] = (char *) malloc (c2 * sizeof(char));

        } else {

            c2++;

            //Réallocation mémoire pour la nouvelle colonne
            data[c1 - 1] = (char *) realloc (data[c1 - 1], c2 * sizeof(char));
            data[c1 - 1][c2 - 2] = currentChar;
        }
    }

    dataConverted.data = data;
    dataConverted.sizeLines = c1;

    return dataConverted;
}

int errorRate(data_lines data1, data_lines data2) {    //Fonction calculant le taux de perte et d'erreur dans la transmission

	//Initialisations
    float nb_errors, nb_data = 0;
    double loss_rate, error_rate = 0.0;
    int minLines, maxColumns, deltaLines = 0;

	//Transformations en float (pour les divisions)
    float Lines1 = data1.sizeLines;
    float mLines1 = data1.missingLines;
    float Lines2 = data2.sizeLines;
    float mLines2 = data2.missingLines;
    
	//Calcul du taux de perte
    if (Lines1 >= Lines2){   //Le fichier 1 est plus long que le 2 (ou de même taille)
        loss_rate = (Lines1 - Lines2 + mLines1 + mLines2) / Lines1 * 100;
        minLines = Lines2;
        deltaLines = Lines1 - Lines2;
        maxColumns = data1.sizeColumns;
    } else {				 //Le fichier 2 est plus long que le 1
        loss_rate = (Lines2 - Lines1 + mLines1 + mLines2) / Lines2 * 100;
        minLines = Lines1;
        deltaLines = Lines2 - Lines1;
        maxColumns = data1.sizeColumns;
    }

	//Calcul du taux de perte de données
    printf("Taux de perte de %.10lf pourcents\n", loss_rate);

	//Initialisations
    int i, j = 0;

	//Calcul du taux d'erreur
    for (i=0;i<minLines;i++){    //On doit prendre la plus petite longueur de fichier pour éviter de parcourir un fichier fini
        for (j=0;j<maxColumns;j++){
            nb_data++;
            if (data1.data[i][j] != data2.data[i][j]){      //Si les char sont différents
                nb_errors++;
            }
        }
    }

    //Ajout des lignes manquantes (car on a pris la plus petite longueur de fichier possible)
    nb_errors += deltaLines * maxColumns;
    
	//Calcul du taux d'erreur dans les données
    error_rate = nb_errors / nb_data * 100;
    printf("Taux d'erreur de %.10lf pourcents\n", error_rate);

    return 0;
}

int main(int argc , char ** argv){   //Fonction de réception des données

	//Création du socket de réception et initialisations des données
	struct sockaddr_l2 loc_addr = { 0 } , rem_addr = { 0 } ; // struct de socket
	int s, client , bytes_read ;
	unsigned int opt = sizeof(rem_addr ) ;
	int i,j=0;
	data_lines data;

	//Définition de la priorité du script en priorité temps réel
	struct sched_param sched_p;					// Création d'une structure d'ordonancement temps réel pour le programme
	sched_p.sched_priority = 50;                // Affectation d'une priorité temps réel entre 0 et 99
	if(sched_setscheduler(0, SCHED_RR, &sched_p) == -1)  {   // Affectation d'un ordonancement Round-robin avec le paramètre de priorité défini précédemment si l'opération se passe sans erreur
		perror ("sched_setscheduler \n");       // Sinon le programme se termine via la fonction perror()
	}

	//Modification du MTU
	int mtu_value = 31240;		//Valeur modifiée par le fichier python
	set_l2cap_mtu(s , mtu_value );
	
	//Conversion du fichier initial
	data_lines initial_data;
	initial_data = DataConvert("/home/pi/Documents/Numerical_Results_capteur.txt");

	//Initialisations de réception
	char buf[mtu_value];
	memset(buf,0,mtu_value * sizeof(char));   //Initialisation du buffer avec des zéros
	char test[mtu_value];
	memset(test,0,mtu_value * sizeof(char));   //Initialisation de test avec des zéros

	//Ouverture du fichier de résultat et du fichier de réception des données
	FILE* fichier = NULL;
	fichier = fopen("test.txt", "w+");
	FILE* resultat = NULL;
	resultat = fopen("result.txt","w+"); 

	//Allocation mémoire
	data = (char **)malloc(data.sizeLines * sizeof(char *));
	for (i=0; i<initial_data.sizeLines; i++){
		data[i] = (char *)calloc(initial_data.sizeColumns, sizeof(char));
	}

	//Allocation du socket
	s = socket ( AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP );
	loc_addr.l2_family = AF_BLUETOOTH;
	loc_addr.l2_bdaddr = *BDADDR_ANY;
	loc_addr.l2_psm = htobs(0x1001);
	bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));
	listen (s, 1);

	//Acceptation de la connexion entre les Raspberry
	client = accept (s , (struct sockaddr *)&rem_addr , &opt ) ;
	ba2str ( &rem_addr.l2_bdaddr , buf ) ;
	fprintf(stderr , "accepted connection from %s\n" , buf ) ;
	memset(buf , 0, sizeof(buf ));

	//Réception de données
	int check, k = 1;
	long tempsboucle, temps_envoi = 0;
	struct timeval start, end;			//Initialisation de variables de temps

	gettimeofday(&start, NULL); // ATTENTION BRUNO VA LE MODIFIER
	while(check) {
		bytes_read = recv (client , buf , sizeof(buf), 0); //Réception des données du client
		if( bytes_read > 0 ) {
			if( strcmp(buf, "stop") == 0 ){		//Quand on est arrivés à la fin des 10 envois
				check = 0;
			}else if(strcmp(buf, "next") == 0){	//Fin d'une transmission
				gettimeofday(&end, NULL);   //Initialisation du temps de fin
				temps_envoi = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
				tempsboucle+= temps_envoi;
				printf("Temps pour n = %d : %ld micro seconds\n",k, temps_envoi);  //Temps total de transmission
				fprintf(resultat, "%d : %ld ms\n",k, temps_envoi);
				k++;
				gettimeofday(&start, NULL);    //Initialisation du temps de début
			}else{
				strcpy(test, buf);   //Copie du buffer vers la mémoire "test"
				if (fichier != NULL){
					if (k==1) {     //Pour n'écrire qu'une seule fois les résultats de la transmission et non 10 fois !!
						fprintf(fichier, "%s", test);
					}
				}
			}
			memset(buf , 0, sizeof(buf ));
		}
	}
	gettimeofday(&end, NULL);  //Initialisation du temps de fin
	tempsboucle += ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
	printf("Temps total moyen : %ld micro seconds\n", tempsboucle/10);		//Temps total de transmission
	fprintf(resultat, "Moyenne : %ld ms \n", tempsboucle/10);

	data_lines final_data = DataConvert("/home/pi/Documents/test.txt");  //Conversion du fichier de transmission de données en char **

	//Calcul du taux de perte et d'erreur
	errorRate(initial_data,final_data);

	//Fermeture des sockets
	close (client) ;
	close (s) ; 
}

