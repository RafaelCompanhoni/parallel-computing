#include <stdio.h>
#include "mpi.h"

#define HOST_TAG = 1;
#define BASE_MATRIX_TAG = 2;

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

/*
int multiply()
{
    int i, j, k;

    #pragma omp parallel for private(j, k) num_threads(16)
    for (i = 0; i < SIZE; i++)
    {
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
*/

main(int argc, char **argv)
{
    int my_rank;        // process identifier
    int workers_total;  // total amount of workers
    int m2[SIZE][SIZE]; // base matrix
    MPI_Status status;  // communication status
    
    // for host identification
    int processor_buffer_length = MPI_MAX_PROCESSOR_NAME;   
    char hostname[processor_buffer_length];

    // MPI initialization
    MPI_Init(&argc, &argv); 
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 
    MPI_Comm_size(MPI_COMM_WORLD, &workers_total);  
    MPI_Get_processor_name(hostname, &processor_buffer_length);  

    if (my_rank == 0)
    {
        /**************** MASTER ****************/
        printf("\nMESTRE em %s\n", hostname);

        int m1[SIZE][SIZE], mres[SIZE][SIZE];
        int i, j, worker;
        int k = 1;
        char workerHostname[processor_buffer_length];
        int linesPerWorker[workers_total];

        // determine how many lines each worker can process at a time
        for (worker=1; worker < workers_total; worker++) {
            MPI_Recv(
                &workerHostname,                
                processor_buffer_length,        
                MPI_CHAR,
                worker,
                HOST_TAG,
                MPI_COMM_WORLD,
                &status
            );

            printf("\nWorker %d can process %d lines", worker);
        }

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

        // send second matrix to slaves
        for (worker=1; worker < workers_total; worker++)
        {
            printf("\nSending to %d", worker);
            MPI_Send(
                &m2,                // initial address of send buffer (choice)
                SIZE*SIZE,          // number of elements in send buffer (nonnegative integer)
                MPI_INT,            // datatype of each send buffer element (handle)
                worker,               // rank of destination (integer)
                BASE_MATRIX_TAG,    // message tag (integer)
                MPI_COMM_WORLD      // communicator (handle)
            ); 
        }
        
        // TODO - 'spread' the first matrix rows to the workers

        /*
        
        // TODO -- assemble the results
        int processedRows = 0;
        do {
            
            MPI_Recv(&message,          // buffer onde será colocada a mensagem
                        1,              // uma unidade do dado a ser recebido 
                        MPI_INT,        // dado do tipo inteiro 
                        MPI_ANY_SOURCE, // ler mensagem de qualquer emissor 
                        MPI_ANY_TAG,    // ler mensagem de qualquer etiqueta (tag) 
                        MPI_COMM_WORLD, // comunicador padrão 
                        &status);       // estrtura com informações sobre a mensagem recebida 
        } while (processedRows < SIZE);
        */
    }
    else
    {
        /**************** WORKER ****************/
        printf("\nESCRAVO[%d] em %s\n", my_rank, hostname);

        // inform the master its current host
        MPI_Send(
            &hostname,                    // initial address of send buffer (choice)
            processor_buffer_length,      // number of elements in send buffer (nonnegative integer)
            MPI_INT,                      // datatype of each send buffer element (handle)
            HOST_TAG,                     // rank of destination (integer)
            1,                            // message tag (integer)
            MPI_COMM_WORLD                // communicator (handle)
        ); 

        // receive base matrix
        MPI_Recv(
            &m2,                // initial address of receive buffer (choice)
            SIZE*SIZE,          // maximum number of elements in receive buffer (integer)
            MPI_INT,            // datatype of each receive buffer element (handle)
            0,                  // rank of source (integer)
            BASE_MATRIX_TAG,    // message tag (integer)
            MPI_COMM_WORLD,     // communicator (handle)
            &status             // status object (Status)
        );
        printMatrix(m2);

        // receive partial matrix 
        int partial[SIZE][SIZE];


        // retorno resultado para o mestre
        // MPI_Send(&message, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
    }

    MPI_Finalize();
}