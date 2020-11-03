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
class CellGrowth {
private:
	int width;
	int height;
	int dirs[8][2] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1} };

public:
	CellGrowth(const int w, const int h);
	void init();
	void execute();
	void injectMedicine(const int x, const int y);
	int getCell(const int x, const int y);
	int currentCancerCount;
	int currentHealthyCount;
	int currentMedicineCount;
};