#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// DADOS COMPARTILHADOS
int m1[SIZE][SIZE], m2[SIZE][SIZE], mres[SIZE][SIZE];
int l1, c1, l2, c2, lres, cres;

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

    // print both
    printf("MATRIZ 1:\n");
    for (i = 0; i < SIZE; i++)
    {
        for (j = 0; j < SIZE; j++)
        {
            printf("%d     ", m1[i][j]);
        }
        printf("\n");
    }

    printf("\nMATRIZ 2:\n");
    for (i = 0; i < SIZE; i++)
    {
        for (j = 0; j < SIZE; j++)
        {
            printf("%d     ", m2[i][j]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    initializeMatrixes();
    return 0;
}
