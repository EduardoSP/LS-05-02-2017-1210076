run: compile
	mpirun -np 6 --hostfile machinefile ./mpiex
compile:
	mpicc anillo.c -o anillo
clean:
	rm -rf mpiex
