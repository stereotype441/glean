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

// tvertprog1.cpp:  Test GL_ARB_vertex_program extension.
// Brian Paul  22 October 2005
//
// See tfragprog.cpp for comments (this test is very similar).

#include "tvertprog1.h"
#include <cassert>
#include <cmath>
#include <math.h>


namespace GLEAN {


static PFNGLPROGRAMLOCALPARAMETER4FVARBPROC glProgramLocalParameter4fvARB_func;
static PFNGLGENPROGRAMSARBPROC glGenProgramsARB_func;
static PFNGLPROGRAMSTRINGARBPROC glProgramStringARB_func;
static PFNGLBINDPROGRAMARBPROC glBindProgramARB_func;
static PFNGLISPROGRAMARBPROC glIsProgramARB_func;
static PFNGLDELETEPROGRAMSARBPROC glDeleteProgramsARB_func;

static GLuint progID;


// Clamp X to [0, 1]
#define CLAMP01( X )  ( (X)<(0.0) ? (0.0) : ((X)>(1.0) ? (1.0) : (X)) )
// Absolute value
#define ABS(X)  ( (X) < 0.0 ? -(X) : (X) )
// Max
#define MAX( A, B )   ( (A) > (B) ? (A) : (B) )

#define DONT_CARE_Z -1.0

#define VERTCOLOR { 0.25, 0.75, 0.5, 0.25 }
#define PARAM0 { 0.0, 0.0, 0.0, 0.0 }
#define PARAM1 { 0.5, 0.25, 0.9, 0.5 }
#define PARAM2 { -1.0, 0.0, 0.25, -0.5 }
static const GLfloat VertColor[4] = VERTCOLOR;
static const GLfloat Param0[4] = PARAM0;
static const GLfloat Param1[4] = PARAM1;
static const GLfloat Param2[4] = PARAM2;


// These are the specific vertex programs which we'll test.
// Alphabetical order, please.
static const VertexProgram Programs[] = {
	{
		"ADD test",
		"!!ARBvp1.0\n"
		"PARAM p = program.local[1]; \n"
		"MOV result.position, vertex.position; \n"
		"ADD result.color, vertex.color, p; \n"
		"END \n",
		{ CLAMP01(VertColor[0] + Param1[0]),
		  CLAMP01(VertColor[1] + Param1[1]),
		  CLAMP01(VertColor[2] + Param1[2]),
		  CLAMP01(VertColor[3] + Param1[3])
		},
		DONT_CARE_Z
	},
	{
		"MAX test",
		"!!ARBvp1.0\n"
		"PARAM p1 = program.local[1]; \n"
		"PARAM p2 = program.local[2]; \n"
		"MOV result.position, vertex.position; \n"
		"MAX result.color, p1, p2; \n"
		"END \n",
		{ MAX(Param1[0], Param2[0]),
		  MAX(Param1[1], Param2[1]),
		  MAX(Param1[2], Param2[2]),
		  MAX(Param1[3], Param2[3]),
		},
		DONT_CARE_Z
	},
	{
		"SLT test",
		"!!ARBvp1.0\n"
		"PARAM p0 = program.local[0]; \n"
		"PARAM p2 = program.local[2]; \n"
		"MOV result.position, vertex.position; \n"
		"SLT result.color, p2, p0; \n"
		"END \n",
		{ Param2[0] < Param0[0] ? 1.0 : 0.0,
		  Param2[1] < Param0[1] ? 1.0 : 0.0,
		  Param2[2] < Param0[2] ? 1.0 : 0.0,
		  Param2[3] < Param0[3] ? 1.0 : 0.0,
		},
		DONT_CARE_Z
	},
	{
		"SWZ test",
		"!!ARBvp1.0\n"
		"PARAM p = program.local[1]; \n"
		"MOV result.position, vertex.position; \n"
		"SWZ result.color, p, 1,x,z,0; \n"
		"END \n",
		{ 1.0,
		  Param1[0],
		  Param1[2],
		  0.0
		},
		DONT_CARE_Z
	},
	{
		"SWZ test 2",
		"!!ARBvp1.0\n"
		"PARAM p = program.local[1]; \n"
		"MOV result.position, vertex.position; \n"
		"SWZ result.color, p, z,-y,-1,0; \n"
		"END \n",
		{ CLAMP01(Param1[2]),
		  CLAMP01(-Param1[1]),
		  CLAMP01(-1),
		  0.0
		},
		DONT_CARE_Z
	},

	// XXX add lots more tests here!
	{ NULL, NULL, {0,0,0,0}, 0 } // end of list sentinal
};



void
VertexProgramTest::setup(void)
{
	// get function pointers
	glProgramLocalParameter4fvARB_func = (PFNGLPROGRAMLOCALPARAMETER4FVARBPROC) GLUtils::getProcAddress("glProgramLocalParameter4fvARB");
	assert(glProgramLocalParameter4fvARB_func);

	glGenProgramsARB_func = (PFNGLGENPROGRAMSARBPROC) GLUtils::getProcAddress("glGenProgramsARB");
	assert(glGenProgramsARB_func);

	glProgramStringARB_func = (PFNGLPROGRAMSTRINGARBPROC) GLUtils::getProcAddress("glProgramStringARB");
	assert(glProgramStringARB_func);

	glBindProgramARB_func = (PFNGLBINDPROGRAMARBPROC) GLUtils::getProcAddress("glBindProgramARB");
	assert(glBindProgramARB_func);

	glIsProgramARB_func = (PFNGLISPROGRAMARBPROC) GLUtils::getProcAddress("glIsProgramARB");
	assert(glIsProgramARB_func);

	glDeleteProgramsARB_func = (PFNGLDELETEPROGRAMSARBPROC) GLUtils::getProcAddress("glDeleteProgramsARB");
	assert(glDeleteProgramsARB_func);

	glGenProgramsARB_func(1, &progID);
	glBindProgramARB_func(GL_VERTEX_PROGRAM_ARB, progID);
	glEnable(GL_VERTEX_PROGRAM_ARB);

	// load program inputs
	glColor4fv(VertColor);
	glProgramLocalParameter4fvARB_func(GL_VERTEX_PROGRAM_ARB, 0, Param0);
	glProgramLocalParameter4fvARB_func(GL_VERTEX_PROGRAM_ARB, 1, Param1);
	glProgramLocalParameter4fvARB_func(GL_VERTEX_PROGRAM_ARB, 2, Param2);

	GLenum err = glGetError();
	assert(!err);  // should be OK

	// setup vertex transform (we'll draw a quad in middle of window)
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-4.0, 4.0, -4.0, 4.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDrawBuffer(GL_FRONT);
	glReadBuffer(GL_FRONT); 

	// compute error tolerances (may need fine-tuning)
	int bufferBits[5];
	glGetIntegerv(GL_RED_BITS, &bufferBits[0]);
	glGetIntegerv(GL_GREEN_BITS, &bufferBits[1]);
	glGetIntegerv(GL_BLUE_BITS, &bufferBits[2]);
	glGetIntegerv(GL_ALPHA_BITS, &bufferBits[3]);
	glGetIntegerv(GL_DEPTH_BITS, &bufferBits[4]);

	tolerance[0] = 2.0 / (1 << bufferBits[0]);
	tolerance[1] = 2.0 / (1 << bufferBits[1]);
	tolerance[2] = 2.0 / (1 << bufferBits[2]);
	if (bufferBits[3])
		tolerance[3] = 2.0 / (1 << bufferBits[3]);
	else
		tolerance[3] = 1.0;
	if (bufferBits[4])
		tolerance[4] = 16.0 / (1 << bufferBits[4]);
	else
		tolerance[4] = 1.0;
}


void
VertexProgramTest::reportFailure(const char *programName,
				   const GLfloat expectedColor[4],
				   const GLfloat actualColor[4] ) const
{
	env->log << "FAILURE:\n";
	env->log << "  Program: " << programName << "\n";
	env->log << "  Expected color: ";
	env->log << expectedColor[0] << ", ";
	env->log << expectedColor[1] << ", ";
	env->log << expectedColor[2] << ", ";
	env->log << expectedColor[3] << "\n";
	env->log << "  Observed color: ";
	env->log << actualColor[0] << ", ";
	env->log << actualColor[1] << ", ";
	env->log << actualColor[2] << ", ";
	env->log << actualColor[3] << "\n";
}

void
VertexProgramTest::reportZFailure(const char *programName,
				    GLfloat expectedZ, GLfloat actualZ) const
{
	env->log << "FAILURE:\n";
	env->log << "  Program: " << programName << "\n";
	env->log << "  Expected Z: " << expectedZ << "\n";
	env->log << "  Observed Z: " << actualZ << "\n";
}

void
VertexProgramTest::reportSuccess(int count) const
{
	env->log << "PASS: " << count;
	env->log << " vertex programs tested.\n";
}


bool
VertexProgramTest::equalColors(const GLfloat a[4], const GLfloat b[4]) const
{
	if (fabsf(a[0] - b[0]) > tolerance[0] ||
	    fabsf(a[1] - b[1]) > tolerance[1] ||
	    fabsf(a[2] - b[2]) > tolerance[2] ||
	    fabsf(a[3] - b[3]) > tolerance[3]) 
		return false;
	else
		return true;
}


bool
VertexProgramTest::equalDepth(GLfloat z0, GLfloat z1) const
{
	if (fabsf(z0 - z1) > tolerance[4])
		return false;
	else
		return true;
}


bool
VertexProgramTest::testProgram(const VertexProgram &p)
{
	const GLfloat r = 0.25;

	glProgramStringARB_func(GL_VERTEX_PROGRAM_ARB,
				GL_PROGRAM_FORMAT_ASCII_ARB,
				strlen(p.progString),
				(const GLubyte *) p.progString);

	GLenum err = glGetError();
	if (err) {
		GLint errorPos;
		glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
		env->log << "OpenGL error " << (int) err << "\n";
		env->log << "Invalid Vertex Program:\n";
		env->log << p.progString;
		env->log << "Error position: " << errorPos << "\n";
		return false;
	}

	// to avoid potential issue with undefined result.depth.z
	if (p.expectedZ == DONT_CARE_Z)
		glDisable(GL_DEPTH_TEST);
	else
		glEnable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBegin(GL_POLYGON);
	glVertex2f(-r, -r);
	glVertex2f( r, -r);
	glVertex2f( r,  r);
	glVertex2f(-r,  r);
	glEnd();

	GLfloat pixel[4];
	glReadPixels(windowSize / 2, windowSize / 2, 1, 1,
		     GL_RGBA, GL_FLOAT, pixel);

        if (0) // debug
           printf("%s: Expect: %.3f %.3f %.3f %.3f  found: %.3f %.3f %.3f %.3f\n",
                  p.name,
                  p.expectedColor[0], p.expectedColor[1],
                  p.expectedColor[2], p.expectedColor[3], 
                  pixel[0], pixel[1], pixel[2], pixel[3]);

	if (!equalColors(pixel, p.expectedColor)) {
		reportFailure(p.name, p.expectedColor, pixel);
		return false;
	}

	if (p.expectedZ != DONT_CARE_Z) {
		GLfloat z;
		glReadPixels(windowSize / 2, windowSize / 2, 1, 1,
			     GL_DEPTH_COMPONENT, GL_FLOAT, &z);
		if (!equalDepth(z, p.expectedZ)) {
			reportZFailure(p.name, p.expectedZ, z);
			return false;
		}
	}

	return true;
}

void
VertexProgramTest::runOne(BasicResult &r, Window &w)
{
	(void) w;
	setup();
	int i;
	for (i = 0; Programs[i].name; i++) {
		if (!testProgram(Programs[i])) {
			r.pass = false;
			return;
		}
	}
	reportSuccess(i);
	r.pass = true;
}


void
VertexProgramTest::logOne(BasicResult &r)
{
	if (r.pass) {
		logPassFail(r);
		logConcise(r);
	}
}


// constructor
VertexProgramTest::VertexProgramTest(const char *testName,
					   const char *filter,
					   const char *extensions,
					   const char *description)
	: BasicTest(testName, filter, extensions, description)
{
	fWidth  = windowSize;
	fHeight = windowSize;
}



// The test object itself:
VertexProgramTest vertexProgramTest("vertProg1", "window, rgb, z",
	"GL_ARB_vertex_program",
	"Vertex Program test 1: test a specific set of vertex programs.\n");



} // namespace GLEAN
