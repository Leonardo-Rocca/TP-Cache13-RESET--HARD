/*
 ============================================================================
 Name        : AdministradorMemoria.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include "memoriaMensajes.h"

int main(void) {

	puts("\033[1;31mAdministrador de Memoria\033[0m\n"); /* prints !!!Hello World!!! */
	printf ("\033[1;37mPID:%d\033[0m\n\n",getpid()); // esto para que sea mas rapido saber su pid, despues lo sacamos

	// kill -10 *pid* para tirar SIGUSR1, kill -12 *pid* para tirar SIGUSR2
	logear("\033[1;31mAdministrador de Memoria\033[0m");
	cargarArchivoDeConfiguracion();
	
	inicializarAdministrador();
	inicializarMutex();

	pthread_t hiloConexiones;
	pthread_t hiloConexionesSwap;

	pthread_t hiloSenial1; // el tp dice que las señales se  tienen que manejar con hilos
	pthread_t hiloSenial2;

	pthread_create(&hiloConexionesSwap,NULL,manejoConexionesConSwap,puertoSwap);
	pthread_create(&hiloConexiones,NULL,manejoConexionesConCPUs,NULL);

	pthread_create(&hiloSenial1,NULL,manejoSenial1,NULL);
	pthread_create(&hiloSenial2,NULL,manejoSenial2,NULL);

	signal(SIGUSR1,manejador); // signal es la funcion que atrapa la señal
	signal(SIGUSR2,manejador); // las puede atrapar cualquier sea el momento
	signal(SIGPOLL,manejador);


	pthread_join(hiloConexionesSwap,NULL);
	pthread_join(hiloConexiones,NULL);

	pthread_join(hiloSenial1,NULL);
	pthread_join(hiloSenial2,NULL);



	return EXIT_SUCCESS;
}

int recibirMensajeDe(int socketCpu){
	char idOperacionDeInterrupcion = recibirChar(socketCpu);
	int idProc;
	int resultado = recv(socketCpu, &idProc, sizeof(int), 0);
	while (resultado == -1 && errno == EINTR) // si la señal interrumpe el recv, se le asigna EINTR a errno
				{recv(socketCpu, &idProc, sizeof(int), 0);}

	//printf("idOperacion: %c \n",idOperacionDeInterrupcion);


	switch(idOperacionDeInterrupcion){
			case 'F':
				finalizar(socketCpu,idProc);
				return 1;
			case 'I':
				iniciar(socketCpu,idProc);
				return 1;
			case 'L':
				leer(socketCpu,idProc);
				return 1;
			case 'E':
				escribir(socketCpu,idProc);
				return 1;
			break;
			}

	return 0;
}

void iniciar(int socketCpu,int idProc){
	int cantDePaginas;

	int resultado = recv(socketCpu, &cantDePaginas, sizeof(int), 0);
	while (resultado == -1 && errno == EINTR) // si la señal interrumpe el recv, se le asigna EINTR a errno
			{recv(socketCpu, &cantDePaginas, sizeof(int), 0);}



	char respuesta;

	pthread_mutex_lock(&swap);
	respuesta = avisarSwapIniciar(idProc,cantDePaginas);
	pthread_mutex_unlock(&swap);

	dormir(retardoMemoria);
	inicializarEstructuras(idProc,cantDePaginas);

	send(socketCpu,&respuesta,sizeof(char),0);



	char stringLog[150];
	sprintf(stringLog,"\033[1;32mmProc Iniciado - PID:%d - Cantidad de paginas:%d\033[0m",idProc,cantDePaginas);
	logear(stringLog);
}

void leer(int socketCpu,int pid){
	int numeroPagina;
	int resultado = recv(socketCpu, &numeroPagina, sizeof(int), 0);
	while (resultado == -1 && errno == EINTR)
				{recv(socketCpu, &numeroPagina, sizeof(int), 0);}

	char* texto = leerPagina(pid,numeroPagina);
	//imprimirEstadoRAM();
	//imprimirEstadoTLB();
	//imprimirEstadoPaginasDeUnProceso(pid);

	//char* texto = avisarSwapLeer(pid,numeroPagina);


	mandarCadena(socketCpu,texto);
}

void escribir(int socketCpu,int pid){
	int numeroPagina;
	int resultado = recv(socketCpu, &numeroPagina, sizeof(int), 0);
	while (resultado == -1 && errno == EINTR)
				{recv(socketCpu, &numeroPagina, sizeof(int), 0);}
	char* texto = recibirCadena(socketCpu);


	escribirPagina(pid,numeroPagina,texto);
	//imprimirEstadoRAM();
	//imprimirEstadoTLB();
	//imprimirEstadoPaginasDeUnProceso(pid);


	//avisarSwapEscribir(pid,numeroPagina,texto);

	char exito = 'E';
	send(socketCpu, &exito, sizeof(char), 0);
}

void finalizar(int socketCpu,int pid){
	dormir(retardoMemoria);
	finalizarEstructuras(pid);
	//imprimirEstadoRAM();
	//imprimirEstadoTLB();

	pthread_mutex_lock(&swap);
	char respuesta = avisarSwapFinalizar(pid);
	pthread_mutex_unlock(&swap);

	send(socketCpu, &respuesta, sizeof(char), 0);
}

int manejoSenial1(){
	while (1){
		sem_wait(&semaforo1);
		logear("\033[1;37mSeñal SIGUSR1 recibida - TLB Flush\033[0m");
		pthread_mutex_lock(&mutexTLB);
	    inicializarTLB();
	    pthread_mutex_unlock(&mutexTLB);
		logear("\033[1;37mTratamiendo de señal SIGUSR1 terminado\033[0m");
		//imprimirEstadoTLB();
	         }
return 0;}

int manejoSenial2(){
	while(1){
		sem_wait(&semaforo2);
		logear("\033[1;37mSeñal SIGUSR2 recibida - Limpiar Memoria\033[0m");
		pthread_mutex_lock(&mutexRAM);
		eliminarFramesPorSenial();
		pthread_mutex_unlock(&mutexRAM);
	    logear("\033[1;37mTratamiendo de señal SIGUSR2 terminado\033[0m");
		}
return 0;}

void manejador (int sig){
 switch (sig){
	case SIGUSR1:
		sem_post(&semaforo1);
		break;
	case SIGUSR2 :
		sem_post(&semaforo2);
		break;
	case SIGPOLL :
		  logear("\033[1;37mSeñal SIGPOLL recibida - Volcado de Memoria principal\033[0m");
		  int id = fork();
		  switch(id){
	        case -1:  logear("Error Al crear el hijo");
			     break;
	        case 0:   logearSIGPOLL();
			     break;
			default:  break;
		              }
		break;
     }
}


void mandarCadena(int socket, char* cadena) {
	uint32_t long_cadena = strlen(cadena)+1;
	send(socket, &long_cadena, sizeof(long_cadena), 0);
	send(socket, cadena, long_cadena, 0);
}

char* recibirCadena(int socketQueEscribe) {
	char* cadena;
	uint32_t long_cadena;
	int resultado = recv(socketQueEscribe, &long_cadena, sizeof(long_cadena), 0);
	while (resultado == -1 && errno == EINTR)
				{recv(socketQueEscribe, &long_cadena, sizeof(long_cadena), 0);}
	cadena = malloc(long_cadena);
	resultado = recv(socketQueEscribe, cadena, long_cadena, 0);
	while (resultado == -1 && errno == EINTR)
					{recv(socketQueEscribe, cadena, long_cadena, 0);}
	return cadena;
}

void avisarSwapInstruccionYProceso(char instruccion, int pid){
	char instruccionAEnviar = instruccion;
	send(swapSocket, &instruccionAEnviar, sizeof(char), 0);
	send(swapSocket, &pid, sizeof(int), 0);
	return;
}


char avisarSwapIniciar(int pid, int cantidadPaginas){
	char inicio ='I';
	avisarSwapInstruccionYProceso(inicio,pid);printf("\033[1;32mAviso swap iniciar proceso:%d\033[0m\n",pid);

	send(swapSocket, &cantidadPaginas, sizeof(int), 0);
	char respuesta;
	int resultado = recv(swapSocket, &respuesta, sizeof(char), 0);
	while (resultado == -1 && errno == EINTR)
						{recv(swapSocket, &respuesta, sizeof(char), 0);}
//	printf("%c\n",respuesta);
	return respuesta;
}

char avisarSwapFinalizar(int pid){
	char fin = 'F';
	avisarSwapInstruccionYProceso(fin,pid);

	char respuesta;
	int resultado = recv(swapSocket, &respuesta, sizeof(char), 0);
	while (resultado == -1 && errno == EINTR)
							{recv(swapSocket, &respuesta, sizeof(char), 0);}
	//printf("%c\n",respuesta);
	return respuesta;
}

char* avisarSwapLeer(int pid, int numeroPagina){
	char leer = 'L';
	avisarSwapInstruccionYProceso(leer,pid);

	send(swapSocket, &numeroPagina, sizeof(int), 0);
	char* texto = recibirCadena(swapSocket);
	//printf("%s\n",texto);
	return texto;
}

char avisarSwapEscribir(int pid, int numeroPagina, char* texto){
	char escritura = 'E';
	avisarSwapInstruccionYProceso(escritura,pid);

	send(swapSocket, &numeroPagina, sizeof(int), 0);
	mandarCadena(swapSocket,texto);
	char respuesta;
	int resultado=recv(swapSocket, &respuesta, sizeof(char), 0);
	while (resultado == -1 && errno == EINTR)
							{recv(swapSocket, &respuesta, sizeof(char), 0);}
	//printf("%c\n",respuesta);
	return respuesta;
}

char recibirChar(int unSocket){
	char caracter;
	int resultado =recv(unSocket, &caracter, sizeof(caracter), 0);
	while (resultado == -1 && errno == EINTR)
								{recv(unSocket, &caracter, sizeof(caracter), 0);}
	return caracter;
}

void cargarArchivoDeConfiguracion(){
	
	FILE* punteroAlArchivoDeConfiguracion;
	char readParameterName[64];
	char readParameterValue[64];
	int index;

	int flagPuertoOK = 0;
	int flagIpSwapOK = 0;
	int flagPuertoSwapOK = 0;
	int flagMaximoMarcosPorProcesoOK = 0;
	int flagCantidadMarcosOK = 0;
	int flagTamanioMarcoOK = 0;
	int flagEntradasTLBOK = 0;
	int flagTlbHabilitadaOK = 0;
	int flagRetardoMemoriaOK = 0;
	int flagAlgoritmoReemplazoOK = 0;
	
	int error = 0;
	int endOfFile = 0;

	punteroAlArchivoDeConfiguracion = fopen("MEM_MNGR.CFG", "r""t");
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
	            if (!strcmp(readParameterName, "PUERTO_ESCUCHA")) {strcpy(puerto, readParameterValue); flagPuertoOK = 1;}
        	    if (!strcmp(readParameterName, "IP_SWAP")) {strcpy(ipSwap, readParameterValue); flagIpSwapOK = 1;}
	            if (!strcmp(readParameterName, "PUERTO_SWAP")) {strcpy(puertoSwap, readParameterValue); flagPuertoSwapOK = 1;}
        	    if (!strcmp(readParameterName, "MAXIMO_MARCOS_POR_PROCESO")) {maximoMarcosPorProceso = atoi(readParameterValue); flagMaximoMarcosPorProcesoOK = 1;}
	            if (!strcmp(readParameterName, "CANTIDAD_MARCOS")) {cantidadMarcos = atoi(readParameterValue); flagCantidadMarcosOK = 1;}
	            if (!strcmp(readParameterName, "TAMANIO_MARCO")) {tamanioMarco = atoi(readParameterValue); flagTamanioMarcoOK = 1;}
				if (!strcmp(readParameterName, "ENTRADAS_TLB")) {entradasTLB = atoi(readParameterValue); flagEntradasTLBOK = 1;}
				if (!strcmp(readParameterName, "RETARDO_MEMORIA")) {retardoMemoria = atof(readParameterValue); flagRetardoMemoriaOK = 1;}
				if (!strcmp(readParameterName, "ALGORITMO_REEMPLAZO")) {strcpy(algoritmoReemplazo, readParameterValue); flagAlgoritmoReemplazoOK = 1;}
				if (!strcmp(readParameterName, "TLB_HABILITADA")) {
																	tlbHabilitada = strcmp(readParameterValue, "NO"); //si da 0 era no
																	if(tlbHabilitada) tlbHabilitada = strcmp(readParameterValue, "SI") + 1;
																	if(tlbHabilitada == 0 || tlbHabilitada == 1) flagTlbHabilitadaOK = 1;
																}   	
		}while(endOfFile != 1);
    
		fclose(punteroAlArchivoDeConfiguracion);
	     }
	if(!error && flagPuertoOK && flagIpSwapOK && flagPuertoSwapOK && flagMaximoMarcosPorProcesoOK && flagCantidadMarcosOK && flagTamanioMarcoOK && flagEntradasTLBOK && flagTlbHabilitadaOK && flagRetardoMemoriaOK && flagAlgoritmoReemplazoOK)
        {printf("Archivo de configuraciones del Administrador de memoria cargado correctamente.\n");
         logear("Archivo de configuracion cargado correctamente");
        } else {perror("Error al cargar el archivo de configuraciones del Administrador de Memoria.\n");
        logear("Error al cargar el archivo de configuracion");
        }

	//printf("%s %s %s %d %d %d %d %d %s %d\n",puerto, ipSwap, puertoSwap, maximoMarcosPorProceso, cantidadMarcos, tamanioMarco, entradasTLB, retardoMemoria, algoritmoReemplazo, tlbHabilitada);
		
}


void logear(char *stringAlogear){
	t_log* log = log_create("LogMemo", "AdminMemoria", false, LOG_LEVEL_INFO);
	log_info(log, stringAlogear, "INFO");
	log_destroy(log);
}

void eliminarFramesPorSenial(){
int i;
for(i=0;i<cantidadMarcos;i++){
        if ( list_get(frames,i) != NULL){
             actualizarEstructurasPorSenial(i);
             list_replace(frames,i,NULL);
            }
        else {};
    }
}

void actualizarEstructurasPorSenial(int numeroFrame){
  int i;
  for (i=0;i<list_size(procesos);i ++){
      struct datosDelProceso* datos = list_get(procesos,i);
      int j;
      t_list * listaPaginas = datos->tabla;
      for (j=0; j<list_size(datos->tabla);j++){
           struct tablaPaginas* pagina = list_get (listaPaginas,j);
           if (pagina ->nroFrame == numeroFrame)
              {
                 datos->framesAsignados = datos->framesAsignados -1;
             	 pagina->bitModificado=0;
             	 pagina->bitPresencia=0;
             	 pagina->bitAccedido=-1;
             	 pagina->nroFrame=-1;

              }
           if (tlbHabilitada==1) resetearValoresEnTLB(datos->nroProceso,pagina);
       }
   }
}


/*

----------------------------------------------------------------------
----------------------------------------------------------------------
--------------POSIBLES FUNCIONES APLICABLES SEGUN PATRON--------------
----------------------------------------------------------------------
----------------------------------------------------------------------

//-----------SETTINGS FILE LOAD----------

void cargarArchivoDeConfiguracion(){
	cantidadEntradas = 50;
	cantidadPaginas = 512;
	tamanioPagina = 256;
	retardoAcceso = 60;
}

//----------INITIALIZE-----------

void createTlbCache(int numberOfEntries){


//Recordar que llegada una direccion virtual, la primera parte es el VPN. Ese //VPN se revisa en la tabla para buscar su correspondiente PPN. Ese valor es //la primera parte de la direccion real.
//La segunda parte de la direccion virtual se mantiene (su offset).
//En caso de no encontrarse en la tabla ese VPN, se debe buscar directamente //en la tabla de paginas (tiene todas).



	//allocate memory for Tlb table
	//can use union with struct

	VPN //Virtual Page Number
	PPN //Physical Page Number
}


//Preguntar si quieren asignar memoria al inicio para todos los procesos juntos o la division es para cada proceso en particular.
void allocateMemoryForProcesses(getSize,){

	allocate(tamanioTotalDisponibleParaProcesos);
	
}

organizeMemory(){

	while(spaceEnough)
		splitIntoFrames(FrameSize);
	createEntryIntoPageTable;

}

//----------ACCESS REQUESTED BY PROCESS------------


void translateVirtualAddressToPhysical(){
	//getPageNumber();
	//getOffset();
	//seekPageOnTlb();

	//if(notFound) seekPageOnPageTable
		//if(Page is not allocated) callSwap(); exchangePages(); seekPageOnPageTable();

	createNewPhysicalAddress();
	
}

int accessMainMemory(int Address, int Instruction){

	//instruction es 1 si es notWrite, 0 para Write	
	if(notWriting) return accessedData;

}

//-------------NEW PROCESS REQUESTED BY CPU--------------------


void newProcessRequest(){

	giveFrame();
	createNewPageTable(); //con solo ese frame nuevo

}

void requestForMoreMemory(){

//si se alcanzo el limite de paginas para ese proceso, no le asigna nada
	newPageInsideThatScope();
	updateTable();

}



//---------------PROCESS END REQUESTED BY CPU--------------

void endProcess(){

	freeMainMemory();
	informSwap();

}

//---------------SWAP INTERACTION FUNCTIONS------------

*already implemented: possible algorithm

void exchangePagesWithSwap(){

	askForTableExistence();
	requestToSaveOldPage();
	retrieveNewPage();
	allocateNewPageThenReturnToMemoryAccessFunction();

}

//--------------CPU INTERACTION FUNCTIONS---------------



//-------------INTERNAL FUNCTIONS-------------

	//getPageNumber();
	//getOffset();
	//seekPageOnTlb();
	//seekPageOnTable();
	//writeMemory();
	//readMemory();

void createPageTable(){

	int[] physicalPageNumber;
	informarSwapNumeroDePaginas();

}


















*/
