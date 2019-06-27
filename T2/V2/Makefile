all: final

final:	final.c
		@mpicc -fopenmp final.c -o final -DSIZE=800

clean:
		@rm -f final
