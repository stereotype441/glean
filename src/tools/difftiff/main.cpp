// BEGIN_COPYRIGHT
// 
// Copyright (C) 1999  Allen Akin   All Rights Reserved.
// 
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the
// Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
// KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ALLEN AKIN BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
// OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
// 
// END_COPYRIGHT




#include <stdlib.h>
#include <iostream>
#if(__AGL__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cmath>		// for fabs
#include "image.h"

using namespace std;


enum {
	MENU_IMAGE1,
	MENU_IMAGE2,
	MENU_DIFF,
	MENU_DIFF_ON_1,
	MENU_DIFF_ON_2,
	MENU_QUIT
};

enum {
	SHOW_IMAGE1,
	SHOW_IMAGE2,
	SHOW_DIFF,
	SHOW_DIFF_ON_1,
	SHOW_DIFF_ON_2
} Mode = SHOW_DIFF_ON_1;

enum {
	DIFF_COLOR_MENU_WHITE,
	DIFF_COLOR_MENU_BLACK,
	DIFF_COLOR_MENU_RED,
	DIFF_COLOR_MENU_GREEN,
	DIFF_COLOR_MENU_BLUE,
	DIFF_COLOR_MENU_YELLOW,
	DIFF_COLOR_MENU_MAGENTA,
	DIFF_COLOR_MENU_CYAN
};

void ComputeDifference();
void DiffColorMenu(int item);
void Draw();
void Keypress(unsigned char key, int x, int y);
void Menu(int item);
void Reshape(int width, int height);
void ScreenOrthoView(int w, int h);
void ThreshMenu(int item);


char* ApplicationName;
int WindowHeight;
int WindowWidth;
GLEAN::Image Image1;
GLEAN::Image Image2;
GLEAN::Image Diff;
double Threshold = 0.0 / 256.0;
unsigned char DiffColor[] = {255, 0, 255, 255};


///////////////////////////////////////////////////////////////////////////////
// ComputeDifference - generate the difference image
///////////////////////////////////////////////////////////////////////////////
void
ComputeDifference() {
	double* rgba1 = new double[4 * Image1.width()];
	char* row1 = Image1.pixels();
	double* rgba2 = new double[4 * Image2.width()];
	char* row2 = Image2.pixels();
	unsigned char* rowD = reinterpret_cast<unsigned char*>(Diff.pixels());

	for (GLsizei i = 0; i < Diff.height(); ++i) {
		Image1.unpack(Image1.width(), rgba1, row1);
		Image2.unpack(Image2.width(), rgba2, row2);

		double* p1 = rgba1;
		double* p2 = rgba2;
		unsigned char* pD = rowD;

		for (GLsizei j = 0; j < Diff.width(); ++j) {
			if (fabs(p1[0] - p2[0]) > Threshold
			 || fabs(p1[1] - p2[1]) > Threshold
			 || fabs(p1[2] - p2[2]) > Threshold
			 || fabs(p1[3] - p2[3]) > Threshold) {
				pD[0] = DiffColor[0];
				pD[1] = DiffColor[1];
				pD[2] = DiffColor[2];
				pD[3] = DiffColor[3];
			}
			else
				pD[0] = pD[1] = pD[2] = pD[3] = 0;

			p1 += 4;
			p2 += 4;
			pD += 4;
		}

		row1 += Image1.rowSizeInBytes();
		row2 += Image2.rowSizeInBytes();
		rowD += Diff.rowSizeInBytes();
	}

	delete[] rgba1;
	delete[] rgba2;
}

///////////////////////////////////////////////////////////////////////////////
// DiffColorMenu - handle difference-color menu items
///////////////////////////////////////////////////////////////////////////////
void
DiffColorMenu(int item) {
	unsigned char c[3];

	switch (item) {
	case DIFF_COLOR_MENU_WHITE:
		c[0] = c[1] = c[2] = 255;
		break;
	case DIFF_COLOR_MENU_BLACK:
		c[0] = c[1] = c[2] = 0;
		break;
	case DIFF_COLOR_MENU_RED:
		c[0] = 255;
		c[1] = c[2] = 0;
		break;
	case DIFF_COLOR_MENU_GREEN:
		c[0] = c[2] = 0;
		c[1] = 255;
		break;
	case DIFF_COLOR_MENU_BLUE:
		c[0] = c[1] = 0;
		c[2] = 255;
		break;
	case DIFF_COLOR_MENU_YELLOW:
		c[0] = c[1] = 255;
		c[2] = 0;
		break;
	case DIFF_COLOR_MENU_MAGENTA:
		c[0] = c[2] = 255;
		c[1] = 0;
		break;
	case DIFF_COLOR_MENU_CYAN:
		c[0] = 0;
		c[1] = c[2] = 255;
		break;
	}

	if (c[0]!=DiffColor[0] || c[1]!=DiffColor[1] || c[2]!=DiffColor[2]) {
		DiffColor[0] = c[0];
		DiffColor[1] = c[1];
		DiffColor[2] = c[2];
		ComputeDifference();
		glutPostRedisplay();
	}
}

///////////////////////////////////////////////////////////////////////////////
// Draw - redraw the scene
///////////////////////////////////////////////////////////////////////////////
void
Draw() {
	glClear(GL_COLOR_BUFFER_BIT);
	glRasterPos2f(1, 1);
	switch (Mode) {
	case SHOW_IMAGE1:
		Image1.draw();
		break;
	case SHOW_IMAGE2:
		Image2.draw();
		break;
	case SHOW_DIFF:
		Diff.draw();
		break;
	case SHOW_DIFF_ON_1:
		Image1.draw();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		Diff.draw();
		glDisable(GL_BLEND);
		break;
	case SHOW_DIFF_ON_2:
		Image2.draw();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		Diff.draw();
		glDisable(GL_BLEND);
		break;
	}
	glutSwapBuffers();
}

///////////////////////////////////////////////////////////////////////////////
// Keypress - handle events from the keyboard
///////////////////////////////////////////////////////////////////////////////
void
Keypress(unsigned char key, int x, int y) {
	static_cast<void>(x); static_cast<void>(y);
	switch (key) {
	case '1':
		Mode = SHOW_IMAGE1;
		glutPostRedisplay();
		break;
	case '2':
		Mode = SHOW_IMAGE2;
		glutPostRedisplay();
		break;
	case 0x1b /*ESC*/:
	case 'q':
	case 'Q':
		exit(0);
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
// main - initialization and callback registration
///////////////////////////////////////////////////////////////////////////////
int
main(int argc, char* argv[]) {
	ApplicationName = argv[0];

	if (argc != 3) {
		cerr << "usage: " << ApplicationName
			<< " tiff-file1 tiff-file2\n";
		exit(1);
	}

	try {
		Image1.readTIFF(argv[1]);
	}
	catch (GLEAN::Image::Error) {
		cerr << "can't open or handle " << argv[1] << "\n";
		exit(2);
	}

	try {
		Image2.readTIFF(argv[2]);
	}
	catch (GLEAN::Image::Error) {
		cerr << "can't open or handle " << argv[2] << "\n";
		exit(3);
	}

	if (Image1.width() != Image2.width()
	 || Image1.height() != Image2.height()) {
		cerr << "image dimensions don't match\n";
		exit(4);
	}

	Diff.width(Image1.width());
	Diff.height(Image1.height());
	Diff.format(GL_RGBA);
	Diff.type(GL_UNSIGNED_BYTE);
	Diff.alignment(1);
	Diff.reserve();
	ComputeDifference();

	glutInitWindowSize(Image1.width() + 2, Image1.height() + 2);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE);
	glutInit(&argc, argv);

	glutCreateWindow("difftiff");

	int diffColorMenu = glutCreateMenu(DiffColorMenu);
		glutAddMenuEntry("white", DIFF_COLOR_MENU_WHITE);
		glutAddMenuEntry("black", DIFF_COLOR_MENU_BLACK);
		glutAddMenuEntry("red", DIFF_COLOR_MENU_RED);
		glutAddMenuEntry("green", DIFF_COLOR_MENU_GREEN);
		glutAddMenuEntry("blue", DIFF_COLOR_MENU_BLUE);
		glutAddMenuEntry("yellow", DIFF_COLOR_MENU_YELLOW);
		glutAddMenuEntry("magenta", DIFF_COLOR_MENU_MAGENTA);
		glutAddMenuEntry("cyan", DIFF_COLOR_MENU_CYAN);

	int threshMenu = glutCreateMenu(ThreshMenu);
		glutAddMenuEntry("1/2", 2);
		glutAddMenuEntry("1/4", 4);
		glutAddMenuEntry("1/8", 8);
		glutAddMenuEntry("1/16", 16);
		glutAddMenuEntry("1/32", 32);
		glutAddMenuEntry("1/64", 64);
		glutAddMenuEntry("1/128", 128);
		glutAddMenuEntry("1/256", 256);
		glutAddMenuEntry("0", 0);

	glutCreateMenu(Menu);
		glutAddMenuEntry("Image 1", MENU_IMAGE1);
		glutAddMenuEntry("Image 2", MENU_IMAGE2);
		glutAddMenuEntry("Difference", MENU_DIFF);
		glutAddMenuEntry("Difference + Image 1", MENU_DIFF_ON_1);
		glutAddMenuEntry("Difference + Image 2", MENU_DIFF_ON_2);
		glutAddSubMenu("Threshold", threshMenu);
		glutAddSubMenu("Difference Highlight Color", diffColorMenu);
		glutAddMenuEntry("Quit", MENU_QUIT);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	glutDisplayFunc(Draw);
	glutKeyboardFunc(Keypress);
	glutReshapeFunc(Reshape);

	glutMainLoop();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Menu - handle invocations of menu commands
///////////////////////////////////////////////////////////////////////////////
void
Menu(int item) {
	switch (item) {
	case MENU_IMAGE1:
		Mode = SHOW_IMAGE1;
		glutPostRedisplay();
		break;
	case MENU_IMAGE2:
		Mode = SHOW_IMAGE2;
		glutPostRedisplay();
		break;
	case MENU_DIFF:
		Mode = SHOW_DIFF;
		glutPostRedisplay();
		break;
	case MENU_DIFF_ON_1:
		Mode = SHOW_DIFF_ON_1;
		glutPostRedisplay();
		break;
	case MENU_DIFF_ON_2:
		Mode = SHOW_DIFF_ON_2;
		glutPostRedisplay();
		break;
	case MENU_QUIT:
		exit(EXIT_SUCCESS);
		break;
	default:
		cerr << "Internal error; bogus menu item\n";
		exit(EXIT_FAILURE);
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Reshape - handle window reshape requests
///////////////////////////////////////////////////////////////////////////////
void
Reshape(int width, int height) {
	WindowWidth = width;
	WindowHeight = height;
	glViewport(0, 0, width, height);
	ScreenOrthoView(width, height);
	glutPostRedisplay();
}

///////////////////////////////////////////////////////////////////////////////
// ScreenOrthoView - simple parallel view that matches screen pixel coords
// (using the OpenGL convention for origin at lower left)
///////////////////////////////////////////////////////////////////////////////
void
ScreenOrthoView(int w, int h) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,w, 0,h, -1,1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.375, 0.375, 0.0);
}

///////////////////////////////////////////////////////////////////////////////
// ThreshMenu - handle threshold submenu items
///////////////////////////////////////////////////////////////////////////////
void
ThreshMenu(int item) {
	double newThreshold;
	if (item == 0)
		newThreshold = 0.0;
	else
		newThreshold = 1.0 / static_cast<double>(item);
	if (newThreshold != Threshold) {
		Threshold = newThreshold;
		ComputeDifference();
		glutPostRedisplay();
	}
}
