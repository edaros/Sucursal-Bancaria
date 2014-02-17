Sucursal-Bancaria
=================

Este es un laboratorio para la materia de sistemas operativos de la UdeA,En la realización de este proyecto se van a  ver desde conceptos básicos de programación en c y unos conceptos un poco más avanzados, como son el manejo de primitivas de sincronización implementadas con semáforos y el manejo de aplicaciones multihiladas y los problemas de concurrencia que puedan presentarse.

La aplicacon para este proyecto tiene el nombre de sucursalBancaria.c
El  programa se ejecutaria de la siguiente manera recibiendo los siguientes parametros. 
Hay que decir que este programa es llamado por medio de otro el cual lee un archivo con varios vectores de prueba para luego hacer el llamado al programa sucursalbancaria.c 

“/nombre_programa numeroDeHilos cantidad_tiempo_a_correr numero_cuentas valor_inicial”

numeroDeHilos: este parametro nos indica la cantidad de hilos que el programa manejara para hacer transacciones entre diferentes cuentas bancarias.
cantidad_tiempo_a_correr: Esta cantidad es el tiempo de ejecucion de cada hilo; es decir el tiempo en el que un hilo estará realizando transacciones entres dos cuentas bancarias(como sabemos una transacción se compone retiro y deposito.) luego de que un hilo finalice su tiempo de ejecución, deberá imprimir el valor de todas las cuentas y el balance total, lo que nos indicará si el hilo hizo bien las transacciones entre cuentas.
numero_cuentas: Este parametro nos indica cuantas cuentas se deberán crear en cada ejecución del programa.
valor_inicial: El valor inicial es el saldo con el que cada cuenta iniciará para realizar todas las transacciones pertinentes.
Para  nuestra sucursalBancaria estamos utilizando sem_trywait()
La aplicación empieza a correr y el hilo principal creará todos los demás hilos especificados en los parámetros iniciales en  numeroHilos; los cuales se encargaran cada uno de realizar transacciones en las cuentas bancarias entre dos clientes diferentes.
Ya que han sido creados los hilos y las cuentas se encuentran inicializadas con sus montos respectivos, los hilos empiezan a ejecutarse realizando cada uno transacciones entre dos cuentas. Cuando los hilos se encuentran corriendo puede darse el caso de que dos hilos estén realizando operaciones entre las mismas cuentas entonces para ello utilizamos los semáforos. Cuando un hilo1 accede a una cuenta bancaria activa el semáforo de ésta e intenta acceder a la otra cuenta para realizar la transacción; pero se encuentra con que el semáforo no esta disponible, entonces no puede acceder a dicha cuenta, la cual esta siendo ocupada por el  hilo2 que se encuentra esperando  a que el hilo1 suelte la cuenta que tiene ocupada, cosa que no va a ser posible por que ambos no van a soltar dichas cuentas hasta realizar la transacción con éxito. Lo que nos produce un deadlock. Para evitar este deadlock en nuestra aplicación lo que vamos a hacer es prevenir esto; que seria evitar  una de las cuatro condiciones que se deben cumplir para que haya un deadlock Hold and Wait, la cual consiste al menos un proceso Pi ha adquirido un recurso Ri, y lo retiene mientras espera al menos un recurso Rj que ya ha sido asignado a otro proceso. 

