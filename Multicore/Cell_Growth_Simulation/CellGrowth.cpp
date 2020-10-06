#include "CellGrowth.h"

void storeMedicineCell(std::vector<std::vector<std::set<std::pair<int, int>>>> &medicineCells, const int x, const int y, const int dirx, const int diry);

CellGrowth::CellGrowth(const int w, const int h) : width(w), height(h) {
	grid.resize(width, std::vector<int>(height, 0));
	medicineCells.resize(width, std::vector<std::set<std::pair<int, int>>>(height, std::set<std::pair<int, int>>()));
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

void CellGrowth::execute() {
	this->currentCancerCount = 0;
	this->currentHealthyCount = 0;
	this->currentMedicineCount = 0;
	std::vector<std::vector<int>> tempGrid;
	tempGrid.resize(width, std::vector<int>(height, 0));
	std::vector<std::vector<std::set<std::pair<int, int>>>> tempMedicineCells;
	tempMedicineCells.resize(width, std::vector<std::set<std::pair<int, int>>>(height, std::set<std::pair<int, int>>()));

	std::vector<std::thread> threads;
	int workload = width / THREAD_COUNT;
	for (int i = 0; i < THREAD_COUNT; i++) {
		threads.push_back(std::thread(&CellGrowth::threadWork, this, i * workload, (i + 1) * workload, std::ref(tempGrid), std::ref(tempMedicineCells)));
	}

	for (int i = 0; i < THREAD_COUNT; i++) {
		threads[i].join();
	}

	this->grid = tempGrid;
	this->medicineCells = tempMedicineCells;
	
}

void CellGrowth::threadWork(int startWidth, int endWidth, std::vector<std::vector<int>> &tempGrid, std::vector<std::vector<std::set<std::pair<int, int>>>> &tempMedicineCells) {
	std::list<std::pair<int, int>> healedCancerCells;
	for (int i = startWidth; i < endWidth; i++) {
		for (int j = 0; j < height; j++) {
			int cell = this->grid[i][j];
			int medicineCount = 0;
			int cancerCount = 0;
			int totalCount = 0;
			for (auto &dir : dirs) {
				int nextRow = i + dir[0];
				int nextCol = j + dir[1];
				if (nextRow >= startWidth && nextRow < endWidth && nextCol >= 0 && nextCol < height) {
					int nextCell = this->grid[nextRow][nextCol];
					totalCount++;
					if (nextCell < 0) {
						medicineCount++;
					}
					else if (nextCell == 1) {
						cancerCount++;
					}
				}
			}
			bool isMedicineMajority = medicineCount > totalCount / 2 + 1;
			bool isCancerMajority = cancerCount > totalCount / 2 + 1;

			if (cell == 1 && isMedicineMajority) {
				healedCancerCells.push_back(std::make_pair(i, j));
				tempGrid[i][j] = 2;
			}
			else if (cell == 2 && isCancerMajority) {
				tempGrid[i][j] = 1;
			}
			else {
				tempGrid[i][j] = cell;
			}
		}
	}
	consumeMedicineCells(startWidth, endWidth, healedCancerCells, tempGrid);
	moveMedicineCells(startWidth, endWidth, tempMedicineCells, tempGrid);
}

void CellGrowth::consumeMedicineCells(int startWidth, int endWidth, std::list<std::pair<int, int>> &healedCancerCells, std::vector<std::vector<int>> &tempGrid) {
	for (const auto &healedCell : healedCancerCells) {
		int x = healedCell.first;
		int y = healedCell.second;
		for (auto &dir : dirs) {
			int nextRow = x + dir[0];
			int nextCol = y + dir[1];
			if (nextRow >= startWidth && nextRow < endWidth && nextCol >= 0 && nextCol < height && grid[nextRow][nextCol] < 0) {
				tempGrid[nextRow][nextCol] = 2;
				medicineCells[nextRow][nextCol].clear();
			}
		}
	}
	
}

void CellGrowth::injectMedicine(const int x, const int y) {
	int medicineCellCount = rand() % 6 + 1;

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(std::begin(dirs), std::end(dirs), g);

	for (auto &dir : dirs) {
		int nextRow = x + dir[0];
		int nextCol = y + dir[1];
		
		if (nextRow >= 0 && nextRow < width && nextCol >= 0 && nextCol < height && medicineCellCount > 0) {
			int cell = grid[nextRow][nextCol];
			grid[nextRow][nextCol] = cell > 0 ? cell * -1 : cell;
			storeMedicineCell(this->medicineCells, nextRow, nextCol, dir[0], dir[1]);
			medicineCellCount--;
		}
	}
}

void::CellGrowth::moveMedicineCells(int startWidth, int endWidth, std::vector<std::vector<std::set<std::pair<int, int>>>> &tempMedicineCells, std::vector<std::vector<int>> &tempGrid) {
	for (int i = startWidth; i < endWidth; i++) {
		for (int j = 0; j < height; j++) {
			int currentCellValue = tempGrid[i][j];
			this->mtx.lock();
			this->currentCancerCount += abs(currentCellValue) == 1 ? 1 : 0;
			this->currentHealthyCount += abs(currentCellValue) == 2 ? 1 : 0;
			this->mtx.unlock();

			for (const auto &dir : medicineCells[i][j]) {
				int nextRow = i + dir.first;
				int nextCol = j + dir.second;
				tempGrid[i][j] = currentCellValue > 0 ? currentCellValue : currentCellValue * -1;
				if (nextRow >= startWidth && nextRow < endWidth && nextCol >= 0 && nextCol < height) {
					int cell = grid[nextRow][nextCol];
					this->mtx.lock();
					this->currentMedicineCount++;
					this->currentCancerCount -= cell == 1 ? 1 : 0;
					this->currentHealthyCount -= cell == 2 ? 1 : 0;
					this->mtx.unlock();
					tempGrid[nextRow][nextCol] = cell > 0 ? cell * -1 : cell;
					storeMedicineCell(tempMedicineCells, nextRow, nextCol, dir.first, dir.second);
				}
			}
		}
	}
}

int CellGrowth::getCell(const int x, const int y) {
	return grid[x][y];
}

void storeMedicineCell(std::vector<std::vector<std::set<std::pair<int, int>>>> &medCells, const int x, const int y, const int dirx, const int diry) {
	medCells[x][y].insert(std::make_pair(dirx, diry));
}