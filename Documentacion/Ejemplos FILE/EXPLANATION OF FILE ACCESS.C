#include<stdio.h>

#include<stdlib.h>


//defino el tipo de dato "struct alumno", que tendra en su interior un char, un float, un int y un array de 5 chares

struct alumno	{

	char a;
	float b;
	int c;
	char d[5];

		};



int main(){

  int a;

struct alumno principal,secundaria;		//declaro dos variables del tipo "struct alumno"
FILE *puntero;					//declaro un puntero a un tipo de dato llamado "FILE"


principal.a = 'R';				//lleno la primera de las estructuras declaradas con datos cualesquiera
principal.b = 3.54;
principal.c = 22125;
principal.d[0] = 'H';
principal.d[1] = 'o';
principal.d[2] = 'l';
principal.d[3] = 'a';
principal.d[4] = 0;				//recordar que en una string, el ultimo caracter tiene que ser un NULL (= 0)


//ahora abro un archivo llamado "hola.txt" en el directorio "C:\", en modo "wb", o sea Write Binary(Escribir Binario). Debe ponerse "\\" para que la maquina lo tome como "\"
//ese archivo esta siendo apuntado por un puntero a un tipo de dato "FILE" llmado "puntero" que declaré hace un rato.
puntero=fopen("C:\\hola.txt","wb");


//ahora escribo con la función fwrite, leo desde la dirección de la esctructura, tantos bytes como el tamaño de la misma, 1 por decreto, y envío lo leído al puntero.
fwrite(&principal, sizeof(principal), 1,puntero);

//cierro la conexión entre el puntero y ese archivo
fclose(puntero);

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

//ahora abro de nuevo ese archivo con el mismo puntero apuntandolo, pero en modo lectura binaria (Read Binary).


puntero=fopen("C:\\hola.txt","rb");

//con fread leo: grabo lo leido en la dirección de secundaria, tantos bytes como el tamaño de la misma, 1 por decreto, y leo desde puntero, o sea, el archivo.
fread(&secundaria, sizeof(secundaria), 1,puntero);

fclose(puntero);				//luego cierro la conexión


//ahora imprimo en pantalla el contenido de la estructura secundaria, para ver si quedó igual a la inicial que grabé y leí.

printf("%c\n",secundaria.a);
printf("%f\n",secundaria.b);
printf("%d\n",secundaria.c);
printf("%s\n",secundaria.d);

a=getchar();

return 0;
}