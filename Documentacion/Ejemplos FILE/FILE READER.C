#include<stdio.h>
#include<conio.h>
#include<stdlib.h>


struct FechaNac{int Dia; int Mes; int Anio;};

struct alumno{
  
struct alumno *Siguiente, *Anterior;
char Coincide;
char Ordenado;

char Nombre[30];
char Apellido[30];
struct FechaNac FechaNacim;
char Sexo;
int EstadoCivil;
unsigned long int Legajo;
unsigned long int DNI;	
unsigned long int Telefono;
float Promedio;
int Orientacion;
  
};


main(){


FILE *puntero;
struct alumno *PunteroAux;

puntero=fopen("c:\\BASE.DAT","rb");
printf("Direccion del archivo: %d\n\n",puntero);

do{

PunteroAux=malloc(sizeof(struct alumno));

fread(PunteroAux, sizeof(struct alumno), 1,puntero); 

printf("%s ",PunteroAux->Nombre);
printf("%s ",PunteroAux->Apellido);
printf("%d//",PunteroAux->FechaNacim.Dia);
printf("%d//",PunteroAux->FechaNacim.Mes);
printf("%d ",PunteroAux->FechaNacim.Anio);
printf("%c ",PunteroAux->Sexo);
printf("%c ",PunteroAux->EstadoCivil);
printf("%ld ",PunteroAux->Legajo);
printf("%ld ",PunteroAux->DNI);
printf("%ld ",PunteroAux->Telefono);
printf("%f ",PunteroAux->Promedio);
printf("%d\n",PunteroAux->Orientacion);

PunteroAux=PunteroAux->Siguiente;
}while(PunteroAux!=NULL);

fclose(puntero);

getch();}