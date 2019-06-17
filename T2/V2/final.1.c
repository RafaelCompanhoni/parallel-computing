#include <stdio.h>
#include "mpi.h"
#include <stdlib.h>
#include <omp.h>

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
    int my_rank;                                // process identifier
    int workers_total;                          // total amount of workers
    MPI_Status status;                          // communication status
    int base_matrix[SIZE][SIZE];                // base matrix

    // host where the master process is currently running on
    int processor_buffer_length = MPI_MAX_PROCESSOR_NAME;  
    char masterHostname[processor_buffer_length];    

    // types of messages (protocol)
    int const BASE_MATRIX_TAG = 1;
    int const REQUEST_BATCH_TAG = 2;
    int const RESPONSE_BATCH_TAG = 3;
    int const PARTIAL_RESULT_TAG = 4;
    int const STOP_CONDITION_TAG = 5;

    // MPI initialization
    MPI_Init(&argc, &argv); 
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 
    MPI_Comm_size(MPI_COMM_WORLD, &workers_total);  

    // master sends its hostname to all workers (this same BCast call is used by both master and slaves)
    if (my_rank == 0) {
        MPI_Get_processor_name(masterHostname, &processor_buffer_length); 
    }
    MPI_Bcast(&masterHostname, processor_buffer_length, MPI_CHAR, 0, MPI_COMM_WORLD);
    
    // measurement
    double start = MPI_Wtime();

    if (my_rank == 0)
    {
        /**************** MASTER ****************/
        int row, column, workerId;
        int value = 1;
        int m1[SIZE][SIZE], mres[SIZE][SIZE];

        printf("[MESTRE] - eu estou no host %s\n", masterHostname);

        // initialize matrixes
        for (row = 0; row < SIZE; row++)
        {
            for (column = 0; column < SIZE; column++)
            {
                if (value % 2 == 0)
                    m1[row][column] = -value;
                else
                    m1[row][column] = value;
            }
            value++;
        }
        
        value = 1;
        for (column = 0; column < SIZE; column++)
        {
            for (row = 0; row < SIZE; row++)
            {
                if (value % 2 == 0)
                    base_matrix[row][column] = -value;
                else
                    base_matrix[row][column] = value;
            }
            value++;
        }

        // sends the base matrix to all workers
        for (workerId = 1; workerId < workers_total; workerId++)
        {
            MPI_Send(&base_matrix, SIZE*SIZE, MPI_INT, workerId, BASE_MATRIX_TAG, MPI_COMM_WORLD); 
        }

        int currentRowToProcess = 0;
        while(currentRowToProcess < SIZE) {
            // worker request for batch
            int batchSize;
            MPI_Recv(&batchSize, 1, MPI_INT, MPI_ANY_SOURCE, REQUEST_BATCH_TAG, MPI_COMM_WORLD, &status);

            // extract batch from m1 
            // int batchToProcess[batchSize][SIZE];
            int *batchToProcess =(int*)malloc(batchSize * SIZE * sizeof(int));
            for (row = 0; row < batchSize; row++) {
                for (column = 0; column < SIZE; column++) {
                    batchToProcess[row * SIZE + row] = m1[row + currentRowToProcess][column];
                }
            }
            
            // send batch to worker
            MPI_Send(batchToProcess, batchSize*SIZE, MPI_INT, status.MPI_SOURCE, RESPONSE_BATCH_TAG, MPI_COMM_WORLD); 

            // receives partial result from worker and updates the final result
            int *partialResult =(int*)malloc(batchSize * SIZE * sizeof(int));
            MPI_Recv(partialResult, batchSize*SIZE, MPI_INT, status.MPI_SOURCE, PARTIAL_RESULT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (row = 0; row < batchSize; row++) {
                for (column = 0; column < SIZE; column++) {
                    mres[row + currentRowToProcess][column] = partialResult[row * SIZE + column];
                }
            }

            // update index for next iteration
            currentRowToProcess += batchSize;

            // send stop condition
            int stopWorker = 0;
            if (currentRowToProcess == SIZE) {
                stopWorker = 1;
            }
            MPI_Send(&stopWorker, 1, MPI_INT, status.MPI_SOURCE, STOP_CONDITION_TAG, MPI_COMM_WORLD); 

            free(batchToProcess);
            free(partialResult);
        }
        
        double end = MPI_Wtime();
        printf("[MESTRE] - encerrando. Tempo: %lf\n", (end-start));
    }
    else
    {
        /**************** WORKER ****************/
        int workerCapacity = 1;

        // gets the worker hostname and determines how many threads it can process by comparing its own hostname with the master's
        char workerHostname[processor_buffer_length];
        MPI_Get_processor_name(workerHostname, &processor_buffer_length);
        int threadCapacity = 10;
        if(strcmp(workerHostname, masterHostname) == 0) {
            threadCapacity = 10;
        }
        printf("[ESCRAVO-%d] - host %s -- capacidade: %d threads\n", my_rank, workerHostname, threadCapacity);

        // receives the base matrix
        MPI_Recv(&base_matrix, SIZE*SIZE, MPI_INT, 0, BASE_MATRIX_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // main loop: requests batches from the master no more data is returned
        int stopWorker = 0;
        // int batch_to_process[workerCapacity][SIZE]; // current batch of rows to process
        // int partialResult[workerCapacity][SIZE];    // current result
        int *batch_to_process =(int*)malloc(workerCapacity * SIZE * sizeof(int));
        int *partialResult =(int*)malloc(workerCapacity * SIZE * sizeof(int));
        while(!stopWorker) {
            // requests batch from the master
            MPI_Send(&workerCapacity, 1, MPI_INT, 0, REQUEST_BATCH_TAG, MPI_COMM_WORLD);

            // receives batch from the master
            MPI_Recv(batch_to_process, workerCapacity*SIZE, MPI_INT, 0, RESPONSE_BATCH_TAG, MPI_COMM_WORLD, &status);

            // multiply partialMatrix with base matrix 'm2'
            int i, j, k;
            #pragma omp parallel for private(j, k) num_threads(threadCapacity)
            for (i = 0; i < workerCapacity; i++)
            {
                for (j = 0; j < SIZE; j++)
                {
                    partialResult[i * SIZE + j] = 0;
                    for (k = 0; k < SIZE; k++)
                    {
                        partialResult[i * SIZE + j] +=  batch_to_process[i * SIZE + k] * base_matrix[k][j];
                    }
                }
            }

            // sends results back to the master
            MPI_Send(partialResult, workerCapacity*SIZE, MPI_INT, 0, PARTIAL_RESULT_TAG, MPI_COMM_WORLD); 

            // check if the worker should stop
            MPI_Recv(&stopWorker, 1, MPI_INT, 0, STOP_CONDITION_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (stopWorker) {
                printf("[ESCRAVO-%d] - encerrando tudo\n", my_rank);
                MPI_Abort(MPI_COMM_WORLD, 0);
            }

            free(batch_to_process);
            free(partialResult);
        }
    }

    MPI_Finalize();
}