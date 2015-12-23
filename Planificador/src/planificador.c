/*
 * planificador.c
 *
 *  Created on: 1/9/2015
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include "planificadorMensajes.h"

char idPlanificador = 'P';
int contadorGlobalDeProcesos =0;

int main(void) {

	logear(" ");
	puts("\033[1;31mPlanificador\033[0m\n");
	logear("Planificador iniciado");

	iniciarTodosLosMutex();

	pthread_t hiloConexiones;
	pthread_t consola;
	cargarArchivoDeConfiguracion();
	colaDeEspera = list_create();

	pthread_create(&hiloConexiones,NULL,manejoConexionesConCPUs,NULL);
	pthread_create(&consola,NULL,manejoConsola,NULL);

	pthread_join(hiloConexiones,NULL);
	pthread_join(consola,NULL);
	return EXIT_SUCCESS;
}


//----------------CONSOLA-----------------------
void* manejoConsola(){
	char *p;
	printf("1: correr mProc\n2: finalizar PID\n3: ps\n4: cpu\n ");
	while(1){
		char *cadena = (char *)malloc(sizeof(char)*250);

			printf("Ingrese un comando: ");
			scanf("%s",cadena);
			p = cadena;

		do {
		if (!strcmp(p,"ps") || !strcmp(p,"3")) {
			imprimirEstadoProcesos();
				break;
			}

		if (!strcmp(p,"correr") || !strcmp(p,"1")) {
				char* path = (char *) malloc(sizeof(char)*200);
			//	printf("Ingrese la ruta relativa al programa “mCod” a ejecutar:\n");
				scanf("%s", path);
			//	printf("Se va a ejecutar el programa que esta en: %s",path);
				correr(path);  //en un futuro va a haber hilos aca, o capaz no
				free(path);
				break;
			}
		if (!strcmp(p,"finalizar") || !strcmp(p,"2")) {
				char* idProcesoAFinalizar = (char *) malloc(sizeof(char)*200);
				scanf("%s", idProcesoAFinalizar);
			//	printf("Se termino el programa con ID: %s\n",path);
				finalizarForzado(atoi(idProcesoAFinalizar));
				char log[500];
				idGlobalDeComparacion = atoi(idProcesoAFinalizar);
		//			if(list_any_satisfy(colaDeEspera,(void*)tieneMismoIdProceso))
		//		sprintf(log," Finalizar proceso - mProc:%d - mCod: %s ",atoi(idProcesoAFinalizar),obtenerPcb(idProcesoAFinalizar)->ruta);
				free(idProcesoAFinalizar);

				break;
			}
		if (!strcmp(p,"cpu") || !strcmp(p,"4")) {
			//TO-DO
			printf("%d cpus disponibles\n",list_size(socketsConectados));

			send(socketCpuPorcentaje, &idPlanificador, sizeof(char), 0); //envio operacion P
			obtenerEImprimirPorcentajesCpus();
				break;
					}

		if (1) {
			printf("El comando no es valido\n");
			break;
		}

		}while(1);

		free(cadena);

	}
}

void obtenerEImprimirPorcentajesCpus(){

	//	t_list* lista = list_create();
		int tamanio;

		recv(socketCpuPorcentaje, &tamanio, sizeof(int), 0);

		int i = 0;
		int elemento;

		for(i;i<tamanio;i++){

			recv(socketCpuPorcentaje, &elemento, sizeof(int), 0);

			printf("cpu %d : %d % \n",i,elemento);
		//	list_add(lista,elemento);
		}

}

void correr(char* path){

	pcb laPcb = cargarPcb(path);

	char log[500];
	sprintf(log,"Comienzo de mCod - mProc:%d - Ruta mCod:%s",laPcb.idProceso,laPcb.ruta);
	logear(log);

	pcb* pcbDelNuevoProceso = malloc(sizeof(pcb));
	*pcbDelNuevoProceso = laPcb;
	list_add(colaDeEspera,pcbDelNuevoProceso);

	verSiSePuedeMandarAejecutarUnProceso();

	return;
}

void enviarALaCpuQueEjecuteProceso(int socketDeCpuDisponible, pcb* laPcb) {

	send(socketDeCpuDisponible, &idPlanificador, sizeof(char), 0);
//	send(socketDeCpuDisponible, &idOperacion, sizeof(char), 0);  //no es necesario
	send(socketDeCpuDisponible, &(laPcb->idProceso), sizeof(int), 0);
	mandarCadena(socketDeCpuDisponible, laPcb->ruta);
	send(socketDeCpuDisponible, &quantum, sizeof(int), 0);
	send(socketDeCpuDisponible, &(laPcb->instructionPointer), sizeof(int), 0);

	//printf("Se va a ejecutar el programa que esta en: %s\n", laPcb->ruta);
//log_info(log_create(PLANIF_LOG_FILE, laPcb->ruta, 0, log_level_from_string("INFO")),strcat("Se envia a CPU a ejecutar el programa consignado con un PID de ",string->itoa(laPcb->idProceso)));

	laPcb->estado = 'E';

	//imprimirEstadoProcesos();
}

int verSiSePuedeMandarAejecutarUnProceso(){
	char log[300];
	char estadoDescripcion[50];
	char modoInicio,modoFin;
	int i;
	char idOperacion = 'C';
	
	for(i=list_size(colaDeEspera);i>0;i--) {
		pcb* pcbDeLaCola = (pcb*)list_get(colaDeEspera,i-1);
		switch(pcbDeLaCola->estado){
			case 'E': strcpy(estadoDescripcion,"Ejecutando"); modoInicio = '*'; modoFin = '*'; break;
			case 'F': strcpy(estadoDescripcion,"Finalizado"); modoInicio = '('; modoFin = ')'; break;
			case 'L': strcpy(estadoDescripcion,"Listo"); modoInicio = ' '; modoFin = ' '; break;
			case 'B': strcpy(estadoDescripcion,"Bloqueado"); modoInicio = '('; modoFin = ')'; break;
			case 'X': strcpy(estadoDescripcion,"Falla de programa"); modoInicio = '('; modoFin = ')'; break;
			default: strcpy(estadoDescripcion,"Estado desconocido"); modoInicio = ' '; modoFin = '?'; break;
		};
			
		sprintf(log,"Estado de la cola de ejecucion - Orden actual: %c%d%c - Proceso: %d - Estado: %s ",modoInicio,i-1,modoFin,pcbDeLaCola->idProceso,estadoDescripcion);
		logear(log);
	 }
	
	pthread_mutex_lock(&mutexLeo);
	if(  (!list_is_empty(colaDeEspera)) && list_any_satisfy(colaDeEspera,(void*)tieneEstadoReady)){
		//	PONER MUTEX
		pcb* laPcb = (pcb*)list_find(colaDeEspera,(void*)tieneEstadoReady);
		int socketDeCpuDisponible = obtenerCpuDisponible();
		if(socketDeCpuDisponible != -1){
			sprintf(log,"Seleccionado proceso para ejecutar - mProc:%d ",laPcb->idProceso);
			logear(log);
			enviarALaCpuQueEjecuteProceso(socketDeCpuDisponible,laPcb);
			}

		//	SACAR MUTEX
	}
	pthread_mutex_unlock(&mutexLeo);
	return 0;
}

int recibirMensajeDe(int socketQueEscribe){

	char verificacionCpu = recibirChar(socketQueEscribe); //verifica que no este caido
	char idOperacionDeInterrupcion = recibirChar(socketQueEscribe);
	int idProcesoMproc;

	recv(socketQueEscribe, &idProcesoMproc, sizeof(int), 0);
	pcb* laPcb = obtenerPcb(idProcesoMproc);
	int cantidadDeOperacionesRealizadas;

	recv(socketQueEscribe, &cantidadDeOperacionesRealizadas, sizeof(int), 0);
	laPcb->instructionPointer = cantidadDeOperacionesRealizadas+laPcb->instructionPointer;

	t_list* rafagaDeInstrucciones = recibirListaString(socketQueEscribe);
	//imprimirListaString(rafagaDeInstrucciones);

   //- -- --Verificamos que no este caido. Caso contrario no lo agregamos- --  --
	if(verificacionCpu=='C')agregarCpuComoDisponible(socketQueEscribe);

	char logCod[400];
	sprintf(logCod,"Rafaga de instrucciones terminada - mProc:%d - Ruta mCod:%s",laPcb->idProceso,laPcb->ruta);
	logear(logCod);
	logearListaString(rafagaDeInstrucciones);

	list_destroy(rafagaDeInstrucciones);

	pthread_mutex_lock(&mutexLeo);
	switch(idOperacionDeInterrupcion){
			case 'F': //finalizo				//printf("fin proceso %d\n",laPcb->idProceso);

				logear("Motivo de interrupcion: fin del programa ");//"Fin del programa consignado ;
				laPcb->estado = 'F';
				pthread_mutex_unlock(&mutexLeo);
				imprimirEstadoProcesos();
				verSiSePuedeMandarAejecutarUnProceso();
				return 1;
			case 'E': //entrada/salida

				logear("Motivo de interrupcion: programa en entrada-salida");//"Inicio de una Entrada-Salida.");
				laPcb->estado = 'B';
				pthread_mutex_unlock(&mutexLeo);
				manejarProcesoEntradaSalida(socketQueEscribe,laPcb);
			//	imprimirEstadoProcesos();
				verSiSePuedeMandarAejecutarUnProceso();
				return 1;
			case 'Q': //por fin de quantum

				logear("Motivo de interrupcion: fin de quantum"); //"Rafaga de CPU completada para el proceso ";
				laPcb->estado = 'L';
				pthread_mutex_unlock(&mutexLeo);
				mandarAlFinalDeLaCola(laPcb);
				verSiSePuedeMandarAejecutarUnProceso();
				return 1;
			case 'X': //Fallo de programa

				logear("Motivo de interrupcion: fallo el programa");
				printf("Fallo el programa %d\n",laPcb->idProceso);
				laPcb->estado = 'F';
				pthread_mutex_unlock(&mutexLeo);
				verSiSePuedeMandarAejecutarUnProceso();
				return 1;
			break;
	}
	return 0;
}

void manejarProcesoEntradaSalida(int socketQueEscribe,pcb* laPcb){
	pthread_t entradaSalida;
	int tiempoDeEspera;

	recv(socketQueEscribe, &tiempoDeEspera, sizeof(int), 0);
//	printf("time %d\n",tiempoDeEspera);

	t_Package* paquete = malloc(sizeof(t_Package));   				   //ver si funciona sin malloc
	paquete->laPcbDelProceso = laPcb;
	paquete->socketCpu=socketQueEscribe;
	paquete->tiempoDeEspera=tiempoDeEspera;
	pthread_create(&entradaSalida,NULL,(void*)suspenderProceso,paquete);
//	agregarCpuComoDisponible(socketQueEscribe);


}
void suspenderProceso(t_Package* paquete){
//	printf("time2 %d\n",paquete->tiempoDeEspera);
	sleep(paquete->tiempoDeEspera);

	(paquete->laPcbDelProceso)->estado='L';
	mandarAlFinalDeLaCola(paquete->laPcbDelProceso);

	verSiSePuedeMandarAejecutarUnProceso();

}

void mandarAlFinalDeLaCola(pcb* laPcb){
	//PONER MUTEX
	pthread_mutex_lock(&mutexLeo2);
	idGlobalDeComparacion = laPcb->idProceso;
	list_remove_by_condition(colaDeEspera,(void*)tieneMismoIdProceso);
	list_add(colaDeEspera,laPcb);
	pthread_mutex_unlock(&mutexLeo2);
	return;
}

pcb* obtenerPcb(int idProcesoMproc){
	//PONER MUTEX
	pthread_mutex_lock(&mutexLeo2);
	pcb* laPcb;
	idGlobalDeComparacion = idProcesoMproc;
	if(list_any_satisfy(colaDeEspera,(void*)tieneMismoIdProceso)){
	laPcb = (pcb*)list_find(colaDeEspera,(void*)tieneMismoIdProceso);
	}else{
		printf("ERROR, NO EXISTE PROCESO %d\n",idProcesoMproc);
		laPcb = malloc(sizeof(pcb));
		laPcb->idProceso=-1;
	}
	pthread_mutex_unlock(&mutexLeo2);
	//sacar MUTEX
	return laPcb;
}

int obtenerCpuDisponible(){
	if(!list_is_empty(socketsConectados)){
		int* unSockCpu = list_get(socketsConectados,0);
		int soc =*unSockCpu;
		list_remove(socketsConectados,0);
		free(unSockCpu);
	return soc;}

	printf("Todavia no hay cpu disponible para ejecutar \n");
	return -1;
}
void agregarCpuComoDisponible(int socketCliente){
	int* socketDelCpuParaAgregar = malloc(sizeof(int));
  	*socketDelCpuParaAgregar = socketCliente;
 	list_add(socketsConectados,socketDelCpuParaAgregar);

}
pcb cargarPcb(char* path){
	pcb laPcb;
	//POSIBLE MUTEX
	pthread_mutex_lock(&mutexLeo4);
	laPcb.idProceso =contadorGlobalDeProcesos+1;
	contadorGlobalDeProcesos += 1;
	pthread_mutex_unlock(&mutexLeo4);
	//laPcb.estado = 'N'; //nuevo (capaz valla ready)
	laPcb.estado = 'L';
	char* ruta = malloc(strlen(path)+1);
	strcpy(ruta,path);
	laPcb.ruta = ruta;
	laPcb.instructionPointer = 0;
	return laPcb;
}

void finalizarForzado(int idProcesoAFinalizar){
	pcb* laPcb = obtenerPcb(idProcesoAFinalizar);
	laPcb->instructionPointer=-9999; //ADVERTENCIA: si son muy largos los .cod esto deberia cambiar
	if(laPcb->idProceso != -1 ) {printf("Se finalizo proceso %d \n",laPcb->idProceso);}

}

void imprimirEstadoProcesos(){
	printf(" Estado de los Procesos: \n");
	int i = 0;
	int pid;
	char* path;
	char estado;
	char* elEstado;
	for(i;i<list_size(colaDeEspera);i++){
		pcb* lapcb = list_get(colaDeEspera,i);
		pid = lapcb->idProceso;
		path = lapcb->ruta;
		estado = lapcb->estado;
		switch(estado){
		case 'E':
			elEstado = "Ejecutando";
			break;
		case 'F':
			elEstado = "Finalizado";
			break;
		case 'L':
			elEstado = "Listo     ";
			break;
		case 'B':
			elEstado = "Bloqueado";
			break;
		}
		printf("mProc %d: %s -> %s \n",pid,path,elEstado);
	}
	if(list_is_empty(colaDeEspera))
		printf("todavia no se ha ejecutado ningun proceso\n");
}

int tieneEstadoReady(void* unaPcb){
	pcb laPcb = *(pcb*)unaPcb;
	return laPcb.estado == 'L';
}
int tieneMismoIdProceso(pcb* pC){
	int id =pC->idProceso;
	return idGlobalDeComparacion==id;
}


void mandarCadena(int socket, char* cadena) {
	uint32_t long_cadena = strlen(cadena)+1;
	send(socket, &long_cadena, sizeof(long_cadena), 0);
	send(socket, cadena, long_cadena, 0);
}
char* recibirCadena(int socketQueEscribe) {
	char* cadena;
	uint32_t long_cadena;
	recv(socketQueEscribe, &long_cadena, sizeof(long_cadena), 0);
	cadena = malloc(long_cadena);
	recv(socketQueEscribe, cadena, long_cadena, 0);
	return cadena;
}

t_list* recibirListaString(int sock){
	t_list* lista = list_create();
	int tamanio;

	recv(sock, &tamanio, sizeof(int), 0);

	int i = 0;
	char* cadena;
	for(i;i<tamanio;i++){

		cadena = recibirCadena(sock);

		list_add(lista,cadena);
	}
return lista;
}

void imprimirListaString(t_list* lista){
	 int tamanio = list_size(lista);
	int i = 0;
	char* cadena;
	for(i;i<tamanio;i++){
		cadena = list_get(lista,i);
		printf("%s\n",cadena);
	}
return ;
}

void eliminarLista(t_list* lista){
	int tamanio = list_size(lista);
	int i = 0;
	int* elemento;
	for(i;i<tamanio;i++){
		elemento = list_get(lista,i);
		free(elemento);
	}
	list_destroy(lista);
	return;
}








void logearListaString(t_list* lista){
	pthread_mutex_t mutexLista;
	int tamanio = list_size(lista);
	int i = 0;
	char* cadena;
	pthread_mutex_lock(&mutexLista);
	for(i;i<tamanio;i++){
		cadena = list_get(lista,i);
		logear(cadena);
	}
    pthread_mutex_unlock(&mutexLista);
return ;
}

void cargarArchivoDeConfiguracion(){

	FILE* punteroAlArchivoDeConfiguracion;
	char readParameterName[64];
	char readParameterValue[64];
	int index;

	char algoritmo[20];

	int flagPortParamOK = 0;
	int flagAlgorithmNameParamOK = 0;
	int flagQuantumParamOK = 0;
	int error = 0;
	int endOfFile = 0;

	punteroAlArchivoDeConfiguracion = fopen("PLANIF.CFG", "r""t");
	if (punteroAlArchivoDeConfiguracion == NULL)
		error = 1;
	else {
    		fseek(punteroAlArchivoDeConfiguracion, 0, SEEK_SET);	    
		do{
    		    index = 0;

    		    do {
		        fread(&readParameterName[index], 1, 1, punteroAlArchivoDeConfiguracion);
		        index++;
        	    } while (!feof(punteroAlArchivoDeConfiguracion) && index < 63 && ((readParameterName[index-1] >= 'A' && readParameterName[index-1] <= 'Z') || readParameterName[index-1] == '_'));

			if (feof(punteroAlArchivoDeConfiguracion)) break;
	   	    if (index >= 63 || (readParameterName[index-1] != ' ' && readParameterName[index-1] != '\n')) {error = 1; break;}
	    	if (readParameterName[index-1] == '\n') continue;
				readParameterName[index-1] = 0;

	    //Now, read the value:
		    index = 0;
		    do {
		        fread(&readParameterValue[index], 1, 1, punteroAlArchivoDeConfiguracion);
		        index++;
		    } while (index-1 < 63 && readParameterValue[index-1] != ' ' && readParameterValue[index-1] != '\n' && !feof(punteroAlArchivoDeConfiguracion));

	   	    if(feof(punteroAlArchivoDeConfiguracion)) readParameterValue[index-1] = 0;
	    	    switch (readParameterValue[index-1]) {
		        case 0: endOfFile = 1; break;
		        case '\n': readParameterValue[index-1] = 0; break;
	        	default: error = 1;
		    };
    
	     	    if(index == 1) error = 1;
		    if(error == 1) break;
	            if (!strcmp(readParameterName, "PUERTO_ESCUCHA")) {strcpy(puerto, readParameterValue); flagPortParamOK = 1;}
        	    if (!strcmp(readParameterName, "ALGORITMO_PLANIFICACION")) {strcpy(algoritmo, readParameterValue); flagAlgorithmNameParamOK = 1;}
	            if (!strcmp(readParameterName, "QUANTUM")) {quantum = atoi(readParameterValue); flagQuantumParamOK = 1;}
    
		}while(endOfFile != 1);
    
		fclose(punteroAlArchivoDeConfiguracion);
	     }

	if(!strcmp(algoritmo,"FIFO")) {quantum = -1; flagQuantumParamOK = 1;} //FIFO (no RR)

	if(!error && flagPortParamOK && flagAlgorithmNameParamOK && flagQuantumParamOK)
		logear("Archivo de configuracion cargado correctamente");
		else
		logear("Error al cargar el archivo de configuracion");
        
	//printf("%s %s %d\n",puerto,algoritmo,quantum);

}

void logear(char *stringAlogear){
    t_log* log = log_create("LogPlanificador", "Planificador", false, LOG_LEVEL_INFO);
	log_info(log, stringAlogear, "INFO");
	log_destroy(log);
}

void iniciarTodosLosMutex(){
	if(pthread_mutex_init(&mutexLeo,NULL)){
			printf("\n Mutex init failed \n");
		}

	if(pthread_mutex_init(&mutexLeo2,NULL)){
			printf("\n Mutex init failed \n");
		}

	if(pthread_mutex_init(&mutexLeo3,NULL)){
			printf("\n Mutex init failed \n");
		}

	if(pthread_mutex_init(&mutexLeo4,NULL)){
				printf("\n Mutex init failed \n");
			}
}


/*
char * itoa(int value) {
  char* stringBuffer = malloc(8*sizeof(char));
  sprintf(stringBuffer, "%d", value);
  sprintf(stringBuffer, 0);
  return stringBuffer;
}*/
