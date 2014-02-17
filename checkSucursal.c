#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


int main(int argc, char* argv[]) {
    
    FILE *vectores;
    char **argParam;
    
/*
 Crítica al a info luego se hace porque qué pereza!
*/
    char nombre_programa[30];
    sprintf(nombre_programa, "%s", argv[0]);
    
    vectores = fopen(argv[1], "r");
    
    char * param[5];      //se guardarán los parámetros del programa de la sucursal
    
    //vamos a procesar la información dependiendo de cuánta cantidad de vectores hayan en el 
    //archivo para hacer el checking a el programa de sucursal bancaria.
    
    char filas[100];
    int nroFilas = 0;
    while(!feof(vectores)){     //a partir del número de filas vamos a saber cuántos vectores hay!
		fgets(filas,100,vectores);		
		if(filas!= NULL){
			nroFilas +=1;		
		}
    }
    //ahora vamos a crear los argumentos de cada uno de los procesos hijos! (vectores)
    int i, j;
    fseek(vectores, 0, SEEK_SET);       //apuntamos otra vez al inicio del archivo
    param[0] = nombre_programa;
    char *paramTemp;
    for(i=0; i < nroFilas-1; i++){
        
        while(!(feof(vectores))){
            fgets(filas,100,vectores);
            * paramTemp = strtok(filas," "); 
            j = 1;
            while(paramTemp != NULL){
                param[j] = paramTemp;
                * paramTemp = strtok(NULL," ");
                j++;
            }
            //esta es un nuevo parámetro para saber en qué posición de la memoria compartida
            //se va a guarda la información
            param[j] = NULL;       //base de la posición de mmria compartida que le toca   
        }
        argParam[i] = *param;   //guardamos todos los parámetros de los vectores
    }
    
/*
 * vamos a "chekear" la sucursal bancaria N veces (siendo n la suma de todas las veces que se va a repetir cada vector de prueba)
*/
    
    int producciones = 0;
    for(i=0; i < nroFilas-1; i++){
        producciones +=  atoi(&(argParam[i])[4]);
    }
    
    
    char dirPrograma[30];
    sprintf(dirPrograma, "./%s", nombre_programa);
    int retorno, balanceInicial;
    
    for(i=0; i < producciones; i++){
        if(fork==0){
            if(execvp(dirPrograma,&argParam[i])){
                exit(11);
            }else{
                i--;//por si no se puede crear el hijo se cree otro en su reemplzado en el siguiente paso
                continue;
            }
        }else{
            wait(&retorno);
            printf("La información de la producción N° %d de los vectores de prueba es:\n", i+1);
            balanceInicial = (atoi((&(argParam[i])[2]))*(atoi(&(argParam[i])[3])));
            printf("Balance inicial: %d | Balance final: %d\n", balanceInicial, retorno);
            if(balanceInicial == retorno){
                printf("El proceso %d pasó la prueba!!! :D \n", i+1);
            }else{
                printf("El proceso %d NO pasó la prueba!!! :( \n", i+1);
            }
        }
    }
    return(0);
}

