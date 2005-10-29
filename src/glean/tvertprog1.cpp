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
// Min
#define MIN( A, B )   ( (A) < (B) ? (A) : (B) )
// Duplicate value four times
#define SMEAR(X)  (X), (X), (X), (X)

#define DONT_CARE_Z -1.0
#define DONT_CARE_COLOR -1.0

#define VERTCOLOR { 0.25, 0.75, 0.5, 0.25 }
#define PARAM0 { 0.0, 0.0, 0.0, 0.0 }     // all zero
#define PARAM1 { 0.5, 0.25, 0.9, 0.5 }    // in [0,1]
#define PARAM2 { -1.0, 0.0, 0.25, -0.5 }  // in [-1,1]
static const GLfloat VertColor[4] = VERTCOLOR;
static const GLfloat Param0[4] = PARAM0;
static const GLfloat Param1[4] = PARAM1;
static const GLfloat Param2[4] = PARAM2;


// These are the specific vertex programs which we'll test.
// Alphabetical order, please.
static const VertexProgram Programs[] = {
	// ============= Basic instructions tests =============================
	{
		"ABS test",
		"!!ARBvp1.0\n"
		"PARAM p2 = program.local[2]; \n"
		"MOV result.position, vertex.position; \n"
		"ABS result.color, p2; \n"
		"END \n",
		{ CLAMP01(ABS(Param2[0])),
		  CLAMP01(ABS(Param2[1])),
		  CLAMP01(ABS(Param2[2])),
		  CLAMP01(ABS(Param2[3])),
		},
		DONT_CARE_Z
	},
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
		"ARL test",
		"!!ARBvp1.0\n"
		"ADDRESS addr; \n"
		"PARAM indexes = {-1, 0, 1, 2}; \n"
		"PARAM myArray[4] = {{0.11, 0.12, 0.13, 0.14}, \n"
		"                    {0.21, 0.22, 0.23, 0.24}, \n"
		"                    {0.31, 0.32, 0.33, 0.34}, \n"
		"                    {0.41, 0.42, 0.43, 0.44}}; \n"
		"MOV result.position, vertex.position; \n"
		""
		"# Load ARL with -1, get array[0].x \n"
		"ARL addr.x, indexes.x; \n"
		"MOV result.color.x, myArray[addr.x + 1]; \n"
		""
		"# Load ARL with 0, get array[1].y \n"
		"ARL addr.x, indexes.y; \n"
		"MOV result.color.y, myArray[addr.x + 1]; \n"
		""
		"# Load ARL with 1, get array[2].z \n"
		"ARL addr.x, indexes.z; \n"
		"MOV result.color.z, myArray[addr.x + 1]; \n"
		""
		"# Load ARL with 2, get array[3].w\n"
		"ARL addr.x, indexes.w; \n"
		"MOV result.color.w, myArray[addr.x + 1]; \n"
		"END \n",
		{ 0.11,
		  0.22,
		  0.33,
		  0.44
		},
		DONT_CARE_Z
	},
	{
		"DP3 test",
		"!!ARBvp1.0\n"
		"PARAM p2 = program.local[2]; \n"
		"PARAM bias = { 0.5, 0.5, 0.5, 0.5 }; \n"
		"TEMP t; \n"
		"MOV result.position, vertex.position; \n"
		"DP3 t, p2, vertex.color; \n"
		"ADD result.color, t, bias; \n"
		"END \n",
		{ SMEAR(Param2[0] * VertColor[0] +
			Param2[1] * VertColor[1] +
			Param2[2] * VertColor[2] + 0.5)
		},
		DONT_CARE_Z
	},
	{
		"DP4 test",
		"!!ARBvp1.0\n"
		"PARAM p2 = program.local[2]; \n"
		"PARAM bias = { 0.5, 0.5, 0.5, 0.5 }; \n"
		"TEMP t; \n"
		"MOV result.position, vertex.position; \n"
		"DP4 t, p2, vertex.color; \n"
		"ADD result.color, t, bias; \n"
		"END \n",
		{ SMEAR(Param2[0] * VertColor[0] +
			Param2[1] * VertColor[1] +
			Param2[2] * VertColor[2] +
			Param2[3] * VertColor[3] + 0.5)
		},
		DONT_CARE_Z
	},
	{
		"DPH test",
		"!!ARBvp1.0\n"
		"PARAM p2 = program.local[2]; \n"
		"TEMP t; \n"
		"MOV result.position, vertex.position; \n"
		"DPH result.color, p2, vertex.color; \n"
		"END \n",
		{ SMEAR(CLAMP01(Param2[0] * VertColor[0] +
				Param2[1] * VertColor[1] +
				Param2[2] * VertColor[2] +
				VertColor[3]))
		},
		DONT_CARE_Z
	},
	{
		"DST test",
		"!!ARBvp1.0\n"
		"# let d = 0.4 \n"
		"PARAM v1 = {9.9, 0.16, 0.16, 9.9}; \n"
		"PARAM v2 = {9.9, 2.5, 9.9, 2.5}; \n"
		"MOV result.position, vertex.position; \n"
		"DST result.color, v1, v2; \n"
		"END \n",
		{ 1.0,
		  0.4,           // v1.y * v2.y
		  0.16,          // v1.z
		  CLAMP01(2.5)   // v2.w
		},
		DONT_CARE_Z
	},
	{
		"EX2 test",
		"!!ARBvp1.0\n"
                "PARAM scale = {0.01, 0.01, 0.01, 0.01}; \n"
		"PARAM values = {0.0, 1.0, 4.0, -2.0 }; \n"
                "TEMP t; \n"
		"MOV result.position, vertex.position; \n"
		"EX2 t.x, values.x; \n"
		"EX2 t.y, values.y; \n"
		"EX2 t.z, values.z; \n"
		"EX2 t.w, values.w; \n"
		"MUL result.color, t, scale; \n"
		"END \n",
		{  1.0 * 0.01,
                   2.0 * 0.01,
                  16.0 * 0.01,
                  0.25 * 0.01 },
		DONT_CARE_Z
	},
	{
		"EXP test",
		"!!ARBvp1.0\n"
                "PARAM scale = {0.01, 0.01, 0.01, 0.01}; \n"
		"PARAM values = {4.5, 0, 0, 0}; \n"
                "TEMP t; \n"
		"MOV result.position, vertex.position; \n"
		"EXP t, values.x; \n"
		"MUL result.color, t, scale; \n"
		"END \n",
		{  16.0  * 0.01,
                    0.5  * 0.01,
                   22.62 * 0.01,
                    1.0  * 0.01 },
		DONT_CARE_Z
	},
	{
		"FLR test",
		"!!ARBvp1.0\n"
		"PARAM values = {4.8, 0.3, -0.2, 1.2}; \n"
		"PARAM scale = {0.1, 0.1, 0.1, 0.1}; \n"
		"MOV result.position, vertex.position; \n"
		"TEMP t; \n"
		"FLR t, values; \n"
		"MUL result.color, t, scale; \n"
		"END \n",
		{ 0.4,
		  0.0,
		  CLAMP01(-0.1),
		  0.1
		},
		DONT_CARE_Z
	},
	{
		"FRC test",
		"!!ARBvp1.0\n"
		"PARAM values = {1.344, -1.5, -10.1, 4.2}; \n"
		"MOV result.position, vertex.position; \n"
		"FRC result.color, values; \n"
		"END \n",
		{ 0.344,
		  0.5,
		  0.9,
		  0.2
		},
		DONT_CARE_Z
	},
	{
		"LG2 test",
		"!!ARBvp1.0\n"
		"PARAM values = {64.0, 1, 30, 4}; \n"
		"PARAM scale = {0.1, 0.1, 0.1, 0.1}; \n"
		"MOV result.position, vertex.position; \n"
		"TEMP t; \n"
		"LG2 t.x, values.x; \n"
		"LG2 t.y, values.y; \n"
		"LG2 t.z, values.z; \n"
		"LG2 t.w, values.w; \n"
		"MUL result.color, t, scale; \n"
		"END \n",
		{ 0.6,
		  0.0,
		  0.49,
		  0.2
		},
		DONT_CARE_Z
	},
	{
		"LIT test 1",
		"!!ARBvp1.0\n"
		"PARAM values = {0.65, 0.9, 0.0, 8.0}; \n"
		"MOV result.position, vertex.position; \n"
		"LIT result.color, values; \n"
		"END \n",
		{ 1.0,
		  0.65,    // values.x
		  0.443,   // roughly Pow(values.y, values.w)
		  1.0
		},
		DONT_CARE_Z
	},
	{
		"LIT test 2 (degenerate case: 0 ^ 0 -> 1)",
		"!!ARBvp1.0\n"
		"PARAM values = {0.65, 0.0, 0.0, 0.0}; \n"
		"MOV result.position, vertex.position; \n"
		"LIT result.color, values; \n"
		"END \n",
		{ 1.0,
		  0.65,    // values.x
		  1.0,     // 0^0
		  1.0
		},
		DONT_CARE_Z
	},
	{
		"LIT test 3 (case x < 0)",
		"!!ARBvp1.0\n"
		"PARAM values = {-0.5, 0.0, 0.0, 0.0}; \n"
		"MOV result.position, vertex.position; \n"
		"LIT result.color, values; \n"
		"END \n",
		{ 1.0,
		  CLAMP01(-0.5),    // values.x
		  0.0,
		  1.0
		},
		DONT_CARE_Z
	},
	{
		"LOG test",
		"!!ARBvp1.0\n"
		"PARAM values = {64.0, 50, 30, 4}; \n"
		"PARAM scale = {0.1, 0.1, 0.1, 0.1}; \n"
		"MOV result.position, vertex.position; \n"
		"TEMP t; \n"
		"LOG t.x, values.x; \n"
		"LOG t.y, values.y; \n"
		"LOG t.z, values.z; \n"
		"LOG t.w, values.w; \n"
		"MUL result.color, t, scale; \n"
		"END \n",
		{ 0.6,   // floor(log2(value.x))
		  0.15,  // value.y / 2^(floor(log2(value.y)))
		  0.49,  // roughApproxLog2(value.z)
		  0.1
		},
		DONT_CARE_Z
	},
	{
		"MAD test",
		"!!ARBvp1.0\n"
		"PARAM p1 = program.local[1]; \n"
		"PARAM p2 = program.local[2]; \n"
		"MOV result.position, vertex.position; \n"
		"MAD result.color, vertex.color, p1, p2; \n"
		"END \n",
		{ CLAMP01(VertColor[0] * Param1[0] + Param2[0]),
		  CLAMP01(VertColor[1] * Param1[1] + Param2[1]),
		  CLAMP01(VertColor[2] * Param1[2] + Param2[2]),
		  CLAMP01(VertColor[3] * Param1[3] + Param2[3])
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
		"MIN test",
		"!!ARBvp1.0\n"
		"PARAM p1 = program.local[1]; \n"
		"MOV result.position, vertex.position; \n"
		"MIN result.color, p1, vertex.color; \n"
		"END \n",
		{ MIN(Param1[0], VertColor[0]),
		  MIN(Param1[1], VertColor[1]),
		  MIN(Param1[2], VertColor[2]),
		  MIN(Param1[3], VertColor[3]),
		},
		DONT_CARE_Z
	},
	{
		"MOV test (with swizzle)",
		"!!ARBvp1.0\n"
		"MOV result.position, vertex.position; \n"
		"MOV result.color, vertex.color.wzxy; \n"
		"END \n",
		{ VertColor[3],
		  VertColor[2],
		  VertColor[0],
		  VertColor[1]
		},
		DONT_CARE_Z
	},
	{
		"MUL test (with swizzle and masking)",
		"!!ARBvp1.0\n"
		"PARAM p1 = program.local[1]; \n"
		"MOV result.position, vertex.position; \n"
		"MUL result.color.xy, p1.wzww, vertex.color.wzww; \n"
		"MUL result.color.zw, p1.xxyx, vertex.color.xxyx; \n"
		"END \n",
		{ CLAMP01(Param1[3] * VertColor[3]),
		  CLAMP01(Param1[2] * VertColor[2]),
		  CLAMP01(Param1[1] * VertColor[1]),
		  CLAMP01(Param1[0] * VertColor[0]),
		},
		DONT_CARE_Z
	},
	{
		"POW test (exponentiation)",
		"!!ARBvp1.0\n"
		"PARAM values = {0.5, 2, 3, 4}; \n"
		"MOV result.position, vertex.position; \n"
		"POW result.color.x, values.x, values.y; \n"
		"POW result.color.y, values.x, values.z; \n"
		"POW result.color.z, values.x, values.w; \n"
		"POW result.color.w, values.w, values.x; \n"
		"END \n",
		{ 0.5 * 0.5,
		  0.5 * 0.5 * 0.5,
		  0.5 * 0.5 * 0.5 * 0.5,
		  CLAMP01(2.0) },
		DONT_CARE_Z
	},
	{
		"RCP test (reciprocal)",
		"!!ARBvp1.0\n"
		"PARAM values = {8, -10, 1, 12 }; \n"
		"MOV result.position, vertex.position; \n"
		"RCP result.color.x, values.x; \n"
		"RCP result.color.y, values.y; \n"
		"RCP result.color.z, values.z; \n"
		"RCP result.color.w, values.w; \n"
		"END \n",
		{ 1.0 / 8.0, CLAMP01(1.0 / -10.0), 1, 1.0 / 12.0 },
		DONT_CARE_Z
	},
	{
		"RSQ test (reciprocal square root)",
		"!!ARBvp1.0\n"
		"PARAM values = {1, 4, 9, 100 }; \n"
		"MOV result.position, vertex.position; \n"
		"RSQ result.color.x, values.x; \n"
		"RSQ result.color.y, values.y; \n"
		"RSQ result.color.z, values.z; \n"
		"RSQ result.color.w, values.w; \n"
		"END \n",
		{ 1.0, 0.5, 0.3333, 0.1 },
		DONT_CARE_Z
	},
	{
		"SGE test",
		"!!ARBvp1.0\n"
		"PARAM p0 = program.local[0]; \n"
		"PARAM p2 = program.local[2]; \n"
		"MOV result.position, vertex.position; \n"
		"SGE result.color, p2, p0; \n"
		"END \n",
		{ Param2[0] >= Param0[0] ? 1.0 : 0.0,
		  Param2[1] >= Param0[1] ? 1.0 : 0.0,
		  Param2[2] >= Param0[2] ? 1.0 : 0.0,
		  Param2[3] >= Param0[3] ? 1.0 : 0.0,
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
		"SUB test (with swizzle)",
		"!!ARBvp1.0\n"
		"PARAM p1 = program.local[1]; \n"
		"MOV result.position, vertex.position; \n"
		"SUB result.color, p1.yxwz, vertex.color.primary.yxwz; \n"
		"END \n",
		{ CLAMP01(Param1[1] - VertColor[1]),
		  CLAMP01(Param1[0] - VertColor[0]),
		  CLAMP01(Param1[3] - VertColor[3]),
		  CLAMP01(Param1[2] - VertColor[2])
		},
		DONT_CARE_Z
	},
	{
		"SWZ test 1",
		"!!ARBvp1.0\n"
		"PARAM p = program.local[1]; \n"
		"MOV result.position, vertex.position; \n"
		"SWZ result.color, p, w,x,x,y; \n"
		"END \n",
		{ Param1[3],
		  Param1[0],
		  Param1[0],
		  Param1[1]
		},
		DONT_CARE_Z
	},
	{
		"SWZ test 2",
		"!!ARBvp1.0\n"
		"PARAM p = program.local[1]; \n"
		"MOV result.position, vertex.position; \n"
		"SWZ result.color, p, -w,-x,x,y; \n"
		"END \n",
		{ CLAMP01(-Param1[3]),
		  CLAMP01(-Param1[0]),
		  Param1[0],
		  Param1[1]
		},
		DONT_CARE_Z
	},
	{
		"SWZ test 3",
		"!!ARBvp1.0\n"
		"PARAM p = program.local[1]; \n"
		"MOV result.position, vertex.position; \n"
		"SWZ result.color, p, 0,1,0,1; \n"
		"END \n",
		{ 0.0, 1.0, 0.0, 1.0 },
		DONT_CARE_Z
	},
	{
		"SWZ test 4",
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
		"SWZ test 5",
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
	{
		"XPD test 1",
		"!!ARBvp1.0\n"
		"PARAM p1 = program.local[1]; \n"
		"PARAM p2 = program.local[2]; \n"
		"MOV result.position, vertex.position; \n"
		"XPD result.color, p1, p2; \n"
		"END \n",
		{ CLAMP01(Param1[1] * Param2[2] - Param1[2] * Param2[1]),
		  CLAMP01(Param1[2] * Param2[0] - Param1[0] * Param2[2]),
		  CLAMP01(Param1[0] * Param2[1] - Param1[1] * Param2[0]),
		  DONT_CARE_COLOR
		},
		DONT_CARE_Z
	},
	{
		"XPD test 2 (same src/dst arg)",
		"!!ARBvp1.0\n"
		"PARAM p1 = program.local[1]; \n"
		"PARAM p2 = program.local[2]; \n"
		"TEMP t; \n"
		"MOV result.position, vertex.position; \n"
		"MOV t, p1; \n"
		"XPD t, t, p2; \n"
		"MOV result.color, t; \n"
		"END \n",
		{ CLAMP01(Param1[1] * Param2[2] - Param1[2] * Param2[1]),
		  CLAMP01(Param1[2] * Param2[0] - Param1[0] * Param2[2]),
		  CLAMP01(Param1[0] * Param2[1] - Param1[1] * Param2[0]),
		  DONT_CARE_COLOR
		},
		DONT_CARE_Z
	},

	// ============= Test result.position.z writes ========================
	{
		"Z-write test",
		"!!ARBvp1.0\n"
		"PARAM p1 = program.local[1]; \n"
		"MOV result.position, vertex.position; \n"
		"MOV result.position.z, p1.y; \n"
		"MOV result.color, vertex.color; \n"
		"END \n",
		VERTCOLOR,
		Param1[1] * 0.5 + 0.5  // map clip Z to win Z
	},

	// ============= Global state reference tests =========================
	// XXX to do

	// ============= More swizzle/mask/negation tests =====================
	// XXX to do

	// ============= Texcoord output tests ================================
	// XXX to do

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


// Compare actual and expected colors
bool
VertexProgramTest::equalColors(const GLfloat act[4], const GLfloat exp[4]) const
{
	if (fabsf(act[0] - exp[0]) > tolerance[0] ||
	    fabsf(act[1] - exp[1]) > tolerance[1] ||
	    fabsf(act[2] - exp[2]) > tolerance[2] ||
	    (fabsf(act[3] - exp[3]) > tolerance[3] && exp[3] != DONT_CARE_COLOR))
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
		env->log << "Error message: " << glGetString(GL_PROGRAM_ERROR_STRING_ARB) << "\n";
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

	if (0) // debug
	   printf("%s passed\n", p.name);

	return true;
}

void
VertexProgramTest::runOne(BasicResult &r, Window &w)
{
	(void) w;
	setup();
	r.pass = true;
	int i;
	for (i = 0; Programs[i].name; i++) {
		if (!testProgram(Programs[i])) {
			r.pass = false;
			// continue with next test
		}
	}
	if (r.pass) {
		reportSuccess(i);
	}
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
