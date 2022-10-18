#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    char ** data;
    int sizeLines;
    int missingLines;
} data_lines ;

data_lines DataConvert ( char * lien ){

    // Ouverture du fichier
	FILE * fichier = fopen(lien, "r");

    // Initialisations
    data_lines dataConverted;
    int sizeColumns = 0;
    int sizeLines = 0;
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
            } else {
                if (c2 == c3 + 1 || c2 == c3 + 2 || c2 == c3 + 3 || c2 == c3 - 1 || c2 == c3 - 2 || c2 == c3 - 3){    // Présence (ou non) des - dans les données
                    c3 = c2;     // Stockage du nouveau nombre "normal" de colonnes
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

    // printf("Nombre de lignes du fichier %s: %d\n", lien, c1);

    //printf("%s\n", data[144170]);

    dataConverted.data = data;
    dataConverted.sizeLines = c1;

    return dataConverted;
}


float errorRate(data_lines data1, data_lines data2){

    float nb_errors = 0;
    float nb_data = 0;
    float nb_loss = 0;
    int maxLines = 0;

    if (data1.sizeLines >= data2.sizeLines){
        nb_loss = (data1.sizeLines - data2.sizeLines + data1.missingLines + data2.missingLines) / data1.sizeLines;
        maxLines = data1.sizeLines;
    } else {
        nb_loss = (data2.sizeLines - data1.sizeLines + data1.missingLines + data2.missingLines) / data2.sizeLines;
        maxLines = data2.sizeLines;
    }

    if (nb_loss == 0){
        int i = 0;
        int j = 0;
        for (i=0;i<maxLines;i++){
            //printf("%c\n", data1[i][j]);
            printf("%c %c\n", data1.data[i][j], data2.data[i][j]);
            //while (data1.data[i][j] != NULL && data2.data[i][j] != NULL){
            //    printf("%c %c\n", data1.data[i][j], data2.data[i][j]);
             //   nb_data++;
              //  if (data1.data[i][j] != data2.data[i][j]){
              //      nb_errors++;
              //  }
                //j++;
            }
            j=0;
        //}
        // Calcul du taux d'erreur
        
        float error_rate = nb_errors / nb_data * 100;
        
        printf("Taux d'erreur : %f\n", error_rate);
        
        return nb_loss, error_rate;
    } else {
        printf("Taux de perte de %f\n", nb_loss);
        return nb_loss;
    }
    
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
    //printf("%s\n", data1.data[144170-0]);   // on a une erreur sur la toute dernière ligne
    //printf("%d\n",data1.sizeLines);

    errorRate(data1, data2); 
    //printf("Taux d'erreur : %f\n", errorRate(data1, data2));
}