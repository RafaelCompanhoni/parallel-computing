/*
    Variar o número de threads criadas e analisar a saída (usar comando omp_set_num_threads(4) e 
    parâmetro num_threads(4) no pragma omp parallel

    Variar o número de threads com variáveis de sistema setenv OMP_NUM_THREADS 2 ou 
    export OMP_NUM_THREADS=2 (bash)

    Qual a vantagem desta forma?
    
    Qual a desvantagem?

    Incluir leitura do número de cores da máquina (comando omp_get_num_procs()) e imprimir 
    esta informação no programa

    
*/

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
 
 int main (int argc, char *argv[]) {
   int th_id, nthreads;

   // omp_set_num_threads(4); // disparar 4 threads pois se trata de uma maquina Quad-Core
   
   #pragma omp parallel private(th_id, nthreads) num_threads(4)
   {
     th_id = omp_get_thread_num();
	 nthreads = omp_get_num_threads();

     printf("Hello World from thread %d of %d threads.\n", th_id, nthreads);
   }

   getchar();
   return EXIT_SUCCESS;
 }

