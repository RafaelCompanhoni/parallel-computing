#include <stdio.h>
#include <string.h>
#include <mpi.h>

int main(int argc, char* argv[]) {
  int id, p;            /* Identificador e numero de processos */
  int source, dest = 0; /* Processo fonte e processo destino */
  int tag = 50;         /* Tag para as mensagens */
  char message[100];    /* Buffer para as mensagens */
  MPI_Status status;    /* Status de retorno */
  int hostsize;
  char hostname[MPI_MAX_PROCESSOR_NAME];

  MPI_Init(&argc , & argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &id);
  MPI_Comm_size(MPI_COMM_WORLD, &p);
  MPI_Get_processor_name(hostname, &hostsize);

  if (id != 0) {
     sprintf(message, "Greetings from process %d! [%s]", id, hostname);
     MPI_Send(message, strlen(message)+1, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
  }
  else {
     printf("id = 0 [%s]\n",hostname);
     for (source = 1; source < p; source++) {
         MPI_Recv(message, 100, MPI_CHAR, MPI_ANY_SOURCE, tag,MPI_COMM_WORLD, &status);
         printf("%s [received from %d]\n", message,status.MPI_SOURCE);
     }
  }
  
  MPI_Finalize();
  return 0;
}

