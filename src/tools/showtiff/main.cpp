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
#include <GL/glut.h>
#include "image.h"

using namespace std;

#define MENU_QUIT 1

void Draw(void);
void Keypress(unsigned char key, int x, int y);
void Menu(int item);
void Reshape(int width, int height);
void ScreenOrthoView(int w, int h);


char* ApplicationName;
int WindowHeight;
int WindowWidth;
GLEAN::Image Image;


///////////////////////////////////////////////////////////////////////////////
// Draw - redraw the scene
///////////////////////////////////////////////////////////////////////////////
void
Draw(void) {
	glClear(GL_COLOR_BUFFER_BIT);
	glRasterPos2f(1, 1);
	Image.draw();
	glutSwapBuffers();
}

///////////////////////////////////////////////////////////////////////////////
// Keypress - handle events from the keyboard
///////////////////////////////////////////////////////////////////////////////
void
Keypress(unsigned char key, int x, int y) {
	static_cast<void>(x); static_cast<void>(y);
	switch (key) {
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

	if (argc != 2) {
		cerr << "usage: " << ApplicationName << " tiff-file\n";
		exit(1);
	}

	try {
		Image.readTIFF(argv[1]);
	}
	catch (GLEAN::Image::CantOpen) {
		cerr << "can't open " << argv[1] << "\n";
		exit(2);
	}
	catch (GLEAN::Image::Error) {
		cerr << "can't handle " << argv[1] << "\n";
		exit(2);
	}

	glutInitWindowSize(Image.width() + 2, Image.height() + 2);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE);
	glutInit(&argc, argv);

	glutCreateWindow("showtiff");

	glutCreateMenu(Menu);
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
