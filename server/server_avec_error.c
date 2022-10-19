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

/*pthread_attr_t tattr1;  // Création d'un attribut de thread

set_thread_param(&tattr1);                                                               // Appel à la fonction définisant le type de scheduling

struct sched_param param1;                                                               // Création d'un paramètre de scheduling

param1.sched_priority = 5;                                                              // Définition de la valeur de priorité temps réel pour les threads

               
err = pthread_attr_setschedparam (&tattr1, &param1);                                 // Affectation d'une priorité temps réel au paramètre tattr à partir de la valeur définie précédemment

if (err != 0)  {                                                                   // Si l'affectation de la priorité s'est terminé avec une erreur

        handle_error_en(err, "pthread_attr_setschedparam");                     // Afficher un message d'erreur dans la console

}*/

typedef struct {
    char ** data;
    int sizeLines;
    int sizeColumns;
    int missingLines;
} data_lines;

//fonction avec la librarie bluetooth
int set_l2cap_mtu( int s , uint16_t mtu ) { 
	struct l2cap_options opts ; 													//struct des packets bluetooth
	int optlen = sizeof(opts );
	int status = getsockopt ( s , SOL_L2CAP , L2CAP_OPTIONS , &opts ,&optlen ) ; 	//ecrire options d'un socket
	if( status == 0) {																// si tout va bien
		opts.omtu = opts.imtu = mtu ; 												// definir la taille du mtu envoyé et reçu
		status = setsockopt( s , SOL_L2CAP , L2CAP_OPTIONS , &opts ,optlen ) ;
	}
	return status ;
}

data_lines DataConvert ( char * lien ){

    // Ouverture du fichier
	FILE * fichier = fopen(lien, "r");

    // Initialisations
    data_lines dataConverted;
    dataConverted.data = NULL;
    dataConverted.sizeColumns, dataConverted.sizeLines, dataConverted.missingLines = 0;
    int sizeColumns, sizeLines = 0;
    char currentChar;
    char ** data;
    int c1 = 1;  // compteur de lignes
    int c2 = 1;  // compteur de colonnes
    int c3 = 0;  // compteur annexe

    // Allocation mémoire initiale
    data = (char **) malloc (c1 * sizeof(char *));
    data[c1 - 1] = (char *) malloc (c2 * sizeof(char));

    while ( ! feof(fichier)) {

        // Récupération du caractère lu
        currentChar = fgetc(fichier);
        int currentInt = currentChar;

        if (currentInt == 10) {    // Si on a un saut de ligne

            c1++;
            if (c1==2){      
                c3 = c2;       // Stockage du nombre de colonnes "normal" du fichier
                dataConverted.sizeColumns = c3;
            } else {
                if (c2 == c3 + 1 || c2 == c3 + 2 || c2 == c3 + 3 || c2 == c3 - 1 || c2 == c3 - 2 || c2 == c3 - 3){    // Présence (ou non) des - dans les données
                    c3 = c2;     // Stockage du nouveau nombre "normal" de colonnes
                    dataConverted.sizeColumns = c3;
                }
                if (c2 != c3){
                    printf("Il manque %d caracteres dans la ligne %d du fichier %s\n", abs(c2-c3), c1-1, lien);
                    dataConverted.missingLines++;
                }
            }
            c2 = 1;      // Retour à la première colonne

            // Réallocation mémoire pour la nouvelle ligne
            data = (char **) realloc (data, c1 * sizeof(char *));
            data [c1 - 1] = (char *) malloc (c2 * sizeof(char));

        } else {

            c2++;

            // Réallocation mémoire pour la nouvelle colonne
            data[c1 - 1] = (char *) realloc (data[c1 - 1], c2 * sizeof(char));
            data[c1 - 1][c2 - 2] = currentChar;
        }
    }

    dataConverted.data = data;
    dataConverted.sizeLines = c1;

    return dataConverted;
}

//Fonction taux d'erreur
int errorRate(data_lines data1, data_lines data2) {

	// Initialisations
    float nb_errors, nb_data = 0;
    double loss_rate, error_rate = 0.0;
    int minLines, maxColumns, deltaLines = 0;

	// Transformations en float (pour les divisions)
    float Lines1 = data1.sizeLines;
    float mLines1 = data1.missingLines;
    float Lines2 = data2.sizeLines;
    float mLines2 = data2.missingLines;
    
	// Calcul du taux de perte
    if (Lines1 >= Lines2){   // le fichier 1 est plus long que le 2 (ou de même taille)
        loss_rate = (Lines1 - Lines2 + mLines1 + mLines2) / Lines1 * 100;
        minLines = Lines2;
        deltaLines = Lines1 - Lines2;
        maxColumns = data1.sizeColumns;
    } else {				 // le fichier 2 est plus long que le 2
        loss_rate = (Lines2 - Lines1 + mLines1 + mLines2) / Lines2 * 100;
        minLines = Lines1;
        deltaLines = Lines2 - Lines1;
        maxColumns = data1.sizeColumns;
    }

    printf("Taux de perte de %.10lf pourcents\n", loss_rate);

	// Initialisations
    int i, j = 0;

	// Calcul du taux d'erreur
    for (i=0;i<minLines;i++){    // On doit prendre la plus petite longueur de fichier pour éviter les bugs
        for (j=0;j<maxColumns;j++){
            nb_data++;
            if (data1.data[i][j] != data2.data[i][j]){      // Si les char sont différents
                nb_errors++;
            }
        }
    }

    // Ajout des lignes manquantes (car on a pris la plus petite longueur de fichier)
    nb_errors += deltaLines * maxColumns;
    
    error_rate = nb_errors / nb_data * 100;
        
    printf("Taux d'erreur de %.10lf pourcents\n", error_rate);

    return 0;
}

int main(int argc , char ** argv){

	struct sockaddr_l2 loc_addr = { 0 } , rem_addr = { 0 } ; // struct de socket
	char buf[10000] = { 0 } ;
	int s, client , bytes_read ;
	unsigned int opt = sizeof(rem_addr ) ;
	int i,j;
	data_lines data;

	data_lines initial_data = DataConvert("/home/pi/Documents/Numerical_Results_capteur.txt");

	char test[10000] = { 0 } ;
	int mtu_value = 31240;
	//Définition de la priorité du script en priorité temps réel

	//Structure that describes scheduling parameters
	struct sched_param sched_p;					// Création d'une structure d'ordonancement temps réel pour le programme

	sched_p.sched_priority = 50;                // Affectation d'une priorité temps réel entre 0 et 99

	if(sched_setscheduler(0, SCHED_RR, &sched_p) == -1)  {   // Affectation d'un ordonancement Round-robin avec le paramètre de priorité défini précédemment si l'opération se passe sans erreur

		perror ("sched_setscheduler \n");       // Sinon le programme se termine via la fonction perror()

	}

	FILE* fichier = NULL;
	fichier = fopen("test.txt", "w+");
	// char *** datadebase = DataConvert (lien);

	FILE* resultat = NULL;
	resultat = fopen("result.txt","a"); 

	//allocation mémoire
	data = (char **)malloc(n * sizeof(char *));
	for (i=0; i<initial_data.sizeLines; i++){
		data[i] = (char *)calloc(initial_data.sizeColumns, sizeof(char));
	}

	//création socket
	s = socket ( AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP );
	
	

	set_l2cap_mtu(s , mtu_value ); // change la MTU 


	// bind socket to port 5 of the first available bluetooth adapter
	loc_addr.l2_family = AF_BLUETOOTH;
	loc_addr.l2_bdaddr = *BDADDR_ANY;
	loc_addr.l2_psm = htobs(0x1001);

	bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));
	

	// put socket into listening mode
	listen (s, 1);

	// accept one connection
	client = accept (s , (struct sockaddr *)&rem_addr , &opt ) ;
	

    //Convert MAC addresses between string representation and little-endian byte arrays
	ba2str ( &rem_addr.l2_bdaddr , buf ) ;

	//write in the buffer
	fprintf(stderr , "accepted connection from %s\n" , buf ) ;
	memset(buf , 0, sizeof(buf ));

	// read data from the client

	int check = 1;
	i=0;
	int k = 1;
	long tempsboucle = 0;
	long temps_envoi = 0;
	struct timeval start, end;
	// ATTENTION 2 - En fait ce ligne ci dessous c'est inutile
	// ATTENTION 3 - LA COMPTAGE DES TEMPS N'EST PAS DE TOUT CORRECT
	gettimeofday(&start, NULL); // ATTENTION c'est premiere temps donné une mauvaise moyenne, parce que c'est très different du prochaine temps, pas la même chose
	while(check) {
		bytes_read = recv (client , buf , sizeof(buf), 0); //recevoir le data du module connecté(client)
		if( bytes_read > 0 ) {
			if( strcmp(buf, "stop") == 0 ){// quand on est arrivé a la fin du envoie
				check = 0;
			}else if(strcmp(buf, "next") == 0){// si y'en a encore des données
				gettimeofday(&end, NULL);
				//ATTENTION tv_sec n'est pas égale a tv_usec, mais en second ? en dessus on fait simplement tv_sec*2, non?
				temps_envoi = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)); 
				tempsboucle+= temps_envoi;
				printf("Temps pour n = %d : %ld micro seconds\n",k, temps_envoi);
				fprintf(resultat, "%d : %ld ms\n",k, temps_envoi);
				k++;
				gettimeofday(&start, NULL);
			}else{
				strcpy(test, buf);
				if (fichier != NULL){
					if (k==1) {     // pour n'écrire qu'une seule fois les résultats de la transmission et non 10 fois !!
						fprintf(fichier, "%s", test);
						//printf("%s \n", test);
					}
				}
			}
			memset(buf , 0, sizeof(buf ));
		}
	}
	gettimeofday(&end, NULL);
	tempsboucle += ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)); //*2
	printf("Temps total moyen : %ld micro seconds\n", tempsboucle/10) ;
	fprintf(resultat, "Moyenne : %ld ms \n", tempsboucle/10);

	data_lines final_data = DataConvert("/home/pi/Documents/test.txt");

	errorRate(initial_data,final_data);

	close (client) ;
	close (s) ; 
}

