#include "CellGrowth.h"

CellGrowth::CellGrowth(const int w, const int h) : width(w), height(h) {
	grid.resize(width, std::vector<int>(height, 0));
	srand(time(0));
};

void CellGrowth::init() {
	int cancerCount = width * height * 0.25;
	std::vector<int> tmp;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			int cell = cancerCount > 0 ? 1 : 2;
			tmp.push_back(cell);
			cancerCount -= cancerCount >= 0 ? 1 : 0;
		}
	}

	std::shuffle(tmp.begin(), tmp.end(), std::default_random_engine());

	for (int i = 0; i < tmp.size(); i++)
	{
		int row = i / width;
		int col = i % height;
		grid[row][col] = tmp[i];
	}

}

//void CellGrowth::iterate() {
//	for (size_t i = 0; i < width; ++i) {
//		for (size_t j = 0; j < height; ++j) {
//			int neighbors = countNeighbors(i, j);
//
//			if (grid[i][j] == true) {
//				if (neighbors == 2 || neighbors == 3) {
//					// organism survives 
//					tempGrid[i][j] = true;
//				}
//				else {
//					// organism dies
//					tempGrid[i][j] = false;
//				}
//			}
//			else {
//				if (neighbors == 3) {
//					// organism is born
//					tempGrid[i][j] = true;
//				}
//				else {
//					// continues empty
//					tempGrid[i][j] = false;
//				}
//			}
//		}
//	}
//
//	bool** t = grid;
//	grid = tempGrid;
//	tempGrid = t;
//}
//
//int CellGrowth::countNeighbors(const int x, const int y) {
//	int neighbors = 0;
//
//	xdomain[0] = (x == 0 ? width - 1 : x - 1);
//	xdomain[1] = x;
//	xdomain[2] = (x == width - 1 ? 0 : x + 1);
//
//	ydomain[0] = (y == 0 ? height - 1 : y - 1);
//	ydomain[1] = y;
//	ydomain[2] = (y == height - 1 ? 0 : y + 1);
//
//	for (size_t i = 0; i < 3; ++i) {
//		for (size_t j = 0; j < 3; ++j) {
//			if (!(xdomain[i] == x && ydomain[j] == y)) {
//				if (grid[xdomain[i]][ydomain[j]]) {
//					++neighbors;
//				}
//			}
//		}
//	}
//
//	return neighbors;
//}

void CellGrowth::injectMedicine(const int x, const int y) {
	int dirs[][2] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1} };
	int medicineCellCount = rand() % 6 + 1;

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(std::begin(dirs), std::end(dirs), g);

	for (auto &dir : dirs) {
		int nextRow = x + dir[0];
		int nextCol = y + dir[1];
		if (nextRow >= 0 && nextRow < width && nextCol >= 0 && nextCol < height && medicineCellCount > 0) {
			grid[nextRow][nextCol] *= -1;
			medicineCellCount--;
		}
	}
}

int CellGrowth::getCell(const int x, const int y) {
	return grid[x][y];
}