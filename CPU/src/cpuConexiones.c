/*
 * cpuConexiones.c
 *
 *  Created on: 1/9/2015
 *      Author: utnso
 */
#include "cpuMensajes.h"


int conectar(char* ip ,char* puerto,char saludo) {
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ip, puerto, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion

	int socketObtenido = socket(serverInfo->ai_family, serverInfo->ai_socktype,	serverInfo->ai_protocol);
	int seLogroConectar = 0;
	if (connect(socketObtenido, serverInfo->ai_addr, serverInfo->ai_addrlen)
			!= -1) {
		printf("Conectado \n");//	printf("Conectado con Planificador\n");
		//	loguearConexionConMarta(seLogroConectar,ipPlanificador,puertoPlanificador);
	}else{
		seLogroConectar = -1;
		//  loguearConexionConMarta(seLogroConectar,ipPlanificador,puertoPlanificador);
	}
	freeaddrinfo(serverInfo);

	//char saludo = 'C';
	send(socketObtenido, &saludo, sizeof(char), 0); //Enviar Saludo

	return socketObtenido;
}
