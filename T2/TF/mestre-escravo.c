#include <stdio.h>
#include "mpi.h"

// HOST_TAG = 1
// BASE_MATRIX_TAG = 2
// PARTIAL_RESULT = 3

struct workerInfo 
{ 
   char hostname[MPI_MAX_PROCESSOR_NAME],
   bool isAvailable
}; 

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
    int my_rank;                    // process identifier
    int workers_total;              // total amount of workers
    int m2[SIZE][SIZE];             // base matrix
    int partialResult[SIZE][SIZE];  // partial result computed by a worker
    MPI_Status status;              // communication status
    
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
        int workerInfo[workers_total];

        // determines how many lines each worker can process at a time
        for (worker=1; worker < workers_total; worker++) {
            MPI_Recv(
                workerHostname,                
                processor_buffer_length,        
                MPI_CHAR,            
                worker,              
                1, // HOST_DISCOVERY
                MPI_COMM_WORLD,
                &status
            );

            int lines = 16;
            if(strcmp(workerHostname, hostname) == 0) {
                lines = 15; // whenever the worker is at the same host as the master
            }

            workerInfo[worker] = lines;
            printf("\nESCRAVO[%d] pode processar %d linhas", worker, lines);
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

        // initialize matrix m2 (base matrix)
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

        // send base matrix to workers
        for (worker=1; worker < workers_total; worker++)
        {
            printf("\nEnviando matriz base para ESCRAVO[%d]", worker);
            MPI_Send(
                &m2,                // initial address of send buffer (choice)
                SIZE*SIZE,          // number of elements in send buffer (nonnegative integer)
                MPI_INT,            // datatype of each send buffer element (handle)
                worker,             // rank of destination (integer)
                2,                  // message tag (integer)
                MPI_COMM_WORLD      // communicator (handle)
            ); 
        }

        // TODO - send lines and process results
        /*
        int processedRows = 0;
        do {
            // 1. get next batch of lines from 'm1'
            // 2. send it to the first available worker
            // 3. read the results and assemble the final matrix

            // MPI_Recv(
            //     &message,           // buffer onde será colocada a mensagem
            //     1,                  // uma unidade do dado a ser recebido 
            //     MPI_INT,            // dado do tipo inteiro 
            //     MPI_ANY_SOURCE,     // ler mensagem de qualquer emissor 
            //     3,                  // PARTIAL_RESULT 
            //     MPI_COMM_WORLD,     // comunicador padrão 
            //     &status             // estrtura com informações sobre a mensagem recebida 
            // );           

            processedRows++;
        } while (processedRows < SIZE);
        */
    }
    else
    {
        /**************** WORKER ****************/
        printf("\nESCRAVO[%d] em %s\n", my_rank, hostname);

        // inform the master its current host
        MPI_Send(
            &hostname,                    
            processor_buffer_length,      
            MPI_CHAR,                      
            0,                     
            1,                            
            MPI_COMM_WORLD                
        ); 

        // receive base matrix
        MPI_Recv(
            &m2,                // initial address of receive buffer (choice)
            SIZE*SIZE,          // maximum number of elements in receive buffer (integer)
            MPI_INT,            // datatype of each receive buffer element (handle)
            0,                  // rank of source (integer)
            2,                  // message tag (integer)
            MPI_COMM_WORLD,     // communicator (handle)
            &status             // status object (Status)
        );

        // TODO - receive partial matrix 
        
        // TODO - multiply

        // TODO - send results back to the master
    }

    MPI_Finalize();
}