/**
Este codigo busca hacer la multiplicacion de una matrix 'A' de dimension m x n
con un vector 'x' de dimension n x 1 y obtener como resultado el vector 'b' de
dimension m x 1.
El objetivo de este programa es evidenciar la utilidad de los TAGs en los 
mensajes enviados a traves de MPI.
Este programa funciona bajo el esquema maestro/trabajador y donde el proceso 
cuyo identificador es el '0' se denotara como el maestro. Los trabajadores 
tendran un 'rank' diferente de '0'. Si el numero total de procesos es 'p' 
entonces el numero total de trabajadores es 'p - 1'.
Algoritmo:
1- El maestro en un ciclo hara lo siguiente
	1.a- Despachara 'p - 1' filas entre los procesos trabajadores con un TAG
	     que indique que es una tarea a procesar
	1.b- Si el numero de filas 'm' es menor que el numero de procesos 
	     'p - 1', a los 'm - ( p - 1 )' procesos se les enviara un mensaje
	     indicandoles que no hay mas tareas que procesar
	1.c- El maestro recibira los resultados de los trabajadores ocupados y enviara 
	     un mensaje a estos que respondieron con un mensaje con un TAG que dice 
	     que no habra mas que procesar
*/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXFILAS 23
#define MAXCOL 5
#define TAGTAREA 1 // marca que denota que el mensaje es una tarea
#define TAGPARAR 2 // marca que denota que ya no hay mas mensajes por enviar
#define TAGVECTOR 4 // marca para enviar vector
#define TAGRESULT 8


/**
Funcion usada por el maestro y se encargara de enviar el vector 'b' de longitud 
NUMFILAS a todos los procesos en esta ejecucion. Recordar que el maestro no procesara
datos.
**/
int sendvector(int, int, int*);
/**
Funcion usada por el maestro y se encargara de pasar una matrix de tamano NUMFILAS x 
NUMCOL y el vector donde residira el resultado, asi como el numero de procesos que se 
pueden usar para procesar los datos
**/
int distributereceive(int, int, int*, int*, int);
/**
Funcion usada por el trabajador y quien se encarga de recibir filas de parte del 
maestro. El trabajador puede recibir o una fila o un mensaje que le indique no hay mas
tareas a procesar.
Para ello el trabajador debe recibir mensajes con cualquier TAG, MPI_ANY_TAG, y despues
validar, en la variable 'status' que tipo de mensaje es, si TAGVECTOR o TAGPARAR.
**/
int receive(int);
/**
Funcion usada para imprimir una matrix de tamano MAXFILAS x MAXCOL.
**/
int printmatrix(int, int, int*);
/**
Funcion usada para imprimir un vector.
**/
int printvector(int, int*);

/**
Funcion 'main'
**/
int main(int argc, char **argv) {
	int matrix[MAXFILAS * MAXCOL];
	int b[MAXCOL];
	int x[MAXFILAS];
	int rank;
	int size;
	int i,j;

	/* Inicializacion de 'matrix' y vector 'b' */
	for (i = 0; i < MAXFILAS; i++) 
		for (j = 0; j < MAXCOL; j++) {
			matrix[i*MAXCOL + j] = i*MAXCOL + j;
			if (i == 0) 
				b[j] = i*MAXCOL + j;
		}

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	MPI_Status Stat;//stat editado
	
	/** 
	El maestro 
		- imprime por pantalla la matrix y el vector
		- envia el vector 'b' a todos los trabajadores
		- envia cada una de las filas de la matrix a los trabajadores
		- imprime el resultado 
	*/
	if (rank == 0) {
		printf("MATRIX\n");
		printmatrix(MAXFILAS, MAXCOL, matrix);
		printf("VECTOR\n");
		printvector(MAXCOL, b);
		sendvector(MAXCOL, size, b);
		distributereceive(MAXFILAS, MAXCOL, matrix, x, size);
		printf("Vector resultado\n");
		printvector(MAXFILAS, x);

		if (Stat.MPI_TAG != TAGPARAR){
			printf("no hay mas filas a procesar\n");
		}

	} else { // el trabajador espera por filas que procesar
		receive(rank);
	}

	MPI_Finalize();
}

/**
Este metodo se encarga de enviar el vector 'v' a cada uno de los procesos.
*/
int sendvector(int dim, int size, int *v) {
	int i;
	for (i = 1; i < size; i++) // Enviar vector 'b' a todos
		MPI_Send(v,dim,MPI_INT,i,TAGVECTOR,MPI_COMM_WORLD);
	return 0;
}

/**
Metodo usado por el maestro para distribuir las filas
*/
int distributereceive(int f, int c, int *m, int *x, int size) {
	/*
	Mientras haya filas de la matrix que procesar, envie a los procesos. Siga
	enviando filas hasta que se recorran todas las filas de la matrix. 
	Reciba los resultados de los procesos, cada proceso debe devolver un valor
	entero resultado de multiplicar una fila de la matrix por el vector 'b'.
	Si no hay mas filas que procesar enviar un mensaje a los procesos para que 
	detengan su ejecucion.
	*/

/*	Mientras haya filas de la matrix que procesar, envie a los procesos. Siga
	enviando filas hasta que se recorran todas las filas de la matrix. */	
	int procesoActual = 1;
	int i;
	int k;
	for(i=0; i<=MAXFILAS; i++){
		int array[MAXCOL];
		for(k=0; k<MAXCOL; k++){
			array[k]=m[k+(MAXCOL*i)];
		}
		MPI_Send(array, MAXCOL, MPI_INT, procesoActual, TAGTAREA, MPI_COMM_WORLD);
		procesoActual++;
		if (procesoActual >= size)
		{
			procesoActual = 1;
		}		
	}

 /*Recibe los resultados de los procesos, cada proceso debe devolver un valor
	entero resultado de multiplicar una fila de la matrix por el vector 'b'.*/
	procesoActual = 1;
	MPI_Status Stat;
	int j;
	for (j = 0; j <= MAXFILAS; ++j)
	{
		int elemento;
		//recibe los resultados de los procesos
		MPI_Recv(&elemento, 1, MPI_INT, procesoActual, TAGRESULT, MPI_COMM_WORLD, &Stat);
		x[j] = elemento;
		procesoActual++;
		if (procesoActual >= size)
		{
			procesoActual = 1;
		}
	}

/*Si no hay mas filas que procesar envia un mensaje a los procesos para que 
	detengan su ejecucion.*/
	int l;
	for (l = 1; l < size; l++)
	{
		//detiene todos los procesos
		MPI_Send(&f, 1, MPI_INT, l, TAGPARAR, MPI_COMM_WORLD);
	}

	return 0;
}

/**
Esta funcion es usada por un proceso trabajador. El proceso recibe inicialmente
el vector 'b' el cual se usa para multiplicar las diferentes filas de la matrix
'A'. La matrix 'A' sera enviada, fila x fila, por parte del maestro y en el 
proceso trabajador recibe dicha fila y la multiplica por el b. El resultado de
dicha multiplicacion es enviada al maestro que ira guardando el resultado en 
cada posicion del vector respuesta 'x'.
*/
int receive(int rank) {
	/*
	Este metodo ejecutado por cada proceso trabajador debe 
	1- recibir el vector 'b'
	2- Recibir mensaje del maestro
		2.a- Una fila de la matrix para multiplicar por 'b' y ese valor 
		     enviarselo al nodo maestro
		2.b- Que no hay mas filas a procesar
	*/

	int b[MAXCOL];
	MPI_Status Stat;
	/*1- recibir el vector 'b'*/
	MPI_Recv(b, MAXCOL, MPI_INT, 0, TAGVECTOR, MPI_COMM_WORLD, &Stat);
	int detener = 0;
	int i;
	while(!detener){
		int fila[MAXCOL];
		/*2- Recibir mensaje del maestro*/
		MPI_Recv(fila, MAXCOL, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &Stat);
		if (Stat.MPI_TAG != TAGPARAR)
		{
			int resultado = 0;	
			for (i = 0; i < MAXCOL; ++i)
			{
				resultado += (b[i]*fila[i]);
			}
			/*2.a- Una fila de la matrix para multiplicar por 'b' y ese valor 
		     enviarselo al nodo maestro*/
			MPI_Send(&resultado, 1, MPI_INT, 0, TAGRESULT, MPI_COMM_WORLD);
		}else{
			detener = 1;
			/*2.b- Envia que no hay mas filas a procesar*/
			MPI_Send(&detener, 1, MPI_INT, 0, TAGPARAR, MPI_COMM_WORLD);
		}
		
	}

	return 0;
}

/** 
Metodo usado para imprimir los elementos de una matrix de dimensiones 'm x n'
*/
int printmatrix(int m, int n, int *mt) {
	int i, j;

	for (i = 0; i < m; i++) {
		for (j = 0; j < n; j++)
			printf("%5.1d ", *(mt+i*MAXCOL + j));
		printf("\n");
	}
	return 0;
}

/** 
Metodo usado para imprimir los elementos de un vector de longitud 'n'
*/
int printvector(int n, int *v) {
	int i;
	for (i = 0; i < n; i++)
		printf("%5.1d ", v[i]);
	printf("\n");
	return 0;
}
