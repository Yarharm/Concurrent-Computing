#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int countDartsInCircle(int darts);
void srand(unsigned seed);
int main(int argc, char *argv[])
{
    srand(time(0));
    MPI_Init(&argc, &argv);

    MPI_Comm parent;
    MPI_Comm_get_parent(&parent);

    int parallelWorkload;
    MPI_Bcast(&parallelWorkload, 1, MPI_INT, 0, parent);

    int sendbuf = countDartsInCircle(parallelWorkload);
    int recvbuf[0];
    MPI_Gather(&sendbuf, 1, MPI_INT, recvbuf, 1, MPI_INT, 0, parent);

    MPI_Finalize();
    return 0;
}

int countDartsInCircle(int darts)
{
    double x_coord, y_coord, pi, r, dist;
    int score, n;

    for (n = 1; n <= darts; n++)
    {
        r = (double)rand() / RAND_MAX;
        x_coord = pow(r, 2);
        r = (double)rand() / RAND_MAX;
        y_coord = pow(r, 2);
        dist = sqrt(x_coord + y_coord);
        if (dist <= 1.0)
        {
            score++;
        }
    }
    return score;
}