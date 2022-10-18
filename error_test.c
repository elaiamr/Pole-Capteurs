#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    char ** data;
    int sizeLines;
} data_lines ;

data_lines DataConvert ( char * lien ){

    // Ouverture du fichier
	FILE * fichier = fopen(lien, "r");

    // Initialisations
    data_lines dataConverted;
    size_t sizeColumns = 0;
    size_t sizeLines = 0;
    char currentChar;
    char ** data;
    int c1 = 1;  // compteur de lignes
    int c2 = 1;  // compteur de colonnes
    int c3 = 0;  // compteur annexe
    int lostLines = 0;

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
            } else {
                if (c2 == c3 + 1 || c2 == c3 - 1){    // Présence (ou non) des - dans les données
                    c3 = c2;     // Stockage du nouveau nombre "normal" de colonnes
                }
                if (c2 != c3){
                    printf("Il manque %d caracteres dans la ligne %d du fichier %s\n", abs(c2-c3), c1, lien);
                    lostLines++;
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
        if (c1 == 144171){
            printf("%d ", c2);
        }
    }

    // printf("Nombre de lignes du fichier %s: %d\n", lien, c1);

    printf("%s\n", data[144170]);

    dataConverted.data = data;
    dataConverted.sizeLines = c1-1;

    return dataConverted;
}


float errorRate(char ** data1, char ** data2, int sizeLines){

    float nb_errors = 0;
    float nb_data = 0;

    int i = 0;
    int j = 0;
    for (i=0;i<sizeLines;i++){
        printf("%c\n", data1[i][j]);
        while (data1[i][j] != NULL && data2[i][j] != NULL){
            nb_data++;
            if (data1[i][j] != data2[i][j]){
                nb_errors++;
            }
            j++;
        }
        j=0;
    }
    // Calcul du taux d'erreur
    float error_rate = nb_errors / nb_data * 100;
    return error_rate;
}


int main(){

    // Initialisations
    data_lines data1;
    data_lines data2;
    data_lines data3;

    // Conversion des fichiers txt
    data1 = DataConvert("Numerical_Results_capteur.txt");
    data2 = DataConvert("Numerical_Results_capteur_2.txt");
    data3 = DataConvert("Numerical_Results_capteur_3.txt");

    // Mise en application
    printf("%s\n", data1.data[144171]);   // on a une erreur sur la toute dernière ligne
    printf("%d\n",data1.sizeLines);
    //printf("Taux d'erreur : %f\n", errorRate(data1.data, data3.data, data1.sizeLines));
}