#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PRESSION_ATM 101325   //pression atmosphérique au sol
#define G 9.81                                  //accélération de la pesanteur
#define VERTICAL_GRADIENT_TEMP 0.0065           //gradient vertical de température pour 100 m
#define T0 288.15                               //équivaut à 15°C en moyenne

float generate_pression(int alt){       //formule internationale du nivellement barométrique (normalement seulement valable dans la troposphère)
    return (PRESSION_ATM * pow((1 - (VERTICAL_GRADIENT_TEMP * alt) / T0), 5.255));
}

float generate_temperature(int alt){    //décroissance de 6°C/km dans les premières couches en moyenne
    return (T0 - (6 * alt / 1000));
}

float generate_differential_pression(float pression){
    return (PRESSION_ATM - pression);
}

void main(){
    float pression, temperature, differential_pression;
    int alt = 0;        //altitude en mètres
    printf("Pression          Température   Pression Différentielle\n");
    while (alt <= 30000){
        pression = generate_pression(alt);
        temperature = generate_temperature(alt);
        differential_pression = generate_differential_pression(pression);
        printf("%lf     %lf     %lf\n", pression, temperature, differential_pression);
        alt++;
    }
    printf("Fini !\n");
}