#pragma once

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
#include "tbb/task_scheduler_init.h"
#include "tbb/parallel_for.h"
#include "tbb/parallel_for_each.h"
#include "tbb/blocked_range.h"
#include "tbb/tick_count.h"
#include "tbb/concurrent_vector.h"

using namespace tbb;
extern std::vector<std::vector<int>> grid;
extern tbb::concurrent_vector<std::string> tbbMedicineCells;
extern tbb::concurrent_vector<std::string> updatedMedicineCells;
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
	void consumeMedicineCells(int startWidth, int endWidth, std::list < std::pair<int, int>> &healedCancerCells, std::vector<std::vector<int>> &tempGrid);
	int getCell(const int x, const int y);
	std::string serializeMedicineCell(const int x, const int y, const int dirX, const int dirY);
	int currentCancerCount;
	int currentHealthyCount;
	int currentMedicineCount;
};