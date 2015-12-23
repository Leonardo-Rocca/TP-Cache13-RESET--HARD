/*
 * cpu.c

 *
 *  Created on: 1/9/2015
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include "cpuMensajes.h"


int main(void) {
	puts("\033[1;31mCPU\033[0m\n");
	pthread_t hiloCpu;
	pthread_t hiloCpuPorcentaje;
	pthread_t hiloReset; //(--hard)
	iniciarTodosLosMutex();

	logear ("CPU iniciado");
	cargarArchivoDeConfiguracion();
	listaInstruccionesCpu = list_create();
	listaPorcentajeUltMinuto = list_create();

	int socketPorcentaje = conectar(ipPlanificador, puertoPlanificador,'P');
	int* sockP= malloc(sizeof(int));
	*sockP=socketPorcentaje;
	pthread_create(&hiloCpuPorcentaje,NULL,comunicarPorcentaje,sockP);
	pthread_create(&hiloReset,NULL,recetPorcentajes,NULL);

	int i = 0;
	for(i;i<cantidadDeHilosCpu;i++){
		socketAdminMemoria = conectar(ipAdminMemoria,puertoAdminMemoria,'C');

		char stringAlogear[300];
		sprintf (stringAlogear,"Instancia de CPU creada. Conectado con Administrador de Memoria - PID:%d",i);
		logear(stringAlogear);

		int socketPlanificador = conectar(ipPlanificador, puertoPlanificador,'C');

		//--Para el porcentaje de los Cpus---
		int* instruccionInicial = malloc(sizeof(int));
		*instruccionInicial=0;
		pthread_mutex_lock(&mutexSobreListaDeInstrucciones);
		list_add(listaInstruccionesCpu,instruccionInicial);
		pthread_mutex_unlock(&mutexSobreListaDeInstrucciones);

		//--Cargo el HISTORIAL Para el porcentaje de los Cpus---
		int* instruccionHistorial = malloc(sizeof(int));
		*instruccionHistorial=0;
		pthread_mutex_lock(&mutexSobreListaPorcentajeUltMinuto);
		list_add(listaPorcentajeUltMinuto,instruccionHistorial);
		pthread_mutex_unlock(&mutexSobreListaPorcentajeUltMinuto);

		t_Package* paquete = malloc(sizeof(paquete));
		(*paquete).idCpu = i;
		(*paquete).socketPlanificador = socketPlanificador;
		(*paquete).socketAdminMemoria = socketAdminMemoria;
	//	printf("soy CPU: %d\n",i);
		pthread_create(&hiloCpu,NULL,ejecutarCpu,paquete);
	}
	pthread_join(hiloCpu,NULL);
	pthread_join(hiloCpuPorcentaje,NULL);

	return 0;
}

void ejecutarCpu(t_Package* paquete){
	int socketPlanificador = paquete->socketPlanificador;
	int socketAdminMemoria = paquete->socketAdminMemoria;
	int idCpu = paquete->idCpu;
	int status = 1;
    while (status){
	    status = recibirMensajeDe(socketPlanificador,paquete);
	  }
	   // loguearDesconexionDeProceso(nombreProceso,nroSocket);
	    printf("Planificador Desconectado.\n");

	 if(status == -1)
	   {
	       perror("recv failed");
	   }

	    return ;
}

void dormir(float numero){
	if(numero >= 1){
		sleep(numero);
	} else {
		usleep(numero*1000000);
	}
}

void comunicarPorcentaje(int* sockPlanificador){

	while(1){
	int sockPlanif = *sockPlanificador;
	char ordenOperacion = recibirChar(sockPlanif);
	if(ordenOperacion=='P'){
	//	t_list* porcentajes = obtenerListaPorcentajes();  //sin promedio

		t_list* p = obtenerListaPorcentajes();
		pthread_mutex_lock(&mutexSobreListaPorcentajeUltMinuto);
		t_list* porcentajes = promediarListas(p,listaPorcentajeUltMinuto);
		pthread_mutex_unlock(&mutexSobreListaPorcentajeUltMinuto);
		enviarListaEntero(sockPlanif,porcentajes);

		eliminarLista(p);
		eliminarLista(porcentajes);
	}else{
		printf("Planificador desconectado o error");
		break;
	}
	}
}

t_list* promediarListas(t_list* listaPorcentajes, t_list* listaPorcentajeUltMin){
	t_list* lPorcentaje=list_create();
	int tamanio = list_size(listaPorcentajes);
	int i = 0;
	int* porcentaje;
	int* porcentajeHistorico;
	for(i;i<tamanio;i++){
		porcentaje = list_get(listaPorcentajes,i);
		porcentajeHistorico = list_get(listaPorcentajeUltMin,i);

		int* porcentajeAagregar = malloc(sizeof(int));
		float calculo =(0.9 * (*porcentaje) + 0.1 * (*porcentajeHistorico));
	//	float calculo =(0 * (*porcentaje) + 1 * (*porcentajeHistorico));
		*porcentajeAagregar =(int)calculo ;
		//*porcentaje =(int)calculo ;
		list_add(lPorcentaje,porcentajeAagregar);
	}
	return lPorcentaje;
}

pcb recibirPCB(int socketQueEscribe) {
	pcb pcbDelProceso;

	int idProc;
	recv(socketQueEscribe, &idProc, sizeof(int), 0);
	pcbDelProceso.idProceso = idProc;

	pcbDelProceso.ruta = recibirCadena(socketQueEscribe);

	int quantum;
	recv(socketQueEscribe, &quantum, sizeof(int), 0);
	pcbDelProceso.quantum=quantum;

	recv(socketQueEscribe, &(pcbDelProceso.instructionPointer), sizeof(int), 0);

	return pcbDelProceso;
}

void enviarRespuestaAlPlanificador(int socket_Planificador,informacionSobreLasInstrucciones respuesta,pcb pcbDelProceso) {
	char soyProceso = 'C';

	send(socket_Planificador, &(soyProceso), sizeof(char), 0);
	send(socket_Planificador, &(respuesta.instruccionQueCausoLaInterrupccion),	sizeof(char), 0);
	send(socket_Planificador, &(pcbDelProceso.idProceso), sizeof(int), 0);
	int cantidadDeOperacionesRealizadas = respuesta.cantidadDeInstrucciones;
	send(socket_Planificador, &cantidadDeOperacionesRealizadas, sizeof(int), 0);

	enviarListaString(socket_Planificador,respuesta.resultadosDeInstrucciones);

	eliminarLista(respuesta.resultadosDeInstrucciones);

	if ((respuesta.instruccionQueCausoLaInterrupccion) == 'E')
		send(socket_Planificador, &(respuesta.tiempo), sizeof(int), 0);	//mando el tiempo}
return;
}

int recibirMensajeDe(int socketQueEscribe,t_Package* paquete){

	char idProcesoConElQueMeComunico = recibirChar(socketQueEscribe);
	pcb pcbDelProceso;

	FILE* archivoAAbrir;
	switch(idProcesoConElQueMeComunico){
		case 'P': //para mensajes de Planificador

				pcbDelProceso = recibirPCB(socketQueEscribe);  //RECIBE datos del proceso

				char stringAlogear[350 + sizeof(pcbDelProceso.ruta)];
				if(pcbDelProceso.quantum == -1) //o sea, si es FIFO y el quantum no me interesa
					sprintf(stringAlogear,"PCB recibida - PID:%d - Ruta:%s - Instruction Pointer:%d",paquete->idCpu,pcbDelProceso.ruta,pcbDelProceso.instructionPointer);
				else sprintf(stringAlogear,"PCB recibida - PID:%d - Ruta:%s - Quantum:%d - Instruction Pointer:%d",paquete->idCpu,pcbDelProceso.ruta,pcbDelProceso.quantum,pcbDelProceso.instructionPointer);
				logear(stringAlogear);

				informacionSobreLasInstrucciones respuesta;
				inicializarRespuesta(&respuesta);

				if(pcbDelProceso.instructionPointer>-1){
					archivoAAbrir = fopen(pcbDelProceso.ruta,"r");

					if(archivoAAbrir!= NULL){

						respuesta = leerInterpretarInstruccion(paquete,pcbDelProceso,archivoAAbrir);

					}
					else {
						printf("\nERROR AL ABRIR ARCHIVO %s \n",pcbDelProceso.ruta);
						respuesta.instruccionQueCausoLaInterrupccion = 'X';
					}
				}else{
					respuesta = finalizarForzado(pcbDelProceso.idProceso,paquete->socketAdminMemoria);
					}

				printf("\033[1;33mCpu: %d - Interrupcion:%c\033[0m\n",paquete->idCpu,( respuesta.instruccionQueCausoLaInterrupccion));

				enviarRespuestaAlPlanificador(socketQueEscribe, respuesta, pcbDelProceso);

				return 1;
		break;

	}

	return 0;
}


//--------IDENTIFICAR INSTRUCCIONES------------
informacionSobreLasInstrucciones leerInterpretarInstruccion(t_Package* paquete,pcb laPcb,FILE* archivoAbierto){

	int socketAdminMemoriaDelCpu = paquete->socketAdminMemoria;
	informacionSobreLasInstrucciones respuesta;
	inicializarRespuesta(&respuesta);

	char* path=laPcb.ruta;
	int quantum = laPcb.quantum;
	int id = laPcb.idProceso;

//	FILE* archivoAbierto = fopen(path,"r");
	int offset = calculaOffsetConElIp(path,laPcb.instructionPointer);
	fseek(archivoAbierto,offset,SEEK_SET);


	do{
	char* instruccion = (char*)malloc(sizeof(char)*1040);
	fgets(instruccion,1040,archivoAbierto);

	if (quantum != -1) quantum--; //quiere decir que es FIFO
	char* cuerpo=malloc(sizeof(char)*20);
	char* parametro1=malloc(sizeof(char)*10);
	char* parametro2=malloc(sizeof(char)*1024);;
	desarmarInstruccion(instruccion,&cuerpo,&parametro1,&parametro2);
//	printf(" cuerpo: %s\n parametro1: %s parametro2: %s\n",cuerpo,parametro1,parametro2);

	char* resultadoDeIntruccion;
	int parametro1ComoEntero ;
	if(parametro1!=NULL)parametro1ComoEntero= atoi(parametro1);
	respuesta.cantidadDeInstrucciones ++;

	char log[400];
	int operacionFallida=0;
	while(1){
	if(!strcmp(cuerpo,"iniciar")){
		resultadoDeIntruccion = iniciar(parametro1ComoEntero,id,socketAdminMemoriaDelCpu,&operacionFallida);

		if (operacionFallida == 0){
			sprintf(log,"Instruccion ejecutada - PID:%d - Instruccion:%s - Parametro 1:%d - Resultado: Exito",paquete->idCpu,cuerpo,parametro1ComoEntero);
		}
		else{
			sprintf(log,"Instruccion ejecutada - PID:%d - Instruccion:%s - Parametro 1:%d - Resultado: Fallo",paquete->idCpu,cuerpo,parametro1ComoEntero);
		}
		break;
		}

	if(!strcmp(cuerpo,"leer")){
		resultadoDeIntruccion = leer(id,parametro1ComoEntero,socketAdminMemoriaDelCpu);

		sprintf(log,"Instruccion ejecutada - PID:%d - Instruccion:%s - Parametro 1:%d - Resultado:Exito",paquete->idCpu,cuerpo,parametro1ComoEntero);

		break;
		}

	if(!strcmp(cuerpo,"escribir")){
		resultadoDeIntruccion = escribir(id,parametro1ComoEntero,parametro2,socketAdminMemoriaDelCpu);

		sprintf(log,"Instruccion ejecutada - PID:%d - Instruccion:%s - Parametro 1:%d - Parametro 2:%s - Resultado: Exito ",paquete->idCpu,cuerpo,parametro1ComoEntero,parametro2);

		break;
		}

	if(!strcmp(cuerpo,"entrada-salida")){

		respuesta.tiempo = parametro1ComoEntero;
		respuesta.instruccionQueCausoLaInterrupccion = 'E';
		resultadoDeIntruccion = entradaSalida(respuesta.tiempo);

		sprintf(log,"Instruccion ejecutada - PID:%d - Instruccion:%s - Parametro 1:%d - Resultado: Exito",paquete->idCpu,cuerpo,parametro1ComoEntero);

		quantum=-2;
		break;
		}

	if(!strcmp(cuerpo,"finalizar")){
		respuesta.instruccionQueCausoLaInterrupccion = 'F';
		resultadoDeIntruccion = finalizar(id,socketAdminMemoriaDelCpu);

		sprintf(log,"Instruccion ejecutada - PID:%d - Instruccion:%s - Resultado:Exito ",paquete->idCpu,cuerpo);
		printf("\033[1;32mFinalizo mProc %d\033[0m\n",id);
		quantum=-2;
		break;
		}

	if(1){
		printf("Instruccion invalida\n");
		//return; lo comento para probar
		break;
		}
	}

	logear(log);

	char* palabra = concatenarStrings("mProc ", string_itoa(id));
	palabra = concatenarStrings(palabra,resultadoDeIntruccion);

	list_add(respuesta.resultadosDeInstrucciones,palabra);//agregarStringALista(respuesta.resultadosDeInstrucciones,palabra);
	printf("\033[1;37mCpu: %d - Instruccion: %s \033[0m\n",paquete->idCpu,palabra);
	liberarMemoriaDeInstruccion(cuerpo, parametro1, parametro2,	instruccion);

	agregarInstruccionCpuPorcentaje(paquete->idCpu); //para el %

	if(operacionFallida==0){
	dormir(retardo);}
	else {
		respuesta.instruccionQueCausoLaInterrupccion='X';
		quantum=-2;
	}

	}while((quantum > 0 || quantum == -1) && !feof(archivoAbierto));

char logearFin[100];
sprintf (logearFin,"Rafaga de CPU concluida - PID:%d",paquete->idCpu);
logear(logearFin);

fclose(archivoAbierto);
return respuesta;
}

//al socketAdminMemoria
void enviarCharIntInt(char idOperacion, int idProcesoAiniciar,	int cantDePaginas,int sock) {
	pthread_mutex_lock(&mutexFede);
	send(sock, &idOperacion, sizeof(char), 0);
	send(sock, &idProcesoAiniciar, sizeof(int), 0);
	send(sock, &cantDePaginas, sizeof(int), 0);
	pthread_mutex_unlock(&mutexFede);
}

 char* iniciar(int cantDePaginas,int idProcesoAiniciar,int socketAdminMemoriaDelCpu,int* operacionFallida){
	char idOperacion = 'I';
	enviarCharIntInt(idOperacion, idProcesoAiniciar, cantDePaginas,socketAdminMemoriaDelCpu);

	recv(socketAdminMemoriaDelCpu,&idOperacion,sizeof(char),0);
	if(idOperacion=='E')return "- Iniciado";
	*operacionFallida = 1;
	return "- Fallo";
}

char* leer(int pid,int numeroPagina,int socketMemoria){
	char leer = 'L';
	enviarCharIntInt(leer, pid, numeroPagina,socketMemoria);

	char* texto = recibirCadena(socketMemoria);

	char* palabra = concatenarStrings("- Pagina ", string_itoa(numeroPagina));
	palabra = concatenarStrings(palabra," leida:");
	palabra = concatenarStrings(palabra,texto);
	return palabra;
}

char* escribir (int pid,int numeroPagina,char* texto ,int socketMemoria){
	char escrito = 'E';
	enviarCharIntInt(escrito, pid, numeroPagina,socketMemoria);
	mandarCadena(socketMemoria,texto);

	char respuesta;
	recv(socketMemoria, &respuesta, sizeof(char), 0);
	//printf("%c\n",respuesta);

	char* palabra = concatenarStrings("- Pagina ", string_itoa(numeroPagina));
	palabra = concatenarStrings(palabra," escrita:");
	palabra = concatenarStrings(palabra,texto);
	return palabra;
}

char* entradaSalida(int tiempo){
	//char palabra[strlen("en entrada-salida de tiempo ")+1];
	char*palabra1 = " en entrada-salida de tiempo ";
	char* palabra = concatenarStrings(palabra1, string_itoa(tiempo));
	return palabra;
}

char* finalizar(int pid,int socketMemoria){
	char fin = 'F';
	send(socketMemoria, &fin, sizeof(char), 0);
	send(socketMemoria, &pid, sizeof(int), 0);
	char respuesta;
	recv(socketMemoria, &respuesta, sizeof(char), 0);
	//printf("%c\n",respuesta);
	return " finalizado";
}

informacionSobreLasInstrucciones finalizarForzado(int pid,int socketAdminMemoriaDelCpu){
	informacionSobreLasInstrucciones respuesta;
	inicializarRespuesta(&respuesta);

	char* resultadoDeIntruccion;
	respuesta.instruccionQueCausoLaInterrupccion = 'F';
	resultadoDeIntruccion = finalizar(pid,socketAdminMemoriaDelCpu);
	printf("\033[1;35mFinalizo forzado\033[0m\n");

	char* palabra = concatenarStrings("mProc ", string_itoa(pid));
	palabra = concatenarStrings(palabra,resultadoDeIntruccion);

	list_add(respuesta.resultadosDeInstrucciones,palabra);//agregarStringALista(respuesta.resultadosDeInstrucciones,palabra);

return respuesta;
}

//----CALCULA BIEN EL PUNTERO DE INSTRUCCION-------

int calculaOffsetConElIp(char* path,int linea){
	char * mapeo;
	int fd = open(path,O_RDONLY);
	int tamanioArchivo = tamanio_archivo(fd);

	if( (mapeo = mmap(NULL, tamanioArchivo, PROT_READ, MAP_SHARED, fd, 0 )) == MAP_FAILED){
		//Si no se pudo ejecutar el MMAP, imprimir el error y aborta;
		printf("Error con MMAP");
		abort();
		}

	int a=0;
	int b=0;
	while(a != linea){
		while(mapeo[b]!='\n'){
			b++;
		}
		b++;
		a++;
	}

	munmap(mapeo,tamanioArchivo);
	close(fd);
	return b;
}

int tamanio_archivo(int fd){
	struct stat buf;
	fstat(fd, &buf);
	return buf.st_size;
}


//--------HASTA ACA ES NECESARIO PARA CALCULAR BIEN EL IP----------


void logear(char *stringAlogear){
    t_log* log = log_create("LogCPU", "CPU", false, LOG_LEVEL_INFO);
	log_info(log, stringAlogear, "INFO");
	log_destroy(log);
}



void liberarMemoriaDeInstruccion(char* cuerpo, char* parametro1,char* parametro2, char* instruccion) {
	free(cuerpo);
	free(parametro1);
	free(parametro2);
	free(instruccion);
}



void inicializarRespuesta(informacionSobreLasInstrucciones* respuesta) {
	respuesta->instruccionQueCausoLaInterrupccion = 'Q'; //por default es quantum
	respuesta->cantidadDeInstrucciones = 0;
	respuesta->resultadosDeInstrucciones = list_create();
}


char* obtenerDatoSinPYC(char* dato){  //PYC:Punto y coma
	int i=0;
	while(dato[i] != ';' && dato[i] != '\n'){
		i++;
	}
	return string_substring_until(dato,i);
}
void desarmarInstruccion(char* instruccion,char** cuerpo,char** parametro1,char** parametro2){
	char** array = string_n_split(instruccion,3," ");
	*cuerpo = obtenerDatoSinPYC(array[0]);
	int entro=0;

	if(!strcmp(*cuerpo,"finalizar")){
		*parametro1 = NULL;
		*parametro2 = NULL;
		entro=1;
	}
	if(!strcmp(*cuerpo,"escribir")){
		*parametro1 = obtenerDatoSinPYC(array[1]);
		char** arrayAux = string_split(obtenerDatoSinPYC(array[2]),"\"");
		*parametro2 = arrayAux[0];
		entro=1;
	}
	if(!strcmp(*cuerpo,"iniciar") || !strcmp(*cuerpo,"entrada-salida") || !strcmp(*cuerpo,"leer")){
		*parametro1 = obtenerDatoSinPYC(array[1]);
		*parametro2 = NULL;
		entro=1;
	}
	if(!entro){
		*parametro1=NULL;
		*parametro2 = NULL;
	}
}
//------ FIN DESARME DE INSTRUCCIONES --------

//---------Porcentaje cpu--------
void agregarInstruccionCpuPorcentaje(int idCpu){

	pthread_mutex_lock(&mutexSobreListaDeInstrucciones);
	int* cantInstrucciones= list_get(listaInstruccionesCpu,idCpu);
	pthread_mutex_unlock(&mutexSobreListaDeInstrucciones);
	*cantInstrucciones= (*cantInstrucciones)+1;

}

t_list* obtenerListaPorcentajes(){
	t_list* lPorcentaje=list_create();
	pthread_mutex_lock(&mutexSobreListaDeInstrucciones);
	int tamanio = list_size(listaInstruccionesCpu);
	pthread_mutex_unlock(&mutexSobreListaDeInstrucciones);
	int i = 0;
	int* instrucciones;
	for(i;i<tamanio;i++){
		pthread_mutex_lock(&mutexSobreListaDeInstrucciones);
		instrucciones = list_get(listaInstruccionesCpu,i);
		pthread_mutex_unlock(&mutexSobreListaDeInstrucciones);
		int* porcentaje = malloc(sizeof(int));
		*porcentaje = calcularPorcentaje(*instrucciones);
		list_add(lPorcentaje,porcentaje);
	}
	return lPorcentaje;

}

int calcularPorcentaje(int cantInstrucciones){
	//---calculo la cantidad maxima de instrucciones que podria hacer. Le resto 5 para que pueda llegar al 100%
	//---el 60 es por que son las instrucciones en 1 minuto
	float total = 60.0 / retardo - 5;
	if(total<0)total=2.0;  //por si el retardo es muy grande
	int porcentaje = (cantInstrucciones / total) * 100;
	if(porcentaje>100)return 100;
	if(porcentaje<0)return 0;

	//printf("tot %.3f cantinstr %d  porcent %d \n",total,cantInstrucciones,porcentaje);

	return porcentaje;
}

void recetPorcentajes(){

	sleep(10); //por el tiempo que tardamos en manadar a ejecutar un proceso
	while(1){
	//-------Lo hace cada 1 minunto----------
	sleep(60);
	pthread_mutex_lock(&mutexSobreListaPorcentajeUltMinuto);
	listaPorcentajeUltMinuto = obtenerListaPorcentajes();
	pthread_mutex_unlock(&mutexSobreListaPorcentajeUltMinuto);

//	printf("RESET de Porcentajes \n");
	pthread_mutex_lock(&mutexSobreListaDeInstrucciones);
	int tamanio = list_size(listaInstruccionesCpu);
	pthread_mutex_unlock(&mutexSobreListaDeInstrucciones);
	int i = 0;
	int* instrucciones;
	for(i;i<tamanio;i++){
		pthread_mutex_lock(&mutexSobreListaDeInstrucciones);
		instrucciones = list_get(listaInstruccionesCpu,i);
		pthread_mutex_unlock(&mutexSobreListaDeInstrucciones);
		*instrucciones = 0;
	}
	}
	return ;
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
char recibirChar(int unSocket){
	char caracter;
	recv(unSocket, &caracter, sizeof(caracter), 0);
	return caracter;
}

char* concatenarStrings(char* palabra1, char* palabra2) {//advertencia: el string lo devuelve mallockeado
	char* palabraFinal = string_new();

	string_append(&palabraFinal,palabra1);
	string_append(&palabraFinal, palabra2);

	char* palabraAux = palabraFinal;

	return palabraAux;
}

void enviarListaString(int sock,t_list* lista){
	int tamanio = list_size(lista);
	send(sock, &tamanio, sizeof(int), 0);
	int i = 0;
	char* cadena;
	for(i;i<tamanio;i++){
		cadena = list_get(lista,i);
		mandarCadena(sock,cadena);
	}
	return;
}
void enviarListaEntero(int sock,t_list* lista){
	int tamanio = list_size(lista);
	send(sock, &tamanio, sizeof(int), 0);
	int i = 0;
	int* elemento;
	for(i;i<tamanio;i++){
		elemento = list_get(lista,i);
		send(sock, elemento, sizeof(int), 0);
//		printf("%d %\n",*elemento);
	}
	return;
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

void iniciarTodosLosMutex(){
	if(pthread_mutex_init(&mutexSobreListaDeInstrucciones,NULL)){
			printf("\n Mutex init failed \n");
		}

	if(pthread_mutex_init(&mutexFede,NULL)){
				printf("\n Mutex init failed \n");
			}

	if(pthread_mutex_init(&mutexSobreListaPorcentajeUltMinuto,NULL)){
				printf("\n Mutex init failed \n");
			}

}
//-----------FUNCIONES NO USADAS---------

void desarmarInstruccionSinPuntoYComa(char* instruccion,char** cuerpo,char** parametro1,char** parametro2){
	char** array = string_split(instruccion," ");
	*cuerpo = array[0];
	int entro=0;
	if(!strcmp(*cuerpo,"finalizar")){
		*parametro1 = NULL;
		*parametro2 = NULL;
		entro=1;
	}
	if(!strcmp(*cuerpo,"escribir")){
		*parametro1 = array[1];
		*parametro2 = array[2];
		entro=1;
	}
	if(!strcmp(*cuerpo,"iniciar") || !strcmp(*cuerpo,"entrada-salida") || !strcmp(*cuerpo,"leer")){
		*parametro1 = array[1];
		*parametro2 = NULL;
		entro=1;
	}
	if(!entro){
		*parametro1=NULL;
		*parametro2 = NULL;
	}
}

int ejecutarProcesos(int idCpu){

	 int quantum;
	 int proximaInstruccion;
	 t_list* cosasAdevolver = list_create ();   // tendriamos que hacer que la lista sea global no? medio trucho pero seria facil
	 int numeroMproc;
	 char *ruta;
	 char *copiaInstruccion;

	 int socketAdminPlanificador = 4;  //FALSO, SOLO PARA Q COMPILE
	 ruta = recibirCadena(socketAdminPlanificador);                                             //recibo la ruta
          recv(socketAdminPlanificador, &quantum, sizeof(int), 0);
	 recv(socketAdminPlanificador, &proximaInstruccion, sizeof(int), 0);
	 recv(socketAdminPlanificador, &numeroMproc,sizeof(int),0);                     // numero del mproc
			 char args[2][70];
			 FILE *fichero;
			 fichero = fopen (ruta,"r+");
			rewind (fichero);
			char instruccion[60];
			 char* stringAdevolver;

while(proximaInstruccion > 1)                                   // si es fifo, podriamos hacer que reciba proximaInstruccion = 1, y funciona

		{
fgets(instruccion, 60,fichero);                          // aca se mueve hasta la proxima instruccion, funciona, pero talvez no es la mejor forma
proximaInstruccion --;
        }


while (quantum > 0)                        // hacemos la que dijo leo de ir restando el quantum para el polimorfismo
	 {

        fgets(instruccion, 60,fichero);
		 copiaInstruccion = instruccion;

		 int nargs = extrae_argumentos(copiaInstruccion,args);              // trae los argumentos de cada instruccion en args[i], y en nargs la cantidad

	    if   (strcmp(args[0],"iniciar")== 0){

	    	if ( (iniciar (atoi(args[1]),idCpu,socketAdminMemoria,&nargs)) == 0)                                       //atoi() transforma el string de numeros a un int, si iniciar devuelve 0 es que salio bien

	    	{
	    		sprintf(stringAdevolver, "mProc %d - Iniciado",numeroMproc);
	    		list_add (cosasAdevolver, stringAdevolver);                         // agrega el string a lo que hay que devolver
	    	}
	    	else

	    	{
	    		sprintf(stringAdevolver, "mProc %d - Fallo",numeroMproc);
	    		list_add (cosasAdevolver, stringAdevolver);
	    	}
         quantum --;
	    }

       else if (strcmp(args[0],"leer")== 0)

	    	printf ("%s\n",args[0]);

	    else if (strcmp(args[0],"escribir")== 0)
	printf ("%s\n",args[0]);

	    else if (strcmp(args[0],"entrada-salida")== 0){

	    	int tiempo = atoi(args[1]);     // aca atoi() devuelta

              sprintf(stringAdevolver,"mProc %d en entrada-salida de tiempo %d",numeroMproc,tiempo);

             list_add(cosasAdevolver,stringAdevolver);
	//entrada_salida(tiempo,idCpu);
	exit(0);                     // aca no se si hace asi, con este exit, el status de status = ejecutarProceso se haria 0 ?

	    	printf ("%s\n",args[0]);
	    quantum--;}

	    else if (strcmp(args[0],"finalizar")== 0){
	//    	finalizar(idCpu);										//no esta echa el finalizar!!
	    	sprintf(stringAdevolver,"mProc %d finalizado",numeroMproc);
	    	list_add(cosasAdevolver,stringAdevolver);
	    	exit(0);                // la misma duda aca
	    }


	    else
	                    	  printf ("algo salio mal\n");

}
return 0;}

int extrae_argumentos(char *orig, char args[][100])  // esto anda joya, pero es muy poco elegante
{
char *tmp;
  char *delim = " ";
  int i=0;
  char *str = malloc(strlen(orig)+1);
  strcpy(str, orig);
  tmp=strtok(str, delim);

  if (strcmp(tmp,"finalizar;\n")==0){
	  while(i<=strlen(tmp+1))
		  {if (tmp[i] != ';'){
			  i++;}
		  else{
			  tmp[i]= '\0';
		  strcpy(args[0], tmp);}

		  }
  return 1;}

  else if (strcmp(tmp,"escribir")== 0){
		  strcpy (args[0],tmp);

		  tmp=strtok(NULL, delim);
		  strcpy(args[1],tmp);
		  tmp = strtok(NULL,"");

		  while(i<=strlen(tmp+1))
		  	  {if (tmp[i] != ';'){
		  		  i++;}
		  	  else{
		  		  tmp[i]= '\0';
		  	  strcpy (args[2], tmp);
		  	 				   }}

		  	  return 3;}
  else{
	  strcpy (args[0], tmp);
  tmp=strtok(NULL, delim);
  while(i<=strlen(tmp+1))
   	  {if (tmp[i] != ';'){
   		  i++;}
   	  else
   		  {tmp[i]= '\0';
   	  strcpy (args[1], tmp);}

   	  }
  return 2;}
free (str);

}

