import os
from time import sleep

MTU_Sizes = [80,600,2000,3200,6000,12800,24000,35600,49600,59400,65520]
for i, MTU_Size in zip(range(len(MTU_Sizes)), MTU_Sizes):
    sleep(5)
    dossier = "C:/Users/flaph/OneDrive/Documents/GitHub-PERSEUS/Pole-Capteurs/client"
    filename = "clientMTU_Changed_" + str(i)
    file = str(dossier) + "/" + str(filename) + ".c"

    f = open(file, "w")

    #MTU_Size = 15620

    print("#include <stdio.h>\n#include <stdlib.h>\n#include <unistd.h>\n#include <sys/socket.h>\n#include <bluetooth/bluetooth.h>\n#include <bluetooth/l2cap.h>\n#include <assert.h>\n#include <time.h>\n#include <sys/time.h>\n#include <errno.h>\n\nchar ** DataConvert ( char * lien ){\n\n	//ouverture fichier\n	FILE * fichier = fopen(lien, \"r\");\n\n	assert (fichier != NULL);\n\n	char ** data;\n	int i,j,k,ind;\n\n	//allocation memoire\n	data = (char **)malloc(6248 * sizeof(char *));\n	for (i = 0; i < 6248; i++){\n		data[i] = (char *)malloc(130 * sizeof(char ));\n	}\n	//lecture fichier\n	char chaine[130]; // cdc correspondant a 1 ligne\n	for(i=0;i<6248;i++){\n		fgets (chaine, 130, fichier);\n		strcpy(data[i], chaine);\n		printf(\"%d : %s \\n\",i, data[i]);\n		memset (chaine, 0, 130);\n	}\n	return data;\n}\n	 \n\nint set_l2cap_mtu( int s , uint16_t mtu ) { //fonction qui change La MTU d'un socket\n\n	struct l2cap_options opts ;\n	int optlen = sizeof(opts ) ;\n	int status = getsockopt(s, SOL_L2CAP, L2CAP_OPTIONS, &opts, &optlen);\n	if( status == 0) {\n		opts.omtu = opts.imtu = mtu ;\n		status = setsockopt( s , SOL_L2CAP , L2CAP_OPTIONS , &opts ,optlen ) ;\n	}\n	return status ;\n};\n\nint envoie(char ** data){\n	//Creation du socket dans le but de connecter entre elles les 2 raspberry\n	struct sockaddr_l2 addr = { 0 } ;\n	int s , status ;\n	char dest[18] = \"DC:A6:32:78:6C:7E\";\n	//de:81:c6:b4:7c:5f:46:58\n	// allocate a socket\n	s = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP) ;\n", file=open(file, "a"))
                
    print("\n    set_l2cap_mtu( s , " + str(MTU_Size) + " );\n", file=open(file, "a"))

    print("\n    // set the connection parameters (who to connect to)\n    addr.l2_family = AF_BLUETOOTH;\n    addr.l2_psm = htobs(0x1001);\n    str2ba( dest , &addr.l2_bdaddr ) ;\n\n    // connect to server\n    status = connect (s , (struct sockaddr *)&addr , sizeof(addr ));\n    // send a message\n    int i,j,k;\n    k = 0;\n    struct timeval start, end;\n    char paquet[4200] = {0};\n\n    if( 0 == status ) {\n	    printf(\"Connexion reussie\\n\");\n	    gettimeofday(&start, NULL);\n	    for (k=0;k<10;k++){\n		    for (i=0; i<6240;i+=32){\n                memset (paquet, 0, 4200);\n                for(j=0; j<32; j++){\n	                strcat (paquet, data[i+j]);\n	                //strcat (paquet, \"\\n\");\n                }\n                //printf(\"Envoie ligne %d \\n\", i);\n                send(s, paquet, 4200, 0);\n            }\n            send(s, \"next\",4,0);\n	    }	\n	    send(s,\"stop\",4,0);\n	    gettimeofday(&end, NULL);\n	    printf(\"Temps total : %ld micro seconds\\n\",\n	    ((end.tv_sec * 1000000 + end.tv_usec) -\n	    (start.tv_sec * 1000000 + start.tv_usec)));\n    }\n    if( status < 0 ) {\n	    fprintf(stderr, \"error code %d: %s\\n\", errno, strerror(errno));\n	    perror( \"Connexion echouee\" ) ;\n    }\n    close (s ) ;\n    return 0;\n}\n\nint main(int argc , char ** argv){\n    char ** data;\n    data = DataConvert(\"/home/pi/Desktop/Numerical_Results_capteur_v1.txt\");\n    envoie(data);\n}", file=open(file, "a"))

    os.system("gcc -o " + str(filename) + " " + str(filename) + ".c -lbluetooth")
    os.system("./"+str(filename))