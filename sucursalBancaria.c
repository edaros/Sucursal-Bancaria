#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

/*
 * Creamos la estructura cuentaBancaria que será la cuenta de cada usuario
 * donde tendrá el dinero guardado y "seguro".
 */
struct cuentaBancaria{
    int nroCuenta;
    float saldo;
};

//creamos un apuntador a cuentaBancaria para poder enviarlo como parámetro al nuevo hilo
struct cuentaBancaria * cuentas;
//creamos un apuntador a semaforos para tener organizados los semáforos de protección de las cuentas
sem_t * semaforos;
//creamos los métodos para poder realizar los retiros, consignaciones y por último transferencias

//consignaciones a la cuenta específica.
void *consignar(void * monto, void * cuenta){
    float * montoConsig = (float *) monto;
    struct cuentaBancaria * cuentaBank = (struct cuentaBancaria * ) cuenta;
    cuentaBank->saldo += montoConsig;
    return NULL;
}

//retiro a la cuenta específica.
void *retirar(void * monto, void * cuenta){
    float * montoRet= (float *) monto;
    struct cuentaBancaria * cuentaBank = (struct cuentaBancaria * ) cuenta;
    cuentaBank->saldo -= montoRet;
    return NULL;
}

/*
 *transferencia que toma consignar y retirar,  
*/

//monto: cantidad de dinero a transferir, cuentaOut: cuende de donde sale el dinero
//cuentaIn: cuenda a donde se dirige el dinero
void *transferir(void * monto, void * cuentaOut, void * cuentaIn){
    retirar((float *)monto, (struct cuentaBancaria *) cuentaOut);
    consignar((float *)monto, (struct cuentaBancaria *) cuentaIn);
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
void *mtoBancario(void *arg){
    
    float tiempo_espera = atoi(arg[0]); //hay que ver como se le mete el tiempo...
    int numeroCuentas = atoi(arg[1]);
    int random1, random2, dinero;
    sem_t semaforo_retir, semaforo_consig;
    int hora = time(NULL);
/*
 *Acá se realiza lo que respecta al tiempo de espera una especia de while supongo para poder realizar
 * hasta el tiempo límite los procesos por hilo
*/
    
    //seleccionamos dos cuentas aleatoriamente para poder transferir dinero de una a otra.
    srand(hora);        //semilla del random
    random1 = rand()%(numeroCuentas);
    struct cuentaBancaria *cuenta_retir = cuentas[random1];
    semaforo_retir = semaforos[random1];
    do{ //si con iguales entonces vuelve y busque otra cuenta para hacer una transacción
        hora = time(NULL);
        srand(hora);        //semilla del random
        random2 = rand()%(numeroCuentas);
    }while(random1==random2);
    //tomamos una cuenta y semáforo diferente de las primeros
    struct cuentaBancaria *cuenta_consig = cuentas[random2];
    semaforo_consig = semaforos[random2];
        
/*
 * Luego de obtener los semáforos de las cuentas vamos a revisar si están disponibles
 * o si ya están siendo usados por otro hilo
*/
    int wait = 1;
    int semaDisponible;
    
    while(wait){
        //primero miramos si está disponible el semáforo y lo bloqueamos!
        semaDisponible = sem_trywait(semaforo_retir);
        
        if(semaDisponible){     //si estaba disponible y se pudo bloquear!
            
            while(semaDisponible){      //ciclo para volver a iniciar la búsqueda del semáforo de la cuenta a consignar
                
                semaDisponible = sem_trywait(semaforo_consig);
                if(semaDisponible){ //si estaba disponible y se pudo bloquear!
                    //creamos el monto a retirar a partir de la cuenta de donde va a salir la consignación.
                    hora = time(NULL);
                    srand(hora);
                    dinero = rand()%(cuenta_retir->saldo);
                    transferir(dinero, cuenta_retir, cuenta_consig);
                    break;
                }else{  //si no entonces se espera a que lo suelten y se continúa.
                    sleep(100);
                    continue;
                }
            }
        }else{  //si no está diponible esperamos a que sea liberado y luego vuelve y juega
            sleep(100);
            continue;
        }
        wait = 0;
    }
    //soltamos los semáforos
    sem_post(&semaforo_retir);
    sem_post(&semaforo_consig);	
    
    return NULL;
}


int main(int argc, char* argv[]) {
    
/*
 Falta hacer la crítica luego la hago que no es tan importante.
*/
    int numero_cuentas = atoi(argv[2]), numeroHilos = atoi(argv[0]);
    float valor_inicial = (float) atoi(argv[3]), timerRun = (float)atoi(argv[1]);
    
/*
 Procedemos a crear las cuentas bancarias como lo indica el enunciado
*/
    struct cuentaBancaria cuentasB[numero_cuentas];
    
    int i;
    for(i=0; i < numero_cuentas; i++){
        cuentasB[i].nroCuenta = i;
        cuentasB[i].saldo = valor_inicial;
    }//listo cuentas creadas!
    cuentas = &cuentasB;
    
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
    semaforos = &vecSemaforos;
    
    //creamos el hilar!
    pthread_t hilar[numeroHilos];
    //mandamos como argumento del método, el tiempo de ejecución dividido por el número de hilos
    //para que cada uno procese el mismo tiempo
    
    void * argumentos[] = {((float)(timerRun/numeroHilos)), numero_cuentas};        //argumentos para el desarrollo del método
    
    for(i=0; i < numeroHilos; i++) pthread_create ( &hilar[i], NULL, (void *) mtoBancario, &argumentos);
    //esperamos a que termine el hilar!
    for(i=0; i < numeroHilos; i++) pthread_join ( hilar[i], NULL);
    
    for(i=0; i < numero_cuentas; i++){
        sem_unlink(&vecSemaforos[i]);   //desconectamos cada semáforo
    }//creados todos!
    return (0);
}

