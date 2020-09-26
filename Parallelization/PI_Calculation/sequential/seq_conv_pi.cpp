// Sequential PI

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

double calcPI(int darts);
struct timeval stop, start;
int main(int argc, char **argv)
{
    double pi = 0.0;
    double realPI = 3.1415926535897;
    double precision = 0.0001; // Precision up to 2 decimals

    gettimeofday(&start, NULL);

    double runningVal = 1.0;
    int sign = 1;
    while (fabs(4 * pi - realPI) > precision)
    {
        pi += (sign * 1 / runningVal);
        runningVal += 2;
        sign *= -1;
    }
    pi *= 4;

    gettimeofday(&stop, NULL);
    printf("The value of pi = %10.8f\n", pi);
    printf("Real value of PI: 3.1415926535897 \n");
    long seconds = stop.tv_sec - start.tv_sec;
    long micro_seconds = stop.tv_usec - start.tv_usec;
    long total_micro_seconds = (seconds * 1000000) + abs(micro_seconds);

    printf("\nExecution time is: %10.8f miliseconds\n", (double)total_micro_seconds / 1000);
    return 0;
}