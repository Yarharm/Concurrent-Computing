#include "CellGrowth.h"
int grid[GAME_SIZE];
int medicineCells[GAME_SIZE];
int medCellDirX[GAME_SIZE];
int medCellDirY[GAME_SIZE];

void CleanupMoveMedicineCells(cl_context context, cl_command_queue commandQueue, cl_program program, cl_kernel kernel, cl_mem memObjects[4]) {
	clReleaseMemObject(memObjects[0]);
	clReleaseMemObject(memObjects[1]);
	clReleaseMemObject(memObjects[2]);
	clReleaseMemObject(memObjects[3]);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(commandQueue);
	clReleaseContext(context);
}

void CleanupUpdateCells(cl_context context, cl_command_queue commandQueue, cl_program program, cl_kernel kernel, cl_mem memObjects[3]) {
	clReleaseMemObject(memObjects[0]);
	clReleaseMemObject(memObjects[1]);
	clReleaseMemObject(memObjects[2]);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(commandQueue);
	clReleaseContext(context);
}

bool CreateMemObjects(cl_context context, cl_mem memObjects[4],
	int *medCells, int *cellDirX, int *cellDirY)
{
	memObjects[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * GAME_SIZE, medCells, NULL);
	memObjects[1] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * GAME_SIZE, cellDirX, NULL);
	memObjects[2] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * GAME_SIZE, cellDirY, NULL);
	memObjects[3] = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int) * GAME_SIZE, NULL, NULL);

	if (memObjects[0] == NULL || memObjects[1] == NULL ||
		memObjects[2] == NULL || memObjects[3] == NULL)
	{
		cerr << "Error creating memory objects." << endl;
		return false;
	}
	return true;
}

cl_program CreateProgram(cl_context context, cl_device_id device,
	const char* fileName)
{
	cl_int errNum;
	cl_program program;
	ifstream kernelFile(fileName, ios::in);
	if (!kernelFile.is_open())
	{
		cerr << "Failed to open file for reading: " << fileName <<
			endl;
		return NULL;
	}
	ostringstream oss;
	oss << kernelFile.rdbuf();
	string srcStdStr = oss.str();
	const char *srcStr = srcStdStr.c_str();
	program = clCreateProgramWithSource(context, 1, (const char**)&srcStr, NULL, NULL);
	if (program == NULL)
	{
		cerr << "Failed to create CL program from source." << endl;
		return NULL;
	}
	errNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (errNum != CL_SUCCESS)
	{
		// Determine the reason for the error
		char buildLog[16384];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buildLog), buildLog, NULL);
		cerr << "Error in kernel: " << endl;
		cerr << buildLog;
		clReleaseProgram(program);
		return NULL;
	}
	return program;
}

cl_command_queue CreateCommandQueue(cl_context context, cl_device_id *device)
{
	cl_int errNum;
	cl_device_id *devices;
	cl_command_queue commandQueue = NULL;
	size_t deviceBufferSize = -1;
	// First get the size of the devices buffer
	errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &deviceBufferSize);
	if (errNum != CL_SUCCESS)
	{
		cerr << "Failed get device buffer";
		return NULL;
	}
	if (deviceBufferSize <= 0)
	{
		cerr << "No devices available.";
		return NULL;
	}

	// Allocate memory for the devices buffer
	devices = new cl_device_id[deviceBufferSize / sizeof(cl_device_id)];
	errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceBufferSize, devices, NULL);
	if (errNum != CL_SUCCESS)
	{
		cerr << "Failed to get device IDs";
		return NULL;
	}
	// In this example, we just choose the first available device.
	// In a real program, you would likely use all available
	// devices or choose the highest performance device based on
	// OpenCL device queries.
	commandQueue = clCreateCommandQueue(context, devices[0], 0, NULL);
	if (commandQueue == NULL)
	{
		cerr << "Failed to create commandQueue for device 0";
		return NULL;
	}
	*device = devices[0];
	delete[] devices;
	return commandQueue;
}

cl_context CreateContext(cl_device_type deviceType)
{
	cl_int errNum;
	cl_uint numPlatforms = 0;
	cl_context context = NULL;
	// First, select an OpenCL platform to run on.
	// For this example, we simply choose the first available
	// platform. Normally, you would query for all available
	// platforms and select the most appropriate one.
	errNum = clGetPlatformIDs(0, NULL, &numPlatforms);
	if (errNum != CL_SUCCESS || numPlatforms <= 0)
	{
		cerr << "Failed to find any OpenCL platforms." << endl;
		return NULL;
	}
	
	vector<cl_platform_id> platforms(numPlatforms);
	clGetPlatformIDs(numPlatforms, &platforms[0], NULL);
	for (cl_uint i = 0; i < numPlatforms; i++) {
		cl_uint numDevices = 0;
		clGetDeviceIDs(platforms[i], deviceType, 0, NULL, &numDevices);

		if (0 != numDevices)
		{
			// Next, create an OpenCL context on the platform.
			cl_context_properties contextProperties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[i], 0 };
			context = clCreateContextFromType(contextProperties, deviceType, NULL, NULL, &errNum);
			if (errNum != CL_SUCCESS)
			{
				cout << "Could not create device context, trying CPU..." << endl;
			}
			return context;
		}
	}
}

bool liveMedicineCell(const int row, const int col);
void convertMedicineCell(const int row, const int col);
void activateMedicineCell(const int row, const int col, const int dirX, const int dirY);

CellGrowth::CellGrowth(const int w, const int h) : width(w), height(h) {
	srand(time(0));
};

void CellGrowth::init() {
	int cancerCount = width * height * 0.25;
	std::vector<int> tmp;
	for (int i = 0; i < GAME_SIZE; i++) {
		int cell = cancerCount > 0 ? 1 : 2;
		tmp.push_back(cell);
		cancerCount -= cancerCount >= 0 ? 1 : 0;
	}

	std::shuffle(tmp.begin(), tmp.end(), std::default_random_engine());

	for (int i = 0; i < GAME_SIZE; i++)
	{
		grid[i] = tmp[i];
	}
}

void CellGrowth::execute() {
	this->currentCancerCount = 0;
	this->currentHealthyCount = 0;
	this->currentMedicineCount = 0;

	// OpenCL
	cl_context context = 0;
	cl_command_queue commandQueue = 0;
	cl_device_id device = 0;
	cl_program program = 0;
	cl_kernel kernel = 0;
	cl_int errNum;

	cl_mem updateCellsMemObjects[3] = { 0, 0, 0 };
	cl_mem moveMedCellsMemObjects[4] = { 0, 0, 0, 0 };
	size_t globalWorkSize[1] = { GAME_SIZE };
	size_t localWorkSize[1] = { 1 };

	context = CreateContext(CL_DEVICE_TYPE_GPU);
	if (context == NULL)
	{
		cerr << "Failed to create OpenCL context." << endl;
		exit(1);
	}

	// Create a command-queue on the first device available on the created context
	commandQueue = CreateCommandQueue(context, &device);
	if (commandQueue == NULL)
	{
		CleanupUpdateCells(context, commandQueue, program, kernel, updateCellsMemObjects);
		exit(1);
	}

	// Create OpenCL program from UpdateCells.cl kernel source
	program = CreateProgram(context, device, "UpdateCells.cl");
	if (program == NULL)
	{
		CleanupUpdateCells(context, commandQueue, program, kernel, updateCellsMemObjects);
		exit(1);
	}

	// Create OpenCL kernel for Update Cells
	kernel = clCreateKernel(program, "update_cells_kernel", NULL);
	if (kernel == NULL)
	{
		cerr << "Failed to create kernel" << endl;
		CleanupUpdateCells(context, commandQueue, program, kernel, updateCellsMemObjects);
		exit(1);
	}

	// Create memory objects for Update Cells
	updateCellsMemObjects[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * GAME_SIZE, grid, NULL);
	updateCellsMemObjects[1] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * GAME_SIZE, medicineCells, NULL);
	updateCellsMemObjects[2] = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int) * GAME_SIZE, NULL, NULL);
	if (updateCellsMemObjects[0] == NULL || updateCellsMemObjects[1] == NULL || updateCellsMemObjects[2] == NULL)
	{
		cerr << "Error creating memory objects." << endl;
		CleanupUpdateCells(context, commandQueue, program, kernel, updateCellsMemObjects);
		exit(1);
	}

	// Set the kernel arguments
	errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &updateCellsMemObjects[0]);
	errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &updateCellsMemObjects[1]);
	errNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &updateCellsMemObjects[2]);
	errNum |= clSetKernelArg(kernel, 3, sizeof(cl_int), &GAME_WIDTH);
	errNum |= clSetKernelArg(kernel, 4, sizeof(cl_int), &GAME_HEIGHT);
	if (errNum != CL_SUCCESS)
	{
		cerr << "Error setting kernel arguments." << endl;
		CleanupUpdateCells(context, commandQueue, program, kernel, updateCellsMemObjects);
		exit(1);
	}

	// Queue the kernel up for execution across the array
	errNum = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
	if (errNum != CL_SUCCESS)
	{
		cerr << "Error queuing kernel for execution." << endl;
		CleanupUpdateCells(context, commandQueue, program, kernel, updateCellsMemObjects);
		exit(1);
	}

	int updatedGridCells[GAME_SIZE];
	// Read the output buffer back to the Host
	errNum = clEnqueueReadBuffer(commandQueue, updateCellsMemObjects[2], CL_TRUE, 0, GAME_SIZE * sizeof(int), updatedGridCells, 0, NULL, NULL);
	if (errNum != CL_SUCCESS)
	{
		cerr << "Error reading result buffer." << endl;
		CleanupUpdateCells(context, commandQueue, program, kernel, updateCellsMemObjects);
		exit(1);
	}

	// Convert medicine cells
	for (int i = 0; i < GAME_SIZE; i++) {
		int row = i / GAME_WIDTH;
		int col = i % GAME_HEIGHT;
		int cell = updatedGridCells[i];
		if (cell == -1) {
			grid[i] = 2;
			for (auto &dir : dirs) {
				int nextRow = row + dir[0];
				int nextCol = col + dir[1];
				if (nextRow >= 0 && nextRow < GAME_WIDTH && nextCol >= 0 && nextCol < GAME_HEIGHT) {
					convertMedicineCell(nextRow, nextCol);
				}
			}
		}
		else {
			grid[i] = cell;
		}
	}
	clReleaseMemObject(updateCellsMemObjects[0]);
	clReleaseMemObject(updateCellsMemObjects[1]);
	clReleaseMemObject(updateCellsMemObjects[2]);

	// Create OpenCL program from ModeMedicineCells.cl kernel source
	program = CreateProgram(context, device, "MoveMedicineCells.cl");
	if (program == NULL)
	{
		CleanupMoveMedicineCells(context, commandQueue, program, kernel, moveMedCellsMemObjects);
		exit(1);
	}

	// Create OpenCL kernel for Move Medicine Cells
	kernel = clCreateKernel(program, "move_medicine_cells_kernel", NULL);
	if (kernel == NULL)
	{
		cerr << "Failed to create kernel" << endl;
		CleanupMoveMedicineCells(context, commandQueue, program, kernel, moveMedCellsMemObjects);
		exit(1);
	}

	if (!CreateMemObjects(context, moveMedCellsMemObjects, medicineCells, medCellDirX, medCellDirY))
	{
		CleanupMoveMedicineCells(context, commandQueue, program, kernel, moveMedCellsMemObjects);
		exit(1);
	}

	// Set the kernel arguments
	errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &moveMedCellsMemObjects[0]);
	errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &moveMedCellsMemObjects[1]);
	errNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &moveMedCellsMemObjects[2]);
	errNum |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &moveMedCellsMemObjects[3]);
	errNum |= clSetKernelArg(kernel, 4, sizeof(cl_int), &GAME_WIDTH);
	errNum |= clSetKernelArg(kernel, 5, sizeof(cl_int), &GAME_HEIGHT);
	if (errNum != CL_SUCCESS)
	{
		cerr << "Error setting kernel arguments." << endl;
		CleanupMoveMedicineCells(context, commandQueue, program, kernel, moveMedCellsMemObjects);
		exit(1);
	}
	

	// Queue the kernel up for execution across the array
	errNum = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
	if (errNum != CL_SUCCESS)
	{
		cerr << "Error queuing kernel for execution." << endl;
		CleanupMoveMedicineCells(context, commandQueue, program, kernel, moveMedCellsMemObjects);
		exit(1);
	}

	int updatedMedCells[GAME_SIZE];
	// Read the output buffer back to the Host
	errNum = clEnqueueReadBuffer(commandQueue, moveMedCellsMemObjects[3], CL_TRUE, 0, GAME_SIZE * sizeof(int), updatedMedCells, 0, NULL, NULL);
	if (errNum != CL_SUCCESS)
	{
		cerr << "Error reading result buffer." << endl;
		CleanupMoveMedicineCells(context, commandQueue, program, kernel, moveMedCellsMemObjects);
		exit(1);
	}
	
	set<int> visited;
	for (int i = 0; i < GAME_SIZE; i++) {
		if (medicineCells[i] == 1 && visited.find(i) == visited.end()) {
			int nxtPos = updatedMedCells[i];
			visited.insert(nxtPos);
			if (nxtPos >= 0 && nxtPos < GAME_SIZE) {
				medicineCells[nxtPos] = 1;
				medCellDirX[nxtPos] = medCellDirX[i];
				medCellDirY[nxtPos] = medCellDirY[i];
				this->currentMedicineCount++;
			}
			
			medicineCells[i] = 0;
			medCellDirX[i] = 0;
			medCellDirY[i] = 0;
		}
		else {
			this->currentCancerCount += grid[i] == 1 ? 1 : 0;
			this->currentHealthyCount += grid[i] == 2 ? 1 : 0;
		}
		
	}
	CleanupMoveMedicineCells(context, commandQueue, program, kernel, moveMedCellsMemObjects);
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
			activateMedicineCell(nextRow, nextCol, dir[0], dir[1]);
			medicineCellCount--;
		}
	}
}

int CellGrowth::getCell(const int x, const int y) {
	return liveMedicineCell(x, y) ? -1 : grid[x * GAME_WIDTH + y];
}

bool liveMedicineCell(const int row, const int col) {
	return medicineCells[row * GAME_WIDTH + col] == 1;
}

void convertMedicineCell(const int row, const int col) {
	medicineCells[row * GAME_WIDTH + col] = 0;
}

void activateMedicineCell(const int row, const int col, const int dirX, const int dirY) {
	int pos = row * GAME_WIDTH + col;
	medicineCells[pos] = 1;
	medCellDirX[pos] = dirX;
	medCellDirY[pos] = dirY;
}