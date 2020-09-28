#pragma once

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <random>
#include <algorithm>

class CellGrowth {
private:
	std::vector<std::vector<int>> grid;
	int width;
	int height;

	int countNeighbors(const int, const int);
public:
	CellGrowth(const int w, const int h);
	void init();
	void iterate();
	int getCell(const int x, const int y);
	void injectMedicine(const int x, const int y);
};