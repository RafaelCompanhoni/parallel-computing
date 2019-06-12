#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>

// DADOS COMPARTILHADOS
int m1[SIZE][SIZE], m2[SIZE][SIZE], mres[SIZE][SIZE];
int l1, c1, l2, c2, lres, cres;

void printMatrix(int matrix[SIZE][SIZE])
{
    int i, j;

    for (i = 0; i < SIZE; i++)
    {
        for (j = 0; j < SIZE; j++)
        {
            printf("%d     ", matrix[i][j]);
        }
        printf("\n");
    }
}

int initializeMatrixes()
{
    int i, j;
    int k = 1;

    // initialize matrix m1
    for (i = 0; i < SIZE; i++)
    {
        for (j = 0; j < SIZE; j++)
        {
            if (k % 2 == 0)
                m1[i][j] = -k;
            else
                m1[i][j] = k;
        }
        k++;
    }

    // initialize matrix m2
    k = 1;
    for (j = 0; j < SIZE; j++)
    {
        for (i = 0; i < SIZE; i++)
        {
            if (k % 2 == 0)
                m2[i][j] = -k;
            else
                m2[i][j] = k;
        }
        k++;
    }
}

int multiply()
{
    int i, j, k;
    int th_id, nthreads;

    #pragma omp parallel for private(j, k, th_id, nthreads)
    for (i = 0; i < SIZE; i++)
    {
        th_id = omp_get_thread_num();
	    nthreads = omp_get_num_threads();
        printf("Hello World from thread %d of %d threads.\n", th_id, nthreads);
        
        for (j = 0; j < SIZE; j++)
        {
            mres[i][j] = 0;
            for (k = 0; k < SIZE; k++)
            {
                mres[i][j] += m1[i][k] * m2[k][j];
            }
        }
    }
}

int main(int argc, char *argv[])
{
    int id, p;
    double elapsed_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    if (id != 0)
    {
        MPI_Finalize();
        exit(0);
    }

    initializeMatrixes();

    elapsed_time = -MPI_Wtime();
    multiply();
    elapsed_time += MPI_Wtime();

    printf("%lf", elapsed_time);
    MPI_Finalize();
    return 0;
}
