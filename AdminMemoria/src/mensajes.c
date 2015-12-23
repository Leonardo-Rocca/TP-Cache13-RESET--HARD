#include "memoriaMensajes.h"

//--PRUEBAS--
void imprimirEstadoTLB(){
	if(!tlbHabilitada)return;
	int i=0;
	for(;i<entradasTLB;i++){
		struct tlb* tlb;
		tlb = list_get(TLB,i);
		printf("TLB Acc: %d Uso: %d Mod: %d Pres: %d FrmAsig: %d Frame: %d Pag: %d Proc: %d\n",tlb->bitAccedido,tlb->bitUso,tlb->bitModificado,tlb->bitPresencia,tlb->framesAsignados,tlb->nroFrame,tlb->nroPagina,tlb->nroProceso);
	}
}

void imprimirEstadoRAM(){
	int i=0;
	for(;i<cantidadMarcos;i++){
		printf("Marco %d -- %s\n",i,(char*)list_get(frames,i));
	}
}

void imprimirEstadoPaginasDeUnProceso(int pid){
	int o=0;
	struct datosDelProceso* t;
	t = obtenerTablaDeProceso(pid);
	t_list* listaPaginas = obtenerTablaDePaginas(pid);
	int nroPaginas = list_size(listaPaginas);
	for(;o<nroPaginas;o++){
		struct tablaPaginas* tp = list_get(t->tabla,o);
		printf("FraAsig: %d NroProc: %d Acc: %d Mod: %d Pres: %d Uso: %d Frame: %d Pag: %d \n",t->framesAsignados,t->nroProceso,tp->bitAccedido,tp->bitModificado,tp->bitPresencia,tp->bitUso,tp->nroFrame,tp->nroPagina);
		}
}
//--FIN FUNCIONES DE PRUEBA--

//--INICIALIZACIONES--
void inicializarAdministrador(){
	if(tlbHabilitada==1){
		inicializarTLB();
	}
	if(!strcmp(algoritmoReemplazo,"CLKMODIF")){
		indiceParaAlgoritmoClockModificado=0;
	}
	inicializarRAM();
	procesos = list_create();
}

void inicializarTLB(){
	TLB = list_create();
	int i=0;
	for(;i<entradasTLB;i++){
		struct tlb* tlb = malloc(sizeof(struct tlb));
		tlb->bitAccedido=-1;
		tlb->bitModificado=-1;
		tlb->bitUso=-1;
		tlb->bitPresencia=-1;
		tlb->framesAsignados=-1;
		tlb->nroFrame=-1;
		tlb->nroPagina=-1;
		tlb->nroProceso=-1;
		list_add(TLB,tlb);
	}

	//para probar
	//imprimirEstadoTLB();

}

void inicializarRAM(){
	frames = list_create();
	int i=0;
	for(;i<cantidadMarcos;i++){
		char* frm = malloc(tamanioMarco);
		frm = NULL;
		list_add(frames,frm);
	}

	//para probar
	//imprimirEstadoRAM();
}
//-- FIN INICIALIZACIONES --


//-- CUANDO LA CPU AVISA EL INICIO DE UN MPROG --
void inicializarEstructuras(int pid,int nroPaginas){
	struct datosDelProceso* datos = malloc(sizeof(struct datosDelProceso));
	datos->nroProceso = pid;
	datos->punteroClock = 0;
	t_list* listaPaginas = list_create();

	int i = 0;
	for(;i<nroPaginas;i++){
		struct tablaPaginas* pagina = malloc(sizeof(struct tablaPaginas));
		pagina->nroPagina = i;
		pagina->bitAccedido = -1;
		pagina->bitModificado = -1;
		pagina->bitUso = -1;
		pagina->bitPresencia = -1;
		pagina->nroFrame = -1;
		list_add(listaPaginas,pagina);
	}

	datos->tabla = listaPaginas;
	datos->framesAsignados=0;
	list_add(procesos,datos);

	//para probar
	//imprimirEstadoPaginasDeUnProceso(pid);

}
//-- FIN INICIO MPROG --



//-- LECTURA --
//----CUANDO LA CPU AVISA LECTURA EN UN MPROG----
char* leerPagina(int pid,int nroPagina){
	
	char log[700];
	int frame = obtenerFrame(pid,nroPagina,1); //el 1 seria not write (read)
	char* pagina;
	int modif=0;
	if(frame != -1){
		dormir(retardoMemoria);
		pagina = list_get(frames,frame);
		sprintf(log,"\033[1;35mAcceso a Memoria - Lectura de pagina - PID:%d - Numero de pagina:%d - Numero de marco:%d\033[0m",pid,nroPagina,frame);
		logear(log);
		actualizarEstructuras(pid,nroPagina,frame,modif,0,0,0);
	}else{
		pagina = pedirASWAP(pid,nroPagina); //le pide a swap la pagina que necesita
		frame = obtenerFrameDisponibleEnRAM(pid,nroPagina,pagina,&modif); //devuelve lugar donde poner la pagina, invocando si corresponde el reemplazo para que ese lugar sea valido
		if(frame == -1){
				//sprintf(log,"Solicitud de lectura recibida - Marco invalido - PID:%d - Numero de pagina:%d ",pid, nroPagina);
				//logear(log);
				//aborto
			}
		dormir(retardoMemoria);//escritura
		char* exPagina= list_replace(frames,frame,pagina);
		logearReemplazoPagina(pid,nroPagina,frame,exPagina,pagina);
		dormir(retardoMemoria);//lectura
		//actualizarEstructuras(pid,nroPagina,frame,modif,1,1);
		actualizarEstructuras(pid,nroPagina,frame,modif,1,1,1);
	}
	if(tlbHabilitada) actualizarTLB(pid,nroPagina,frame,0);
	return pagina;
}

char* pedirASWAP(int pid,int nroPagina){
	char* pagina = malloc(tamanioMarco);
	pthread_mutex_lock(&swap);
	pagina = avisarSwapLeer(pid, nroPagina);
	pthread_mutex_unlock(&swap);
	return pagina;
}

//-- FIN LECTURA --

int obtenerFrameDisponibleEnRAM(int pid,int nroPagina,char* pagina,int*modif){
	char log[400];
	dormir(retardoMemoria);
	struct datosDelProceso* tablaDeProceso = obtenerTablaDeProceso(pid);
	int framesAsignadosAlProceso = tablaDeProceso->framesAsignados;
	int entro =0;
	int hayFrames = hayFramesDisponibles();

	if(entro == 0 && framesAsignadosAlProceso == 0 && hayFrames == -1){ //no entra, no tiene frames asignados y no hay disponibles
		entro = 1;
		return -1; //no va a poder ser, ese proceso sono
	}

	if(entro == 0 && framesAsignadosAlProceso < maximoMarcosPorProceso && hayFrames != -1){ //hay frames disponibles y no llego al maximo
		entro = 1;
		*modif=1;
		sprintf(log,"\033[1;36mAcceso a Swap - Volcado sin reemplazo - PID:%d - Numero de pagina:%d - Numero de marco:%d\033[0m",pid,nroPagina,hayFrames);
		logear(log);
		return hayFrames;	//devuelve el frame a usar, que estaba disponible
	}
	
	return obtenerFrameAReemplazarEnRamPara(pid);	//devuelve el frame a usar, pero que va a pisar una pagina que estaba
	//esto se hace cuando hay frames, pero el proceso ya alcanzo su maximo, asi que debera sacar uno para reemplazarlo
}

//-- ESCRITURA --
void escribirPagina(int pid,int nroPagina,char* pagina){
	char log[500];
	int modif = 0;
	int frame = obtenerFrame(pid,nroPagina,0);//el 0 representa Write
	int estabaEnRam = 0;
	if(frame == -1){ //es decir si hubo page fault, va a buscar un frame disponible para meter la que tiene que traerse de swap
		frame = obtenerFrameDisponibleEnRAM(pid,nroPagina,pagina,&modif);
		estabaEnRam=1;
		}
	if(frame == -1){
		//sprintf(log,"Solicitud de escritura recibida - No hay marco disponible para cargar pagina desde swap - PID:%d - Numero de pagina:%d ",pid, nroPagina);
		//logear(log);
		//aborto
	}

	dormir(retardoMemoria);
	char* exPagina = list_replace(frames,frame,pagina); //obtengo (solo para el logueo) la pagina que estaba, que quedó residualmente en ese marco
	logearReemplazoPagina(pid,nroPagina,frame,exPagina,pagina);

	actualizarEstructuras(pid,nroPagina,frame,modif,1,estabaEnRam,0);
	if(tlbHabilitada) actualizarTLB(pid,nroPagina,frame,1);
}

//----CUANDO LA CPU AVISA EL FIN DE UN MPROG----
void finalizarEstructuras(int pid){
	pthread_mutex_lock(&mutexFede);
	int indice = obtenerIndiceTablaDeProceso(pid);
	char* reinicializar = NULL;
	dormir(retardoMemoria);
	struct datosDelProceso* datos = list_remove(procesos,indice);
	t_list* paginas = datos->tabla;

	int t=0;
	int cantidad= list_size(paginas);

	for(;t<cantidad;t++){
		struct tablaPaginas* pag = list_remove(paginas,0);
		if(pag->nroFrame != -1)list_replace(frames,pag->nroFrame,reinicializar);
		free(pag);
	}

	list_destroy(paginas);
	free(datos);
	//printf("\nEND: %d\n",pid);
	pthread_mutex_unlock(&mutexFede);
}

void eliminarDeTLB(int pid, int nroPagina,int posicionTLB){
	struct tlb* tlb = malloc(sizeof(struct tlb));
	tlb->bitModificado=0;
	tlb->bitPresencia=0;
	tlb->framesAsignados=0;
	tlb->nroFrame=-1;
	free(list_replace(TLB,posicionTLB,tlb));
}

void eliminarSinTLB(t_list* paginas,int pid){
	int i=0;
	int cantidad=list_size(paginas);
	for(;i<cantidad;i++){
		struct tablaPaginas* pagina;
		pagina = list_get(paginas,i);
		if(pagina->bitPresencia > -1){
			char* reinicializar = NULL;
			list_replace(frames,pagina->nroFrame,reinicializar);
		}
	}
}

void eliminarConTLB(t_list* paginas,int pid){
	int i=0;
	int t=0;
	int cantidad=entradasTLB;
	for(;i<cantidad;i++){
		struct tlb* tlb;
		tlb = list_get(TLB,t);
		int estaEnTP;
		if(pid == tlb->nroProceso){
			estaEnTP = verSiEstaEnLaTablaDePaginas(tlb->nroPagina,paginas);
		}else estaEnTP =-1;

		if(estaEnTP != -1 || tlb->nroPagina ==-1){
			eliminarDeTLB(pid, tlb->nroPagina,i);
			t++;
		}
	}
}
//--Listo cuando finaliza un proceso--


//----MISTERIOS OCULTOS------

//devuelve el indice de la t_list de los datos del proceso (siempre encuentra) P
int obtenerIndiceTablaDeProceso(int pid){
	int t=0;
	int indice=-1;
	for(;t<list_size(procesos);t++){
			struct datosDelProceso* tablaDeProceso;
			tablaDeProceso = list_get(procesos,t);
				if(tablaDeProceso->nroProceso == pid){
					indice = t;
				}
		}
	return indice;
}

//devuelve los datos del proceso (siempre encuentra) P
struct datosDelProceso* obtenerTablaDeProceso(int pid){
	int indice = obtenerIndiceTablaDeProceso(pid);
	return list_get(procesos,indice);
}

//devuelve la tabla de paginas (siempre encuentra) P
t_list* obtenerTablaDePaginas(int pid){
	t_list* pagina;
	struct datosDelProceso* tablaDeProceso;
	int indice = obtenerIndiceTablaDeProceso(pid);
	tablaDeProceso = list_get(procesos,indice);
	pagina = tablaDeProceso->tabla;
	return pagina;
}

//devuelve una pagina (siempre encuentra)
struct tablaPaginas* obtenerPagina(int pid,int nroPagina){
	t_list* paginas = obtenerTablaDePaginas(pid);
	struct tablaPaginas* tablaPagina;
	int i=0;
	for(;i<list_size(paginas);i++){
		tablaPagina = list_get(paginas,i);
		if(tablaPagina->nroPagina == nroPagina){
			return tablaPagina;
		}
	}
	return tablaPagina;
}

//devuelve el indice de la t_list o -1
int obtenerIndiceEnTLB(int pid,int nroPagina){
	int i=0;
	for(;i<entradasTLB;i++){
		struct tlb* tlb;
		tlb = list_get(TLB,i);
		int pag = tlb->nroPagina;
		int proc = tlb->nroProceso;
		if(proc == pid && pag == nroPagina){
			return i;
		}
	}
	return -1;
}

//devuelve el frame donde esta o -1
int revisarTLB(int pid,int nroPagina){
	int i=0;
	for(;i<entradasTLB;i++){
		struct tlb* tlb;
		tlb = list_get(TLB,i);
		int pag = tlb->nroPagina;
		int proc = tlb->nroProceso;
		if(pag == nroPagina && proc == pid){
			int frame = tlb->nroFrame;
			return frame;
		}
	}
	return -1;
}

//obtiene el frame donde esta una pagina o -1 (revisa tlb y memoria)
int obtenerFrame(int pid,int nroPagina, int tipoDeAccesoSolicitado){

	if(tlbHabilitada == 1){
		int intentoTLB = revisarTLB(pid,nroPagina);
		if(intentoTLB != -1){
			logearTLBhit(pid,tipoDeAccesoSolicitado,nroPagina,intentoTLB);
			return intentoTLB;
		}
	}

	int intentoRAM = revisarRAM(pid,nroPagina,tipoDeAccesoSolicitado);
	return intentoRAM;
	
	}

//devuelve el frame donde esta o -1
int revisarRAM(int pid,int nroPagina, int tipoDeAccesoSolicitado){

	char log[500];
	char tipo[10];
	dormir(retardoMemoria);
	if(tipoDeAccesoSolicitado == 1) sprintf(tipo,"lectura"); else sprintf(tipo,"escritura");
	
	struct tablaPaginas* tablaPagina = obtenerPagina(pid,nroPagina);
	if(tablaPagina->bitPresencia == 1){
		if(tlbHabilitada == 1)
			sprintf(log,"\033[1;35mSolicitud de %s recibida - PID:%d - Numero de pagina:%d - TLB miss - No Page Fault - Numero de marco:%d\033[0m",tipo,pid,nroPagina,tablaPagina->nroFrame);
		else
			sprintf(log,"\033[1;35mSolicitud de %s recibida - PID:%d - Numero de pagina:%d - No Page Fault - Numero de marco:%d\033[0m",tipo,pid,nroPagina,tablaPagina->nroFrame);
		logear(log);
		return tablaPagina->nroFrame;
	}
	
	if(tlbHabilitada == 1)
		sprintf(log,"\033[1;33mSolicitud de %s recibida - PID:%d - Numero de pagina:%d - TLB miss - Page Fault\033[0m",tipo,pid,nroPagina);
	else
		sprintf(log,"\033[1;33mSolicitud de %s recibida - PID:%d - Numero de pagina:%d - Page Fault\033[0m",tipo,pid,nroPagina);
	
	logear(log);
	return -1;
}

//devuelve el primer frame disponible o -1
int hayFramesDisponibles(){
	int i = 0;
	for(;i<cantidadMarcos;i++){
		if(list_get(frames,i)==NULL) return i;
	}
	if(i==cantidadMarcos)return -1;
}

//actualiza framesAsignados y su tabla
void actualizarEstructuras(int pid, int nroPagina,int nroFrame,int modif,int lecturaEscritura,int aux,int aux2){
	struct datosDelProceso* datos = obtenerTablaDeProceso(pid);
	if(modif<2)datos->framesAsignados = datos->framesAsignados + modif;
	struct tablaPaginas* pagina = obtenerPagina(pid,nroPagina);
	pagina->bitPresencia = 1;
	//printf("\n\n %d \n\n",nroPagina);
	pagina->bitUso = 1;
	pagina->nroFrame = nroFrame;
	pagina->nroPagina = nroPagina;
	if(pagina->bitModificado < 1) pagina->bitModificado = lecturaEscritura;
	determinarAcceso(obtenerTablaDePaginas(pid),nroPagina);
	if(lecturaEscritura==1 && aux==1)reorganizarPaginasParaFifo(datos->tabla,nroPagina);
	if(aux2==1)pagina->bitModificado=0;
}

//determina y actualiza el orden de acceso a las paginas (tabla de paginas)
void determinarAcceso(t_list* paginas,int nroPagina){
	int i=0;
	for(;i<list_size(paginas);i++){
		struct tablaPaginas* pagina;
		pagina = list_get(paginas,i);
		if(pagina->bitPresencia != -1){
			if(pagina->nroPagina == nroPagina){
				pagina->bitAccedido=0;
			}else{
				pagina->bitAccedido=pagina->bitAccedido+1;
			}
		}
	}

}

//Si esta en TLB actualiza, sino reemplaza
void actualizarTLB(int pid, int nroPagina,int nroFrame,int escrituraLectura){
	int i=0;
	int estaEnTLB = revisarTLB(pid,nroPagina);
	if(estaEnTLB == -1){
		reemplazo(pid,nroPagina,nroFrame,escrituraLectura);
	}else{
		actualizo(pid,nroPagina,nroFrame,escrituraLectura);
	}
}

//Determina acceso a paginas solo si estan en TLB
void determinarAccesoAPaginasEnTLB(int pid,int nroPagina){
	int i=0;
	for(;i<entradasTLB;i++){
		struct tlb* tlb;
		tlb = list_get(TLB,i);
		if(tlb->bitPresencia != -1){
			if(tlb->nroPagina == nroPagina && tlb->nroProceso == pid){
				tlb->bitAccedido=0;
			}else{
				tlb->bitAccedido=tlb->bitAccedido+1;
			}
		}
	}
}

//actualiza la tlb (solo se llama si la pagina esta en tlb)
void actualizo(int pid, int nroPagina,int nroFrame,int escrituraLectura){
		int indice;
		if(revisarTLB(pid,nroPagina)==-1){
			indice=obtenerPaginaAReemplazarDeLaTLB(pid,nroPagina,escrituraLectura);
		}else{
			indice=obtenerIndiceEnTLB(pid,nroPagina);
		}
		//printf("\nINDICE_ %d\n",indice);
		struct tlb* tlb;
		tlb = list_get(TLB,indice);
		tlb->bitPresencia = 1;
		tlb->bitUso=1;
		tlb->nroPagina	= nroPagina;
		tlb->nroProceso = pid;
		tlb->nroFrame = nroFrame;
		if(tlb->bitModificado<1){
			tlb->bitModificado = escrituraLectura;
			}
		struct datosDelProceso* tabla;
		tabla = obtenerTablaDeProceso(pid);
		tlb->framesAsignados = tabla->framesAsignados;
		determinarAccesoAPaginasEnTLB(pid,nroPagina);

}

//obtiene que pagina se saca de la TLB
int obtenerPaginaAReemplazarDeLaTLB(int pid,int nroPagina,int escrituraLectura){
	int posicion=-1;
	//if(!strcmp(algoritmoReemplazo,"LRU")){
	//	posicion = reemplazarEnTLBSegunLRU(pid,nroPagina,escrituraLectura);
	//	return posicion;
	//}else if(!strcmp(algoritmoReemplazo,"FIFO")){
		posicion = reemplazarEnTLBSegunFifo(pid,nroPagina,escrituraLectura);
	//	return posicion;
	//}else if(!strcmp(algoritmoReemplazo,"CLKMODIF")){
	//	posicion = reemplazarEnTLBSegunClockModif(pid,nroPagina,escrituraLectura);
	//	return posicion;
	//}

	return posicion;
}

//obtiene el de mayor acceso en TLB
int obtenerMaxTLB(){
	int i=0;
	int max=-1;
	while(i<entradasTLB){
			struct tlb* tlb;
			tlb = list_get(TLB,i);
			if(tlb->bitAccedido > max){
				max = tlb->bitAccedido;
			}
	i++;
	}
	return max;
}

//remplaza una pagina para poner una que no estaba en TLB
void reemplazo(int pid,int nroPagina,int nroFrame,int escrituraLectura){
	int indiceAReemplazar = obtenerPaginaAReemplazarDeLaTLB(pid,nroPagina,escrituraLectura);
	struct tlb* tlb = malloc(sizeof(struct tlb));
	tlb->bitModificado=escrituraLectura;
	tlb->bitPresencia=1;
	tlb->bitUso=1;
	tlb->nroFrame=nroFrame;
	tlb->nroPagina=nroPagina;
	tlb->nroProceso=pid;
	struct datosDelProceso* tabla;
	tabla = obtenerTablaDeProceso(pid);
	actualizarFramesAsignadosPorProcesoEnTLB(tabla->framesAsignados,pid);
	tlb->framesAsignados = tabla->framesAsignados;
	tlb->bitAccedido=-1;
	list_replace(TLB,indiceAReemplazar,tlb);
	determinarAccesoAPaginasEnTLB(pid,nroPagina);
}

//obtiene el frame a reemplazar en ram
int obtenerFrameAReemplazarEnRamPara(int pid){
	t_list* paginas = obtenerTablaDePaginas(pid);
	int indice=-1;
	int nroPagina=-1;
	if(!strcmp(algoritmoReemplazo,"FIFO")){
		reemplazarFrameEnRamFifo(paginas,&indice,&nroPagina,pid);
	}else if(!strcmp(algoritmoReemplazo,"LRU")){
		reemplazarFrameEnRamLRU(paginas,&indice,&nroPagina,pid);
	}else if(!strcmp(algoritmoReemplazo,"CLKMODIF")){
		clockEnProcesoParaReemplazarRAM(paginas,&indice,&nroPagina,pid);
		//reemplazarFrameEnRamClockModif(paginas,&indice,&nroPagina,pid);
		}
//aca ya consiguio saber la pagina que va a irse
	struct tablaPaginas* pagina = obtenerPagina(pid,nroPagina);
	analizarSiEsModificada(pid,pagina); //obtiene esa pagina que se va a ir, y si se modifico la manda a swap para que la grabe, y ya esta lista para descartarse
	if(tlbHabilitada){
		resetearValoresEnTLB(pid,pagina);
	}
	resetearValoresPagina(pagina);
	return indice;
	//al menos hay uno presente ya que sino no llega a llamarse esta funcion
}

//se fija si una pagina a reemplazar esta modificada para avisarle al swap
void analizarSiEsModificada(int pid,struct tablaPaginas* pagina){
	if(pagina->bitModificado==1){
		pthread_mutex_lock(&swap);
		avisarSwapEscribir(pid,pagina->nroPagina,list_get(frames,pagina->nroFrame));
		pthread_mutex_unlock(&swap);
	}

}

int reemplazarEnTLBSegunFifo(int pid,int nroPagina,int escrituraLectura){
	if(escrituraLectura){
		struct tlb* tlb;
		tlb = list_remove(TLB,0);
		list_add(TLB,tlb);
		return entradasTLB-1;
	}
	return obtenerIndiceEnTLB(pid,nroPagina);

}

int reemplazarEnTLBSegunLRU(int pid, int nroPagina,int escrituraLectura){
	if(!escrituraLectura || revisarTLB(pid,nroPagina)!=-1)return obtenerIndiceEnTLB(pid,nroPagina);
	int i=0;
	int posicion=-1;
	int max = obtenerMaxTLB();
	while(i<entradasTLB){
		struct tlb* tlb;
		tlb = list_get(TLB,i);


		if(tlb->nroPagina == -1){
			posicion = i;
			break;
		}


		if(tlb->bitAccedido == max){
			posicion=i;
		}

		i++;
	}
	return posicion;
}

int reemplazarEnTLBSegunClockModif(int pid,int nroPagina,int escrituraLectura){
	
	int i=0;	//uso i para contar de 0 a tamTotal, el i va contando igual que el indiceParaAlgoritmoClockModificado pero offseteado para que empiece de 0. cuando i llegue al final, es porque indiceParaAlgoritmoClockModificado recorrio toda la lista circular
	int mode=0;
	struct tlb* tlb;
	
	if(escrituraLectura){
	
		do{
			while(i<entradasTLB){
				if(indiceParaAlgoritmoClockModificado < list_size(TLB)) indiceParaAlgoritmoClockModificado = 0; //cuando llegues al fin de la lista segun el indice, empeza desde el principio (por ser lista circular)
				tlb = list_get(TLB,indiceParaAlgoritmoClockModificado);
				if(tlb->bitAccedido == 0 && tlb->bitModificado == mode%2) { //se debe fijar que u y m sean cero, en tal caso se marca y fin
					return indiceParaAlgoritmoClockModificado;				//cuando la halla, devuelve ese indice
				}
				if(mode%2 == 1) tlb->bitAccedido=0;		//si no hallo nada, en caso de estar en la segunda vuelta pongo U en 0 como indica el algoritmo
				i++;
				indiceParaAlgoritmoClockModificado++;
			}

		mode++; //fin del ciclo, si no hallo nada, vuelve a hacerlo pero buscando (0,1), o (0,0) de nuevo si ya estaba en eso
		i=0; //de cualquier manera, reseteo el recorredor
		}while(mode < 8);	//fin del do, si luego de esas pasadas (4 en realidad es lo max) no hallo nada es que algo no esta bien y vuelve sin resultados
	}
	return obtenerIndiceEnTLB(pid,nroPagina);
}

//Actualiza el campo frames Asignados en la TLB
void actualizarFramesAsignadosPorProcesoEnTLB(int cantidad,int pid){
	int i=0;
	while(i<entradasTLB){
		struct tlb* tlb;
		tlb = list_get(TLB,i);
		if(tlb->nroProceso==pid){
			tlb->framesAsignados=cantidad;
			}
		i++;
		}
}

//Resetea la pagina reemplazada en ram
void resetearValoresPagina(struct tablaPaginas* pagina){
	pagina->nroFrame=-1;
	pagina->bitModificado=-1;
	pagina->bitUso=-1;
	pagina->bitPresencia=-1;
	pagina->bitAccedido=-1;
}

//Resetea la pagina reemplazada en tlb
void resetearValoresEnTLB(int pid,struct tablaPaginas* pagina){
	int i=0;
	while(i<entradasTLB){
		struct tlb* tlb;
		tlb = list_get(TLB,i);
		if(tlb->nroProceso==pid && tlb->nroPagina == pagina->nroPagina){
			tlb->bitPresencia=0;
			tlb->bitModificado=0;
			tlb->nroFrame=-1;
			}
		i++;
	}
}

//reemplazo local, REEMPLAZO!
void reemplazarFrameEnRamFifo(t_list* paginas,int*indice,int *nroPagina,int pid){
	
	char log[400];
	int i=0;
	for(;i<list_size(paginas);i++){
		struct tablaPaginas* pagina;
		pagina = list_get(paginas,i);
			if(pagina->bitPresencia == 1){
				*indice = pagina->nroFrame;
				*nroPagina=pagina->nroPagina;
				sprintf(log,"Acceso a Swap - Volcado mediante reemplazo FIFO - PID:%d - Pagina reemplazada:%d (%d en la cola) - Marco de reemplazo:%d",pid,*nroPagina,i,*indice);
				logear(log);
				i=list_size(paginas);
			}
		}
}

void reemplazarFrameEnRamLRU(t_list* paginas,int*indice,int *nroPagina,int pid){
	char log[400];
	int i=0;
	int accedidoMax = -1;
	for(;i<list_size(paginas);i++){
		struct tablaPaginas* pagina;
		pagina = list_get(paginas,i);
			if(pagina->bitPresencia != -1){
				if(pagina->bitAccedido > accedidoMax){
					*indice = pagina->nroFrame;
					*nroPagina=pagina->nroPagina;
					accedidoMax = pagina->bitAccedido;
				}
			}
		}
	sprintf(log,"Acceso a Swap - Volcado mediante reemplazo LRU - PID:%d - Pagina reemplazada:%d - Marco de reemplazo:%d",pid,*nroPagina,*indice);
	logear(log);
}

void clockEnProcesoParaReemplazarRAM(t_list* paginas,int*indice,int *nroPagina,int pid){
	struct datosDelProceso* datos = obtenerTablaDeProceso(pid);
	char log[400];
	int i=datos->punteroClock;
    //printf("puntero: %d\n",datos->punteroClock);

	//BUSCA 0 0
	for(;i<list_size(paginas);i++){
		struct tablaPaginas* pagina;
		pagina = list_get(paginas,i);
			if(pagina->bitPresencia != -1){
				if(pagina->bitUso == 0 && pagina->bitModificado == 0){
					*indice = pagina->nroFrame;
					*nroPagina=pagina->nroPagina;
					sprintf(log,"Acceso a Swap - Volcado mediante reemplazo CLOCK-MODIFICADO - PID:%d - Pagina reemplazada:%d - Marco de reemplazo:%d",pid,*nroPagina,*indice);
					logear(log);
					datos->punteroClock=i;
					if(i == list_size(paginas)-1)datos->punteroClock=0;
					return;
				}
			}
		}

	i=0;
	for(;i<datos->punteroClock;i++){
		struct tablaPaginas* pagina;
		pagina = list_get(paginas,i);
			if(pagina->bitPresencia != -1){
				if(pagina->bitUso == 0 && pagina->bitModificado == 0){
					*indice = pagina->nroFrame;
					*nroPagina=pagina->nroPagina;
					datos->punteroClock=i;
					if(i == list_size(paginas)-1)datos->punteroClock=0;
					sprintf(log,"Acceso a Swap - Volcado mediante reemplazo CLOCK-MODIFICADO - PID:%d - Pagina reemplazada:%d - Marco de reemplazo:%d",pid,*nroPagina,*indice);
					logear(log);
					return;
				}
			}
		}

//BUSCA 0 1
		i=datos->punteroClock;
		for(;i<list_size(paginas);i++){
			struct tablaPaginas* pagina;
			pagina = list_get(paginas,i);
				if(pagina->bitPresencia != -1){
					if(pagina->bitUso == 0 && pagina->bitModificado == 1){
						*indice = pagina->nroFrame;
						*nroPagina=pagina->nroPagina;
						datos->punteroClock=i;
						if(i == list_size(paginas)-1)datos->punteroClock=0;
						sprintf(log,"Acceso a Swap - Volcado mediante reemplazo CLOCK-MODIFICADO - PID:%d - Pagina reemplazada:%d - Marco de reemplazo:%d",pid,*nroPagina,*indice);
						logear(log);
						return;
					}
					pagina->bitUso=0;
				}
			}

		i=0;
			for(;i<datos->punteroClock;i++){
				struct tablaPaginas* pagina;
				pagina = list_get(paginas,i);
					if(pagina->bitPresencia != -1){
						if(pagina->bitUso == 0 && pagina->bitModificado == 1){
							*indice = pagina->nroFrame;
							*nroPagina=pagina->nroPagina;
							datos->punteroClock=i;
							if(i == list_size(paginas)-1)datos->punteroClock=0;
							sprintf(log,"Acceso a Swap - Volcado mediante reemplazo CLOCK-MODIFICADO - PID:%d - Pagina reemplazada:%d - Marco de reemplazo:%d",pid,*nroPagina,*indice);
							logear(log);
							return;
						}
						pagina->bitUso=0;
					}
				}


			clockEnProcesoParaReemplazarRAM(paginas,indice,nroPagina,pid);

}



void reemplazarFrameEnRamClockModif(t_list* paginas,int*indice,int *nroPagina,int pid){
	char log[400];
	int i=0;	//uso i para contar de 0 a tamTotal, el i va contando igual que el indiceParaAlgoritmoClockModificado pero offseteado para que empiece de 0. cuando i llegue al final, es porque indiceParaAlgoritmoClockModificado recorrio toda la lista circular
	int mode=0;
	do{
		for(;i<list_size(paginas);i++,indiceParaAlgoritmoClockModificado++){
			if(indiceParaAlgoritmoClockModificado < list_size(paginas)) indiceParaAlgoritmoClockModificado = 0; //cuando llegues al fin de la lista segun el indice, empeza desde el principio (por ser lista circular)
			struct tablaPaginas* pagina;
			pagina = list_get(paginas,indiceParaAlgoritmoClockModificado);
				if(pagina->bitPresencia == 1){						 //si la pagina esta en ram
					if(pagina->bitAccedido == 0 && pagina->bitModificado == mode%2){ //se debe fijar que u y m sean cero, en tal caso se marca y fin
						*indice = pagina->nroFrame;		
						*nroPagina=pagina->nroPagina;
						sprintf(log,"Acceso a Swap - Volcado mediante reemplazo Clock Mejorado (%d pasadas) - PID:%d - Pagina reemplazada:%d - Marco de reemplazo:%d",mode,pid,*nroPagina,*indice);					
						return;				//cuando la halla, vuelve
					}
					if(mode%2 == 1) pagina->bitAccedido=0;		//si no hallo nada, en caso de estar en la segunda vuelta pongo U en 0 como indica el algoritmo
				}
			
			} mode++;//fin del ciclo, si no hallo nada, vuelve a hacerlo pero buscando (0,1), o (0,0) de nuevo si ya estaba en eso
	i=0;	//de cualquier manera, reseteo el recorredor de paginas
	}while(mode < 8);	//fin del do, si luego de esas pasadas (4 en realidad es lo max) no hallo nada es que algo no esta bien y vuelve sin resultados
	
	logear(log);
}

void reorganizarPaginasParaFifo(t_list* paginas,int nroPagina){
	struct tablaPaginas* pagina;
	int indice=0;
	while(indice<list_size(paginas)){
		struct tablaPaginas* paginaAux=list_get(paginas,indice);
		if(paginaAux->nroPagina==nroPagina)break;
		indice++;
	}
	pagina = list_remove(paginas,indice);
	list_add(paginas,pagina);

	struct tablaPaginas* pagina2;
		int indice2=0;
		int i2=0;
		while(i2<list_size(paginas)){
			struct tablaPaginas* paginaAux2=list_get(paginas,indice2);
			if(paginaAux2->nroFrame!=-1){
			pagina2 = list_remove(paginas,indice2);
			list_add(paginas,pagina2);
			}else{
			indice2++;
			}
			i2++;
		}

}

int verSiEstaEnLaTablaDePaginas(int nroPagina,t_list* paginas){
	int i=0;
	for(;i<list_size(paginas);i++){
		struct tablaPaginas* pag= list_get(paginas,i);
		if(pag->nroPagina==nroPagina)return i;
	}
	return -1;
}

void logearReemplazoPagina(int pid,int nroPagina,int frame,char* exPagina,char* pagina){
	char log[800];
	sprintf(log,"\033[1;34mAcceso a memoria - Reemplazo de pagina - Algoritmo:%s - PID:%d - Numero de pagina:%d - Numero de marco:%d - Contenido anterior:%s - Nuevo contenido:%s\033[0m",algoritmoReemplazo,pid,nroPagina,frame,exPagina,pagina);
	logear(log);
}

void logearTLBhit(int pid,int tipoDeAccesoSolicitado,int nroPagina,int frame){	
	char log[800];
	char tipo[10];
	if(tipoDeAccesoSolicitado == 1) sprintf(tipo,"lectura"); else sprintf(tipo,"escritura");
	sprintf(log,"\033[1;32mSolicitud de %s recibida - PID:%d - Numero de pagina:%d - TLB hit - Numero de marco:%d\033[0m",tipo,pid,nroPagina,frame);
	logear(log);
}

void logearSIGPOLL(){
	char log[500];
	int i=0;
	pthread_mutex_lock(&logSIGPOLL);
	for(;i<cantidadMarcos;i++)
		{
		sprintf(log,"\033[1;30mMarco %d -- %s\033[0m",i,(char*)list_get(frames,i));
		logear(log);
		}
	pthread_mutex_unlock(&logSIGPOLL);
	logear("\033[1;37mTratamiento de señal SIGPOLL terminado\033[0m");
}

void inicializarMutex(){
	if(pthread_mutex_init(&swap,NULL)){
		printf("\n Mutex init failed \n");
	}
	if(pthread_mutex_init(&mutexFede,NULL)){
			printf("\n Mutex init failed \n");
	}
}

void dormir(float numero){
	if(numero >= 1){
		sleep(numero);
	} else {
		usleep(numero*1000000);
	}
}
