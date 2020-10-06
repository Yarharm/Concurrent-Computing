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

class CellGrowth {
private:
	std::vector<std::vector<int>> grid;
	std::vector<std::vector<std::set<std::pair<int, int>>>> medicineCells;
	int width;
	int height;
	int dirs[8][2] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1} };
	const int THREAD_COUNT = 3;
	std::mutex mtx;

public:
	CellGrowth(const int w, const int h);
	void init();
	void execute();
	void threadWork(int startWidth, int endWidth, std::vector<std::vector<int>> &tempGrid, std::vector<std::vector<std::set<std::pair<int, int>>>> &tempMedicineCells);
	void injectMedicine(const int x, const int y);
	void consumeMedicineCells(int startWidth, int endWidth, std::list < std::pair<int, int>> &healedCancerCells, std::vector<std::vector<int>> &tempGrid);
	void moveMedicineCells(int startWidth, int endWidth, std::vector<std::vector<std::set<std::pair<int, int>>>> &tempMedicineCells, std::vector<std::vector<int>> &tempGrid);
	int getCell(const int x, const int y);

	int currentCancerCount;
	int currentHealthyCount;
	int currentMedicineCount;
};