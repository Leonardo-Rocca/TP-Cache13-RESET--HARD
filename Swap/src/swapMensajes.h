/*
 * AdminSwapMensajes.h
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

t_list* listaSwap;
t_list* listaCantidadLYE;


int socketAdministradorMemoria;

typedef struct paginasProceso{
	int pid;
	int comienzo;
	int cantidadPaginas;
}paginasProceso;

typedef struct cantidadLYE{
	int pid;
	int cantidadEscritas;
	int cantidadLeidas;
}cantidadLYE;

typedef struct proceso{
	int pid;
	int cantidadPaginas;
}proceso;


char puertoEscucha[9];
char nombreSwap[64];
int cantidadPaginas;
int tamanioPagina;
float retardoCompactacion;
float retardoSwap;

void mandarCadena(int socket,char* cadena);
char* recibirCadena(int socketQueEscribe);
void* manejoConexionesConAdminMemoria();
int recibirMensajeDe(int);
void *manejoHiloAdminMemoria(void *socketCli);
void cargarArchivoDeConfiguracion();
void crearArchivo();
void inicializarEstructura();
void imprimirSwapData();
paginasProceso* crearElementoParaLista(int pid, int comienzo, int cantidadPaginas);
void crearYAgregarElementoEnPosicion(int pid, int comienzo, int cantidadPaginas, int posicion);
int agregarProceso(proceso proceso);
int espacioSwap();
proceso crearProceso(int pid, int cantidadPaginas);
int procesoEntraEnUnHueco(proceso proceso);
void compactar();
void agregarProcesoEnHueco(proceso proceso, int hueco);
int eliminarProceso(int pid);
int procesoSeEncuentraEnSwap(int pid);
void liberarEspacio(int posicion);
void acomodarSwap();
void unirEspaciosLibres(int hueco1, int hueco2);
void moverProceso(int posicion);
char recibirChar(int unSocket);
int escribirPagina(int pagina, char* palabras);
void inicializarArchivo();
void testEscribirEnPaginas();
char* leerPagina(int pagina);
void testLeerPagina();
paginasProceso obtenerProceso(int pid);
void agregarYBorrarLibre(proceso proceso,int hueco);
void moverPaginas(paginasProceso* espacio, paginasProceso* paginasProceso);
void limpiarEspaciosEnSwap();
void indicarCantidadPaginasLeidasYEscritas(int pid);
void agregarNodoLYE(int pid);
void sumarPaginaLeida(int pid);
void sumarPaginaEscrita(int pid);
cantidadLYE* crearElementoParaListaLYE(int pid, int cantidadEscritas, int cantidadLeidas);
char* devolverSwapData();
char* devolverCantidadPaginasLeidasYEscritas(int pid);
void dormir(float numero);

void iniciar();
void leer();
void finalizar();
void escribir();

void logIniciar(paginasProceso paginas);
void logFinalizar(paginasProceso paginas);
void logRechazar(proceso proceso);
void logCompactacionIniciada();
void logCompactacionFinalizada();
void logLectura(paginasProceso paginas,char* texto, int pagina, int numeroPagina);
void logEscritura(paginasProceso paginas, char* texto, int pagina, int numeroPagina);
void logear(char* stringAlogear);
void logPaginasLeidasYEscritas(char* texto);
void logSwapData();

void testSeUnenEspaciosLibres();
void testSeCompactaElEspacio();
void testSeCompactaElEspacio2();
void testAgregarProcesoLlenandoSwap();
void testAgregarProcesoLlenandoSwap2();
void testSobreescribirEnPagina();
void testEscribirEnPaginas();
void testLeerPagina();
void testSeCompactaElEspacioYSeAcomodaArchivo();
void testCargarArchivoDeConfiguracion();

#endif /* MENSAJES_H_ */
