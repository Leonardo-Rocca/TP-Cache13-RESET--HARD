//  Lee el archivo binario del ejemplo anterior
//  25-Oct-2005

#include <stdio.h>
#include <stdlib.h>
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

system("cls");
if(!(fp=fopen("legajo.dat","rb")))
	{
	printf("No se puede abrir el archivo\n");
	return(1);
	}
printf("Contenido del archivo:\t");
fread(&legajo, sizeof(legajo), 1, fp);
while(!feof(fp))
	{
	printf("\n%25s%5c%5u",legajo.nombre, legajo.sexo, legajo.edad);
	fread(&legajo, sizeof(legajo), 1, fp);
	}
fclose(fp);
printf("\n\n");

return 0;
}
