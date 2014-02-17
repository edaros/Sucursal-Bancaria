/*
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>  //Semáforos librerías!
#include <sched.h>
#include <fcntl.h>      //librería para los parámetros de los métodos de los semáforos
#include <sys/mman.h>


int main(int argc, char * argv[]) {
    
    FILE *vectores;
/*
 Crítica al a info luego se hace porque qué pereza!
*/
    //char nombre_programa[30];
    //sprintf(nombre_programa, "%s", argv[1]);
    
    vectores = fopen(argv[2], "r");
    
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
    
    char ** argParam[nroFilas-1];
    //ahora vamos a crear los argumentos de cada uno de los procesos hijos! (vectores)
    int i = 0, j = 0;
    //fseek(vectores, 0, SEEK_SET);       //apuntamos otra vez al inicio del archivo
    vectores = fopen(argv[2], "r");
    char dirPrograma[30];
    sprintf(dirPrograma, "./%s", argv[1]);
    param[0] = dirPrograma;
    char * paramTemp;
    for(i=0; i < nroFilas-1; i++){
        
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
        }
        argParam[i] = param;   //guardamos todos los parámetros de los vectores
    }
    
/*
 * vamos a "chekear" la sucursal bancaria N veces (siendo n la suma de todas las veces que se va a repetir cada vector de prueba)
*/
    
    int producciones = 0;
    for(i=0; i < nroFilas-1; i++){
        producciones +=  atoi((argParam[i][5]));
    }
    
    int retorno, balanceInicial;
    
    //repetición n veces el vector de prueba!
    int l = 0;
    int nrepeticiones;
    for(i=0; i < (nroFilas-1); i++){
        nrepeticiones = atoi((argParam[i][5]));
        while(l < nrepeticiones){
            argParam[i][5] = NULL;
            if(fork()==0){
                if(retorno = execvp(dirPrograma,argParam[i])){
                    exit(11);
                }else{
                    printf("no dio este pirobo!");
                    i--;//por si no se puede crear el hijo se cree otro en su reemplzado en el siguiente paso
                    continue;
                }
            }else{
                wait(&retorno);
                printf("Repetición %d del vector N° %d\n", l+1 , i+1);
                printf("La información de la producción N° %d de los vectores de prueba es:\n", i+1);
                balanceInicial = (atoi((argParam[i][4])))*(atoi((argParam[i][3])));
                printf("Balance inicial: %d | Balance final: %d\n", balanceInicial, retorno);
                if(balanceInicial == retorno){
                    printf("El proceso %d pasó la prueba!!! :D \n", i+1);
                }else{
                    printf("El proceso %d NO pasó la prueba!!! :( \n", i+1);
                }
            }
            l++;
        }
    }
    return(0);
}

