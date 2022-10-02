#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

/*pthread_attr_t tattr1;  // Création d'un attribut de thread

set_thread_param(&tattr1);                                                               // Appel à la fonction définisant le type de scheduling

struct sched_param param1;                                                               // Création d'un paramètre de scheduling

param1.sched_priority = 5;                                                              // Définition de la valeur de priorité temps réel pour les threads

               
err = pthread_attr_setschedparam (&tattr1, &param1);                                 // Affectation d'une priorité temps réel au paramètre tattr à partir de la valeur définie précédemment

if (err != 0)  {                                                                   // Si l'affectation de la priorité s'est terminé avec une erreur

        handle_error_en(err, "pthread_attr_setschedparam");                     // Afficher un message d'erreur dans la console

}*/

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

char ** DataConvert ( char * lien ){

	//ouverture fichier
	FILE * fichier = fopen(lien, "r");

	assert (fichier != NULL);

	char ** data;
	int i,j,k,ind;

	//allocation mémoire
	data = (char **)malloc(6248 * sizeof(char *));
	for (i= 0; i < 6248; i++){
		data[i] = (char *)malloc(130 * sizeof(char ));
	}
	//lecture fichier
	char chaine[130]; // cdc correspondant à 1 ligne
	for(i=0;i<6248;i++){
		fgets (chaine, 130, fichier);
		strcpy(data[i], chaine);
		memset (chaine, 0, 130);
	}
	return data;
}

int sizeLines(char ** data){
    int size = 0;
    while (data[size] != NULL) {
        size++;
    }
    return size;
}

int sizeColumns(char ** data, int i){
    int size = 0;
    while (data[i][size]) {
        size++;
    }
    return size;
}

//Fonction taux d'erreur

float Error_Rate_Fct(char * final_data, char * initial_data){

    // Ouverture du fichier envoyé
    char ** data_i = DataConvert(initial_data);

    // Ouverture du fichier reçu
    char ** data_f = DataConvert(final_data);

    float nb_errors = 0;
    float nb_data = 0;

    int i = 0;
    int j = 0;

    // Récupération du nombre de lignes
    int nb_lignes_i = sizeLines(data_i);
    for (i=0;i<nb_lignes_i;i++){
        //Récupération du nombre de colonnes
        int nb_colonnes_i = sizeColumns(data_i,i);
        for (j=0;j<nb_colonnes_i;j++){
		nb_data++;
		if (data_i[i][j] != data_f[i][j]){
			nb_errors++;	// On compte le nombre d'erreurs dans la fonction
		}
	}
    }

    // Calcul du taux d'erreur
    float error_rate = nb_errors / nb_data * 100;
    return error_rate;
}

//Fonction taux de pertes

float loss_rate_Fct(char * final_data, char * initial_data){

    // Ouverture du fichier envoyé
    char ** data_i = DataConvert(initial_data);

    // Ouverture du fichier reçu
    char ** data_f = DataConvert(final_data);

    float nb_loss = 0;
    float nb_data = 0;

    int i = 0;
    int j = 0;

    int nb_lignes_i = sizeLines(data_i);
    int nb_lignes_f = sizeLines(data_f);

    if (nb_lignes_i > nb_lignes_f) {
        nb_loss = nb_loss + (nb_lignes_i - nb_lignes_f);
        nb_data = nb_lignes_i * 5;
        int nb_donnees_f = sizeColumns(data_f,nb_lignes_f-1);   //on prend la dernière ligne
        nb_loss = nb_loss + (5-nb_donnees_f);   //en gros on vérifie que la dernière soit bien composée de 5 données sinon on ajoute

    } else if (nb_lignes_i < nb_lignes_f) {
        nb_loss = nb_loss + (nb_lignes_f - nb_lignes_i);
        nb_data = nb_lignes_f * 5;
        int nb_donnees_i = sizeColumns(data_i,nb_lignes_i-1);   //on prend la dernière ligne
        nb_loss = nb_loss + (5-nb_donnees_i);   //en gros on vérifie que la dernière soit bien composée de 5 données sinon on ajoute
    
    } else {
        nb_loss = 0;
        nb_data = 100;    // arbitraire pour éviter la division par 0
    }

    float loss_rate = nb_loss / nb_data * 100;
    return loss_rate;

}

int main(int argc , char ** argv){
	struct sockaddr_l2 loc_addr = { 0 } , rem_addr = { 0 } ;
	char buf[10000] = { 0 } ;
	int s, client , bytes_read ;
	unsigned int opt = sizeof(rem_addr ) ;
	int i,j;
	char ** data;
	int n = 6248;
	char test[10000] = { 0 } ;
	
	//Définition de la priorité du script en priorité temps réel

	struct sched_param sched_p;                                                                 // Création d'une structure d'ordonancement temps réel pour le programme

	sched_p.sched_priority = 5;                                                                             // Affectation d'une priorité temps réel entre 0 et 99

	if(sched_setscheduler(0, SCHED_RR, &sched_p) == -1)  {                                            // Affectation d'un ordonancement Round-robin avec le paramètre de priorité défini précédemment si l'opération se passe sans erreur

		perror ("sched_setscheduler \n");                                                             // Sinon le programme se termine via la fonction perror()

	}

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
	
	set_l2cap_mtu(s , 31240 ); // change la MTU à 10000




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
	printf("Temps total moyen : %ld micro seconds\n", tempsboucle/10) ;
	fprintf(resultat, "Moyenne : %ld ms \n", tempsboucle/10);
	
	char * initial_data = "/home/pi/Documents/Numerical_Results_capteur.txt";
	char * final_data = "/home/pi/Documents/test.txt";

	float marge = 0; 	//marge d'erreur acceptable, à définir
	float loss_rate = loss_rate_Fct(final_data, initial_data);
	printf("Taux de perte : %f pourcents \n", loss_rate);

	if (loss_rate <= marge) {
		float error_rate = Error_Rate_Fct(final_data, initial_data);
		printf("Taux d'erreur : %f pourcents \n", error_rate);
	}

	close (client) ;
	close (s) ; 
}

