#include <stdio.h>
#include "mpi.h"

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
    int my_rank;                        // process identifier
    int workers_total;                  // total amount of workers
    int base_matrix[SIZE][SIZE];        // base matrix
    MPI_Status status;                  // communication status

    // host where the master process is currently running on
    int processor_buffer_length = MPI_MAX_PROCESSOR_NAME;  
    char masterHostname[processor_buffer_length];    

    // types of messages (protocol)
    int const BASE_MATRIX_TAG = 1;
    int const REQUEST_BATCH_TAG = 2;
    int const RESPONSE_BATCH_TAG = 3;
    int const FINISH_TAG = 4;
    int const PARTIAL_RESULT_TAG = 5;
    int const STOP_CONDITION_TAG = 6;

    // MPI initialization
    MPI_Init(&argc, &argv); 
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 
    MPI_Comm_size(MPI_COMM_WORLD, &workers_total);  

    // master sends its hostname to all workers (this same BCast call is used by both master and slaves)
    if (my_rank == 0) {
        MPI_Get_processor_name(masterHostname, &processor_buffer_length); 
    }
    MPI_Bcast(&masterHostname, processor_buffer_length, MPI_CHAR, 0, MPI_COMM_WORLD);

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
            printf("[MESTRE] - recebi pedido de batch do escravo[%d] que pode processar %d threads\n", status.MPI_SOURCE, batchSize);

            // extract batch from m1 
            int batchToProcess[batchSize][SIZE];
            for (row = 0; row < batchSize; row++) {
                for (column = 0; column < SIZE; column++) {
                    batchToProcess[row][column] = m1[row + currentRowToProcess][column];
                }
            }
            
            // send batch to worker
            MPI_Send(&batchToProcess, batchSize*SIZE, MPI_INT, status.MPI_SOURCE, RESPONSE_BATCH_TAG, MPI_COMM_WORLD); 

            // receives partial result from worker
            int partialResult[batchSize][SIZE];
            MPI_Recv(&partialResult, batchSize*SIZE, MPI_INT, status.MPI_SOURCE, PARTIAL_RESULT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("[MESTRE] - recebido resultado parcial de escravo[%d]\n", status.MPI_SOURCE);
            printMatrix(batchSize, SIZE, partialResult);

            // update index for next iteration
            currentRowToProcess += batchSize;
            printf("[MESTRE] - processado %d de %d\n", currentRowToProcess, SIZE);
        }

        printf("[MESTRE] - encerrando\n");
    }
    else
    {
        /**************** WORKER ****************/
        printf("[ESCRAVO-%d] - mestre executado no host %s\n", my_rank, masterHostname);

        // gets the worker hostname
        char workerHostname[processor_buffer_length];
        MPI_Get_processor_name(workerHostname, &processor_buffer_length);
        printf("[ESCRAVO-%d] - eu estou no host %s\n", my_rank, workerHostname);

        // determines how many threads it can process by comparing its own hostname with the master's
        int workerCapacity = 1;
        if(strcmp(workerHostname, masterHostname) == 0) {
            workerCapacity = 1;
        }
        printf("[ESCRAVO-%d] - posso processar %d threads\n", my_rank, workerCapacity);

        // receives the base matrix
        MPI_Recv(&base_matrix, SIZE*SIZE, MPI_INT, 0, BASE_MATRIX_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("[ESCRAVO-%d] - recebi a matriz base\n", my_rank);
        printMatrix(SIZE, SIZE, base_matrix);

        // main loop: requests batches from the master no more data is returned
        while(1) {
            // requests batch from the master
            printf("[ESCRAVO-%d] - requisitando batch\n", my_rank);
            MPI_Send(&workerCapacity, 1, MPI_INT, 0, REQUEST_BATCH_TAG, MPI_COMM_WORLD);

            // receives batch from the master
            int batch_to_process[workerCapacity][SIZE];
            MPI_Recv(&batch_to_process, workerCapacity*SIZE, MPI_INT, 0, RESPONSE_BATCH_TAG, MPI_COMM_WORLD, &status);
            printf("[ESCRAVO-%d] - recebido batch para processar\n", my_rank);
            printMatrix(workerCapacity, SIZE, batch_to_process);

            // multiply partialMatrix with base matrix 'm2'
            int i, j, k;
            int partialResult[workerCapacity][SIZE];
            #pragma omp parallel for private(j, k) num_threads(workerCapacity)
            for (i = 0; i < workerCapacity; i++)
            {
                for (j = 0; j < SIZE; j++)
                {
                    partialResult[i][j] = 0;
                    for (k = 0; k < SIZE; k++)
                    {
                        partialResult[i][j] += batch_to_process[i][k] * base_matrix[k][j];
                    }
                }
            }

            // sends results back to the master
            MPI_Send(&partialResult, workerCapacity*SIZE, MPI_INT, 0, PARTIAL_RESULT_TAG, MPI_COMM_WORLD); 
        }
    }

    MPI_Finalize();
}