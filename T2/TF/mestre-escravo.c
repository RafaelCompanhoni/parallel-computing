#include <stdio.h>
#include "mpi.h"

struct WorkerInfo 
{ 
   int workerId;
   int rowCapacity;
   int isAvailable;
   int batchStartIndex;
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

main(int argc, char **argv)
{
    int const HOST_DISCOVERY_TAG = 1;
    int const BASE_MATRIX_TAG = 2;
    int const PARTIAL_MATRIX_TAG = 3;
    int const WORKER_CAPACITY_TAG = 4;
    int const PARTIAL_RESULT_TAG = 5;

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

            int rows = 2; // 16
            if(strcmp(workerHostname, hostname) == 0) {
                rows = 1; // 15, whenever the worker is at the same host as the master
            }

            workers[workerId].workerId = workerId;
            workers[workerId].rowCapacity = rows;
            workers[workerId].isAvailable = 1;
            printf("\nESCRAVO[%d] pode processar %d linhas", workerId, rows);

            // informs the worker how many rows it can process
            MPI_Send(
                &rows,                
                1,          
                MPI_INT,            
                workerId,           
                WORKER_CAPACITY_TAG,    
                MPI_COMM_WORLD      
            ); 
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
        for (workerId=1; workerId < workers_total; workerId++)
        {
            printf("\nEnviando matriz base para ESCRAVO[%d]", workerId);
            MPI_Send(&m2, SIZE*SIZE, MPI_INT, workerId, BASE_MATRIX_TAG, MPI_COMM_WORLD); 
        }

        // TODO - send lines and process results
        int currentRowToProcess = 0;
        do {
            // find an available worker
            int availableWorkerId = 0;
            for (workerId=1; workerId < workers_total; workerId++) {
                if (workers[workerId].isAvailable) {
                    workers[workerId].isAvailable = 0;
                    workers[workerId].batchStartIndex = currentRowToProcess;
                    availableWorkerId = workerId;
                    break;
                }
            }

            if (availableWorkerId) {
                printf("\nESCRAVO[%d] DISPONIVEL\n", availableWorkerId);
                
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

                // send batch to the worker
                MPI_Send(&batchToProcess, rowsToProcess*SIZE, MPI_INT, availableWorkerId, PARTIAL_MATRIX_TAG, MPI_COMM_WORLD); 

                // get the results and flag the worker as available again
                int partialResults[rowsToProcess][SIZE];
                MPI_Recv(&partialResults, rowsToProcess*SIZE, MPI_INT, availableWorkerId, PARTIAL_RESULT_TAG, MPI_COMM_WORLD, &status);
                printf("\nRESULTADO PARCIAL RECEBIDO DE ESCRAVO[%d]\n", status.MPI_SOURCE);
                printMatrix(rowsToProcess, SIZE, partialResults);
                workers[status.MPI_SOURCE].isAvailable = 1;

                // TODO - update final matrix
                int offset = workers[workerId].batchStartIndex;
                int partialResultRow, partialResultColumn;
                for (partialResultRow = 0; partialResultRow < rowsToProcess; partialResultRow++) {
                    for (partialResultColumn = 0; partialResultColumn < SIZE; partialResultColumn++) {
                        mres[partialResultRow + offset][partialResultColumn] = partialResults[partialResultRow][partialResultColumn];
                    }
                }

                printf("\nCURRENT FINAL RESULT\n");
                printMatrix(SIZE, SIZE, mres);
            }
        } while (currentRowToProcess < SIZE);

        printf("\nFINISHED\n");
        printMatrix(SIZE, SIZE, mres);
    }
    else
    {
        /**************** WORKER ****************/

        int currentCapacity;

        // inform the master its current host
        MPI_Send(&hostname, processor_buffer_length, MPI_CHAR, 0, HOST_DISCOVERY_TAG, MPI_COMM_WORLD);

        // receive current capacity (determined by the master)
        MPI_Recv(&currentCapacity, 1, MPI_INT, 0, WORKER_CAPACITY_TAG, MPI_COMM_WORLD, &status);

        // receive base matrix
        MPI_Recv(&m2, SIZE*SIZE, MPI_INT, 0, BASE_MATRIX_TAG, MPI_COMM_WORLD, &status);

        // receive batch of rows to process
        int partialMatrix[currentCapacity][SIZE];
        MPI_Recv(&partialMatrix, currentCapacity*SIZE, MPI_INT, 0, PARTIAL_MATRIX_TAG, MPI_COMM_WORLD, &status);
        printf("\n*** BATCH RECEBIDO DO MESTRE\n");
        printMatrix(currentCapacity, SIZE, partialMatrix);
        
        // multiply partialMatrix with base matrix 'm2'
        int i, j, k;
        int partialResult[currentCapacity][SIZE];
        #pragma omp parallel for private(j, k) num_threads(currentCapacity)
        for (i = 0; i < currentCapacity; i++)
        {
            for (j = 0; j < SIZE; j++)
            {
                partialResult[i][j] = 0;
                for (k = 0; k < SIZE; k++)
                {
                    partialResult[i][j] += partialMatrix[i][k] * m2[k][j];
                }
            }
        }

        // TODO - send results back to the master
        MPI_Send(&partialResult, currentCapacity*SIZE, MPI_INT, 0, PARTIAL_RESULT_TAG, MPI_COMM_WORLD);
    }

    MPI_Finalize();
}