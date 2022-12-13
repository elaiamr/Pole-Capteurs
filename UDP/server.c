#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <sched.h>

int main(int argc , char ** argv){   //Fonction de réception des données

	//Création du socket de réception et initialisations des données
	int s=0;
	size_t size = 1024;
    struct sockaddr_in server = {0}, client = {0}; 
	char * server_ip = "192.168.1.2";         //0.137
	int server_port = 39206;      //40559

	//Allocation du socket
	s = socket (AF_INET, SOCK_DGRAM, 0);
    if (s < 0){
        perror("Opening socket");
    }
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(server_ip);
	server.sin_port = htons(server_port);
	bind (s, (struct sockaddr *)&server, sizeof(server));

	socklen_t len = sizeof(client);
	accept (s , (struct sockaddr *)&client , &len) ;
	
	//ICI DEBUTE LE TEST
	char buf[size];
	int n = 0;
	int i=0;
	printf("%s\n", "Serveur en mode écoute...");

	while (1){
		n = recvfrom(s, buf, size, MSG_WAITALL, (struct sockaddr *)&client, &len);
		buf[n] = '\0';
		printf("Données reçues : %s (de taille %d caractères)\n", buf, n);
		for (i=0;i<n;i++){
			buf[i] = '\0';
		}
	}

	close(s);
}
