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
#include <limits.h>
#include <fstream>
#include <iostream>
#include <string>
using namespace std;

struct timeval stop, start;
int main(int argc, char *argv[])
{
    srand(time(0));
    int n = atoi(argv[1]);
    int rank;
    int worldSize;
    int input[n][n];
    int arr[n][n]; /* for sequential floyd */
    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
        cout << "Input data: " << endl;
        /* read from file */
        vector<string> data;
        ifstream infile("matrix.txt");
        while (infile)
        {
            string s;
            if (!getline(infile, s))
                break;

            istringstream ss(s);
            vector<string> record;

            while (ss)
            {
                string s;
                if (!getline(ss, s, ','))
                    break;
                record.push_back(s);
            }
            data.insert(data.end(), record.begin(), record.end());
        }

        int dataPos = 0;
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                string vecStr = data[dataPos++];
                char *p;
                strtol(vecStr.c_str(), &p, 10);
                int val = *p != 0 ? INT_MAX : atoi(vecStr.c_str());
                input[i][j] = val;
            }
        }
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                arr[i][j] = input[i][j];
                cout << input[i][j] << " ";
            }
            cout << endl;
        }
        cout << endl;
    }
    MPI_Barrier(MPI_COMM_WORLD);

    gettimeofday(&start, NULL);
    int meshDim = (int)sqrt(worldSize);
    int processArrayDim = n / meshDim;
    int processArrayElemCount = processArrayDim * processArrayDim;
    int pRow = rank / meshDim;
    int pCol = rank % meshDim;
    int prevProcessorInColumn = (pRow - 1) * meshDim + pCol;
    int nextProcessorInColumn = (pRow + 1) * meshDim + pCol;
    int prevProcessorInRow = pRow * meshDim + (pCol - 1);
    int nextProcessorInRow = pRow * meshDim + (pCol + 1);
    ;

    /* partition matrix */
    int process = 0;
    vector<int> processDataVector;
    vector<int> processRowsInCharge;
    vector<int> processColsInCharge;
    processDataVector.resize(processArrayElemCount);
    processRowsInCharge.resize(processArrayDim);
    processColsInCharge.resize(processArrayDim);

    if (rank == 0)
    {
        for (int row = 0; row < n; row += processArrayDim)
        {
            for (int col = 0; col < n; col += processArrayDim)
            {
                vector<int> buff;
                vector<int> rowsBuff;
                vector<int> colsBuff;
                for (int i = 0; i < processArrayDim; i++)
                {
                    int nextRow = row + i;
                    rowsBuff.push_back(nextRow);
                    for (int j = 0; j < processArrayDim; j++)
                    {
                        int nextCol = col + j;
                        colsBuff.push_back(nextCol);
                        buff.push_back(input[nextRow][nextCol]);
                    }
                }
                if (process != 0)
                {
                    MPI_Send(&buff[0], processArrayElemCount, MPI_INT, process, 0, MPI_COMM_WORLD);
                    MPI_Send(&rowsBuff[0], processArrayDim, MPI_INT, process, 0, MPI_COMM_WORLD);
                    MPI_Send(&colsBuff[0], processArrayDim, MPI_INT, process, 0, MPI_COMM_WORLD);
                }
                else
                {
                    processDataVector.clear();
                    processDataVector.insert(processDataVector.begin(), buff.begin(), buff.end());
                    processRowsInCharge.insert(processRowsInCharge.end(), rowsBuff.begin(), rowsBuff.end());
                    processColsInCharge.insert(processColsInCharge.end(), colsBuff.begin(), colsBuff.end());
                }
                process++;
            }
        }
    }
    else
    {
        MPI_Recv(&processDataVector[0], processArrayElemCount, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&processRowsInCharge[0], processArrayDim, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&processColsInCharge[0], processArrayDim, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    /*
    cout << "Process with rank " << rank << ": ";
    for (int i = 0; i < processDataVector.size(); i++)
    {
        cout << processDataVector[i] << " ";
    }
    cout << endl;
    */

    /* Construct 2-D data from 1-D vector */
    int processData[processArrayDim][processArrayDim];
    for (int i = 0; i < processArrayElemCount; i++)
    {
        int row = i / processArrayDim;
        int col = i % processArrayDim;
        processData[row][col] = processDataVector[i];
    }

    /*
    cout << "2-D data for rank: " << rank << endl;
    for (int i = 0; i < processArrayDim; i++)
    {
        for (int j = 0; j < processArrayDim; j++)
        {
            cout << processData[i][j] << " ";
        }
        cout << endl;
    }
    cout << endl;
    */

    /* Start Floyd */
    bool finishedProcessingRow = false;
    bool finishedProcessingCol = false;
    for (int k = 0; k < n; k++)
    {
        vector<int> receivedRows;
        vector<int> receivedCols;
        receivedRows.resize(processArrayDim);
        receivedCols.resize(processArrayDim);

        /* send Rows */
        if (find(processRowsInCharge.begin(), processRowsInCharge.end(), k) != processRowsInCharge.end())
        {
            finishedProcessingRow = true;
            /* prepare Row */
            receivedRows.clear();
            int localRow = k % processArrayDim;
            for (int i = 0; i < processArrayDim; i++)
            {
                receivedRows.push_back(processData[localRow][i]);
            }
            /*
            cout << "Process with rank " << rank << " prepared rows to send for k => " << k << ": ";
            for (int i = 0; i < receivedRows.size(); i++)
            {
                cout << receivedRows[i] << " ";
            }
            cout << endl;
            */
            if (pRow > 0)
            {
                MPI_Send(&receivedRows[0], processArrayDim, MPI_INT, prevProcessorInColumn, 0, MPI_COMM_WORLD);
            }
            if (pRow < meshDim - 1)
            {
                MPI_Send(&receivedRows[0], processArrayDim, MPI_INT, nextProcessorInColumn, 0, MPI_COMM_WORLD);
            }
        }
        else
        {
            if (finishedProcessingRow)
            {
                MPI_Recv(&receivedRows[0], processArrayDim, MPI_INT, nextProcessorInColumn, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if (pRow > 0)
                {
                    MPI_Send(&receivedRows[0], processArrayDim, MPI_INT, prevProcessorInColumn, 0, MPI_COMM_WORLD);
                }
            }
            else
            {
                MPI_Recv(&receivedRows[0], processArrayDim, MPI_INT, prevProcessorInColumn, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if (pRow < meshDim - 1)
                {
                    MPI_Send(&receivedRows[0], processArrayDim, MPI_INT, nextProcessorInColumn, 0, MPI_COMM_WORLD);
                }
            }
        }

        /* send Cols */
        if (find(processColsInCharge.begin(), processColsInCharge.end(), k) != processColsInCharge.end())
        {
            finishedProcessingCol = true;
            /* prepare Col */
            receivedCols.clear();
            int localCol = k % processArrayDim;
            for (int i = 0; i < processArrayDim; i++)
            {
                receivedCols.push_back(processData[i][localCol]);
            }

            /*
            cout << "Process with rank " << rank << " prepared cols to send for k => " << k << ": ";
            for (int i = 0; i < receivedCols.size(); i++)
            {
                cout << receivedCols[i] << " ";
            }
            cout << endl;
            */
            if (pCol > 0)
            {
                MPI_Send(&receivedCols[0], processArrayDim, MPI_INT, prevProcessorInRow, 0, MPI_COMM_WORLD);
            }
            if (pCol < meshDim - 1)
            {
                MPI_Send(&receivedCols[0], processArrayDim, MPI_INT, nextProcessorInRow, 0, MPI_COMM_WORLD);
            }
        }
        else
        {
            if (finishedProcessingCol)
            {
                MPI_Recv(&receivedCols[0], processArrayDim, MPI_INT, nextProcessorInRow, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if (pCol > 0)
                {
                    MPI_Send(&receivedCols[0], processArrayDim, MPI_INT, prevProcessorInRow, 0, MPI_COMM_WORLD);
                }
            }
            else
            {
                MPI_Recv(&receivedCols[0], processArrayDim, MPI_INT, prevProcessorInRow, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if (pCol < meshDim - 1)
                {
                    MPI_Send(&receivedCols[0], processArrayDim, MPI_INT, nextProcessorInRow, 0, MPI_COMM_WORLD);
                }
            }
        }

        /* execute Floyd on subPortions */
        for (int i = 0; i < processArrayDim; i++)
        {
            for (int j = 0; j < processArrayDim; j++)
            {
                int kRouteCost = receivedCols[i] == INT_MAX || receivedRows[j] == INT_MAX ? INT_MAX : receivedCols[i] + receivedRows[j];
                processData[i][j] = min(processData[i][j], kRouteCost);
            }
        }
    }

    /* Gather results */
    vector<int> results;
    for (int i = 0; i < processArrayDim; i++)
    {
        for (int j = 0; j < processArrayDim; j++)
        {
            results.push_back(processData[i][j]);
        }
    }

    /*
    cout << "Process with rank " << rank << ": ";
    for (int i = 0; i < results.size(); i++)
    {
        cout << results[i] << " ";
    }
    cout << endl;
    */

    if (rank == 0)
    {
        for (int process = 1; process < worldSize; process++)
        {
            vector<int> receiveBuff;
            receiveBuff.resize(processArrayElemCount);
            MPI_Recv(&receiveBuff[0], processArrayElemCount, MPI_INT, process, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            results.insert(results.end(), receiveBuff.begin(), receiveBuff.end());
        }
    }
    else
    {
        MPI_Send(&results[0], processArrayElemCount, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    gettimeofday(&stop, NULL);

    int vecPos = 0;
    if (rank == 0)
    {
        for (int row = 0; row < n; row += processArrayDim)
        {
            for (int col = 0; col < n; col += processArrayDim)
            {
                for (int i = 0; i < processArrayDim; i++)
                {
                    int nextRow = row + i;
                    for (int j = 0; j < processArrayDim; j++)
                    {
                        int nextCol = col + j;
                        input[nextRow][nextCol] = results[vecPos++];
                    }
                }
            }
        }
    }

    if (rank == 0)
    {
        cout << "Result data: " << endl;
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                cout << input[i][j] << " ";
            }
            cout << endl;
        }
        cout << endl;

        /* Sequential Floyd */
        cout << "Sequential floyd" << endl;
        for (int k = 0; k < n; k++)
        {
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < n; j++)
                {
                    int kRouteCost = arr[i][k] == INT_MAX || arr[k][j] == INT_MAX ? INT_MAX : arr[i][k] + arr[k][j];
                    arr[i][j] = min(arr[i][j], kRouteCost);
                }
            }
        }

        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                cout << arr[i][j] << " ";
            }
            cout << endl;
        }

        long seconds = stop.tv_sec - start.tv_sec;
        long micro_seconds = stop.tv_usec - start.tv_usec;
        long total_micro_seconds = (seconds * 1000000) + abs(micro_seconds);
        printf("\nTotal Execution time is: %10.8f miliseconds\n", (double)total_micro_seconds / 1000);
    }
    MPI_Finalize();
    return 0;
}