/*
 * AdminMemoriaMensajes.h
 *
 *  Created on: 1/9/2015
 *      Author: utnso
 */

#ifndef MENSAJES_H_
#define MENSAJES_H_

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>
#include <ctype.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h>
#include <commons/log.h>
#include <commons/collections/dictionary.h>
#include <commons/bitarray.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netdb.h>
#include <stdbool.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>

sem_t semaforo1;
sem_t semaforo2;
pthread_mutex_t mutexRAM;
pthread_mutex_t mutexTLB;
pthread_mutex_t mutexFede;
pthread_mutex_t logSIGPOLL;
pthread_mutex_t swap;

char puerto[9];
char ipSwap[20];
char puertoSwap[9];
int maximoMarcosPorProceso;
int cantidadMarcos;
int tamanioMarco;
int entradasTLB;
int tlbHabilitada; //cuidado, llega un string SI/NO
float retardoMemoria;
char algoritmoReemplazo[30];
int indiceParaAlgoritmoClockModificado;



int swapSocket;
t_list* socketsConectados;

//Variables globales y estructuras necesarias
t_list* procesos;
t_list* TLB;
t_list* frames;


//------ESTRUCTURAS------------
struct tlb{
	int nroProceso;
	int framesAsignados;
	int nroPagina;
	int bitPresencia;
	int bitUso;
	int bitModificado;
	int bitAccedido;
	int nroFrame;
};

struct datosDelProceso{
	int nroProceso;
	int framesAsignados;
	int punteroClock;
	t_list* tabla;  // LISTA DE tablaP
};

struct tablaPaginas{
	int nroPagina;
	int bitPresencia;
	int bitUso;
	int bitModificado;
	int bitAccedido; //sirve si usamos LRU
	int nroFrame;
};

//--Funciones generales--

void* manejoConexionesConCPUs(void* parametro);
int recibirMensajeDe(int);
void *manejoHiloCpu(void *socketCli);
int manejoConexionesConSwap(char* puerto);

int manejoSenial1();
int manejoSenial2();
void manejador (int sig);

void cargarArchivoDeConfiguracion();
void avisarSwapInstruccionYProceso(char instruccion, int pid);
char avisarSwapIniciar(int pid, int cantidadPaginas);
char avisarSwapFinalizar(int pid);
char* avisarSwapLeer(int pid, int numeroPagina);
char avisarSwapEscribir(int pid, int numeroPagina, char* texto);
void iniciar(int socketCpu,int pid);
void leer(int socketCpu,int pid);
void escribir(int socketCpu,int pid);
void finalizar(int socketCpu,int pid);

void mandarCadena(int socket,char* cadena);
char* recibirCadena(int socketQueEscribe);
char recibirChar(int unSocket);

void logear(char* stringAlogear);
void logearReemplazoPagina(int pid,int nroPagina,int frame,char* exPagina,char* pagina);
void logearTLBhit(int pid, int tipoDeAccesoSolicitado, int nroPagina, int frame);
void logearSIGPOLL();

//Mensajes para administrar la memoria
void imprimirEstadoTLB();
void imprimirEstadoRAM();
void imprimirEstadoPaginasDeUnProceso(int pid);
void inicializarAdministrador();
void inicializarTLB();
void inicializarRAM();
void inicializarEstructuras(int pid,int nroPaginas);
char* leerPagina(int pid,int nroPagina);
char* pedirASWAP(int pid,int nroPagina);
int obtenerFrameDisponibleEnRAM(int pid,int nroPagina,char* pagina,int*modif);
void escribirPagina(int pid,int nroPagina,char* pagina);
void finalizarEstructuras(int pid);
void eliminarDeTLB(int pid, int nroPagina,int posicionTLB);

void eliminarFramesPorSenial();
void actualizarEstructurasPorSenial(int frame);


int obtenerIndiceTablaDeProceso(int pid);
struct datosDelProceso* obtenerTablaDeProceso(int pid);
t_list* obtenerTablaDePaginas(int pid);
struct tablaPaginas* obtenerPagina(int pid,int nroPagina);
int obtenerIndiceEnTLB(int pid,int nroPagina);
int revisarTLB(int pid,int nroPagina);
int obtenerFrame(int pid,int nroPagina, int tipoDeAccesoSolicitado);
int revisarRAM(int pid,int nroPagina, int tipoDeAccesoSolicitado);
int hayFramesDisponibles();
void actualizarEstructuras(int pid, int nroPagina,int nroFrame,int modif,int lecturaEscritura,int aux,int aux2);
void determinarAcceso(t_list* paginas,int nroPagina);
void actualizarTLB(int pid, int nroPagina,int nroFrame,int escrituraLectura);
void determinarAccesoAPaginasEnTLB(int pid,int nroPagina);
void actualizo(int pid, int nroPagina,int nroFrame,int escrituraLectura);
int obtenerPaginaAReemplazarDeLaTLB(int pid, int nroPagina, int escrituraLectura);
void reemplazo(int pid,int nroPagina,int nroFrame,int escrituraLectura);
int obtenerFrameAReemplazarEnRamPara(int pid);
void actualizarFramesAsignadosPorProcesoEnTLB(int cantidad, int pid);
void analizarSiEsModificada(int pid,struct tablaPaginas* pagina);
void analizarSiFueModificadaEnTLB(struct tlb* tlb);
int obtenerMaxTLB();
void resetearValoresPagina(struct tablaPaginas* pagina);
void resetearValoresEnTLB(int pid,struct tablaPaginas* pagina);
void reorganizarPaginasParaFifo(t_list* paginas,int nroPagina);
void reemplazarFrameEnRamFifo(t_list* paginas,int*indice,int *nroPagina,int pid);
void reemplazarFrameEnRamLRU(t_list* paginas,int*indice,int *nroPagina,int pid);
void reemplazarFrameEnRamClockModif(t_list* paginas,int*indice,int *nroPagina,int pid);
void eliminarSinTLB(t_list* paginas,int pid);
void eliminarConTLB(t_list* paginas,int pid);
int verSiEstaEnLaTablaDePaginas(int nroPagina,t_list* paginas);
int reemplazarEnTLBSegunFifo(int pid,int nroPagina,int escrituraLectura);
int reemplazarEnTLBSegunLRU(int pid, int nroPagina,int escrituraLectura);
int reemplazarEnTLBSegunClockModif(int pid,int nroPagina,int escrituraLectura);
void dormir(float numero);


#endif /* MENSAJES_H_ */

