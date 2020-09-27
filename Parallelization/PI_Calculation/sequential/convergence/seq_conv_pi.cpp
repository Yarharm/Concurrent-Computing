// Sequential PI

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

double calcPI(int darts);
struct timeval stop, start;
int main(int argc, char **argv)
{
    double realPI = 3.1415926535897;
    double precision = 0.0001; // Precision up to 2 decimals
    int workload = 100000;
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
    double x_coord, y_coord, pi, r, dist;
    int score, n;

    for (int n = 1; n <= darts; n++)
    {
        /* generate random numbers for x and y coordinates */
        r = (double)rand() / RAND_MAX;
        x_coord = pow(r, 2);
        r = (double)rand() / RAND_MAX;
        y_coord = pow(r, 2);

        dist = sqrt(x_coord + y_coord);
        /* if dart lands in circle, increment score */
        if (dist <= 1.0)
        {
            score++;
        }
    }
    pi = 4 * (double)score / darts;
    return pi;
}