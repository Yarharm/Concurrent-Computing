#include "mpi.h"
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
using namespace std;

void srand(unsigned seed);
void generateInput(vector<int> &nums, int inputSize);
void partition(vector<int> &nums, vector<int> &lowerBuff, vector<int> &upperBuff, int pivot);
int getPartnerRank(int rank, int dimension, int i);
void serialQuickSort(vector<int> &nums, int l, int r);
bool checkIfSorted(vector<int> &nums);

struct timeval stop, start;
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("error: missing command line arguments\n");
        return 1;
    }
    srand(time(0));
    int rank;
    int worldSize;
    vector<int> input;
    int inputSize = atoi(argv[1]);
    generateInput(input, inputSize);

    gettimeofday(&start, NULL);
    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    const int dimension = (int)log2(worldSize);
    int processChunk = inputSize / worldSize;
    int processReminder = inputSize % worldSize;
    vector<int> sequence;

    /* Distribute chunks */
    if (rank == 0)
    {
        cout << "Execute parallel quick sort with " << worldSize << " processes and dimension " << dimension << "\n"
             << endl;
        for (int process = 1; process < worldSize; process++)
        {
            vector<int> tmpBuffer;
            int start = (process - 1) * processChunk;
            int end = start + processChunk;
            tmpBuffer.insert(tmpBuffer.end(), input.begin() + start, input.begin() + end);

            int tmpBufferSize = tmpBuffer.size();
            MPI_Send(&tmpBufferSize, 1, MPI_INT, process, 0, MPI_COMM_WORLD);
            MPI_Send(&tmpBuffer[0], tmpBufferSize, MPI_INT, process, 0, MPI_COMM_WORLD);
        }
        int start = inputSize - processChunk - processReminder;
        sequence.insert(sequence.end(), input.begin() + start, input.end());
    }
    else
    {
        int tmpBuffSize;
        MPI_Recv(&tmpBuffSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        sequence.resize(tmpBuffSize);
        MPI_Recv(&sequence[0], tmpBuffSize, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    int sequenceSize = sequence.size();

    for (int i = 0; i < dimension; i++)
    {
        vector<int> buff;
        vector<int> lowerValsContainer;
        vector<int> higherValsContainer;
        int pivot;
        MPI_Comm groupComm;
        int groupSize = worldSize / ((int)pow(2, i));
        int color = -1;

        for (int process = 0; process < worldSize; process++)
        {
            if (process % groupSize == 0)
            {
                color++;
            }
            if (process == rank)
            {
                MPI_Comm_split(MPI_COMM_WORLD, color, rank, &groupComm);
            }
        }

        if (rank % groupSize == 0)
        {
            int pivotPos = rand() % sequenceSize;
            pivot = sequence[pivotPos];
        }
        MPI_Bcast(&pivot, 1, MPI_INT, 0, groupComm);
        cout << "Process with rank " << rank << " works on pivot " << pivot << " during dimension " << i << endl;

        /* partition */
        partition(sequence, lowerValsContainer, higherValsContainer, pivot);
        int partnerRank = getPartnerRank(rank, dimension, i);

        if (rank < partnerRank)
        {
            /* Send higher vals to 1 bit */
            int higherValsCount = higherValsContainer.size();
            MPI_Send(&higherValsCount, 1, MPI_INT, partnerRank, 0, MPI_COMM_WORLD);
            MPI_Send(&higherValsContainer[0], higherValsCount, MPI_INT, partnerRank, 0, MPI_COMM_WORLD);

            /* Receive lower vals from 1 bit */
            int lowerCountReceived;
            MPI_Recv(&lowerCountReceived, 1, MPI_INT, partnerRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            buff.resize(lowerCountReceived);
            MPI_Recv(&buff[0], lowerCountReceived, MPI_INT, partnerRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            /* Combine all lower values */
            buff.insert(buff.end(), lowerValsContainer.begin(), lowerValsContainer.end());
        }
        else
        {
            /* Receive higher vals from 0 bit */
            int higherCountReceived;
            MPI_Recv(&higherCountReceived, 1, MPI_INT, partnerRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            buff.resize(higherCountReceived);
            MPI_Recv(&buff[0], higherCountReceived, MPI_INT, partnerRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            /* Send lower vals to 0 bit */
            int lowerValsCount = lowerValsContainer.size();
            MPI_Send(&lowerValsCount, 1, MPI_INT, partnerRank, 0, MPI_COMM_WORLD);
            MPI_Send(&lowerValsContainer[0], lowerValsCount, MPI_INT, partnerRank, 0, MPI_COMM_WORLD);

            /* Combine all higher vals */
            buff.insert(buff.end(), higherValsContainer.begin(), higherValsContainer.end());
        }
        sequence = buff;
        sequenceSize = sequence.size();
    }
    cout << "Rank " << rank << " finished with input size " << sequenceSize << endl;
    serialQuickSort(sequence, 0, sequenceSize - 1);

    /* Collect results */
    if (rank == 0)
    {
        input.clear();
        input.insert(input.end(), sequence.begin(), sequence.end());

        for (int process = 1; process < worldSize; process++)
        {
            int receiveBufferSize;
            MPI_Recv(&receiveBufferSize, 1, MPI_INT, process, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            vector<int> receiveBuff;
            receiveBuff.resize(receiveBufferSize);
            MPI_Recv(&receiveBuff[0], receiveBufferSize, MPI_INT, process, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            input.insert(input.end(), receiveBuff.begin(), receiveBuff.end());
        }
    }
    else
    {
        MPI_Send(&sequenceSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(&sequence[0], sequenceSize, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    gettimeofday(&stop, NULL);

    if (rank == 0)
    {
        if (checkIfSorted(input))
        {
            cout << "\nSuccessfully sorted array" << endl;
        }
        else
        {
            cout << "\nFailed to sort the array" << endl;
        }

        for (int i = 0; i < inputSize; i++)
        {
            cout << input[i] << " ";
        }
        cout << endl;

        long seconds = stop.tv_sec - start.tv_sec;
        long micro_seconds = stop.tv_usec - start.tv_usec;
        long total_micro_seconds = (seconds * 1000000) + abs(micro_seconds);
        printf("\nTotal Execution time is: %10.8f miliseconds\n", (double)total_micro_seconds / 1000);
    }

    MPI_Finalize();
    return 0;
}

int getPartnerRank(int rank, int dimension, int i)
{
    int switchBit = 1 << (dimension - i - 1);
    int partner = rank ^ switchBit;
    return partner;
}

void partition(vector<int> &nums, vector<int> &lowerBuff, vector<int> &upperBuff, int pivot)
{
    for (int i = 0; i < nums.size(); i++)
    {
        int val = nums[i];
        if (val <= pivot)
        {
            lowerBuff.push_back(val);
        }
        else
        {
            upperBuff.push_back(val);
        }
    }
}

void serialQuickSort(vector<int> &nums, int l, int r)
{
    if (l >= r)
    {
        return;
    }
    int i = l;
    int pivot = nums[r];
    for (int j = l; j <= r - 1; j++)
    {
        if (nums[j] <= pivot)
        {
            swap(nums[i++], nums[j]);
        }
    }
    swap(nums[i], nums[r]);
    serialQuickSort(nums, l, i - 1);
    serialQuickSort(nums, i + 1, r);
}

void generateInput(vector<int> &nums, int inputSize)
{
    int max = inputSize;
    int min = 1;
    for (int i = 0; i < inputSize; i++)
    {
        int randNum = rand() % (max - min + 1) + min;
        nums.push_back(randNum);
    }
}

bool checkIfSorted(vector<int> &nums)
{
    for (int i = 1; i < nums.size(); i++)
    {
        if (nums[i - 1] > nums[i])
        {
            return false;
        }
    }
    return true;
}