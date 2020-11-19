// TODO: Add OpenCL kernel code here.
__kernel void update_cells_kernel(__global int *grid, __global int *medCells, __global int *tempGrid, const int width, const int height)
{
	int gid = get_global_id(0);
	int row = gid / width;
	int col = gid % height;
	int dirs[8][2] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1} };

	int cell = grid[gid];
	int medicineCount = 0;
	int cancerCount = 0;
	int totalCount = 0;
	for(int dir = 0; dir < 8; dir++) {
		int nextRow = row + dirs[dir][0];
		int nextCol = col + dirs[dir][1];
		if (nextRow >= 0 && nextRow < width && nextCol >= 0 && nextCol < height) {
			int nextPos = nextRow * width + nextCol;
			int nextCell = grid[nextPos];
			totalCount++;
			if (medCells[nextPos] == 1) {
				medicineCount++;
			}
			else if (nextCell == 1) {
				cancerCount++;
			}
		}
	}

	int isMedicineMajority = 0;
	if(medicineCount > (totalCount / 2 + 1)) {
		isMedicineMajority = 1;
	}
	int isCancerMajority = 0;
	if(cancerCount > (totalCount / 2 + 1)) {
		isCancerMajority = 1;
	}

	if (cell == 1 && isMedicineMajority == 1) {
		tempGrid[gid] = -1;
	}
	else if (cell == 2 && isCancerMajority == 1) {
		tempGrid[gid] = 1;
	}
	else {
		tempGrid[gid] = cell;
	}
}
