/*
 * AdminMemoriaConexiones.c
 *
 *  Created on: 1/9/2015
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h>

#include "memoriaMensajes.h"

int colaEspera = 5;

void* manejoConexionesConCPUs(void* parametro){
	int socket_cpu , socketCliente , c , *new_sock;
	struct sockaddr_in server , client;
	pthread_t sniffer_thread; //leo lo puso aca
	socketsConectados = list_create();

	//Crea Socket
	socket_cpu = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_cpu == -1)
	{
		printf("Could not create socket");
	}

	//Prepara la estructura sockaddr_in
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( atoi(puerto) );

	    //Bind
	    if( bind(socket_cpu,(struct sockaddr *)&server , sizeof(server)) < 0)
	    {
	        //print the error message
	        perror("bind failed. Error");
	    }

	    //Listen
	    listen(socket_cpu , colaEspera);

	    //Accept and incoming connection
	    c = sizeof(struct sockaddr_in);
	    while( (socketCliente = accept(socket_cpu, (struct sockaddr *)&client, (socklen_t*)&c)) )
	    {
	    	char mensajeBienvenida;
	    	recv(socketCliente,&mensajeBienvenida,sizeof(char),0);
	    	if(mensajeBienvenida=='C'){
	    		puts("Nuevo proceso. CPU aceptado");

	    //		pthread_t sniffer_thread;
	    		new_sock = malloc(1);
	    		*new_sock = socketCliente;
	    		if( pthread_create( &sniffer_thread , NULL ,  manejoHiloCpu , (void*) new_sock) < 0)
	    		{
	    			perror("could not create thread");
	    		}
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
    char mensajeBienvenida;
    int status=1;
    while(status){  //ESTO VUELA
    status = recibirMensajeDe(nroSocket);
    }


	    printf("Desconectado.\n");

    free(socketCli);

    return 0;
}

int manejoConexionesConSwap(char* puertoCpu) {
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ipSwap, puertoCpu, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion

	swapSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype,	serverInfo->ai_protocol);
	int seLogroConectar = 0;
	if (connect(swapSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)
			!= -1) {
		printf("Conectado con Swap\n");
		//	loguearConexionConMarta(seLogroConectar,ipPlanificador,puertoPlanificador);
	}else{
		seLogroConectar = -1;
		//  loguearConexionConMarta(seLogroConectar,ipPlanificador,puertoPlanificador);
	}
	freeaddrinfo(serverInfo);

	char saludo = 'M';
	send(swapSocket, &saludo, sizeof(char), 0); //Enviar Saludo

	/*avisarSwapIniciar(1,20);
	avisarSwapIniciar(2,10);
	avisarSwapIniciar(3,10);
	avisarSwapEscribir(2,1,"aloh");
	avisarSwapEscribir(2,2,"dn");
	avisarSwapEscribir(2,9,"lala");
	avisarSwapFinalizar(1);
	avisarSwapEscribir(3,5,"123");
	avisarSwapIniciar(4,40);
	avisarSwapLeer(2,1);
	avisarSwapLeer(2,2);
	avisarSwapLeer(2,9);
	avisarSwapLeer(3,5);
	avisarSwapFinalizar(2);
	avisarSwapFinalizar(3);
	avisarSwapFinalizar(4);*/

	return swapSocket;
}
