#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// DADOS COMPARTILHADOS
int m1[SIZE][SIZE], m2[SIZE][SIZE], mres[SIZE][SIZE];
int l1, c1, l2, c2, lres, cres;

int initializeMatrixes()
{
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
    for (int row = 0; row < SIZE; row++)
    {
        for (int columns = 0; columns < SIZE; columns++)
        {
            printf("%d     ", m1[row][columns]);
        }
        printf("\n");
    }

    printf("MATRIZ 2:\n");
    for (int row = 0; row < SIZE; row++)
    {
        for (int columns = 0; columns < SIZE; columns++)
        {
            printf("%d     ", m2[row][columns]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    initializeMatrixes();
    return 0;
}
