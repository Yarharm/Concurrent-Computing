#pragma once
#include "Settings.h"
#include <iostream>
#include <sstream> 
#include <cstdlib>
#include <ctime>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <unordered_map>
#include <set>
#include <list>
#include <string>
#include <thread>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <memory.h>
#include "CL\cl.h"
#include <Windows.h>
#include <fstream>

using namespace std;
extern std::vector<std::vector<int>> grid;
extern std::mutex mtx;
extern std::mutex cell_count_mtx;

class CellGrowth {
private:
	int width;
	int height;
	int dirs[8][2] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1} };
	const int THREAD_COUNT = 3;

public:
	CellGrowth(const int w, const int h);
	void init();
	void execute();
	void threadWork(int startWidth, int endWidth, std::vector<std::vector<int>> &tempGrid);
	void injectMedicine(const int x, const int y);
	void consumeMedicineCells(int startWidth, int endWidth, std::list < std::pair<int, int>> &healedCancerCells);
	int getCell(const int x, const int y);
	std::string serializeMedicineCell(const int x, const int y, const int dirX, const int dirY);
	int currentCancerCount;
	int currentHealthyCount;
	int currentMedicineCount;
};