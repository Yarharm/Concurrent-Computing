// TODO: Add OpenCL kernel code here.
__kernel void move_medicine_cells_kernel(__global int *medCells, __global int *medCellDirX, __global int *medCellDirY, __global int *updatedMedCells, const int width, const int height)
{
	int gid = get_global_id(0);
	if(medCells[gid] == 1) {
		int row = gid / width;
		int col = gid % height;
		int rowDir = medCellDirX[gid];
		int colDir = medCellDirY[gid];
		int nextRow = row + rowDir;
		int nextCol = col + colDir;

		int updatedPos = nextRow * width + nextCol;
		updatedMedCells[gid] = updatedPos;
	}
}
