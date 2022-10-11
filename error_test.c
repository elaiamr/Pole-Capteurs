#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

char ** DataConvert ( char * lien ){

	//ouverture fichier
	FILE * fichier = fopen(lien, "r");

	assert (fichier != NULL);

	char ** data;
	int i,j,k,ind;

	//allocation mémoire
	data = (char **)malloc(144171 * sizeof(char *));
	for (i= 0; i < 144171; i++){
		data[i] = (char *)malloc(130 * sizeof(char ));
	}
	//lecture fichier
	char chaine[130]; // cdc correspondant à 1 ligne
	for(i=0;i<144171;i++){
		fgets (chaine, 130, fichier);
		strcpy(data[i], chaine);
		//printf("%d : %s \n",i, data[i]);
		memset (chaine, 0, 130);
	}
	return data;
}


float errorRate(int nbLignes, int nbColonnes, char final_data[nbLignes][nbColonnes], char initial_data [nbLignes][nbColonnes]){

    /*
    // Ouverture du fichier envoyé
    initial_data = strcat("/home/pi/Desktop/",initial_data);
    char ** initi DataConvert(initial_data);

    // Ouverture du fichier reçu
    initi strcat("/home/pi/Desktop/",initi
    char ** data_f = DataConvert(initi
    */

    float nb_errors = 0;
    float nb_data = 0;

    int i = 0;
    int j = 0;
    for (i=0;i<nbLignes;i++){
        for (j=0;j<nbColonnes;j++){
            nb_data++;
            if (initial_data[i][j] != final_data[i][j]){
                nb_errors++;
            }
        }
    }
    // Calcul du taux d'erreur
    float error_rate = nb_errors / nb_data * 100;
    return error_rate;
}

float loss_rate_Fct(int nbLignes_i, int nbLignes_j, int nbColonnes_i, int nbColonnes_j, char final_data[nbLignes_i][nbColonnes_i], char initial_data[nbLignes_j][nbColonnes_j]) {

    float nb_loss = 0;
    float nb_data = 0;

    int i = 0;
    int j = 0;

    // Vérification du nombre de lignes

    if (nbLignes_i > nbLignes_j) {
        nb_loss = nb_loss + (nbLignes_i - nbLignes_j);
        nb_data = nbLignes_i * 4;

        int nb_donnees_j = sizeof(final_data[nbLignes_i -1]);   //ici pareil à vérifier
        nb_loss = nb_loss + (4-nb_donnees_j);   //en gros on vérifie que la dernière soit bien composée de 5 données sinon on ajoute
    }

    else {
        nb_loss = nb_loss + (nbLignes_j - nbLignes_i);
        nb_data = nbLignes_j * 4;

        int nb_donnees_i = sizeof(final_data[nbLignes_i -1]);   //ici pareil à vérifier
        nb_loss = nb_loss + (4-nb_donnees_i);   //en gros on vérifie que la dernière soit bien composée de 5 données sinon on ajoute
	printf("%d\n", nb_donnees_i);
    
    }

    float loss_rate = nb_loss / nb_data * 100;
    return loss_rate;

}

int sizeLines(char ** data){
    int size = 0;
    while (data[size] != NULL) {
        size++;
    }
    return size;
}

int sizeColumns(char ** data){
    int i=0;
    int c1=0;
    for (i=0;i<sizeLines(data);i++){
        if ((sizeof(data[i])+1) != 5){
            printf("Erreur en ligne %d, seulement %d colonnes\n", i, sizeof(data[i])+1);
        } else {
            c1++;
        }
    }
    if (c1 == sizeLines(data)){
        return 5;
    }
    return -1;
}


int main(){
    char ** data;
    char ** data2;
    char ** data3;

    data = DataConvert("Numerical_Results_capteur.txt");
    printf("Lignes v1 : %d\n", sizeLines(data));
    printf("Colonnes v1 : %d\n", sizeColumns(data));

    data2 = DataConvert("Numerical_Results_capteur_2.txt");
    printf("Lignes v2 : %d\n", sizeLines(data2));
    printf("Colonnes v2 : %d\n", sizeColumns(data2));

    data3 = DataConvert("Numerical_Results_capteur_3.txt");
    
}