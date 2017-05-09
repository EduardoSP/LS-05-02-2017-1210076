#include <stdio.h>
#include "mpi.h"

int main(int argc,char *argv[]){
	int size, rank, N, total=0,tag=1,i,totalt,cs;
	MPI_Status Stat;
	N = 10000;

	MPI_Init(&argc,&argv);

	MPI_Comm_size(MPI_COMM_WORLD, &size);
    cs = N/size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	for(i = (cs*rank)+1; i<= cs*(rank+1);i++){
		total += i;
	}

		if(rank==0){
			for(i=1;i<size;i++){
				MPI_Recv(&totalt, 1, MPI_INT, i, tag, MPI_COMM_WORLD, &Stat);
				total += totalt;
			
			}
            printf ("%d %d\n",rank, total);

		}else{
			
			MPI_Send(&total, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
		
		}



	MPI_Finalize();
	return 0;
}
