// Sequential PI

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>

int countDartsInCircle(int darts);
void srand(unsigned seed);
#define TOTAL_DARTS 10000000 /* number of throws at dartboard */

struct timeval stop, start;
int main()
{
  double pi;
  int totalDartsInCircle;
  srand(time(0));
  gettimeofday(&start, NULL);

  totalDartsInCircle = countDartsInCircle(TOTAL_DARTS);

  /* Compute average for this iteration and all iterations */
  pi = 4 * (double)totalDartsInCircle / TOTAL_DARTS;
  printf("pi = %10.8f\n", pi);

  gettimeofday(&stop, NULL);
  printf("\nReal value of PI: 3.1415926535897 \n");
  long seconds = stop.tv_sec - start.tv_sec;
  long micro_seconds = stop.tv_usec - start.tv_usec;
  long total_micro_seconds = (seconds * 1000000) + abs(micro_seconds);

  printf("\nExecution time is: %10.8f miliseconds\n", (double)total_micro_seconds / 1000);
  return 0;
}

int countDartsInCircle(int darts)
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
  return score;
}
