#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// DADOS COMPARTILHADOS
int m1[SIZE][SIZE], m2[SIZE][SIZE], mres[SIZE][SIZE];
int l1, c1, l2, c2, lres, cres;

void printMatrix(matrix)
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
    int i, j;

    for (i = 0; i < SIZE; i++)
    {
        for (j = 0; j < SIZE; j++)
        {
            mres[i][j] = 0;
            for (k = 0; k < c1; k++)
            {
                mres[i][j] += m1[i][k] * m2[k][j];
            }
        }
    }
}

int main(int argc, char *argv[])
{
    initializeMatrixes();

    printf("MATRIZ 1:\n");
    printMatrix(m1);

    printf("MATRIZ 2:\n");
    printMatrix(m2);

    multiply();
    printf("\nRESULT:\n");
    printMatrix(mres);

    return 0;
}
