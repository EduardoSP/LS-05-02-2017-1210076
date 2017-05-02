run: compile
	mpirun -np 4 --hostfile machinefile ./mpiex
compile:
	mpicc mpiExample2.c -o mpiex
clean:
	rm -rf mpiex
