#include "mpi.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    MPI_Comm parent;
    MPI_Comm_get_parent(&parent);

    char *a = argv[1];
    int num = atoi(a);
    MPI_Send(&num, 1, MPI_INT, 0, 0, parent);

    MPI_Finalize();
    return 0;
}