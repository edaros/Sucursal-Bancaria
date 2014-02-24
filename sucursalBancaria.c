#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

/*
 * Creamos la estructura cuentaBancaria que será la cuenta de cada usuario
 * donde tendrá el dinero guardado y "seguro".
 */
struct cuentaBancaria{
    int nroCuenta;
    int saldo;
};

int provi = 0;

//creamos un apuntador a cuentaBancaria para poder enviarlo como parámetro al nuevo hilo
struct cuentaBancaria * cuentas;
//creamos un apuntador a semaforos para tener organizados los semáforos de protección de las cuentas
sem_t * semaforos;
//creamos una variable estática que tendrá el número de las cuentas
int * numeroCuentas;
//tiempo de espera
int * tiempoEspera;
//balance de las cuentas
int balance;
//creamos los métodos para poder realizar los retiros, consignaciones y por último transferencias

//consignaciones a la cuenta específica.
void *consignar(void * monto, void * cuenta){
    int * montoConsig = (int *) monto;
    struct cuentaBancaria * cuentaBank = (struct cuentaBancaria * ) cuenta;
    cuentaBank->saldo = cuentaBank->saldo + *montoConsig;
    return NULL;
}

//retiro a la cuenta específica.
void *retirar(void * monto, void * cuenta){
    int * montoRet= (int *) monto;
    struct cuentaBancaria * cuentaBank = (struct cuentaBancaria * ) cuenta;
    cuentaBank->saldo = cuentaBank->saldo - *montoRet;
    return NULL;
}

/*
 *transferencia que toma consignar y retirar,  
*/

//monto: cantidad de dinero a transferir, cuentaOut: cuende de donde sale el dinero
//cuentaIn: cuenda a donde se dirige el dinero
void *transferir(void * monto, void * cuentaOut, void * cuentaIn){
    retirar((int *)monto, (struct cuentaBancaria *) cuentaOut);
    consignar((int *)monto, (struct cuentaBancaria *) cuentaIn);
    return NULL;
}

/*
 * Método más importante de esta aplicación.
 * consiste en revisar que se haga "seguro" los procesos de movimientos bancarios
 * y que ningún cliente salga robado o que no se le robe al banco (condiciones de carrera).
 * cabe recordar que la transferencia se hará desde dos cuentas aleatorias hasta un tiempo en 
 * específico. Además la aplicación no debería de bloquearse pues siendo así colapsaría el
 * sistema del banco y se podría perder dinero, recibir demandas y quebrar (deadlock).
*/
void *mtoBancario(){
    
    int  dinero;
    int random1, random2;
    sem_t semaforo_retir, semaforo_consig;
    struct cuentaBancaria * cuenta_retir;
    struct cuentaBancaria * cuenta_consig;
    //vamos a calcular el tiempo de procesamiento (repetición) de cada hilo
    //vamos a comparar las milésimas actuales con las que hayan después de una transacción.
    int tiempoInicial, tiempoFinal;
    struct timeval centHour;
    gettimeofday(&centHour, 0); //agrega el tiempo actual en centHour
    tiempoInicial = centHour.tv_usec;
    tiempoFinal = tiempoInicial + (*tiempoEspera*1000); //micra más 1000, igual milésima.
    
    //Debido al deadlock se puede presentar vamos a dar una pequeña tolerancia de dos recorridos para 
    //cada proceso y si no se libera entonces liberamos las cuentas y se las damos a otro proceso :D

    int semaDisponible1, semaDisponible2;
    int maxEspera = 0;          //variable para evitar que se realice una repetición excesova del siguiente ciclo.
    
    do{//este do while compara si ya la hora anterior superó la hora espefícada por el tiempo de espera.
        //seleccionamos dos cuentas aleatoriamente para poder transferir dinero de una a otra.

        gettimeofday(&centHour, 0);
        srand((int) centHour.tv_usec);        //semilla del random
        random1 = rand()%(*numeroCuentas);
        cuenta_retir = &(cuentas[random1]);
        semaforo_retir = semaforos[random1];
        do{ //si con iguales entonces vuelve y busque otra cuenta para hacer una transacción
            gettimeofday(&centHour, 0);
            srand((int) centHour.tv_usec);        //semilla del random
            random2 = rand()%(*numeroCuentas);
        }while(random1==random2);
        //tomamos una cuenta y semáforo diferente de las primeros
        cuenta_consig = &(cuentas[random2]);
        semaforo_consig = semaforos[random2];
    /*
     * Luego de obtener los semáforos de las cuentas vamos a revisar si están disponibles
     * o si ya están siendo usados por otro hilo
    */

        


            //primero miramos si está disponible el semáforo y lo bloqueamos!
            semaDisponible1 = sem_trywait(&semaforo_retir);
            if((semaDisponible1 == 0)){     //si estaba disponible y se pudo bloquear!
             

                    semaDisponible2 = sem_trywait(&semaforo_consig);
                    if(semaDisponible2 == 0){ //si estaba disponible y se pudo bloquear!
                        //creamos el monto a retirar a partir de la cuenta de donde va a salir la consignación.
                        gettimeofday(&centHour, 0);
                        srand((int) centHour.tv_usec);        //semilla del random
                        dinero = (rand() % cuenta_retir->saldo);
                        transferir(&dinero, cuenta_retir, cuenta_consig);
                       // printf("transferencia %d!\n", provi++);

                        sem_post(&semaforo_consig);
                    }


                    sem_post(&semaforo_retir);
            }

        gettimeofday(&centHour, 0);
        tiempoInicial = centHour.tv_usec;

    //si el tiempo inicial siguiente supera el tiempo final entonces siga! 
    //y si ya se han echo suficientes repeticiones como el tiempo de espera siga también!
    }while((tiempoInicial < tiempoFinal) && (maxEspera++<*tiempoEspera)); 

    return NULL;
}


int main(int argc, char* argv[]) {

    int numero_cuentas = atoi(argv[3]), numeroHilos = atoi(argv[1]);
    int valor_inicial = atoi(argv[4]), timerRun = atoi(argv[2]);
    balance = (numero_cuentas*valor_inicial);
/*
    printf("El balance inicial es : %d\n", balance);
    printf("El valor inicial de cada cuenta es: %d\n", valor_inicial);
*/
    
/*
 Procedemos a crear las cuentas bancarias como lo indica el enunciado
*/
    struct cuentaBancaria cuentasB[numero_cuentas];
    
    int i;
    for(i=0; i < numero_cuentas; i++){
        cuentasB[i].nroCuenta = i+1;
        cuentasB[i].saldo = valor_inicial;
    }//listo cuentas creadas!
    cuentas = cuentasB;
    
/*
 *Cada cuenta cuenta (no es redundancia) con un semáforo para garantizar que el proceso de transferencia
 * sea un proceso atómico, y que no tenga problemas por posibles condiciones de carrera (descuadres).
*/
    //se crea la misma cantidad de semáforos
    sem_t vecSemaforos[numero_cuentas];
    
    //creamos iniciamos todos los semáforos
    for(i=0; i < numero_cuentas; i++){
        sem_init(&vecSemaforos[i], 0, 1); //pshared en 0 para indicar que sólo puede ser usado por el hilo ppal.
    }//creados todos!
    
    semaforos = vecSemaforos;
    //creamos el hilar!
    pthread_t hilar[numeroHilos];
    //mandamos como argumento del método, el tiempo de ejecución dividido por el número de hilos
    //para que cada uno procese el mismo tiempo

    numeroCuentas = &numero_cuentas;
    tiempoEspera = &timerRun;
    for(i=0; i < numeroHilos; i++){
        pthread_create (&hilar[i], NULL, &mtoBancario, NULL);
    }
    //esperamos a que termine el hilar!
    for(i=0; i < numeroHilos; i++) pthread_join ( hilar[i], NULL);
    
    for(i=0; i < numero_cuentas; i++){
        sem_destroy(&vecSemaforos[i]);
    }//destruidos todos!
    //vamos a imprimir el valor totar de todas las cuentas!
/*
    printf("El balance inicial fué : %d\n", balance);
*/
    balance = 0;
    for(i=0; i < numero_cuentas; i++){
/*
        printf("Cuenta N° %d, saldo diponible: %d\n", cuentasB[i].nroCuenta, cuentasB[i].saldo);
*/
        balance+=cuentasB[i].saldo;
    }
/*
    printf("El balance final es : %d\n", balance);
*/
    /*
     * Sección para enviar el resultado en un archivo plano para que pueda ser visto por el programa que hace
     * las pruebas.
     */
    FILE * resultado;
    resultado = fopen("balanceFinal.txt", "w");
    fprintf(resultado, "%d", balance);
    fclose(resultado);
    /*
     * Fin de la creación del archivo con el resultado
     */
    
    return (0);
}
