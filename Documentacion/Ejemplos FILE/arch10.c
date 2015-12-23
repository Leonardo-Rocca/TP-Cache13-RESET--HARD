//  Guarda varios registros de una estructura en un archivo binario
//  25-Oct-2005

#include <stdlib.h>
#include <stdio.h>
#include <stdio_ext.h>

struct datos{
			char nombre[21];
			char sexo;
			unsigned int edad;
			};

int main(void)
{
struct datos legajo;
FILE *fp;
int i, N;

if(!(fp=fopen("legajo.dat","w")))
	{
	printf("No se puede abrir el archivo\n");
	return(1);
	}
printf("Indique cantidad de datos que ingresara:\t");
scanf("%d",&N);
for(i=0; i<N; i++)
	{
	printf("\nIngrese nombre:\t");
	__fpurge(stdin);
	gets(legajo.nombre);
	printf("\nIngrese sexo:\t");
	legajo.sexo=getchar();
	printf("\n\nIngrese edad:\t");
	scanf("%ud",&legajo.edad);
	fwrite(&legajo, sizeof(legajo), 1, fp);
	}
fclose(fp);
printf("\n\n");
return 0;
}
