/**********************************************************************
 * FILE: mpi_pi_reduce.c
 * OTHER FILES: dboard.c
 * DESCRIPTION:  
 *   MPI pi Calculation Example - C Version 
 *   Collective Communication example:  
 *   This program calculates pi using a "dartboard" algorithm.  See
 *   Fox et al.(1988) Solving Problems on Concurrent Processors, vol.1
 *   page 207.  All processes contribute to the calculation, with the
 *   master averaging the values for pi. This version uses mpc_reduce to 
 *   collect results
 * AUTHOR: Blaise Barney. Adapted from Ros Leibensperger, Cornell Theory
 *   Center. Converted to MPI: George L. Gusciora, MHPCC (1/95) 
 * LAST REVISED: 06/13/13 Blaise Barney
**********************************************************************/
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

void srandom(unsigned seed);
int dboard(int darts);
struct timeval stop, start;
#define TOTAL_DARTS 100000000 /* number of throws at dartboard */
#define MASTER 0              /* task ID of master task */

int main(int argc, char *argv[])
{
   double pi;
   int taskid,   /* task ID - also used as seed number */
       numtasks, /* number of tasks */
       rc,       /* return code */
       parallelDartsInCircle,
       totalDartsInCircle;
   gettimeofday(&start, NULL);

   /* Obtain number of tasks and task ID */
   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
   MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
   printf("MPI task %d has started...\n", taskid);

   /* Set seed for random number generator equal to task ID */
   srandom(taskid);
   int parallelJobSize = TOTAL_DARTS / numtasks;
   parallelDartsInCircle = dboard(parallelJobSize);

   rc = MPI_Reduce(&parallelDartsInCircle, &totalDartsInCircle, 1, MPI_INT, MPI_SUM,
                   MASTER, MPI_COMM_WORLD);

   /* Master computes PI */
   if (taskid == MASTER)
   {
      pi = 4 * (double)totalDartsInCircle / TOTAL_DARTS;
      printf("\nCalculated value of PI: %10.8f\n", pi);
      printf("Real value of PI: 3.1415926535897\n");
   }
   gettimeofday(&stop, NULL);
   long seconds = stop.tv_sec - start.tv_sec;
   long micro_seconds = stop.tv_usec - start.tv_usec;
   long total_micro_seconds = (seconds * 1000000) + abs(micro_seconds);
   MPI_Finalize();
   if (taskid == MASTER)
   {
      printf("\nTotal Execution time is: %10.8f miliseconds\n", (double)total_micro_seconds / 1000);
   }
   return 0;
}

int dboard(int darts)
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
