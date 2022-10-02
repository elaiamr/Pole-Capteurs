#include <stdlib.h>
#include <stdio.h>
#include <math.h>

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
		printf("%d : %s \n",i, data[i]);
		memset (chaine, 0, 130);
	}
	return data;
}

//Fonction taux d'erreur

float Error_Rate_Fct(char * final_data, char * initial_data){

    // Ouverture du fichier envoyé
    initial_data = strcat("/home/pi/Documents/",initial_data);
    char ** data_i = DataConvert(initial_data);

    // Ouverture du fichier reçu
    final_data = strcat("/home/pi/Documents/",final_data);
    char ** data_f = DataConvert(final_data);

    float nb_errors = 0;
    float nb_data = 0;

    int i = 0;
    int j = 0;

    // Vérification du nombre de lignes
    int nb_lignes_i = sizeof(data_i);
    int nb_lignes_f = sizeof(data_f);

    if (nb_lignes_i == nb_lignes_f){
        for (i=0;i<nb_lignes_i;i++){

            //Vérification du nombre de colonnes
            int nb_colonnes_i = sizeof(data_i[i]);
            int nb_colonnes_f = sizeof(data_f[i]);
            
            if (nb_colonnes_i == nb_colonnes_f){
                for (j=0;j<nb_colonnes_f;j++){
                    nb_data++;
                    if (data_i[i][j] != data_f[i][j]){
                        nb_errors++;	// On compte le nombre d'erreurs dans la fonction
                    }
                }
            } else {
                printf("Les tableaux ne sont pas de taille identique (initial : %d x %d / final : %d x %d).",nb_lignes_i,nb_colonnes_i,nb_lignes_f,nb_colonnes_f);
            }
        }
    } else {
        printf("Les tableaux ne sont pas de taille identique (initial : %d / final : %d).",nb_lignes_i,nb_lignes_f);
    }

    printf("Les tableaux sont de tailles identiques !");

    // Calcul du taux d'erreur
    float error_rate = nb_errors / nb_data * 100;

    return error_rate;
}

//Fonction taux de pertes

float loss_rate_Fct(char * final_data, char * initial_data){

    // Ouverture du fichier envoyé
    initial_data = strcat("/home/pi/Documents/",initial_data);
    char ** data_i = DataConvert(initial_data);

    // Ouverture du fichier reçu
    final_data = strcat("/home/pi/Documents/",final_data);
    char ** data_f = DataConvert(final_data);

    float nb_loss = 0;
    float nb_data = 0;

    int i = 0;
    int j = 0;

    // Vérification du nombre de lignes
    int nb_lignes_i = sizeof(data_i);
    int nb_lignes_f = sizeof(data_f);    // il va falloir vérifier si ça renvoie bien le nombre de lignes ou si c est multiplié par 8,16 etc parce que c est un char et non un int

    if (nb_lignes_i > nb_lignes_j) {
        nb_loss = nb_loss + (nb_lignes_i - nb_lignes_j);
        nb_data = nb_lignes_i * 5; // 8 parce que c'est un char

        int nb_donnees_j = sizeof(data_j[-1]);   //ici pareil à vérifier
        nb_loss = nb_loss + (5-nb_donnees_j);   //en gros on vérifie que la dernière soit bien composée de 5 données sinon on ajoute
    }

    else {
        nb_loss = nb_loss + (nb_lignes_j - nb_lignes_i);
        nb_data = nb_lignes_j * 5;

        int nb_donnees_i = sizeof(data_i[-1]);   //ici pareil à vérifier
        nb_loss = nb_loss + (5-nb_donnees_i);   //en gros on vérifie que la dernière soit bien composée de 5 données sinon on ajoute
    
    }

    float loss_rate = nb_loss / nb_data * 100;
    return loss_rate;

}


int main(){
    char * initial_data = "test.txt";
    char * final_data = "results.txt";

    float marge = 0; 	//marge d'erreur acceptable, à définir
    float loss_rate = loss_rate_Fct(final_data, initial_data);
    float error_rate = Error_Rate_Fct(final_data, initial_data);

    if (loss_rate <= marge) {			// On peut s'accorder une marge d'erreur
	printf("Il n'y a pas de pertes");
    }

    else {

    printf("Taux d'erreur de %f pourcents et taux de perte de %f pourcents.\n", error_rate, loss_rate);
}
