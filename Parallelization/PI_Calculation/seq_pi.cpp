// Sequential PI

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>

double calcPI(int darts);
void srand(unsigned seed);
#define DARTS 50000 /* number of throws at dartboard */
#define ROUNDS 100  /* number of times "darts" is iterated */

struct timeval stop, start;
int main()
{
  double pi, avepi;
  srand(time(0));
  gettimeofday(&start, NULL);

  for (int i = 0; i < ROUNDS; i++)
  {
    pi = calcPI(DARTS);

    /* Compute average for this iteration and all iterations */
    avepi = ((avepi * i) + pi) / (i + 1);
    printf("After %8d throws, average value of pi = %10.8f\n", (DARTS * (i + 1)), avepi);
  }
  
  gettimeofday(&stop, NULL);
  printf("\nReal value of PI: 3.1415926535897 \n");
  long seconds = stop.tv_sec - start.tv_sec;
  long micro_seconds = stop.tv_usec - start.tv_usec;
  long total_micro_seconds = (seconds * 1000000) + abs(micro_seconds);
  
  printf("\nExecution time is: %10.8f miliseconds\n", (double) total_micro_seconds / 1000);
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
  pi = 4.0 * (double)score / (double)darts;
  return pi;
}
