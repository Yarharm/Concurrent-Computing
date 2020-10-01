#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <sstream>
#include <string.h>
using namespace std;

struct timeval stop, start;
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("error: missing command line arguments\n");
        return 1;
    }
    int n;
    double realPI = 3.1415926535897;
    double precision = 0.0001; // Precision up to 3 decimals
    int initWorkload = 1000;
    int n_spawns = atoi(argv[1]);

    MPI_Request requests[n_spawns];
    int workloads[n_spawns];
    double oldPi[n_spawns];
    double freshPi[n_spawns];
    double oldDiff[n_spawns];
    double freshDiff[n_spawns];
    int indexes[n_spawns];

    for (n = 0; n < n_spawns; n++)
    {
        workloads[n] = initWorkload;
        indexes[n] = n;
    }

    gettimeofday(&start, NULL);
    MPI_Init(&argc, &argv);

    int converged = 0;
    double pi;
    int available_processes = n_spawns;
    while (!converged)
    {
        for (n = 0; n < available_processes; n++)
        {
            int idx = indexes[n];
            if (fabs(freshPi[idx] - realPI) <= precision)
            {
                converged = 1;
                pi = freshPi[idx];
            }

            freshDiff[idx] = fabs(freshPi[idx] - oldPi[idx]);
            if (isgreaterequal(freshDiff[idx], oldDiff[idx]))
            {
                workloads[idx] = workloads[idx] * 10;
            }

            oldPi[idx] = freshPi[idx];
            oldDiff[idx] = freshDiff[idx];

            /* Start new Processes */
            char *spawn_args[1] = {0};
            stringstream strs;
            strs << workloads[idx];
            string tmp = strs.str();
            spawn_args[0] = const_cast<char *>(tmp.c_str());
            MPI_Comm child;
            MPI_Comm_spawn("./worker.o", spawn_args, 1, MPI_INFO_NULL, 0, MPI_COMM_WORLD, &child, MPI_ERRCODES_IGNORE);
            MPI_Irecv(&freshPi[idx], 1, MPI_DOUBLE, 0, 0, child, &requests[idx]);
        }
        if (!converged)
        {
            MPI_Waitsome(n_spawns, requests, &available_processes, indexes, MPI_STATUSES_IGNORE);
        }
    }

    printf("\nCalculated value of PI: %10.8f\n", pi);
    printf("Real value of PI: 3.1415926535897\n");

    gettimeofday(&stop, NULL);
    long seconds = stop.tv_sec - start.tv_sec;
    long micro_seconds = stop.tv_usec - start.tv_usec;
    long total_micro_seconds = (seconds * 1000000) + abs(micro_seconds);
    printf("\nTotal Execution time is: %10.8f miliseconds\n", (double)total_micro_seconds / 1000);

    MPI_Abort(MPI_COMM_WORLD, MPI_SUCCESS);
    return 0;
}