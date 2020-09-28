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
int game_width = 600;
int game_height = 600;

CellGrowth *cellSimulation;

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// dynamically calculate texture size
	double xSize = 1.0 / game_width;
	double ySize = 1.0 / game_height;

	glBegin(GL_QUADS);
	for (GLint x = 0; x < game_width; ++x) {
		for (GLint y = 0; y < game_height; ++y) {
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

void reshape(int w, int h) {
	window_width = w;
	window_height = h;

	glViewport(0, 0, window_width, window_height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, 1.0, 0.0, 1.0); // OpenGL (0, 0) is in the middle map the bottom left

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glutPostRedisplay();
}

void onClick(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		cellSimulation->injectMedicine(x, y);
	}
}

void update(int value) {

	// cellSimulation->iterate();

	glutPostRedisplay();
	glutTimerFunc(1000 / FPS, update, 0);
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);

	glutInitWindowSize(window_width, window_height);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Game of Life");
	glClearColor(1, 1, 1, 1);

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);

	glutMouseFunc(onClick);

	cellSimulation = new CellGrowth(game_width, game_height);
	cellSimulation->init();

	update(0);
	glutMainLoop();

	return 0;
}