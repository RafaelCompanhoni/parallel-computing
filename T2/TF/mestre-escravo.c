#include <stdio.h>
#include "mpi.h"

// HOST_TAG = 1
// BASE_MATRIX_TAG = 2
// PARTIAL_RESULT = 3

struct WorkerInfo 
{ 
   int workerId;
   int rowCapacity;
   int isAvailable;
}; 

void printMatrix(int rows, int columns, int matrix[rows][columns])
{
    int i, j;

    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < columns; j++)
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
    int const HOST_DISCOVERY_TAG = 1;
    int const BASE_MATRIX_TAG = 2;

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
        int i, j, workerId;
        int k = 1;
        char workerHostname[processor_buffer_length];
        struct WorkerInfo workers[workers_total];

        // determines how many lines each worker can process at a time
        for (workerId=1; workerId < workers_total; workerId++) {
            MPI_Recv(
                workerHostname,                
                processor_buffer_length,        
                MPI_CHAR,            
                workerId,              
                HOST_DISCOVERY_TAG,
                MPI_COMM_WORLD,
                &status
            );

            int lines = 2; // 16
            if(strcmp(workerHostname, hostname) == 0) {
                lines = 1; // 15, whenever the worker is at the same host as the master
            }

            workers[workerId].workerId = workerId;
            workers[workerId].rowCapacity = lines;
            workers[workerId].isAvailable = 1;
            printf("\nESCRAVO[%d] pode processar %d linhas", workerId, lines);
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

        printMatrix(SIZE, SIZE, m1);
        printMatrix(SIZE, SIZE, m2);

        // send base matrix to workers
        for (workerId=1; workerId < workers_total; workerId++)
        {
            printf("\nEnviando matriz base para ESCRAVO[%d]", workerId);
            MPI_Send(
                &m2,                
                SIZE*SIZE,          
                MPI_INT,            
                workerId,           
                BASE_MATRIX_TAG,    
                MPI_COMM_WORLD      
            ); 
        }

        // TODO - send lines and process results
        int currentRowToProcess = 0;
        do {
            // find an available worker
            int availableWorkerId = 0;
            for (workerId=1; workerId < workers_total; workerId++) {
                if (workers[workerId].isAvailable) {
                    workers[workerId].isAvailable = 0;
                    availableWorkerId = workerId;
                    break;
                }
            }

            if (availableWorkerId) {
                printf("ESCRAVO[%d] DISPONIVEL\n", availableWorkerId);
                
                int rowsToProcess = workers[availableWorkerId].rowCapacity;
                int batchToProcess[rowsToProcess][SIZE];
                int row, column;

                // get next batch of lines from 'm1'
                for (row = 0; row < rowsToProcess; row++) {
                    for (column = 0; column < SIZE; column++) {
                        batchToProcess[row][column] = m1[currentRowToProcess][column];
                    }
                    currentRowToProcess++;
                }

                // TODO - send batch to the worker
                printf("\n\nENVIANDO BATCH PARA ESCRAVO\n");
                printMatrix(rowsToProcess, SIZE, batchToProcess);

                // TODO - read the results and assemble the final matrix
                // MPI_Recv(
                //     &message,           // buffer onde será colocada a mensagem
                //     1,                  // uma unidade do dado a ser recebido 
                //     MPI_INT,            // dado do tipo inteiro 
                //     MPI_ANY_SOURCE,     // ler mensagem de qualquer emissor 
                //     3,                  // PARTIAL_RESULT 
                //     MPI_COMM_WORLD,     // comunicador padrão 
                //     &status             // estrtura com informações sobre a mensagem recebida 
                // );           
            } else {
                printf("NENHUM ESCRAVO DISPONIVEL\n");
                currentRowToProcess++;
            }
        } while (currentRowToProcess < SIZE);
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
            HOST_DISCOVERY_TAG,                            
            MPI_COMM_WORLD                
        ); 

        // receive base matrix
        MPI_Recv(
            &m2,                
            SIZE*SIZE,          
            MPI_INT,            
            0,                  
            BASE_MATRIX_TAG,    
            MPI_COMM_WORLD,     
            &status             
        );

        // TODO - receive partial matrix 
        
        // TODO - multiply

        // TODO - send results back to the master
    }

    MPI_Finalize();
}