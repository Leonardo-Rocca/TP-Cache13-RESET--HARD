/*
 * planificadorConexiones.c
 *
 *  Created on: 1/9/2015
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h>

#include "planificadorMensajes.h"

int colaEspera = 5;

void* manejoConexionesConCPUs(void* parametro){
	int socketDePosibleCPU , socketCliente , c , *new_sock;
	struct sockaddr_in server , client;
	pthread_t sniffer_thread; //leo lo puso aca
	pthread_t porcentaje_thread; //leo lo puso aca
	socketsConectados = list_create();

	//Crea Socket
	socketDePosibleCPU = socket(AF_INET , SOCK_STREAM , 0);
	if (socketDePosibleCPU == -1)
	{
		printf("Could not create socket");
	}

	//Prepara la estructura sockaddr_in
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( atoi(puerto) );

	    //Bind
	    if( bind(socketDePosibleCPU,(struct sockaddr *)&server , sizeof(server)) < 0)
	    {
	        //print the error message
	        perror("bind failed. Error");
	    }

	    //Listen
	    listen(socketDePosibleCPU , colaEspera);

	    //Accept and incoming connection
	    c = sizeof(struct sockaddr_in);
	    while( (socketCliente = accept(socketDePosibleCPU, (struct sockaddr *)&client, (socklen_t*)&c)) )
	    {
	    	char mensajeBienvenida;
	    	recv(socketCliente,&mensajeBienvenida,sizeof(char),0);
	    	if(mensajeBienvenida=='C'){
	 //   		puts("Nuevo proceso. CPU aceptado");

	    		//esto es nuevo:
	    		int* socketDelCpuParaAgregar = malloc(sizeof(int));
	    		*socketDelCpuParaAgregar = socketCliente;
	    		list_add(socketsConectados,socketDelCpuParaAgregar);

	    		//char log[200];
	    		//sprintf(log,"Conexion de CPU aceptada - Numero de socket:%d",socketCliente);
	    		//logear (log);
	    		logear("Conexion de CPU aceptada");

	    //		pthread_t sniffer_thread;
	    		new_sock = malloc(1);
	    		*new_sock = socketCliente;
	    		if( pthread_create( &sniffer_thread , NULL ,  manejoHiloCpu , (void*) new_sock) < 0)
	    		{
	    			perror("could not create thread");
	    		}
	    	}
	    	if(mensajeBienvenida=='P'){ //porcentaje

	    		socketCpuPorcentaje=socketCliente;

	    	}

	    }

	    if (socketCliente < 0)
	    {
	        perror("accept failed");
	    }
		pthread_join(sniffer_thread,NULL);
	    return 0;
}

void *manejoHiloCpu(void *socketCli)
{
    int nroSocket = *(int*)socketCli;
    int status = 1;
    char resultado;

    verSiSePuedeMandarAejecutarUnProceso();//esto es opcional, segun que es lo que quiera(si quiero que se ejecuten los prog que se mandaron cuando no habia cpus)

    while (status){
        status = recibirMensajeDe(nroSocket);
     }

      logear ("CPU desconectado");

    	   // LOG: loguearDesconexionDeProceso(nombreProceso,nroSocket);
      printf("Cpu Desconectado.\n");

      //Utilizo esta funcion anidada para buscar en la lista
      int esSocketQueBusco(int* sock){
    	  if((*sock)==nroSocket)
    	  return 1;
    	  return 0;
      }
      list_remove_by_condition(socketsConectados,(void*)esSocketQueBusco);
    if(status == -1)
      {
         perror("recv failed");
      }

    free(socketCli);

    return 0;
}

char recibirChar(int unSocket){
	char caracter;
	recv(unSocket, &caracter, sizeof(caracter), 0);
	return caracter;
}
