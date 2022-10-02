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

char ** DataConvert ( char * lien ){

	//ouverture fichier
	FILE * fichier = fopen(lien, "r");

	assert (fichier != NULL);

	char ** data;
	int i,j,k,ind;

	//allocation mémoire
	data = (char **)malloc(6248 * sizeof(char *));
	for (i = 0; i < 6248; i++){
		data[i] = (char *)malloc(130 * sizeof(char ));
	}
	//lecture fichier
	char chaine[130]; // cdc correspondant à 1 ligne
	for(i=0;i<6248;i++){
		fgets (chaine, 130, fichier);
		strcpy(data[i], chaine);
		printf("%d : %s \n",i, data[i]);
		memset (chaine, 0, 130);
	}
	return data;
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

int envoie(char ** data){
	//Création du socket dans le but de connecter entre elles les 2 raspberry
	struct sockaddr_l2 addr = { 0 } ;
	int s , status ;
	char dest[18] = "DC:A6:32:78:6C:7E";
	//de:81:c6:b4:7c:5f:46:58
	// allocate a socket
	s = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP) ;

   	set_l2cap_mtu( s , 15620 );

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

	char paquet[4200] = {0};
	

	if( 0 == status ) {
		printf("Connexion réussie\n");
		gettimeofday(&start, NULL);
		for (k=0;k<10;k++){
			for (i=0; i<6240;i+=32){
				memset (paquet, 0, 4200);
				for(j=0; j<32; j++){
					strcat (paquet, data[i+j]);
					//strcat (paquet, "\n");
				}
				//printf("Envoie ligne %d \n", i);
				send(s, paquet, 4200, 0);
			}			
			send(s, "next",4,0);

		}	
		send(s,"stop",4,0);
		gettimeofday(&end, NULL);
		printf("Temps total : %ld micro seconds\n",
		((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));
	}
	if( status < 0 ) {
		fprintf(stderr, "error code %d: %s\n", errno, strerror(errno));
		perror( "Connexion echouée" ) ;
	}
	close (s ) ;
	return 0;
}

int main(int argc , char ** argv){

	char ** data;
	data = DataConvert("/home/pi/Desktop/Numerical_Results_capteur_v1.txt");

	envoie(data);
}
