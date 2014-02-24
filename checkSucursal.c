#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>



int main(int argc, char * argv[]) {
    
    FILE *vectores;
    
/*
 Crítica, sección para validar la entrada al programa se deja al programador! ;)
 * es una cuestión diferente a los objetivos de esta práctica
*/
    vectores = fopen(argv[2], "r");
/*
 Sección específica para leer el resultado del programa de transacciones.
 */
    FILE *resultadoBalance;
    char balance[30];
    int balanceInt;
 /*
  * Fin de sección específica
  */
    char * param[7];      //se guardarán los parámetros del programa de la sucursal
    //vamos a procesar la información dependiendo de cuánta cantidad de vectores hayan en el 
    //archivo para hacer el checking a el programa de sucursal bancaria.
    char filas[100];
    int nroFilas = 0;
    while(fgets(filas,100,vectores)){     //a partir del número de filas vamos a saber cuántos vectores hay!
		if(filas!= NULL){
			nroFilas +=1;		
		}
    }
    char ** argParam[nroFilas];
    //ahora vamos a crear los argumentos de cada uno de los procesos hijos! (vectores)
    int i = 0, j = 0;
    vectores = fopen(argv[2], "r");
    char dirPrograma[30];
    sprintf(dirPrograma, "./%s", argv[1]);
    param[0] = dirPrograma;
    char * paramTemp;
    int k = 0;
    int nroPruduciones = 0;
    //repetiremos todo este proceso según el número de vectores de prueba
    while(k < nroFilas){
        while(fgets(filas,100,vectores) != NULL){
            paramTemp = strtok(filas," "); 
            j = 1;
            while(paramTemp != NULL){
                param[j] = paramTemp;
                paramTemp = strtok(NULL," ");
                j++;
            }
            //esta es un nuevo parámetro para saber en qué posición de la memoria compartida
            //se va a guarda la información
            param[j] = NULL;       //base de la posición de mmria compartida que le toca   
            break;
        }
        argParam[i] = param;   //guardamos todos los parámetros de los vectores
        int balanceInicial;
        //repetición n veces el vector de prueba!
        int l = 0;
        int nrepeticiones;
        nrepeticiones = atoi((argParam[i][5]));
        
        while(l < nrepeticiones){
            argParam[i][5] = NULL;
            if(fork()==0){
                if(execvp(dirPrograma,argParam[i])){
                    exit(1);
                }else{
                    printf("no dio este pirobo!");
                    i--;//por si no se puede crear el hijo se cree otro en su reemplzado en el siguiente paso
                    continue;
                }
            }else{
                wait(NULL);
                resultadoBalance = fopen("balanceFinal.txt", "r");
                fseek(resultadoBalance, SEEK_END, 0);
                if(ftell(resultadoBalance)<2){ 
                    //Si no hay más de dos caracteres en el archivo entonces no se pudo guardar por lo tanto
                    //repetimos el proceso del vector
                    continue;
                }
                resultadoBalance = fopen("balanceFinal.txt", "r");
                fgets(balance, 30, resultadoBalance);
                balanceInt = atoi(balance);
                printf("Repetición %d del vector N° %d\n", l+1 , i+1);
                printf("La información de la producción N° %d de los vectores de prueba es:\n", ((nroPruduciones++)+1));
                balanceInicial = (atoi((argParam[i][4])))*(atoi((argParam[i][3])));
                printf("Balance inicial: %d | Balance final: %d\n", balanceInicial, balanceInt);
                if(balanceInicial == balanceInt){
                    printf("El proceso de prueba %d pasó la prueba!!! :D \n", i+1);
                }else{
                    printf("El proceso de prueba %d NO pasó la prueba!!! :( \n", i+1);
                }
                sleep(1);
            }
            l++;
        }
        i++;
        k++;
    }
    fclose(resultadoBalance);   //cerramos el archivo.
    remove("balanceFinal.txt"); //eliminamos el archivo.
    return(0);
}
