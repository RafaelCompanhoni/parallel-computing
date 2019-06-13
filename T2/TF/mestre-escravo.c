#include <stdio.h>
#include "mpi.h"

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
    int i;
    int my_rank;       // Identificador deste processo
    int proc_n;        // Numero de processos disparados pelo usuario na linha de comando (np)
    int message;       // Buffer para as mensagens
    MPI_Status status; // estrutura que guarda o estado de retorno

    MPI_Init(&argc, &argv); // funcao que inicializa o MPI, todo o codigo paralelo estah abaixo
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // pega pega o numero do processo atual (rank)
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);  // pega informacao do numero de processos (quantidade total)

    if (my_rank == 0) // qual o meu papel: sou o mestre ou um dos escravos?
    {
        /**************** MASTER ****************/
        printf("\nMESTRE\n");

        int m1[SIZE][SIZE], m2[SIZE][SIZE], mres[SIZE][SIZE];
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

        // send second matrix to slaves
        MPI_Send(&m2, 1, MPI_INT, i, 1, MPI_COMM_WORLD); 
        
        /*
        MPI_Recv(&message,          // buffer onde será colocada a mensagem
                    1,              // uma unidade do dado a ser recebido 
                    MPI_INT,        // dado do tipo inteiro 
                    MPI_ANY_SOURCE, // ler mensagem de qualquer emissor 
                    MPI_ANY_TAG,    // ler mensagem de qualquer etiqueta (tag) 
                    MPI_COMM_WORLD, // comunicador padrão 
                    &status);       // estrtura com informações sobre a mensagem recebida 
        */
    }
    else
    {
        /**************** SLAVE ****************/
        printf("\nESCRAVO[%d]\n", my_rank);

        // receive base matrix
        MPI_Recv(&message, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
        printMatrix(message);

        // retorno resultado para o mestre
        // MPI_Send(&message, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
    }

    MPI_Finalize();
}