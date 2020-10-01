#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>

double calcPI(int darts);
struct timeval stop, start;
int main(int argc, char **argv)
{
    double realPI = 3.1415926535897;
    double precision = 0.0001; // Precision up to 3 decimals
    int workload = 1000;
    gettimeofday(&start, NULL);

    double pi = 0;
    double diff = 0;
    while (fabs(pi - realPI) > precision)
    {
        double currentPi = calcPI(workload);
        double currentDiff = fabs(pi - currentPi);
        if (currentDiff >= diff)
        {
            workload *= 10;
        }
        pi = currentPi;
        diff = currentDiff;
    }

    gettimeofday(&stop, NULL);
    printf("The value of pi = %10.8f\n", pi);
    printf("Real value of PI: 3.1415926535897 \n");
    long seconds = stop.tv_sec - start.tv_sec;
    long micro_seconds = stop.tv_usec - start.tv_usec;
    long total_micro_seconds = (seconds * 1000000) + abs(micro_seconds);

    printf("\nExecution time is: %10.8f miliseconds\n", (double)total_micro_seconds / 1000);
    return 0;
}

double calcPI(int darts)
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

    /* calculate pi */
    pi = 4.0 * (double)score / (double)darts;
    return (pi);
}