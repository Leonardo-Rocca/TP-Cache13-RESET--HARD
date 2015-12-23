/*
 ============================================================================
 Name        : AdministradorSwap.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "swapMensajes.h"

int main(void) {
	puts("\033[1;31mAdministrador de Swap\033[0m\n");
	logear("\033[1;31mSwap iniciado\033[0m");
	cargarArchivoDeConfiguracion();
	crearArchivo();
	inicializarEstructura();
	inicializarArchivo();

	testCargarArchivoDeConfiguracion();
	//testSeCompactaElEspacioYSeAcomodaArchivo();

	manejoConexionesConAdminMemoria();

	return EXIT_SUCCESS;
}

void mandarCadena(int socket, char* cadena) {
	uint32_t long_cadena = strlen(cadena)+1;
	send(socket, &long_cadena, sizeof(long_cadena), 0);
	send(socket, cadena, long_cadena, 0);
}

void dormir(float numero){
	if(numero >= 1){
		sleep(numero);
	} else {
		usleep(numero*1000000);
	}
}

char* recibirCadena(int socketQueEscribe) {
	char* cadena;
	uint32_t long_cadena;
	recv(socketQueEscribe, &long_cadena, sizeof(long_cadena), 0);
	cadena = malloc(long_cadena);
	recv(socketQueEscribe, cadena, long_cadena, 0);
	return cadena;
}

void cargarArchivoDeConfiguracion(){
	FILE* punteroAlArchivoDeConfiguracion;
	char readParameterName[64];
	char readParameterValue[64];
	int index;

	int flagPortParamOK = 0;
	int flagNameParamOK = 0;
	int flagPageNumParamOK = 0;
	int flagPageSizeParamOK = 0;
	int flagDelayCompactOK = 0;
	int flagDelaySwapOK = 0;
	int error = 0;
	int endOfFile = 0;

	punteroAlArchivoDeConfiguracion = fopen("SWAP.CFG", "r""t");
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
	            if (!strcmp(readParameterName, "PUERTO_ESCUCHA")) {strcpy(puertoEscucha, readParameterValue); flagPortParamOK = 1;}
        	    if (!strcmp(readParameterName, "NOMBRE_SWAP")) {strcpy(nombreSwap, readParameterValue); flagNameParamOK = 1;}
	            if (!strcmp(readParameterName, "CANTIDAD_PAGINAS")) {cantidadPaginas = atoi(readParameterValue); flagPageNumParamOK = 1;}
        	    if (!strcmp(readParameterName, "TAMANIO_PAGINA")) {tamanioPagina = atoi(readParameterValue); flagPageSizeParamOK = 1;}
	            if (!strcmp(readParameterName, "RETARDO_COMPACTACION")) {retardoCompactacion = atof(readParameterValue); flagDelayCompactOK = 1;}
	            if (!strcmp(readParameterName, "RETARDO_SWAP")) {retardoSwap = atof(readParameterValue); flagDelaySwapOK = 1;}
    
		}while(endOfFile != 1);
    
		fclose(punteroAlArchivoDeConfiguracion);
	     }
	if(!error && flagPortParamOK && flagNameParamOK && flagPageNumParamOK && flagPageSizeParamOK && flagDelayCompactOK && flagDelaySwapOK)
        printf("Archivo de configuraciones del Swap cargado correctamente.\n"); else perror("Error al cargar el archivo de configuraciones del Swap.\n");

	//printf("%s %s %d %d %d %d\n",puertoEscucha, nombreSwap, cantidadPaginas,tamanioPagina, retardoCompactacion,retardoSwap);

}

void crearArchivo(){
	char* datos = malloc(100);
	sprintf(datos, "dd if=/dev/zero of=%s bs=%d count=%d",nombreSwap,tamanioPagina,cantidadPaginas);
	system(datos);
	free(datos);
	inicializarArchivo();
}

paginasProceso* crearElementoParaLista(int pid, int comienzo, int cantidadPaginas){
	paginasProceso* paginas = malloc(sizeof(paginasProceso));
	paginas->pid = pid;
	paginas->comienzo = comienzo;
	paginas->cantidadPaginas = cantidadPaginas;
	return paginas;
}

void crearYAgregarElementoEnPosicion(int pid, int comienzo, int cantidadPaginas, int posicion){
	paginasProceso* paginas = crearElementoParaLista(pid, comienzo, cantidadPaginas);
	list_add_in_index(listaSwap,posicion,paginas);
}

void inicializarEstructura(){
	listaSwap = list_create();
	crearYAgregarElementoEnPosicion(-1,0,cantidadPaginas,0);
	listaCantidadLYE = list_create();
}

void imprimirSwapData(){
	int tamanioLista = list_size(listaSwap);
	int i = 0;
	printf("SwapData\n");
	while(tamanioLista > i){
		paginasProceso* paginas = list_get(listaSwap,i);
		int fin = paginas->comienzo + paginas->cantidadPaginas - 1;
		if (paginas->pid == -1){
			printf("PID: Libre Comienzo: %d - %d CantidadPaginas: %d \n",paginas->comienzo, fin, paginas->cantidadPaginas);
		} else{
			printf("PID: %d     Comienzo: %d - %d CantidadPaginas: %d \n", paginas->pid,paginas->comienzo, fin, paginas->cantidadPaginas);
		}
		i++;
	}
	printf("\n");
}

char* devolverSwapData(){
	int tamanioLista = list_size(listaSwap);
	int i = 0;
	char* texto = malloc(1000);
	sprintf(texto,"SwapData:");
	while(tamanioLista > i){
		char* texto2 = malloc(100);
		paginasProceso* paginas = list_get(listaSwap,i);
		int fin = paginas->comienzo + paginas->cantidadPaginas - 1;
		if (paginas->pid == -1){
			sprintf(texto2, "\nPID: Libre Comienzo: %d - %d CantidadPaginas: %d",paginas->comienzo, fin, paginas->cantidadPaginas);
		} else{
			sprintf(texto2, "\nPID: %d       Comienzo: %d - %d CantidadPaginas: %d", paginas->pid,paginas->comienzo, fin, paginas->cantidadPaginas);
		}
		strcat(texto,texto2);
		free(texto2);
		i++;
	}
	//printf("%s\n",texto);
	return texto;
}

int agregarProceso(proceso proceso){ //Agrega el proceso si entra en un hueco, compacta si hay espacio (1) o avisa que no entra (0).
	int espacio = espacioSwap();
	if(espacio < proceso.cantidadPaginas){
		//Luego hay que comentarlo...
		printf("\033[1;33mNo entra proceso %d\033[0m \n",proceso.pid);
		return 0;
	}else{
		if(espacio == proceso.cantidadPaginas){
			int hueco = procesoEntraEnUnHueco(proceso);
			if(hueco != -1){
				agregarYBorrarLibre(proceso, hueco);
			} else {
				logCompactacionIniciada();
				compactar();
				logCompactacionFinalizada();
				//printf("El Swap se compacto\n");
				hueco = procesoEntraEnUnHueco(proceso);
				agregarYBorrarLibre(proceso, hueco);
			}
		} else {
		int hueco = procesoEntraEnUnHueco(proceso);
		if(hueco != -1){
			agregarProcesoEnHueco(proceso, hueco);
		} else {
			logCompactacionIniciada();
			compactar();
			logCompactacionFinalizada();
			agregarProceso(proceso);
			}
		}
	}
	return 1;
}

paginasProceso obtenerProceso(int pid){
	int tamanioLista = list_size(listaSwap);
	int i = 0;
	while(tamanioLista > i){
		paginasProceso* paginas = list_get(listaSwap,i);
		if(paginas->pid == pid){
			return *paginas;
		}
		i++;
	}
}

int procesoEntraEnUnHueco(proceso proceso){ //Devuelve N°hueco si el proceso entra en un solo hueco y -1 si no.
	int tamanioLista = list_size(listaSwap);
	int i = 0;
	while(tamanioLista > i){
		paginasProceso* paginas = list_get(listaSwap,i);
		if(paginas->pid == -1){
			if(proceso.cantidadPaginas <= paginas->cantidadPaginas){
				return i;
			}
		}
		i++;
	}
	return -1;
}

int espacioSwap(){
	int espacio = 0;
	int tamanioLista = list_size(listaSwap);
	int i = 0;
	while(tamanioLista > i){
		paginasProceso* paginas = list_get(listaSwap,i);
		if(paginas->pid == -1){
			espacio += paginas->cantidadPaginas;
		}
	i++;
	}
	return espacio;
}

proceso crearProceso(int pid, int cantidadPaginas){
	proceso proceso;
	proceso.pid = pid;
	proceso.cantidadPaginas = cantidadPaginas;
	return proceso;
}

void compactar(){
	dormir(retardoCompactacion);
	int tamanioLista = list_size(listaSwap);
	int i = 0;
	while(tamanioLista-1 > i){
		paginasProceso* paginas1 = list_get(listaSwap,i);
		if(paginas1->pid == -1){ //Si hay un espacio libre el proximo elemento en la lista tiene que ser un proceso (ya que se unen).
			moverProceso(i);
			//imprimirSwapData();
			tamanioLista = list_size(listaSwap);
		}
	i++;
	}
	limpiarEspaciosEnSwap();
}

void agregarProcesoEnHueco(proceso proceso, int hueco){
	paginasProceso* paginasLibres = list_get(listaSwap,hueco);
	paginasLibres->cantidadPaginas = paginasLibres->cantidadPaginas - proceso.cantidadPaginas;
	int comienzoDelHueco = paginasLibres->comienzo;
	paginasLibres->comienzo = paginasLibres->comienzo + proceso.cantidadPaginas;
	crearYAgregarElementoEnPosicion(proceso.pid,comienzoDelHueco,proceso.cantidadPaginas,hueco);
}

void agregarYBorrarLibre(proceso proceso,int hueco){
	paginasProceso* paginasLibres = list_remove(listaSwap,hueco);
	paginasLibres->cantidadPaginas = paginasLibres->cantidadPaginas - proceso.cantidadPaginas;
	int comienzoDelHueco = paginasLibres->comienzo;
	paginasLibres->comienzo = paginasLibres->comienzo + proceso.cantidadPaginas;
	crearYAgregarElementoEnPosicion(proceso.pid,comienzoDelHueco,proceso.cantidadPaginas,hueco);
	free(paginasLibres);
}

int eliminarProceso(int pid){
	if(procesoSeEncuentraEnSwap(pid)){
		int tamanioLista = list_size(listaSwap);
		int i = 0;
		while(tamanioLista > i){
			paginasProceso* paginas = list_get(listaSwap,i);
			if(paginas->pid == pid){
				liberarEspacio(i);
				}
			i++;
			}
		acomodarSwap();
		return 1;
	} else {
		printf("Proceso %d no se encuentra en swap \n",pid);
		return 0;
	}
}

int procesoSeEncuentraEnSwap(int pid){ //Devuelve 1 si el proceso se encuentra en Swap y 0 de lo contrario.
	int tamanioLista = list_size(listaSwap);
	int i = 0;
	while(tamanioLista > i){
		paginasProceso* paginas = list_get(listaSwap,i);
		if(paginas->pid == pid){
			return 1;
			}
		i++;
		}
	return 0;
}

void liberarEspacio(int posicion){
	paginasProceso* paginas = list_get(listaSwap,posicion);
	paginas->pid = -1;
}

void acomodarSwap(){ //Si hay dos espacios libres seguidos los une.
	int tamanioLista = list_size(listaSwap);
	int i = 0;
	while(tamanioLista-1 > i){
		paginasProceso* paginas1 = list_get(listaSwap,i);
		if(paginas1->pid == -1){
			paginasProceso* paginas2 = list_get(listaSwap,i+1);
			if(paginas2->pid == -1){
				unirEspaciosLibres(i,i+1);
				tamanioLista--; //Luego de unir espacios libres el tamanio de la lista cambia.
				i--; //Por si llega a haber 3 espacios seguidos o mas tiene que analizar el mismo 2 veces.
				}
			}
		i++;
	}
}

void unirEspaciosLibres(int hueco1, int hueco2){
	paginasProceso* paginas1 = list_get(listaSwap,hueco1);
	paginasProceso* paginas2 = list_get(listaSwap,hueco2);
	paginas1->cantidadPaginas = paginas1->cantidadPaginas + paginas2->cantidadPaginas;
	list_remove(listaSwap,hueco2);
	free(paginas2);
}

void moverProceso(int posicion){
	paginasProceso* espacio = list_remove(listaSwap,posicion);
	paginasProceso* paginasProceso = list_remove(listaSwap,posicion);
	moverPaginas(espacio,paginasProceso);
	paginasProceso->comienzo = espacio->comienzo;
	espacio->comienzo = paginasProceso->comienzo + paginasProceso->cantidadPaginas;
	list_add_in_index(listaSwap,posicion,espacio);
	list_add_in_index(listaSwap,posicion,paginasProceso);
	acomodarSwap();
}

void moverPaginas(paginasProceso* espacio, paginasProceso* paginasProceso){
	FILE* archivo;
	archivo = fopen(nombreSwap,"r+");
	fseek(archivo,tamanioPagina*paginasProceso->comienzo,SEEK_SET);
	char* texto = malloc(tamanioPagina*paginasProceso->cantidadPaginas);
	fread(texto,tamanioPagina,paginasProceso->cantidadPaginas,archivo);
	texto[tamanioPagina*paginasProceso->cantidadPaginas] = NULL;
	fseek(archivo,tamanioPagina*espacio->comienzo,SEEK_SET);
	fprintf(archivo,"%s",texto);
	fclose(archivo);
	free(texto);
}

void limpiarEspaciosEnSwap(){
	FILE* archivo;
	archivo = fopen(nombreSwap,"r+");
	int tamanioLista = list_size(listaSwap);
	int i = 0;
	while(tamanioLista > i){
		paginasProceso* paginas = list_get(listaSwap,i);
		if(paginas->pid == -1){
			fseek(archivo,tamanioPagina*paginas->comienzo,SEEK_SET);
			char* texto = malloc(paginas->cantidadPaginas*tamanioPagina+1);
			memset(texto,' ',paginas->cantidadPaginas*tamanioPagina);
			//char* texto1 = string_substring_until(texto,tamanioPagina);
			texto[paginas->cantidadPaginas*tamanioPagina] = NULL;
			fprintf(archivo,"%s",texto);
			free(texto);
		}
		i++;
	}
	fclose(archivo);
}

int recibirMensajeDe(int socketQueEscribe){

	char idOperacionDeInterrupcion = recibirChar(socketQueEscribe);

	//printf("idOperacion: %c \n",idOperacionDeInterrupcion);

	switch(idOperacionDeInterrupcion){
			case 'F':
				finalizar();
				return 1;
			case 'I':
				iniciar();
				//imprimirSwapData();
				return 1;
			case 'L':
				leer();
				return 1;
			case 'E':
				escribir();
				return 1;
			break;
			}
	return 0;
}

char recibirChar(int unSocket){
	char caracter;
	recv(unSocket, &caracter, sizeof(caracter), 0);
	return caracter;
}

void finalizar(){
	int pid;
	recv(socketAdministradorMemoria, &pid, sizeof(int), 0);
	if (procesoSeEncuentraEnSwap(pid)){
		dormir(retardoSwap);
		logFinalizar(obtenerProceso(pid));
		eliminarProceso(pid);
		limpiarEspaciosEnSwap();
		logSwapData();
		char exito = 'E';
		send(socketAdministradorMemoria,&exito, sizeof(char), 0);
		indicarCantidadPaginasLeidasYEscritas(pid);
		logPaginasLeidasYEscritas(devolverCantidadPaginasLeidasYEscritas(pid));
	} else{
		printf("El proceso %d a finalizar no se encuentra en Swap\n",pid);
		char fracaso = 'F';
		send(socketAdministradorMemoria,&fracaso, sizeof(char), 0);
	}
}

void indicarCantidadPaginasLeidasYEscritas(int pid){
	int tamanioLista = list_size(listaCantidadLYE);
	int i = 0;
	printf("\033[1;37mProceso: %d\033[0m\n",pid);
	while(tamanioLista > i){
		cantidadLYE* cantidadLYE = list_get(listaCantidadLYE,i);
		if(cantidadLYE->pid == pid){
			printf("\033[1;37mPaginas leidas: %d Paginas escritas: %d\033[0m\n",cantidadLYE->cantidadLeidas,cantidadLYE->cantidadEscritas);
		}
		i++;
	}
}

char* devolverCantidadPaginasLeidasYEscritas(int pid){
	int tamanioLista = list_size(listaCantidadLYE);
	int i = 0;
	char* texto = malloc(100);
	sprintf(texto,"Proceso: %d - ",pid);
	while(tamanioLista > i){
		char* texto2 = malloc (100);
		cantidadLYE* cantidadLYE = list_get(listaCantidadLYE,i);
		if(cantidadLYE->pid == pid){
			sprintf(texto2,"Paginas leidas: %d - Paginas escritas: %d",cantidadLYE->cantidadLeidas,cantidadLYE->cantidadEscritas);
			strcat(texto,texto2);
			return texto;
		}
		i++;
	}
}

void iniciar(){
	int pid;
	int cantidadPaginas;
	recv(socketAdministradorMemoria, &pid, sizeof(int), 0);
	recv(socketAdministradorMemoria, &cantidadPaginas, sizeof(int), 0);
	proceso proceso = crearProceso(pid,cantidadPaginas);
	if (agregarProceso(proceso)){
		dormir(retardoSwap);
		char exito = 'E';
		send(socketAdministradorMemoria,&exito, sizeof(char), 0);
		logIniciar(obtenerProceso(pid));
		logSwapData();
	} else{
		char fracaso = 'F';
		send(socketAdministradorMemoria,&fracaso, sizeof(char), 0);
		logRechazar(proceso);
	}
	devolverSwapData();
	agregarNodoLYE(pid);
}

void agregarNodoLYE(int pid){
	cantidadLYE* cantidadLYE = crearElementoParaListaLYE(pid,0,0);
	list_add(listaCantidadLYE,cantidadLYE);
}

void leer(){
	int pid;
	int numeroPagina;
	recv(socketAdministradorMemoria, &pid, sizeof(int), 0);
	recv(socketAdministradorMemoria, &numeroPagina, sizeof(int), 0);
	if(procesoSeEncuentraEnSwap(pid)){
		paginasProceso paginasProceso = obtenerProceso(pid);
		if(numeroPagina >= paginasProceso.cantidadPaginas){
			printf("La pagina %d no se encuentra en el proceso %d y no puede ser leida\n",numeroPagina,pid);
			char* fallo = malloc(2);
			strcpy(fallo,"F");
			mandarCadena(socketAdministradorMemoria,fallo);
		} else {
			dormir(retardoSwap);
			int paginaALeer = paginasProceso.comienzo + numeroPagina;
			char* texto = leerPagina(paginaALeer);
			//printf("%s\n",texto);
			mandarCadena(socketAdministradorMemoria,texto);
			logLectura(paginasProceso,texto,paginaALeer,numeroPagina);
			free(texto);
			sumarPaginaLeida(pid);
		}
	} else {
		printf("Proceso %d no se encuentra en Swap y no puede ser leido\n",pid);
		char* fallo = malloc(2);
		strcpy(fallo,"F");
		mandarCadena(socketAdministradorMemoria,fallo);
	}
}

void sumarPaginaLeida(int pid){
	int tamanioLista = list_size(listaCantidadLYE);
	int i = 0;
	while(tamanioLista > i){
		cantidadLYE* cantidadLYE = list_get(listaCantidadLYE,i);
		if(cantidadLYE->pid == pid){
			cantidadLYE->cantidadLeidas++;
		}
		i++;
	}
}

void escribir(){
	int pid;
	int numeroPagina;
	recv(socketAdministradorMemoria, &pid, sizeof(int), 0);
	recv(socketAdministradorMemoria, &numeroPagina, sizeof(int), 0);
	//imprimirSwapData();
	//printf("%d%d\n",pid,numeroPagina);
	char* texto = recibirCadena(socketAdministradorMemoria);
	if(procesoSeEncuentraEnSwap(pid)){
		paginasProceso paginasProceso = obtenerProceso(pid);
		if(numeroPagina >= paginasProceso.cantidadPaginas){
			printf("La pagina %d no se encuentra en el proceso %d y no puede ser escrita\n",numeroPagina,pid);
			char fallo = 'F';
			send(socketAdministradorMemoria,&fallo, sizeof(char), 0);
		} else {
			dormir(retardoSwap);
			int paginaAEscribir = paginasProceso.comienzo + numeroPagina;
			escribirPagina(paginaAEscribir,texto);
			char exito = 'E';
			send(socketAdministradorMemoria,&exito, sizeof(char), 0);
			sumarPaginaEscrita(pid);
			logEscritura(paginasProceso,texto,paginaAEscribir,numeroPagina);
		}
	} else {
		printf("Proceso %d no se encuentra en Swap y no puede ser escrito\n",pid);
		char fallo = 'F';
		send(socketAdministradorMemoria,&fallo, sizeof(char), 0);
	}
}

void sumarPaginaEscrita(int pid){
	int tamanioLista = list_size(listaCantidadLYE);
	int i = 0;
	while(tamanioLista > i){
		cantidadLYE* cantidadLYE = list_get(listaCantidadLYE,i);
		if(cantidadLYE->pid == pid){
			cantidadLYE->cantidadEscritas++;
		}
		i++;
	}
}

int escribirPagina(int pagina, char* texto){
	if(strlen(texto)>tamanioPagina){
		printf("El texto es mas grande que el tamaño de pagina entonces se corta\n");
	}
	FILE* archivo;
	archivo = fopen(nombreSwap,"r+");
	//printf("%s\n",texto);
	//printf("%d\n",strlen(texto));
	char* texto2 = malloc(tamanioPagina);
	memset(texto2,' ',tamanioPagina);
	char* texto0 = string_new();
	string_append(&texto0,texto);
	string_append(&texto0,texto2);
	char* texto1 = string_substring_until(texto0,tamanioPagina);
	fseek(archivo,tamanioPagina*pagina,SEEK_SET);
	fprintf(archivo,"%s",texto1);
	fclose(archivo);
	free(texto2);
	//free(texto0);
	return 0;
}

char* leerPagina(int pagina){
	FILE* archivo;
	//printf("%d\n",pagina);
	archivo = fopen(nombreSwap,"r+");
	fseek(archivo,tamanioPagina*pagina,SEEK_SET);
	char* texto = malloc(tamanioPagina+1);
	texto[tamanioPagina] = NULL;
	fread(texto,tamanioPagina,1,archivo);
	fclose(archivo);
	//printf("%s\n",texto);
	return texto;
}

void inicializarArchivo(){
	FILE* archivo;
	archivo = fopen(nombreSwap,"r+");
	fseek(archivo,0,SEEK_SET);
	char* texto = malloc(cantidadPaginas*tamanioPagina);
	memset(texto,' ',cantidadPaginas*tamanioPagina);
	fprintf(archivo,"%s",texto);
	fclose(archivo);
	free(texto);
}

cantidadLYE* crearElementoParaListaLYE(int pid, int cantidadEscritas, int cantidadLeidas){
	cantidadLYE* cantidadLYE = malloc(sizeof(cantidadLYE));
	cantidadLYE->pid = pid;
	cantidadLYE->cantidadEscritas = cantidadEscritas;
	cantidadLYE->cantidadLeidas = cantidadLeidas;
	return cantidadLYE;
}
//----------------------------------------- L O G S -------------------------------------------------------

void logear(char *stringAlogear){
    t_log* log = log_create("LogSwap", "Administrador de Swap", false, LOG_LEVEL_INFO);
	log_info(log, stringAlogear, "INFO");
	log_destroy(log);
}

void logIniciar(paginasProceso paginas){
	char stringAlogear[200];
	sprintf(stringAlogear,"\033[1;32mmProc Asignado - PID: %d - Pagina inicial: %d - Paginas: %d - Tamaño: %d\033[0m",paginas.pid, paginas.comienzo,paginas.cantidadPaginas,paginas.cantidadPaginas*tamanioPagina);
	logear(stringAlogear);
}

void logFinalizar(paginasProceso paginas){
	char stringAlogear[200];
	sprintf(stringAlogear,"\033[1;33mProc Liberado - PID: %d - Pagina inicial: %d - Paginas: %d - Tamaño: %d\033[0m",paginas.pid, paginas.comienzo,paginas.cantidadPaginas,paginas.cantidadPaginas*tamanioPagina);
	logear(stringAlogear);
}

void logSwapData(){
	char stringAlogear[3000];
	sprintf(stringAlogear,"\033[1;37m\n%s\033[0m",devolverSwapData());
	logear(stringAlogear);
}

void logRechazar(proceso proceso){
	char stringAlogear[200];
	sprintf(stringAlogear,"\033[1;30mmProc Rechazado - PID: %d - Falta de espacio\033[0m",proceso.pid);
	logear(stringAlogear);
}

void logCompactacionIniciada(){
	char stringAlogear[100];
	sprintf(stringAlogear,"\033[1;34mCompactacion Iniciada\033[0m");
	logear(stringAlogear);
}


void logCompactacionFinalizada(){
	char stringAlogear[100];
	sprintf(stringAlogear,"\033[1;34mCompactacion Finalizada\033[0m");
	logear(stringAlogear);
}

void logLectura(paginasProceso paginas, char* texto, int paginaSwap, int paginaProceso){
	char stringAlogear[200+strlen(texto)];
	sprintf(stringAlogear,"\033[1;36mLectura - PID: %d - Byte inicial: %d - PaginaSwap: %d - PaginaProceso: %d - Contenido: %s\033[0m",paginas.pid,paginaSwap*tamanioPagina,paginaSwap,paginaProceso,texto);
	logear (stringAlogear);
}

void logEscritura(paginasProceso paginas, char* texto, int paginaSwap, int paginaProceso){
	char stringAlogear[200+strlen(texto)];
	sprintf(stringAlogear,"\033[1;36mEscritura - PID: %d - Byte inicial: %d - PaginaSwap: %d - PaginaProceso: %d - Contenido: %s\033[0m",paginas.pid,paginaSwap*tamanioPagina,paginaSwap,paginaProceso,texto);
	logear(stringAlogear);
}

void logPaginasLeidasYEscritas(char* texto){
	char stringAlogear[200];
	sprintf(stringAlogear,"\033[1;35m%s\033[0m",texto);
	logear(stringAlogear);
}

//----------------------------------------- T E S T S -----------------------------------------------------

void testAgregarProcesoLlenandoSwap() {
	imprimirSwapData();
	proceso proceso1 = crearProceso(1, 512);
	printf("Espacio: %d\n\n", espacioSwap());
	agregarProceso(proceso1);
	imprimirSwapData();
}

void testAgregarProcesoLlenandoSwap2() {
	imprimirSwapData();
	proceso proceso1 = crearProceso(1, 510);
	agregarProceso(proceso1);
	proceso proceso2 = crearProceso(2, 2);
	agregarProceso(proceso2);
	imprimirSwapData();
}

void testSeUnenEspaciosLibres() {
	imprimirSwapData();
	proceso proceso1 = crearProceso(1, 10);
	agregarProceso(proceso1);
	proceso proceso2 = crearProceso(1, 20);
	agregarProceso(proceso2);
	proceso proceso3 = crearProceso(1, 30);
	agregarProceso(proceso3);
	proceso proceso4 = crearProceso(2, 110);
	agregarProceso(proceso4);
	imprimirSwapData();
	eliminarProceso(1);
	imprimirSwapData();
	printf("Espacio: %d\n\n", espacioSwap());
	eliminarProceso(2);
	imprimirSwapData();
	printf("Espacio: %d\n\n", espacioSwap());
}

void testSeCompactaElEspacio() {
	imprimirSwapData();
	proceso proceso1 = crearProceso(1, 100);
	agregarProceso(proceso1);
	proceso proceso2 = crearProceso(2, 200);
	agregarProceso(proceso2);
	proceso proceso3 = crearProceso(3, 100);
	agregarProceso(proceso3);
	eliminarProceso(2);
	imprimirSwapData();
	proceso proceso4 = crearProceso(4, 300);
	agregarProceso(proceso4);
	printf("Espacio: %d\n\n", espacioSwap());
	imprimirSwapData();
}

void testSeCompactaElEspacio2() {
	imprimirSwapData();
	proceso proceso1 = crearProceso(1, 100);
	agregarProceso(proceso1);
	proceso proceso2 = crearProceso(2, 50);
	agregarProceso(proceso2);
	proceso proceso3 = crearProceso(3, 50);
	agregarProceso(proceso3);
	proceso proceso4 = crearProceso(4, 50);
	agregarProceso(proceso4);
	proceso proceso5 = crearProceso(5, 50);
	agregarProceso(proceso5);
	proceso proceso6 = crearProceso(6, 50);
	agregarProceso(proceso6);
	imprimirSwapData();
	eliminarProceso(1);
	eliminarProceso(3);
	eliminarProceso(5);
	imprimirSwapData();
	proceso proceso7 = crearProceso(7, 300);
	agregarProceso(proceso7);
	imprimirSwapData();
}

void testSeCompactaElEspacioYSeAcomodaArchivo() {
	imprimirSwapData();
	proceso proceso1 = crearProceso(1,5);
	agregarProceso(proceso1);
	proceso proceso2 = crearProceso(2,10);
	agregarProceso(proceso2);
	eliminarProceso(1);
	escribirPagina(5,"hola");
	escribirPagina(6,"123");
	imprimirSwapData();
	compactar();
	imprimirSwapData();
	char* texto = leerPagina(0);
	char* texto2 = leerPagina(1);
	printf("%s\n", texto);
	printf("%s\n", texto2);
}

void testEscribirEnPaginas() {
	escribirPagina(2,"1234567890");
	escribirPagina(3,"dadadadada");
	escribirPagina(0,"12345678910111213");
	escribirPagina(1,"dsnaudsabifbasifasifhasufausbfasupfdhusafphsdauensayfgesyafo");
	escribirPagina(0,"hola                                                                                                                                                                                                                                                           F");
	escribirPagina(1,"como andas");
}

void testEscribirEnPaginas2() {
	escribirPagina(2,"32101");
	escribirPagina(3,"dad");
	escribirPagina(0,"1234");
	escribirPagina(1,"dsn");
	escribirPagina(0,"hola");
	escribirPagina(1,"com");
}


void testSobreescribirEnPagina() {
	escribirPagina(0,"12345678910111213");
	escribirPagina(0,"hola");
	char* texto = leerPagina(0);
	printf("%s\n", texto);
}

void testLeerPagina() {
	testEscribirEnPaginas2();
	char* texto = leerPagina(0);
	char* texto2 = leerPagina(1);
	char* texto3 = leerPagina(2);
	char* texto4 = leerPagina(3);
	printf("%s\n", texto);
	printf("%s\n", texto2);
	printf("%s\n", texto3);
	printf("%s\n", texto4);
}

void testCargarArchivoDeConfiguracion() {
	printf("Puerto de escucha: %s\n",puertoEscucha);
	printf("Nombre del archivo swap: %s\n",nombreSwap);
	printf("Cantidad de paginas: %d\n",cantidadPaginas);
	printf("Tamanio de pagina: %d\n",tamanioPagina);
	printf("Retardo de compactacion: %f\n",retardoCompactacion);
	printf("Retardo de swap: %f\n\n",retardoSwap);
}


