#include "CellGrowth.h"

void removeMedicineCell(const int row, const int col);
void storeMedicineCell(std::vector<std::vector<std::set<std::pair<int, int>>>> &medicineCells, const int x, const int y, const int dirx, const int diry);
std::vector<std::vector<int>> grid;
tbb::concurrent_vector<std::string> tbbMedicineCells;
tbb::concurrent_vector<std::string> updatedMedicineCells;
std::mutex mtx;
std::mutex cell_count_mtx;


class MoveMedicineCells {
	int width;
	int height;
public:

	MoveMedicineCells(int w, int h): width(w), height(h) { }

	void operator() (const blocked_range<size_t> &r) const {
		for (size_t i = r.begin(); i != r.end(); i++)
		{
			std::string cellInfo = tbbMedicineCells[i];
			
			std::vector<std::string> tokens;
			std::stringstream ss(cellInfo);
			std::string intermediate;

			while (getline(ss, intermediate, '_')) {
				tokens.push_back(intermediate);
			}
			int r = std::stoi(tokens[0]);
			int c = std::stoi(tokens[1]);
			int dirX = std::stoi(tokens[2]);
			int dirY = std::stoi(tokens[3]);
			if (dirX == 2 || dirY == 2) { continue;  }
			mtx.lock();
			grid[r][c] = grid[r][c] > 0 ? grid[r][c] : grid[r][c] * -1;
			mtx.unlock();
			r += dirX;
			c += dirY;
			if (r >= 0 && r < width && c >= 0 && c < height) {
				mtx.lock();
				grid[r][c] = grid[r][c] > 0 ? grid[r][c] * -1 : grid[r][c];
				mtx.unlock();
				updatedMedicineCells.push_back(std::to_string(r) + "_" + std::to_string(c) + "_" + std::to_string(dirX) + "_" + std::to_string(dirY));
			}
		}
	}
};

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

void CellGrowth::execute() {
	this->currentCancerCount = 0;
	this->currentHealthyCount = 0;
	this->currentMedicineCount = 0;
	std::vector<std::vector<int>> tempGrid;
	tempGrid.resize(width, std::vector<int>(height, 0));
	updatedMedicineCells.clear();

	std::vector<std::thread> threads;
	int workload = width / THREAD_COUNT;
	for (int i = 0; i < THREAD_COUNT; i++) {
		threads.push_back(std::thread(&CellGrowth::threadWork, this, i * workload, (i + 1) * workload, std::ref(tempGrid)));
	}

	for (int i = 0; i < THREAD_COUNT; i++) {
		threads[i].join();
	}

	grid = tempGrid;
	this->currentMedicineCount = width * height - this->currentHealthyCount - this->currentCancerCount;

	parallel_for(blocked_range<size_t>(0, tbbMedicineCells.size()), MoveMedicineCells(width, height), auto_partitioner());
	tbbMedicineCells = updatedMedicineCells;
}

void CellGrowth::threadWork(int startWidth, int endWidth, std::vector<std::vector<int>> &tempGrid) {
	std::list<std::pair<int, int>> healedCancerCells;
	for (int i = startWidth; i < endWidth; i++) {
		for (int j = 0; j < height; j++) {
			int cell = grid[i][j];
			int medicineCount = 0;
			int cancerCount = 0;
			int totalCount = 0;
			for (auto &dir : dirs) {
				int nextRow = i + dir[0];
				int nextCol = j + dir[1];
				if (nextRow >= startWidth && nextRow < endWidth && nextCol >= 0 && nextCol < height) {
					int nextCell = grid[nextRow][nextCol];
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
			cell_count_mtx.lock();
			if (cell == 1 && isMedicineMajority) {
				healedCancerCells.push_back(std::make_pair(i, j));
				tempGrid[i][j] = 2;
				this->currentHealthyCount++;
				
			}
			else if (cell == 2 && isCancerMajority) {
				tempGrid[i][j] = 1;
				this->currentCancerCount++;
			}
			else {
				tempGrid[i][j] = cell;
				this->currentHealthyCount += cell == 2 ? 1 : 0;
				this->currentCancerCount += cell == 1 ? 1 : 0;
			}
			cell_count_mtx.unlock();
		}
	}
	consumeMedicineCells(startWidth, endWidth, healedCancerCells, tempGrid);
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
				removeMedicineCell(nextRow, nextCol);
				cell_count_mtx.lock();
				this->currentHealthyCount++;
				cell_count_mtx.unlock();
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
			tbbMedicineCells.push_back(this->serializeMedicineCell(nextRow, nextCol, dir[0], dir[1]));
			medicineCellCount--;
		}
	}
}

std::string CellGrowth::serializeMedicineCell(const int x, const int y, const int dirX, const int dirY) {
	return std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(dirX) + "_" + std::to_string(dirY);
}

int CellGrowth::getCell(const int x, const int y) {
	return grid[x][y];
}

void storeMedicineCell(std::vector<std::vector<std::set<std::pair<int, int>>>> &medCells, const int x, const int y, const int dirx, const int diry) {
	medCells[x][y].insert(std::make_pair(dirx, diry));
}

void removeMedicineCell(const int row, const int col) {
	int idx = -1;
	for (std::size_t i = 0; i < tbbMedicineCells.size(); i++) {
		std::string cellInfo = tbbMedicineCells[i];

		std::vector<std::string> tokens;
		std::stringstream ss(cellInfo);
		std::string intermediate;

		while (getline(ss, intermediate, '_')) {
			tokens.push_back(intermediate);
		}
		int r = std::stoi(tokens[0]);
		int c = std::stoi(tokens[1]);
		if (r == row && c == col) {
			idx = i;
			break;
		}
	}

	if (idx > -1) {
		tbbMedicineCells[idx] = "0_0_2_2";
	}
}