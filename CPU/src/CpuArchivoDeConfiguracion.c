#include "cpuMensajes.h"

void cargarArchivoDeConfiguracion() {
	idProceso = 'C';

	t_dictionary* diccionario;
	diccionario = dictionary_create();
	char* dato;
	char* clave;

	char textoLeido[200];
	FILE* archivo;

	char* config = "configuracionCPU";
	archivo = fopen(config,"r");
	if (archivo == NULL)
	   {
	      puts("No se pudo abrir el archivo de configuracion");
	   }


	while ((feof(archivo)) == 0) {
		fgets(textoLeido, 200, archivo);

		clave = string_split(textoLeido, ",")[0];
		dato = string_split(textoLeido, ",")[1];
		dictionary_put(diccionario, clave, dato);

	}
	//IMPORTANTE hay que hacer esto que viene abajo pq el dato obtenido del diccionario viene con un caracter
	// de mas entonces tenemos que "limpiarlo" (se lo sacamos)


	puertoPlanificador = obtenerDatoLimpioDelDiccionario(diccionario, "PUERTO_PLANIFICADOR");
	ipPlanificador = obtenerDatoLimpioDelDiccionario(diccionario, "IP_PLANIFICADOR");
	puertoAdminMemoria = obtenerDatoLimpioDelDiccionario(diccionario, "PUERTO_MEMORIA");
	ipAdminMemoria = obtenerDatoLimpioDelDiccionario(diccionario, "IP_MEMORIA");
	cantidadDeHilosCpu = atoi(obtenerDatoLimpioDelDiccionario(diccionario,"CANTIDAD_HILOS"));
	retardo = atof(obtenerDatoLimpioDelDiccionario(diccionario,"RETARDO"));

//	printf("puertoP %s, ipP %s,puertoA %s,ipA %s ,HILOS %d, retardo %d\n",puertoPlanificador,ipPlanificador,puertoAdminMemoria,ipAdminMemoria,cantidadDeHilosCpu,retardo);
}


char* obtenerDatoLimpioDelDiccionario(t_dictionary* diccionario, char* dato) {

	char* datoProvisorio;
	char* datoLimpio;
	datoProvisorio = dictionary_get(diccionario, dato);
	datoLimpio = string_substring_until(datoProvisorio,
			(string_length(datoProvisorio) - 1));
	return datoLimpio;
}


void cargarArchivoDeConfiguracionDammy(){
	ipPlanificador = "127.0.0.1";
	puertoPlanificador = "6666";
	idProceso = 'C';
	cantidadDeHilosCpu = 1;

	ipAdminMemoria = "127.0.0.1";
	puertoAdminMemoria = "6667";
}

