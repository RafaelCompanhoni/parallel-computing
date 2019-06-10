#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char **argv)
{
    int id, p, hostsize;
    char hostname[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    MPI_Get_processor_name(hostname, &hostsize);
    printf("Processo %d/%d [%s]\n", id, p, hostname);
    fflush(stdout);
    MPI_Finalize();
    return 0;
}