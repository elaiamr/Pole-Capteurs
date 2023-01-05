#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

typedef struct {
    char ** data;		//Ensemble de données
    int sizeLines;		//Nombre de lignes de données
    int sizeColumns;	//Nombre de colonnes de données
    int missingLines;	//Nombre de lignes incomplètes
} data_lines;

data_lines DataConvert ( char * lien ){    //Fonction de conversion des fichiers txt en char **

    //Ouverture du fichier
    FILE * fichier = fopen(lien, "r");

    //Initialisations
    data_lines dataConverted;
    char ** data;
    dataConverted.data = NULL;
    dataConverted.sizeColumns, dataConverted.sizeLines, dataConverted.missingLines = 0;
    int sizeColumns, sizeLines = 0;
    char currentChar;    				//Caractère lu actuellement
    int c1 = 1;  						//Compteur de lignes
    int c2 = 1;  						//Compteur de colonnes
    int c3 = 0;  						//Compteur annexe

    //Allocation mémoire initiale
    data = (char **) malloc (c1 * sizeof(char *));
    data[c1 - 1] = (char *) malloc (c2 * sizeof(char));
    while ( ! feof(fichier)) {  		//Tant qu'on est pas arrivés à la fin du fichier

        //Récupération du caractère lu
        currentChar = fgetc(fichier);
        int currentInt = currentChar;

        if (currentInt == 10) {    		//Si on a un saut de ligne (car int "\n" = 10)

            c1++;

            if (c1==2){      
                c3 = c2;       			//Stockage du nombre de colonnes "normal" du fichier
                dataConverted.sizeColumns = c3;
            } else {
                if (c2 == c3 + 1 || c2 == c3 + 2 || c2 == c3 + 3 || c2 == c3 - 1 || c2 == c3 - 2 || c2 == c3 - 3){    
										// Présence (ou non) des - dans les données
                    c3 = c2;     		//Stockage du nouveau nombre "normal" de colonnes
                    dataConverted.sizeColumns = c3;
                }
                if (c2 != c3){
                    printf("Il manque %d caracteres dans la ligne %d du fichier %s\n", abs(c2-c3), c1-1, lien);
                    dataConverted.missingLines++;
                }
            }
            c2 = 1;      				//Retour à la première colonne

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
	 
int set_l2cap_mtu( int s , uint16_t mtu ) { //Fonction qui change le MTU d'un socket

	struct l2cap_options opts ;
	int optlen = sizeof(opts ) ;
	int status = getsockopt(s, SOL_L2CAP, L2CAP_OPTIONS, &opts, &optlen);
	if( status == 0) {
		opts.omtu = opts.imtu = mtu ;
		status = setsockopt( s , SOL_L2CAP , L2CAP_OPTIONS , &opts ,optlen ) ;
	}
	return status ;
};

int progress_bar(float pourcentage){
	if (pourcentage < 0.05){
		printf("[--------------------]\n");
	} else if (pourcentage < 0.1){
		printf("[#-------------------]\n");
	} else if (pourcentage < 0.15){
		printf("[##------------------]\n");
	} else if (pourcentage < 0.2){
		printf("[###-----------------]\n");
	} else if (pourcentage < 0.25){
		printf("[####----------------]\n");
	} else if (pourcentage < 0.3){
		printf("[#####---------------]\n");
	} else if (pourcentage < 0.35){
		printf("[######--------------]\n");
	} else if (pourcentage < 0.4){
		printf("[#######-------------]\n");
	} else if (pourcentage < 0.45){
		printf("[########------------]\n");
	} else if (pourcentage < 0.5){
		printf("[#########-----------]\n");
	} else if (pourcentage < 0.55){
		printf("[##########----------]\n");
	} else if (pourcentage < 0.6){
		printf("[###########---------]\n");
	} else if (pourcentage < 0.65){
		printf("[############--------]\n");
	} else if (pourcentage < 0.7){
		printf("[#############-------]\n");
	} else if (pourcentage < 0.75){
		printf("[##############------]\n");
	} else if (pourcentage < 0.8){
		printf("[###############-----]\n");
	} else if (pourcentage < 0.85){
		printf("[################----]\n");
	} else if (pourcentage < 0.9){
		printf("[#################---]\n");
	} else if (pourcentage < 0.95){
		printf("[##################--]\n");
	} else if (pourcentage < 1){
		printf("[###################-]\n");
	} else {
		printf("[####################]\n");
	}
	return 0;
}

int envoie(data_lines data){   //Fonction d'envoi de données

	//Création du socket dans le but de connecter entre elles les 2 raspberry
	struct sockaddr_l2 addr = { 0 } ;
	int s , status ;
	char dest[18] = "DC:A6:32:78:6C:7E";
	
	//Allocation du socket
	s = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP) ;

	//Modification du MTU
	int mtu_value = 2325;    										//Valeur modifiée par le fichier python
   	set_l2cap_mtu( s , mtu_value );

	//Connexion entre les 2 raspberry
	addr.l2_family = AF_BLUETOOTH;
	addr.l2_psm = htobs(0x1001);
	str2ba( dest , &addr.l2_bdaddr ) ;
	status = connect (s , (struct sockaddr *)&addr , sizeof(addr ));

	//Initialisations d'envoi
	int i,j = 0;
	struct timeval start, end;  									//Initialisation de variables de temps
	char paquet[mtu_value];
	memset(paquet,0,mtu_value * sizeof(char));  					//Initialisation du paquet avec des zéros
	
	//Envoi de données
	if( 0 == status ) {
		printf("Connexion réussie\n");
		gettimeofday(&start, NULL);									//Initialisation du temps de départ
		int nb_data = 0;               								//Nombre de données déjà inscrites dans le paquet
		float nb_data_send = 0;										//Nombre de données déjà envoyées
		float percent = 0;											//Pourcentage actuel de données envoyées
		float nb_data_total = data.sizeColumns * data.sizeLines;	//Nombre total de données à envoyer
		progress_bar(0);
		for (i=0; i<data.sizeLines;i++){
		    if ((nb_data+data.sizeColumns) >= mtu_value){        	//On prend le parti de ne pas transmettre des bouts partiels de ligne
				send(s, paquet, sizeof(paquet), 0);             	//Envoi du paquet
				nb_data_send += nb_data;
				if ((nb_data_send/nb_data_total) - percent > 0.05){
					progress_bar(nb_data_send/nb_data_total);
					percent = nb_data_send/nb_data_total;
				}
				memset(paquet,0,mtu_value * sizeof(char));  		//Initialisation du paquet avec des zéros
				nb_data = 0;                                    	//Remise à zéro du nombre de données dans le paquet
		    }
		    int j=0;
		    int c = data.data[i][j];
		    while(c == 101 || c == 43 || c == 46 || c == 32 || c == 45 || (c >= 48 && c <= 57)){	
																	//Si le caractère correspond aux données à transmettre
				paquet[nb_data] = data.data[i][j];   				//Ajout du caractère à paquet
				nb_data++;                          				//Incrémentation du nombre de données ajoutées à paquet
				j++;
				c = data.data[i][j];
		    }
		    char arobase = 64;
		    paquet[nb_data] = arobase;								//Ajout d'un @ en tant que saut de ligne
		    nb_data++;
		}		
		send(s,"stop",4,0);  										//Fin de la transmission du fichier
		gettimeofday(&end, NULL);									//Initialisation du temps de fin
		printf("Temps de transmission : %ld micro seconds\n",
		((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));  				//Temps total de transmission
	}
	if( status < 0 ) {
		fprintf(stderr, "error code %d: %s\n", errno, strerror(errno));
		perror( "Connexion echouée\n" );
	}
	close (s);  													//Fermeture du socket
	return 0;
}

int main(int argc , char ** argv){

	data_lines data;
	data = DataConvert("/home/pi/Desktop/Numerical_Results_capteur.txt");
	envoie(data);
	
}
