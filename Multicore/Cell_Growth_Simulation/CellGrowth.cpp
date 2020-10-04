#include "CellGrowth.h"

void storeMedicineCell(std::unordered_map<std::string, std::set<std::pair<int, int>>> &medicineCells, const int x, const int y, const int dirx, const int diry);
std::pair<int, int> deserializeCell(std::string str);
std::string serializeCell(const int x, const int y);

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
	std::vector<std::vector<int>> tempGrid;
	tempGrid.resize(width, std::vector<int>(height, 0));

	std::vector<std::thread> threads;
	int workload = width / THREAD_COUNT;
	for (int i = 0; i < THREAD_COUNT; i++) {
		threads.push_back(std::thread(&CellGrowth::threadWork, this, i * workload, (i + 1) * workload, std::ref(tempGrid)));
	}

	for (int i = 0; i < THREAD_COUNT; i++) {
		threads[i].join();
	}

	grid = tempGrid;
	moveMedicineCells();
}

void CellGrowth::threadWork(int startWidth, int endWidth, std::vector<std::vector<int>> &tempGrid) {
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
					if (nextCell < 0) { // Neighour Medicine cell
						medicineCount++;
					}
					else if (nextCell % 2 != 0) { // Neighbour Cancer cell
						cancerCount++;
					}
				}
			}
			bool isMedicineMajority = medicineCount > totalCount / 2 + 1;
			bool isCancerMajority = cancerCount > totalCount / 2 + 1;

			if (cell == 1 && isMedicineMajority) { // Modify cancer 
				healedCancerCells.push_back(std::make_pair(i, j));
				tempGrid[i][j] = 2;
			}
			else if (cell == 2 && isCancerMajority) { // Modify healthy cell
				tempGrid[i][j] = 1;
			}
			else {
				tempGrid[i][j] = cell;
			}
		}
	}
	consumeMedicineCells(startWidth, endWidth, healedCancerCells, tempGrid);
}

void CellGrowth::consumeMedicineCells(int startWidth, int endWidth, std::list<std::pair<int, int>> &healedCancerCells, std::vector<std::vector<int>> &temp) {
	for (const auto &healedCell : healedCancerCells) {
		int x = healedCell.first;
		int y = healedCell.second;
		for (auto &dir : dirs) {
			int nextRow = x + dir[0];
			int nextCol = y + dir[1];
			if (nextRow >= startWidth && nextRow < endWidth && nextCol >= 0 && nextCol < height && grid[nextRow][nextCol] < 0) {
				temp[nextRow][nextCol] = 2;
				eraseMedicineCell(nextRow, nextCol);
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
			storeMedicineCell(medicineCells, nextRow, nextCol, dir[0], dir[1]);
			medicineCellCount--;
		}
	}
}

void::CellGrowth::moveMedicineCells() {
	std::unordered_map<std::string, std::set<std::pair<int, int>>> tempMedicineCells;
	for (auto &cellInfo : medicineCells) {
		std::pair<int, int> cellPosition = deserializeCell(cellInfo.first);
		for (const auto &dir : cellInfo.second) {
			int nextRow = cellPosition.first + dir.first;
			int nextCol = cellPosition.second + dir.second;
			int currentCellValue = grid[cellPosition.first][cellPosition.second];
			grid[cellPosition.first][cellPosition.second] = currentCellValue > 0 ? currentCellValue : currentCellValue * -1;

			if (nextRow >= 0 && nextRow < width && nextCol >= 0 && nextCol < height) {
				int cell = grid[nextRow][nextCol];
				grid[nextRow][nextCol] = cell > 0 ? cell * -1 : cell;
				storeMedicineCell(tempMedicineCells, nextRow, nextCol, dir.first, dir.second);
			}
		}
	}
	medicineCells = tempMedicineCells;
}


void CellGrowth::eraseMedicineCell(const int x, const int y) {
	std::string serialPos = serializeCell(x, y);
	medicineCells.erase(serialPos);
}

void storeMedicineCell(std::unordered_map<std::string, std::set<std::pair<int, int>>> &medicineCells, const int x, const int y, const int dirx, const int diry) {
	std::string serialPos = serializeCell(x, y);
	if (medicineCells.find(serialPos) == medicineCells.end()) {
		std::set<std::pair<int, int>> tempset;
		medicineCells[serialPos] = tempset;
	}
	medicineCells[serialPos].insert(std::make_pair(dirx, diry));
}

int CellGrowth::getCell(const int x, const int y) {
	return grid[x][y];
}

std::string serializeCell(const int x, const int y) {
	return std::to_string(x) + "_" + std::to_string(y);
}

std::pair<int, int> deserializeCell(std::string str) {
	int delim = str.find("_");
	std::string s1 = str.substr(0, delim);
	std::string s2 = str.substr(delim + 1);

	std::stringstream geek1(s1);
	std::stringstream geek2(s2);
	int x = 0;
	int y = 0;
	geek1 >> x;
	geek2 >> y;
	return std::make_pair(x, y);
}