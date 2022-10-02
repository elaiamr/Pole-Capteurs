#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>

int set_l2cap_mtu( int s , uint16_t mtu ) {
	struct l2cap_options opts ;
	int optlen = sizeof(opts );
	int status = getsockopt ( s , SOL_L2CAP , L2CAP_OPTIONS , &opts ,&optlen ) ;
	if( status == 0) {
		opts . omtu = opts . imtu = mtu ;
		status = setsockopt( s , SOL_L2CAP , L2CAP_OPTIONS , &opts ,optlen ) ;
	}
	return status ;
}

int main(int argc , char ** argv){
	struct sockaddr_l2 loc_addr = { 0 } , rem_addr = { 0 } ;
	char buf[4200] = { 0 } ;
	int s, client , bytes_read ;
	unsigned int opt = sizeof(rem_addr ) ;
	int i,j;
	char ** data;
	int n = 6248;
	char test[4200] = { 0 } ;

	FILE* fichier = NULL;
	fichier = fopen("test.txt", "w+");
	// char *** datadebase = DataConvert (lien);

	FILE* resultat = NULL;
	resultat = fopen("result.txt","a"); 

	//allocation mémoire
	data = (char **)malloc(6248 * sizeof(char *));
	for (i=0; i<15; i++){
		data[i] = (char *)calloc(4200 , sizeof(char));
	}

	//création socket
	s = socket ( AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP );
	
	set_l2cap_mtu(s , 10000 ); // change la MTU à 10000




	// bind socket to port 5 of the first available bluetooth adapter
	loc_addr.l2_family = AF_BLUETOOTH;
	loc_addr.l2_bdaddr = *BDADDR_ANY;
	loc_addr.l2_psm = htobs(0x1001);

	bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));
	

	// put socket into listening mode
	listen (s, 1);

	// accept one connection
	client = accept (s , (struct sockaddr *)&rem_addr , &opt ) ;
	


	ba2str ( &rem_addr.l2_bdaddr , buf ) ;

	fprintf(stderr , "accepted connection from %s\n" , buf ) ;
	memset(buf , 0, sizeof(buf ));

	// read data from the client

	int check = 1;
	i=0;
	int k = 1;
	long tempsboucle = 0;
	long temps_envoi = 0;
	struct timeval start, end;
	gettimeofday(&start, NULL);
	while(check) {
		bytes_read = recv (client , buf , sizeof(buf), 0);
		if( bytes_read > 0 ) {
			if( strcmp(buf, "stop") == 0 ){
				check = 0;
			}else if(strcmp(buf, "next") == 0){
				gettimeofday(&end, NULL);
				temps_envoi = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
				tempsboucle+= temps_envoi;
				printf("Temps pour n = %d : %ld micro seconds\n",k, temps_envoi);
				fprintf(resultat, "%d : %ld ms\n",k, temps_envoi);
				k++;
				gettimeofday(&start, NULL);
			}else{
				strcpy(test, buf);
				if (fichier != NULL){
					fprintf(fichier, "%s", test);
					//printf("%s \n", test);
				}
			}
			memset(buf , 0, sizeof(buf ));
		}
	}
	gettimeofday(&end, NULL);
	tempsboucle += ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
	printf("Temps total moyen: %ld micro seconds\n", tempsboucle/10) ;
	fprintf(resultat, "Moyenne : %ld ms \n", tempsboucle/10);
	close (client) ;
	close (s) ; 
}


