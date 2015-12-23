/*
 * cpuMensajes.h
 *
 *  Created on: 1/9/2015
 *      Author: utnso
 */
#ifndef MENSAJES_H_
#define MENSAJES_H_

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
#include <time.h>
#include <commons/txt.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netdb.h>
#include <stdbool.h>
#include <commons/log.h>

#define CPU_LOG_FILE "CPU.LOG"

typedef struct{
		int idCpu;
		int socketPlanificador;
		int socketAdminMemoria;
}t_Package;

typedef struct{
	char instruccionQueCausoLaInterrupccion;
	int cantidadDeInstrucciones;
	int tiempo; //si es por E/S
	t_list* resultadosDeInstrucciones; //por rafagas (char*)
}informacionSobreLasInstrucciones;

typedef struct{
		int idProceso;
		char* ruta;
		int quantum;
		int instructionPointer;
		//FILE* archivo;
}pcb;


t_list* listaInstruccionesCpu;
char idProceso;
int cantidadDeHilosCpu;

char* ipPlanificador;
char* puertoPlanificador;
char* ipAdminMemoria;
char* puertoAdminMemoria;
t_list* cosasAdevolver;
int socketAdminMemoria;
float retardo; //en segundos

//--mutex--
pthread_mutex_t mutexSobreListaDeInstrucciones;
pthread_mutex_t mutexSobreListaPorcentajeUltMinuto;
pthread_mutex_t mutexFede;

t_list* listaPorcentajeUltMinuto;

int conectar(char*,char*,char);
void ejecutarCpu(t_Package* paquete);
void cargarArchivoDeConfiguracion();
int recibirMensajeDe(int socketQueEscribe,t_Package* paquete);
int ejecutarProcesos(int idCpu);
char* iniciar(int cantDePaginas,int idProcesoAiniciar,int socketAdminMemoriaDelCpu,int*resultado);
char* leer(int pid,int numeroPagina,int socketMemoria);
char* escribir (int pid,int numeroPagina,char* texto ,int socketMemoria);
char* entradaSalida(int tiempo);
char* finalizar(int pid,int socketMemoria);
informacionSobreLasInstrucciones finalizarForzado(int pid,int socketAdminMemoriaDelCpu);
void inicializarRespuesta(informacionSobreLasInstrucciones* respuesta);
void dormir(float numero);

void liberarMemoriaDeInstruccion(char* cuerpo, char* parametro1,char* parametro2, char* instruccion);
void desarmarInstruccion(char* instruccion,char** cuerpo,char** parametro1,char** parametro2);
//informacionSobreLasInstrucciones leerInterpretarInstruccion(char* path,	int punteroInstruccion, int quantum, int socketMemoria,	int socketPlanificador, int id,t_Package* paquete,pcb p);
informacionSobreLasInstrucciones leerInterpretarInstruccion(t_Package* paquete,pcb p,FILE* a);
int tamanio_archivo(int fd);
char* obtenerDatoLimpioDelDiccionario(t_dictionary* diccionario, char* dato);

void agregarInstruccionCpuPorcentaje(int idCpu);
void comunicarPorcentaje(int* sockPlanificador);
t_list* obtenerListaPorcentajes();
void recetPorcentajes();
t_list* promediarListas(t_list* listaPorcentajes, t_list* listaPorcentajeUltMin);

void mandarCadena(int socket,char* cadena);
char* recibirCadena(int socketQueEscribe);
char recibirChar(int unSocket);
char* concatenarStrings(char* palabra1, char* palabra2);
void eliminarLista(t_list* lista);

void enviarListaString(int sock,t_list* resultadosDeInstrucciones);
void enviarListaEntero(int sock,t_list* lista);
void logear (char* stringAlogear);
void iniciarTodosLosMutex();

#endif

