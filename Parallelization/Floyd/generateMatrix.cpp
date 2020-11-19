#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <sstream>
#include <string.h>
#include <algorithm>
#include <bitset>
#include <cstddef>
#include <valarray>
#include <iterator>
#include <vector>
#include <limits.h>
#include <fstream>
#include <iostream>
#include <string>
using namespace std;

int main(int argc, char *argv[])
{
    srand(time(0));
    int dim = atoi(argv[1]);
    int matrix[dim][dim];
    ofstream myfile;
    myfile.open("matrix.txt");

    for (int i = 0; i < dim; i++)
    {
        for (int j = 0; j < dim; j++)
        {
            if (i == j)
            {
                myfile << "0";
            }
            else
            {
                double r = ((double)rand() / (double)RAND_MAX);
                if (islessequal(r, 0.15))
                {
                    myfile << "inf";
                }
                else
                {
                    int num = rand() % 10000 + 1;
                    myfile << num;
                }
            }
            if (j != dim - 1)
            {
                myfile << ", ";
            }
        }
        myfile << "\n";
    }
}