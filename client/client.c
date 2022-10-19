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
    char ** data;
    int sizeLines;
    int sizeColumns;
    int missingLines;
} data_lines;

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
	 

int set_l2cap_mtu( int s , uint16_t mtu ) { //fonction qui change La MTU d'un socket

	struct l2cap_options opts ;
	int optlen = sizeof(opts ) ;
	int status = getsockopt(s, SOL_L2CAP, L2CAP_OPTIONS, &opts, &optlen);
	if( status == 0) {
		opts.omtu = opts.imtu = mtu ;
		status = setsockopt( s , SOL_L2CAP , L2CAP_OPTIONS , &opts ,optlen ) ;
	}
	return status ;
};

int envoie(data_lines data){

	//Création du socket dans le but de connecter entre elles les 2 raspberry
	struct sockaddr_l2 addr = { 0 } ;
	int s , status ;
	char dest[18] = "DC:A6:32:78:6C:7E";
	//de:81:c6:b4:7c:5f:46:58
	// allocate a socket
	s = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP) ;

	int mtu_value = 15620;

   	set_l2cap_mtu( s , mtu_value );

	// set the connection parameters (who to connect to)
	addr.l2_family = AF_BLUETOOTH;
	addr.l2_psm = htobs(0x1001);
	str2ba( dest , &addr.l2_bdaddr ) ;
	
	// connect to server
	status = connect (s , (struct sockaddr *)&addr , sizeof(addr ));

	// send a message

	int i,j,k;
	k = 0;
	struct timeval start, end;

	char paquet[mtu_value] = {0};
	

	if( 0 == status ) {
		printf("Connexion réussie\n");
		gettimeofday(&start, NULL);
		for (k=0;k<10;k++){
			for (i=0; i<data.sizeLines;i+=32){//ATTENTION - il utilise la taille ordinaire déjà, c'est meilleur de definir une fonction aussi(on peut mal compter) 
				memset (paquet, 0, mtu_value);  // ATTENTION  - s'il fait la substituition de tous les 0s, il change aussi les donnés 
				for(j=0; j<32; j++){
					strcat (paquet, data.data[i+j]); //ATTENTION - Si data c'est un char **, on envoie a chaque packet 32 lignes ? pq?
					//strcat (paquet, "\n");
				}
				//printf("Envoie ligne %d \n", i);
				send(s, paquet, mtu_value, 0);
			}			
			send(s, "next",4,0); // ATTENTION - je propose de change "next" par un autre chose utile, par exemple /0 ou /1

		}	
		send(s,"stop",4,0);
		gettimeofday(&end, NULL);
		printf("Temps total : %ld micro seconds\n",
		((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));// ATTENTION - fois 2
	}
	if( status < 0 ) {
		fprintf(stderr, "error code %d: %s\n", errno, strerror(errno));
		perror( "Connexion echouée" ) ;
	}
	close (s ) ;
	return 0;
}

int main(int argc , char ** argv){

	data_lines data;
	data = DataConvert("/home/pi/Desktop/Numerical_Results_capteur_v1.txt");

	envoie(data);
}
