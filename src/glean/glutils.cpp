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




// glutils.cpp:  frequently-used OpenGL operations

#include "glwrap.h"
#include "environ.h"

#include "glutils.h"

namespace GLEAN {

namespace GLUtils {

///////////////////////////////////////////////////////////////////////////////
// useScreenCoords:  Map object coords directly to screen coords.
///////////////////////////////////////////////////////////////////////////////
void
useScreenCoords(int windowW, int windowH) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, windowW, 0, windowH, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glViewport(0, 0, windowW, windowH);
	glTranslatef(0.375, 0.375, 0.0);
} // useScreenCoords

} // namespace GLUtils

} // namespace GLEAN
