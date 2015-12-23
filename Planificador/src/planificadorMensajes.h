/*
 * planificadorMensajes.h
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
#include <commons/log.h>
#include <commons/collections/node.h>
#include <commons/collections/dictionary.h>
#include <commons/bitarray.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netdb.h>
#include <stdbool.h>
#include <commons/log.h>


#define PLANIF_LOG_FILE "PLANIF.LOG"

typedef struct{
		int idProceso;
		char* ruta;
		char estado;
		/*Contador de programa
		Registros de la CPU
		Información de planificación de CPU
		Información de gestión de memoria
		Información contable
		Información de estado de E/S
		Punteros*/
		int instructionPointer;
}pcb;

typedef struct{
		int tiempoDeEspera; //solo sirve para la E/S
		int socketCpu;
		pcb* laPcbDelProceso;
}t_Package;

t_list* socketsConectados;
t_list* colaDeEspera;
int idGlobalDeComparacion;

int socketCpuPorcentaje;

int quantum;
char puerto[9];

pthread_mutex_t mutexLeo;
pthread_mutex_t mutexLeo2;
pthread_mutex_t mutexLeo3;
pthread_mutex_t mutexLeo4;

void iniciarTodosLosMutex();
void cargarArchivoDeConfiguracion();
void* manejoConsola();
void* manejoConexionesConCPUs(void* parametro);
int recibirMensajeDe(int);
void *manejoHiloCpu(void *socketCli);
pcb cargarPcb(char* path);
void correr(char* path);
int obtenerCpuDisponible();
void agregarCpuComoDisponible(int socketCliente);
int verSiSePuedeMandarAejecutarUnProceso();
void manejarProcesoEntradaSalida(int socketQueEscribe,pcb* laPcb);
pcb* obtenerPcb(int idProcesoMproc);
void suspenderProceso(t_Package* paquete);
void finalizarForzado(int idProcesoAFinalizar);

void mandarAlFinalDeLaCola(pcb* laPcb);
int tieneMismoIdProceso(pcb* pC);
int tieneEstadoReady(void* unaPcb);
void imprimirEstadoProcesos();

void mandarCadena(int socket,char* cadena);
char* recibirCadena(int socketQueEscribe);
char recibirChar(int socketQueEscribe);
t_list* recibirListaString(int sock);
void imprimirListaString(t_list* lista);

void eliminarLista(t_list* lista);

void logearListaString(t_list* lista);
void logear(char* stringAlogear);
char* cortarCod (string);
logearLista (t_list* lista);  //que hiciste aca martin?? y esta funcion donde esta??

#endif /* MENSAJES_H_ */
