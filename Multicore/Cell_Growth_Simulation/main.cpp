#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"

#include "CellGrowth.h"

#include <iostream>
#include <sstream>

#define RED 1.0, 0.0, 0.0
#define GREEN 0.0, 1.0, 0.0
#define YELLOW 1.0, 1.0, 0.0

int FPS = 30;
int window_width = 600;
int window_height = 600;
int game_width = 120;
int game_height = 120;
CellGrowth *cellSimulation;

void drawBitmapText(const char *string, float x, float y, float z)
{
	const char *c;
	glRasterPos3f(x, y, z);//define position on the screen where to draw text.

	for (c = string; *c != '\0'; c++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
	}
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT);
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
			
			glVertex2f(x, y);
			glVertex2f(x + 1, y);
			glVertex2f(x + 1, y + 1);
			glVertex2f(x, y + 1);
			
		}
	}

	glEnd();
	glFlush();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();//Resets to identity Matrix.
	gluOrtho2D(0, game_width, game_height, 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor3f(0, 0, 1);

	std::string cancer = "Cancer count: " + std::to_string(cellSimulation->currentCancerCount);
	std::string healthy = "Healthy count: " + std::to_string(cellSimulation->currentHealthyCount);
	std::string medicine = "Medicine count: " + std::to_string(cellSimulation->currentMedicineCount);
	drawBitmapText(cancer.c_str(), 0, game_height * 0.1, 0);
	drawBitmapText(healthy.c_str(), 0, game_height * 0.15, 0);
	drawBitmapText(medicine.c_str(), 0, game_height * 0.20, 0);
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
	glutInitDisplayMode(GLUT_RGB);
	glutInitWindowSize(window_width, window_height);
	glutCreateWindow("COMP Cells");
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0.0, game_width, 0.0, game_height);

	glutDisplayFunc(display);
	glutMouseFunc(onClick);

	cellSimulation = new CellGrowth(game_width, game_height);
	cellSimulation->init();

	update(0);
	glutMainLoop();

	return 0;
}