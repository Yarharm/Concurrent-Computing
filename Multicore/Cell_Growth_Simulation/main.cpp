#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"

#include "CellGrowth.h"

#include <iostream>

#define RED 1.0, 0.0, 0.0
#define GREEN 0.0, 1.0, 0.0
#define YELLOW 1.0, 1.0, 0.0

int FPS = 30;
int window_width = 600;
int window_height = 600;
int game_width = 120;
int game_height = 120;
CellGrowth *cellSimulation;

void display() {
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();

	// dynamically calculate texture size
	double xSize = 1.0 / game_width;
	double ySize = 1.0 / game_height;

	glBegin(GL_QUADS);
	for (int x = 0; x < game_width; ++x) {
		for (int y = 0; y < game_height; ++y) {
			int cellState = cellSimulation->getCell(x, y);
			if (cellState > 0) {
				cellState == 1 ? glColor3f(RED) : glColor3f(GREEN);
			}
			else {
				glColor3f(YELLOW);
			}
			// Point must be projected on [0.0 ... 1.0] range
			glVertex2f(x*xSize, y*ySize);
			glVertex2f((x + 1)*xSize, y*ySize);
			glVertex2f((x + 1)*xSize, (y + 1)*ySize);
			glVertex2f(x*xSize, (y + 1)*ySize);
		}
	}
	glEnd();

	glFlush();
	glutSwapBuffers();
}

void onClick(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		cellSimulation->injectMedicine(x, y);
	}
}

void update(int value) {
	auto begin = std::chrono::high_resolution_clock::now();
	cellSimulation->execute();
	auto end = std::chrono::high_resolution_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
	std::cout << ms << std::endl;
	glutPostRedisplay();
	glutTimerFunc(400, update, 0);
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);

	glutInitWindowSize(window_width, window_height);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("COMP Cells");
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0.0, 1.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glutDisplayFunc(display);
	glutMouseFunc(onClick);

	cellSimulation = new CellGrowth(game_width, game_height);
	cellSimulation->init();

	update(0);
	glutMainLoop();

	return 0;
}