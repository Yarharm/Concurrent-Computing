#include <mpi.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // Print off a hello world message
    printf("Hello world from processor %s, rank %d out of %d processors\n",
           processor_name, world_rank, world_size);

    // Finalize the MPI environment.
    MPI_Finalize();
}

double calcPI(int darts)
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
    pi = 4 * (double)score / darts;
    return pi;
}