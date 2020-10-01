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
#define sqr(x) ((x) * (x))
    long random(void);
    double x_coord, y_coord, pi, r;
    int score, n;
    unsigned int cconst; /* must be 4-bytes in size */
    /*************************************************************************
 * The cconst variable must be 4 bytes. We check this and bail if it is
 * not the right size
 ************************************************************************/
    if (sizeof(cconst) != 4)
    {
        printf("Wrong data size for cconst variable in dboard routine!\n");
        printf("See comments in source file. Quitting.\n");
        exit(1);
    }
    /* 2 bit shifted to MAX_RAND later used to scale random number between 0 and 1 */
    cconst = 2 << (31 - 1);
    score = 0;

    /* "throw darts at board" */
    for (n = 1; n <= darts; n++)
    {
        /* generate random numbers for x and y coordinates */
        r = (double)random() / cconst;
        x_coord = (2.0 * r) - 1.0;
        r = (double)random() / cconst;
        y_coord = (2.0 * r) - 1.0;

        /* if dart lands in circle, increment score */
        if ((sqr(x_coord) + sqr(y_coord)) <= 1.0)
            score++;
    }

    return score;
}
