#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    char ** data;
    int sizeLines;
    int sizeColumns;
    int missingLines;
} data_lines ;

data_lines DataConvert ( char * lien ){

    // Ouverture du fichier
	FILE * fichier = fopen(lien, "r");

    // Initialisations
    data_lines dataConverted;
    dataConverted.data = NULL;
    dataConverted.sizeColumns = 0;
    dataConverted.sizeLines = 0;
    dataConverted.missingLines = 0;
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

    //printf("Nombre de mauvaises lignes : %d\n", dataConverted.missingLines);

    //printf("%s\n", data[144170]);

    dataConverted.data = data;
    dataConverted.sizeLines = c1;

    return dataConverted;
}


float errorRate(data_lines data1, data_lines data2) {

    int nb_errors = 0;
    int nb_data = 0;
    double loss_rate = 0;
    int minLines = 0;
    int maxColumns = 0;
    int deltaLines = 0;
    double error_rate = 0;

    if (data1.sizeLines >= data2.sizeLines){
        //printf("%d\n", data1.sizeLines - data2.sizeLines + data1.missingLines + data2.missingLines);
        loss_rate = (data1.sizeLines - data2.sizeLines + data1.missingLines + data2.missingLines) / data1.sizeLines * 100;
        minLines = data2.sizeLines;
        deltaLines = data1.sizeLines - data2.sizeLines;
        maxColumns = data1.sizeColumns;
    } else {
        //printf("%d\n", data2.sizeLines - data1.sizeLines + data1.missingLines + data2.missingLines);
        loss_rate = (data2.sizeLines - data1.sizeLines + data1.missingLines + data2.missingLines) / data2.sizeLines * 100;
        minLines = data1.sizeLines;
        deltaLines = data2.sizeLines - data1.sizeLines;
        maxColumns = data1.sizeColumns;
    }

    printf("Taux de perte de %.100lf pourcents\n", loss_rate);

    int i = 0;
    int j = 0;

    for (i=0;i<minLines;i++){
        for (j=0;j<maxColumns;j++){
            if (i>144100){
                //printf("%c %c\n", data1.data[i][j], data2.data[i][j]);
                //printf("%d\n", i);
            }
            nb_data++;
            if (data1.data[i][j] != data2.data[i][j]){
                nb_errors++;
                //printf("Nb d'erreurs : %d\n", nb_errors);
            }
        }
    }

    // Ajout des lignes manquantes
    nb_errors += deltaLines * maxColumns;

    // Calcul du taux d'erreur
    
    error_rate = nb_errors / nb_data * 100;
        
    printf("Taux d'erreur de %.100lf pourcents\n", error_rate);

    return 0;
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