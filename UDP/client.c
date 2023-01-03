#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

int main(int argc , char ** argv){   //Fonction d'envoi de données

	//Création du socket dans le but de connecter entre elles les 2 raspberry
	struct sockaddr_in server;
	int s = 0;
	int size = 1024;
	int n, status;
    char * server_ip = "192.168.1.2";   		//0.137
    unsigned int server_port = 39206;         	//40559
	
	//Allocation du socket
	s = socket(AF_INET, SOCK_DGRAM, 0) ;

	//Connexion entre les 2 raspberry
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(server_ip);
	server.sin_port = htons(server_port);

	socklen_t len = sizeof(server);
	status = connect(s, (struct sockaddr *)&server, sizeof(server));

    //ICI DEBUTE LE TEST
    char value[size];

	while (1){
		printf("Quel message envoyer ? ");
		fflush(stdout);
		scanf("%[^\n]%*c", &value);
		value[strlen(value)] = '\0';
		n = sendto(s, value, strlen(value), MSG_CONFIRM, (struct sockaddr *)&server, len);
		printf("Données envoyées : %s (de taille %d caractères)\n", value, n);
	}

	close(s);
}
