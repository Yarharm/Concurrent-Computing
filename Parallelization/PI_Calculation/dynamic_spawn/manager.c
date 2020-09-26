#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

struct timeval stop, start;
int main(int argc, char *argv[])
{
    int n;
    int totalDarts = 10000000;
    int n_spawns = 4;
    MPI_Comm children;

    gettimeofday(&start, NULL);
    MPI_Init(&argc, &argv);

    MPI_Comm_spawn("./worker.o", MPI_ARGV_NULL, n_spawns, MPI_INFO_NULL, 0, MPI_COMM_WORLD, &children, MPI_ERRCODES_IGNORE);

    int parallelWorkload = totalDarts / n_spawns;
    MPI_Bcast(&parallelWorkload, 1, MPI_INT, MPI_ROOT, children);

    int sendbuf;
    int recvbuf[n_spawns]; // redundant for master.
    MPI_Gather(&sendbuf, 1, MPI_INT, recvbuf, 1, MPI_INT, MPI_ROOT, children);

    int totalDartsInCircle = 0;
    for (n = 0; n < n_spawns; n++)
    {
        totalDartsInCircle += recvbuf[n];
    }
    double pi = 4 * (double)totalDartsInCircle / totalDarts;
    printf("\nCalculated value of PI: %10.8f\n", pi);
    printf("Real value of PI: 3.1415926535897\n");

    gettimeofday(&stop, NULL);
    long seconds = stop.tv_sec - start.tv_sec;
    long micro_seconds = stop.tv_usec - start.tv_usec;
    long total_micro_seconds = (seconds * 1000000) + abs(micro_seconds);
    printf("\nTotal Execution time is: %10.8f miliseconds\n", (double)total_micro_seconds / 1000);

    MPI_Finalize();
    return 0;
}