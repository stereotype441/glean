// BEGIN_COPYRIGHT -*- glean -*-
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


// ttexcube.cpp:  Test the GL_ARB_texture_cube_map extension
// Author: Brian Paul (brianp@valinux.com)  March 2001
//
// Test procedure:
//   We build a 6-sided texture cube map in which each side is a simple 2x2
//   checkboard pattern with known colors.
//   We then draw 24 test polygons, one at a time.  We use the same 3-component
//   texture coordinate for all vertices.  We choose the texture coordinates
//   such that we'll hit one of the four quadrants of each texture cube face.
//   We compare the polygon color to the expected texel color to check if
//   cube map sampling is correct.
//
//   This extension does not exercise the new texgen modes (yet).
//

#include "ttexcube.h"
#include <stdio.h>
#include <cmath>

namespace GLEAN {


//
// Test if two colors are close enough to be considered the same
//
bool
TexCubeTest::TestColor(const GLfloat c1[3], const GLfloat c2[3],
			const GLfloat tolerance[3]) {
	if (fabs(c1[0] - c2[0]) <= tolerance[0] &&
	    fabs(c1[1] - c2[1]) <= tolerance[1] &&
	    fabs(c1[2] - c2[2]) <= tolerance[2])
		return true;
	else
		return false;
}


//
// Define a 2x2 checkerboard texture image using the given four colors.
//
void
TexCubeTest::BuildTexImage(GLenum target, const GLfloat color[4][3]) {
	const GLint w = 8, h = 8;
	GLfloat texImage[8][8][4];
	for (int i = 0; i < h; i++) {
		const int ibit = (i >= (h / 2));
		for (int j = 0; j < w; j++) {
			const int jbit = (j >= (w / 2));
			const int c = ibit * 2 + jbit;
			texImage[i][j][0] = color[c][0];
			texImage[i][j][1] = color[c][1];
			texImage[i][j][2] = color[c][2];
			texImage[i][j][3] = 1.0;
		}
	}
	glTexImage2D(target, 0, GL_RGB, w, h, 0, GL_RGBA, GL_FLOAT, texImage);
}


void
TexCubeTest::runOne(BasicResult& r, Window& w) {

	(void) w;

	const char *faceName[6] = {
		"POSITIVE_X",
		"NEGATIVE_X",
		"POSITIVE_Y",
		"NEGATIVE_Y",
		"POSITIVE_Z",
		"NEGATIVE_Z"
	};

	// each of six faces needs four test colors
	GLfloat colors[6][4][3];
	for (int i = 0; i < 6 * 4; i++) {
		GLint r = i % 3;
		GLint g = (i / 3) % 3;
		GLint b = (i / 9) % 3;
		colors[i / 4][i % 4][0] = r * 0.5;
		colors[i / 4][i % 4][1] = g * 0.5;
		colors[i / 4][i % 4][2] = b * 0.5;
	}

	glDrawBuffer(GL_FRONT);
	glReadBuffer(GL_FRONT);

	BuildTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB, colors[0]);
	BuildTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB, colors[1]);
	BuildTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB, colors[2]);
	BuildTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB, colors[3]);
	BuildTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB, colors[4]);
	BuildTexImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB, colors[5]);

	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_TEXTURE_CUBE_MAP_ARB);

	// compute RGB error tolerance
	GLfloat tol[3];
	{
		GLint rBits, gBits, bBits;
		GLint rTexBits, gTexBits, bTexBits;
		glGetIntegerv(GL_RED_BITS, &rBits);
		glGetIntegerv(GL_GREEN_BITS, &gBits);
		glGetIntegerv(GL_BLUE_BITS, &bBits);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,
			0, GL_TEXTURE_RED_SIZE, &rTexBits);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,
			0, GL_TEXTURE_GREEN_SIZE, &gTexBits);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,
			0, GL_TEXTURE_BLUE_SIZE, &bTexBits);
		// find smaller of frame buffer and texture bits
		rBits = (rBits < rTexBits) ? rBits : rTexBits;
		gBits = (gBits < gTexBits) ? gBits : gTexBits;
		bBits = (bBits < bTexBits) ? bBits : bTexBits;
		tol[0] = 2.0 / (1 << rBits);
		tol[1] = 2.0 / (1 << gBits);
		tol[2] = 2.0 / (1 << bBits);
	}

	// texture coordinates to hit the four colors of each cube face
	static const GLfloat texcoord[6][4][3] = {
		// +X
		{
			{ 1.0,  0.5,  0.5 },
			{ 1.0,  0.5, -0.5 },
			{ 1.0, -0.5,  0.5 },
			{ 1.0, -0.5, -0.5 },
		},
		// -X
		{
			{ -1.0,  0.5, -0.5 },
			{ -1.0,  0.5,  0.5 },
			{ -1.0, -0.5, -0.5 },
			{ -1.0, -0.5,  0.5 },
		},
		// +Y
		{
			{ -0.5, 1.0, -0.5 },
			{  0.5, 1.0, -0.5 },
			{ -0.5, 1.0,  0.5 },
			{  0.5, 1.0,  0.5 },
		},
		// -Y
		{
			{ -0.5, -1.0,  0.5 },
			{  0.5, -1.0,  0.5 },
			{ -0.5, -1.0, -0.5 },
			{  0.5, -1.0, -0.5 },
		},
		// +Z
		{
			{ -0.5,  0.5, 1.0 },
			{  0.5,  0.5, 1.0 },
			{ -0.5, -0.5, 1.0 },
			{  0.5, -0.5, 1.0 },
		},
		// -Z
		{
			{  0.5,  0.5, -1.0 },
			{ -0.5,  0.5, -1.0 },
			{  0.5, -0.5, -1.0 },
			{ -0.5, -0.5, -1.0 },
		}
	};

	// only need small viewport
	glViewport(0, 0, 10, 10);

	for (int face = 0; face < 6; face++) {
		for (int quadrant = 0; quadrant < 4; quadrant++) {

			// draw the test quad
			glTexCoord3fv(texcoord[face][quadrant]);
			glColor3f(0, 1, 0);
			glBegin(GL_POLYGON);
			glVertex2f(-1, -1);
			glVertex2f( 1, -1);
			glVertex2f( 1,  1);
			glVertex2f(-1,  1);
			glEnd();

			// check the color
			GLfloat result[3];
			glReadPixels(1, 1, 1, 1, GL_RGB, GL_FLOAT, result);

			if (!TestColor(colors[face][quadrant], result, tol)) {
				env->log << name
					 << ":  FAIL: face="
					 << faceName[face]
					 << " quadrant="
					 << quadrant
					 << " expected=("
					 << colors[face][quadrant][0] << ", "
					 << colors[face][quadrant][1] << ", "
					 << colors[face][quadrant][2]
					 << ") measured=("
					 << result[0] << ", "
					 << result[1] << ", "
					 << result[2]
					 << ")\n";
				r.pass = false;
				return;
			}
		}
	}

	r.pass = true;
} // TexCubeTest::runOne

void
TexCubeTest::logOne(BasicResult& r) {
	logPassFail(r);
	logConcise(r);
} // TexCubeTest::logOne


///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
TexCubeTest texCubeTest("texCube", "window, rgb",

	"GL_ARB_texture_cube_map verification test.\n");


} // namespace GLEAN
