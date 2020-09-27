#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char *argv[])
{
    MPI_Comm child;

    MPI_Init(&argc, &argv);

    MPI_Comm_spawn("./worker.o", spawn_args, 1, MPI_INFO_NULL, 0, MPI_COMM_SELF, &child, MPI_ERRCODES_IGNORE);

    int number;
    MPI_Recv(&number, 1, MPI_INT, 0, 0, child, MPI_STATUS_IGNORE);
    printf("Received number from child spawn %d\n", number);
    MPI_Finalize();
    return 0;
}