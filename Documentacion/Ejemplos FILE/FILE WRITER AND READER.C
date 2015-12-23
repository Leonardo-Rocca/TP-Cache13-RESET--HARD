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
struct alumno *PunteroAux,Estructura;

puntero=fopen("c:\\BASE.DAT","rb");
PunteroAux=malloc(sizeof(struct alumno));
fread(&PunteroAux, sizeof(struct alumno), 1,puntero); 
printf("Leido: %s\n",PunteroAux->Nombre);
printf("Leido: %s\n\n\n",PunteroAux->Apellido);
getch();
clrscr();



puntero=fopen("c:\\BASE.DAT","wb");
PunteroAux = &Estructura;

printf("Poner Nombre: ");gets(Estructura.Nombre);
printf("Poner Apellido: ");gets(Estructura.Apellido);


fwrite(&PunteroAux, sizeof(struct alumno), 1,puntero); 



fclose(puntero);



puntero=fopen("c:\\BASE.DAT","rb");
printf("Direccion del archivo: %d\n\n",puntero);

PunteroAux=malloc(sizeof(struct alumno));
/*Estructura = *PunteroAux;*/

fread(&PunteroAux, sizeof(struct alumno), 1,puntero); 

printf("%s\n",PunteroAux->Nombre);
printf("%s\n",PunteroAux->Apellido);


fclose(puntero);

getch();}