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

// tglsl1.h:  Test OpenGL shading language
// Brian Paul  6 March 2007

#define GL_GLEXT_PROTOTYPES

#include "tglsl1.h"
#include <cassert>
#include <math.h>


namespace GLEAN {


static PFNGLATTACHSHADERPROC glAttachShader_func = NULL;
static PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation_func = NULL;
static PFNGLCOMPILESHADERPROC glCompileShader_func = NULL;
static PFNGLCREATEPROGRAMPROC glCreateProgram_func = NULL;
static PFNGLCREATESHADERPROC glCreateShader_func = NULL;
static PFNGLDELETEPROGRAMPROC glDeleteProgram_func = NULL;
static PFNGLDELETESHADERPROC glDeleteShader_func = NULL;
static PFNGLGETATTACHEDSHADERSPROC glGetAttachedShaders_func = NULL;
static PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation_func = NULL;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog_func = NULL;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog_func = NULL;
static PFNGLGETSHADERIVPROC glGetShaderiv_func = NULL;
static PFNGLGETPROGRAMIVPROC glGetProgramiv_func = NULL;
static PFNGLGETSHADERSOURCEPROC glGetShaderSource_func = NULL;
static PFNGLGETUNIFORMFVPROC glGetUniformfv_func = NULL;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation_func = NULL;
static PFNGLISPROGRAMPROC glIsProgram_func = NULL;
static PFNGLISSHADERPROC glIsShader_func = NULL;
static PFNGLLINKPROGRAMPROC glLinkProgram_func = NULL;
static PFNGLSHADERSOURCEPROC glShaderSource_func = NULL;
static PFNGLUNIFORM1IPROC glUniform1i_func = NULL;
static PFNGLUNIFORM1FVPROC glUniform1fv_func = NULL;
static PFNGLUNIFORM2FVPROC glUniform2fv_func = NULL;
static PFNGLUNIFORM3FVPROC glUniform3fv_func = NULL;
static PFNGLUNIFORM4FVPROC glUniform4fv_func = NULL;
static PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv_func = NULL;
static PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv_func = NULL;
static PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv_func = NULL;
static PFNGLUSEPROGRAMPROC glUseProgram_func = NULL;
static PFNGLVERTEXATTRIB1FPROC glVertexAttrib1f_func = NULL;
static PFNGLVERTEXATTRIB2FPROC glVertexAttrib2f_func = NULL;
static PFNGLVERTEXATTRIB3FPROC glVertexAttrib3f_func = NULL;
static PFNGLVERTEXATTRIB4FPROC glVertexAttrib4f_func = NULL;


#define DONT_CARE_Z -1.0
#define DONT_CARE_COLOR -1.0

#define NO_VERTEX_SHADER NULL
#define NO_FRAGMENT_SHADER NULL

#define VERTCOLOR { 0.25, 0.75, 0.5, 0.25 }
#define AMBIENT { 0.2, 0.4, 0.6, 0.8 }
#define DIFFUSE { 0.1, 0.3, 0.5, 0.7 }
#define UNIFORM1 {1.0, 0.25, 0.75, 0.0 }  // don't change!
static const GLfloat VertColor[4] = VERTCOLOR;
static const GLfloat Ambient[4] = AMBIENT;
static const GLfloat Diffuse[4] = DIFFUSE;
static const GLfloat Uniform1[4] = UNIFORM1;


// Shader program test cases
static const ShaderProgram Programs[] = {
	// Simple tests =======================================================
	{
		"Directly set fragment color",  // name
		NO_VERTEX_SHADER,  // vertex shader
		// fragment shader:
		"void main() { \n"
		"   gl_FragColor = vec4(1.0, 0.5, 0.25, 0.0); \n"
		"} \n",
		{ 1.0, 0.5, 0.25, 0.0 }, // expectedColor
		DONT_CARE_Z,  // expectedZ
		FLAG_NONE  // flags
	},

	{
		"Directly set vertex color",
		"void main() { \n"
		"   gl_Position = ftransform(); \n"
		"   gl_FrontColor = vec4(0.5, 1.0, 0.25, 0.0); \n"
		"} \n",
		NO_FRAGMENT_SHADER,
		{ 0.5, 1.0, 0.25, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Pass-through vertex color",
		// vert shader:
		"void main() { \n"
		"   gl_Position = ftransform(); \n"
		"   gl_FrontColor = vec4(0.25, 1.0, 0.75, 0.0); \n"
		"} \n",
		// frag shader:
		"void main() { \n"
		"   gl_FragColor = gl_Color; \n"
		"} \n",
		{ 0.25, 1.0, 0.75, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	// Swizzle, writemask =================================================
	{
		"Swizzle",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   vec4 a = vec4(0.5,  0.25, 0.0, 1.0); \n"
		"   gl_FragColor = a.yxxz; \n"
		"} \n",
		{ 0.25, 0.5, 0.5, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Writemask",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor = vec4(1.0); \n"
		"   gl_FragColor.x = 0.5; \n"
		"   gl_FragColor.z = 0.25; \n"
		"} \n",
		{ 0.5, 1.0, 0.25, 1.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Swizzled writemask",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor.zwxy = vec4(1.0, 0.5, 0.25, 0.75); \n"
		"} \n",
		{ 0.25, 0.75, 1.0, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Swizzled writemask (2)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor.zy = vec2(1.0, 0.5); \n"
		"   gl_FragColor.wx = vec2(0.25, 0.75); \n"
		"} \n",
		{ 0.75, 0.5, 1.0, 0.25 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	// Z-write ============================================================
	{
		"gl_FragDepth test",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor = vec4(0.5); \n"
		"   gl_FragDepth = 0.25; \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		0.25,  // Z value
		FLAG_NONE
	},

	// Basic arithmetic ===================================================
	{
		"Addition",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   vec4 a = vec4(0.5,  0.25, 0.0, 0.0); \n"
		"   vec4 b = vec4(0.25, 0.0,  0.2, 0.0); \n"
		"   gl_FragColor = a + b; \n"
		"} \n",
		{ 0.75, 0.25, 0.2, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"vec4, scalar arithmetic",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   vec4 a = vec4(0.5,  0.25, 0.2, 0.0); \n"
		"   vec4 b = vec4(0.25, 0.0,  0.0, 0.0); \n"
		"   gl_FragColor = a * 2.0 - b; \n"
		"} \n",
		{ 0.75, 0.50, 0.4, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"integer, float arithmetic",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   int k = 100; \n"
		"   gl_FragColor.x = k * 0.01; \n"
		"   gl_FragColor.y = k * 0.005; \n"
		"   gl_FragColor.z = k * 0.0025; \n"
		"   gl_FragColor.w = k * 0.0; \n"
		"} \n",
		{ 1.0, 0.5, 0.25, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"unary negation",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   vec4 v = vec4(-1.0, -0.5, 0.5, -0.25); \n"
		"   gl_FragColor = -v; \n"
		"} \n",
		{ 1.0, 0.5, 0.0, 0.25 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"integer division",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   int i = 15, j = 6; \n"
		"   int k = i / j; \n"
		"   gl_FragColor = vec4(k * 0.1); \n"
		"} \n",
		{ 0.2, 0.2, 0.2, 0.2 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"integer division with uniform var",
		NO_VERTEX_SHADER,
		"// as above, but prevent compile-time evaluation \n"
		"uniform vec4 uniform1; \n"
		"void main() { \n"
		"   int i = int(15 * uniform1.x); \n"
		"   int j = 6; \n"
		"   int k = i / j; \n"
		"   gl_FragColor = vec4(k * 0.1); \n"
		"} \n",
		{ 0.2, 0.2, 0.2, 0.2 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"assignment operators",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   vec4 v = vec4(0.0, 0.25, 0.5, 0.75); \n"
		"   v *= 2.0; \n"
		"   v -= vec4(-0.5, 0.0, 0.25, 1.0); \n"
		"   gl_FragColor = v; \n"
		"} \n",
		{ 0.5, 0.5, 0.75, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"post increment (x++)",
		NO_VERTEX_SHADER,
		"uniform vec4 uniform1; \n"
		"void main() { \n"
		"   float x = uniform1.y; // should be 0.25 \n"
		"   float y = x++; // y should be 0.25 \n"
		"   gl_FragColor = vec4(y); \n"
		"} \n",
		{ 0.25, 0.25, 0.25, 0.25 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"pre increment (++x)",
		NO_VERTEX_SHADER,
		"uniform vec4 uniform1; \n"
		"void main() { \n"
		"   float x = uniform1.y; // should be 0.25 \n"
		"   float y = ++x; // y should be 1.25 \n"
		"   gl_FragColor = vec4(y); \n"
		"} \n",
		{ 1.0, 1.0, 1.0, 1.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"post decrement (x--)",
		NO_VERTEX_SHADER,
		"uniform vec4 uniform1; \n"
		"void main() { \n"
		"   float x = uniform1.y; // should be 0.25 \n"
		"   float y = x--; // y should be 0.25 \n"
		"   gl_FragColor = vec4(y); \n"
		"} \n",
		{ 0.25, 0.25, 0.25, 0.25 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"pre decrement (--x)",
		NO_VERTEX_SHADER,
		"uniform vec4 uniform1; \n"
		"void main() { \n"
		"   float x = uniform1.y; // should be 0.25 \n"
		"   float y = --x; // y should be -0.75 \n"
		"   gl_FragColor = vec4(-y); // negate \n"
		"} \n",
		{ 0.75, 0.75, 0.75, 0.75 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	// built-in functions ================================================
	{
		"dot product",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   vec4 u = vec4(-1.0, 0.5, 0.5, -0.25); \n"
		"   vec4 v = vec4(0.5, 1.0, 0.5, 0.0); \n"
		"   gl_FragColor = vec4(dot(u, v)); \n"
		"} \n",
		{ 0.25, 0.25, 0.25, 0.25 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"max() function",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   vec4 u = vec4(-1.0, 0.5, 0.5, -0.25); \n"
		"   vec4 v = vec4(0.5, 1.0, 0.5, 0.0); \n"
		"   gl_FragColor = max(u, v); \n"
		"} \n",
		{ 0.5, 1.0, 0.5, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"length() function",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   vec3 u = vec3(0.25, 0.1, 0.2); \n"
		"   gl_FragColor = vec4(length(u)); \n"
		"} \n",
		{ 0.335, 0.335, 0.335, 0.335 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"clamp() function",
		NO_VERTEX_SHADER,
		"uniform vec4 uniform1; \n"
		"void main() { \n"
		"   vec4 u = uniform1 * vec4(3.0); \n"
		"   gl_FragColor = clamp(u, 0.0, 1.0); \n"
		"} \n",
		{ 1.0, 0.75, 1.0, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"clamp() function, vec4",
		NO_VERTEX_SHADER,
		"uniform vec4 uniform1; \n"
		"void main() { \n"
		"   vec4 u = uniform1; \n"
		"   gl_FragColor = clamp(u, vec4(0.2), vec4(0.8)); \n"
		"} \n",
		{ 0.8, 0.25, 0.75, 0.2 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	// Flow Control ======================================================
	{
		"while-loop",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   float sum = 0.0; \n"
		"   while (sum < 0.5) { \n"
		"      sum += 0.1; \n"
		"   } \n"
		"   gl_FragColor = vec4(sum); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"do-loop",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   float sum = 0.0; \n"
		"   do { \n"
		"      sum += 0.1; \n"
		"   } while (sum < 0.5); \n"
		"   gl_FragColor = vec4(sum); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"for-loop",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   vec4 sum = vec4(0.0); \n"
		"   int i; \n"
		"   for (i = 0; i < 5; ++i) { \n"
		"      sum += vec4(0.1); \n"
		"   } \n"
		"   gl_FragColor = sum; \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"simple if statement",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   // this should always be true \n"
		"   if (gl_FragCoord.x >= 0.0) { \n"
		"      gl_FragColor = vec4(0.5, 0.0, 0.5, 0.0); \n"
		"   } \n"
		"} \n",
		{ 0.5, 0.0, 0.5, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"simple if/else statement",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   // this should always be false \n"
		"   if (gl_FragCoord.x < 0.0) { \n"
		"      gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0); \n"
                "   } else { \n"
		"      gl_FragColor = vec4(0.5, 0.25, 0.5, 0.0); \n"
		"   } \n"
		"} \n",
		{ 0.5, 0.25, 0.5, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"do-loop with break",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   float sum = 0.0; \n"
		"   do { \n"
		"      sum += 0.1; \n"
		"      if (sum >= 0.5) \n"
		"         break; \n"
		"   } while (1); \n"
		"   gl_FragColor = vec4(sum); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"do-loop with continue and break",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   float sum = 0.0; \n"
		"   do { \n"
		"      sum += 0.1; \n"
		"      if (sum < 0.5) \n"
		"         continue; \n"
		"      break; \n"
		"   } while (1); \n"
		"   gl_FragColor = vec4(sum); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"simple subroutine",
		NO_VERTEX_SHADER,
		"vec4 avg(const in vec4 a, const in vec4 b) { \n"
		"   return (a + b) * 0.5; \n"
		"} \n"
		"\n"
		"void main() { \n"
		"   vec4 a = vec4(1.0, 0.0, 0.5, 0.0); \n"
		"   vec4 b = vec4(0.0, 0.8, 0.5, 0.0); \n"
		"   gl_FragColor = avg(a, b); \n"
		"} \n",
		{ 0.5, 0.4, 0.5, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"discard statement (1)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor = vec4(1.0); \n"
		"   if (gl_TexCoord[0].x < 0.5) \n"
		"      discard; \n"
		"} \n",
		{ 0.0, 0.0, 0.0, 0.0 },  // glClear color
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"discard statement (2)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor = vec4(1.0); \n"
		"   if (gl_TexCoord[0].x > 0.5) \n"
		"      discard; \n"
		"} \n",
		{ 1.0, 1.0, 1.0, 1.0 },  // fragment color
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"conditional expression",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor = gl_FragCoord.x < 0.0 ? vec4(0.0) : vec4(0.5); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"array with constant indexing",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   float ar[4]; \n"
		"   ar[0] = 0.5; \n"
		"   ar[1] = 1.0; \n"
		"   ar[2] = 0.25; \n"
		"   ar[3] = 0.2; \n"
		"   gl_FragColor.x = ar[0]; \n"
		"   gl_FragColor.y = ar[1]; \n"
		"   gl_FragColor.z = ar[2]; \n"
		"   gl_FragColor.w = ar[3]; \n"
		"} \n",
		{ 0.5, 1.0, 0.25, 0.2 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	// Logical operators =================================================
	{
		"&& operator (1)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor = vec4(0.25); \n"
		"   // this should always be true \n"
		"   if (gl_FragCoord.x >= 0.0 && gl_FragCoord.y >= 0.0) { \n"
		"      gl_FragColor = vec4(0.5, 0.0, 0.5, 0.0); \n"
		"   } \n"
		"} \n",
		{ 0.5, 0.0, 0.5, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"&& operator (2)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor = vec4(0.25); \n"
		"   // this should always be false \n"
		"   if (gl_FragCoord.x >= 0.0 && gl_FragCoord.y < 0.0) { \n"
		"      gl_FragColor = vec4(0.5, 0.0, 0.5, 0.0); \n"
		"   } \n"
		"} \n",
		{ 0.25, 0.25, 0.25, 0.25 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"&& operator, short-circuit test",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   float x = 0.75; \n"
		"   // this should always be false \n"
		"   if (x <= 0.5 && ++x) { \n"
		"      x += 0.1; \n"
		"   } \n"
		"   gl_FragColor = vec4(x); \n"
		"} \n",
		{ 0.75, 0.75, 0.75, 0.75 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"|| operator (1)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor = vec4(0.25); \n"
		"   // this should always be true \n"
		"   if (gl_FragCoord.x < 0.0 || gl_FragCoord.y >= 0.0) { \n"
		"      gl_FragColor = vec4(0.5, 0.0, 0.5, 0.0); \n"
		"   } \n"
		"} \n",
		{ 0.5, 0.0, 0.5, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"|| operator (2)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor = vec4(0.25); \n"
		"   // this should always be false \n"
		"   if (gl_FragCoord.x < 0.0 || gl_FragCoord.y < 0.0) { \n"
		"      gl_FragColor = vec4(0.5, 0.0, 0.5, 0.0); \n"
		"   } \n"
		"} \n",
		{ 0.25, 0.25, 0.25, 0.25 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"|| operator, short-circuit test",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   float x = 0.75; \n"
		"   // this should always be true \n"
		"   if (x >= 0.5 || ++x) { \n"
		"      x += 0.1; \n"
		"   } \n"
		"   gl_FragColor = vec4(x); \n"
		"} \n",
		{ 0.85, 0.85, 0.85, 0.85 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"^^ operator (1)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor = vec4(0.25); \n"
		"   // this should always be true \n"
		"   if (gl_FragCoord.x < 0.0 ^^ gl_FragCoord.y >= 0.0) { \n"
		"      gl_FragColor = vec4(0.5); \n"
		"   } \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"^^ operator (2)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor = vec4(0.25); \n"
		"   // this should always be false \n"
		"   if (gl_FragCoord.x >= 0.0 ^^ gl_FragCoord.y >= 0.0) { \n"
		"      gl_FragColor = vec4(0.5); \n"
		"   } \n"
		"} \n",
		{ 0.25, 0.25, 0.25, 0.25 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	// Uniform & Varying vars ============================================
	{
		"uniform variable (fragment shader)",
		NO_VERTEX_SHADER,
		"uniform vec4 uniform1; \n"
		"void main() { \n"
		"   gl_FragColor = uniform1; \n"
		"} \n",
		UNIFORM1,
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"uniform variable (vertex shader)",
		"uniform vec4 uniform1; \n"
		"void main() { \n"
		"   gl_FrontColor = uniform1; \n"
		"   gl_Position = ftransform(); \n"
		"} \n",
		NO_FRAGMENT_SHADER,
		UNIFORM1,
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"varying variable",
		// vertex program:
		"varying vec4 var1; \n"
		"void main() { \n"
		"   var1 = vec4(1.0, 0.5, 0.25, 0.0); \n"
		"   gl_Position = ftransform(); \n"
		"} \n",
		// fragment program:
		"varying vec4 var1; \n"
		"void main() { \n"
		"   gl_FragColor = var1; \n"
		"} \n",
		{ 1.0, 0.5, 0.25, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	// GL state refs =====================================================
	{
		"GL state variable reference (gl_FrontMaterial.ambient)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor = gl_FrontMaterial.ambient; \n"
		"} \n",
		AMBIENT,
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"GL state variable reference (gl_LightSource[0].diffuse)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor = gl_LightSource[0].diffuse; \n"
		"} \n",
		DIFFUSE,
		DONT_CARE_Z,
		FLAG_NONE
	},

	// Texture functions ==================================================
	{
		"2D Texture lookup",
		NO_VERTEX_SHADER,
		"uniform sampler2D tex2d; \n"
		"void main() { \n"
		"   gl_FragColor = texture2D(tex2d, gl_TexCoord[0].xy);\n"
		"} \n",
		{ 1.0, 0.0, 0.0, 1.0 },
		DONT_CARE_Z,
		FLAG_NONE

	},

	{
		"2D Texture lookup, computed coordinate",
		NO_VERTEX_SHADER,
		"uniform sampler2D tex2d; \n"
		"void main() { \n"
		"   vec2 coord = gl_TexCoord[0].xy + vec2(0.5); \n"
		"   gl_FragColor = texture2D(tex2d, coord, 0.0); \n"
		"} \n",
		{ 1.0, 1.0, 1.0, 1.0 },  // upper-right tex color
		DONT_CARE_Z,
		FLAG_NONE

	},

	{
		"2D Texture lookup with bias",
		NO_VERTEX_SHADER,
		"uniform sampler2D tex2d; \n"
		"void main() { \n"
		"   gl_FragColor = texture2D(tex2d, gl_TexCoord[0].xy, 1.0);\n"
		"} \n",
		{ 0.5, 0.0, 0.0, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE

	},

#if 0 // XXX not enabled yet
	{
		"2D Texture lookup with explicit lod (Vertex shader)",
		"uniform sampler2D tex2d; \n"
		"void main() { \n"
		"   //gl_FrontColor = texture2DLod(tex2d, gl_TexCoord[0].xy, 2.0);\n"
		"   gl_Position = ftransform(); \n"
		"} \n",
		NO_FRAGMENT_SHADER,
		{ 0.25, 0.0, 0.0, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE

	},
#endif

	{
		"2D Texture lookup with projection",
		NO_VERTEX_SHADER,
		"uniform sampler2D tex2d; \n"
		"void main() { \n"
		"   vec4 coord = gl_TexCoord[0] * vec4(2.25); \n"
		"   // 'proj' will divide components by w (=2.25) \n"
		"   gl_FragColor = texture2DProj(tex2d, coord);\n"
		"} \n",
		{ 1.0, 0.0, 0.0, 1.0 },
		DONT_CARE_Z,
		FLAG_NONE

	},

	{
		"1D Texture lookup",
		NO_VERTEX_SHADER,
		"uniform sampler1D tex1d; \n"
		"void main() { \n"
		"   gl_FragColor = texture1D(tex1d, gl_TexCoord[0].x);\n"
		"} \n",
		{ 1.0, 0.0, 0.0, 1.0 },
		DONT_CARE_Z,
		FLAG_NONE

	},

	{
		"3D Texture lookup",
		NO_VERTEX_SHADER,
		"uniform sampler3D tex3d; \n"
		"void main() { \n"
		"   gl_FragColor = texture3D(tex3d, gl_TexCoord[0].xyz);\n"
		"} \n",
		{ 1.0, 0.0, 0.0, 1.0 },
		DONT_CARE_Z,
		FLAG_NONE

	},

	{
		"3D Texture lookup, computed coord",
		NO_VERTEX_SHADER,
		"uniform sampler3D tex3d; \n"
		"void main() { \n"
		"   vec3 coord = gl_TexCoord[0].xyz; \n"
		"   coord.y = 0.75; \n"
		"   coord.z = 0.75; \n"
		"   gl_FragColor = texture3D(tex3d, coord); \n"
		"} \n",
		{ 0.0, 0.0, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE

	},

	// Preprocessor tests ================================================
	{
		"Preprocessor test (1)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"#if 0 \n"
		"   gl_FragColor = vec4(0.5); \n"
		"#else \n"
		"   gl_FragColor = vec4(0.3); \n"
		"#endif \n"
		"} \n",
		{ 0.3, 0.3, 0.3, 0.3 },
		DONT_CARE_Z,
		FLAG_NONE

	},

	{
		"Preprocessor test (2)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"#if 1 \n"
		"   gl_FragColor = vec4(0.5); \n"
		"#else \n"
		"   gl_FragColor = vec4(0.3); \n"
		"#endif \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE

	},

	{
		"Preprocessor test (3)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"#define SYMBOL 3 \n"
		"#if SYMBOL == 3 \n"
		"   gl_FragColor = vec4(0.5); \n"
		"#else \n"
		"   gl_FragColor = vec4(0.3); \n"
		"#endif \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE

	},

	{ NULL, NULL, NULL, {0,0,0,0}, 0, FLAG_NONE } // end of list sentinal
};



// Get ptrs to 2.0 API functions.
bool
GLSLTest::getFunctions(void)
{
	glAttachShader_func = (PFNGLATTACHSHADERPROC) GLUtils::getProcAddress("glAttachShader");
	if (!glAttachShader_func)
		return false;
	glBindAttribLocation_func = (PFNGLBINDATTRIBLOCATIONPROC) GLUtils::getProcAddress("glBindAttribLocation");
	if (!glBindAttribLocation_func)
		return false;
	glCompileShader_func = (PFNGLCOMPILESHADERPROC) GLUtils::getProcAddress("glCompileShader");
	if (!glCompileShader_func)
		return false;
	glCreateProgram_func = (PFNGLCREATEPROGRAMPROC) GLUtils::getProcAddress("glCreateProgram");
	if (!glCreateProgram_func)
		return false;
	glCreateShader_func = (PFNGLCREATESHADERPROC) GLUtils::getProcAddress("glCreateShader");
	if (!glCreateShader_func)
		return false;
	glDeleteProgram_func = (PFNGLDELETEPROGRAMPROC) GLUtils::getProcAddress("glDeleteProgram");
	if (!glDeleteProgram_func)
		return false;
	glDeleteShader_func = (PFNGLDELETESHADERPROC) GLUtils::getProcAddress("glDeleteShader");
	if (!glDeleteShader_func)
		return false;
	glGetAttachedShaders_func = (PFNGLGETATTACHEDSHADERSPROC) GLUtils::getProcAddress("glGetAttachedShaders");
	if (!glGetAttachedShaders_func)
		return false;
	glGetAttribLocation_func = (PFNGLGETATTRIBLOCATIONPROC) GLUtils::getProcAddress("glGetAttribLocation");
	if (!glGetAttribLocation_func)
		return false;
	glGetProgramInfoLog_func = (PFNGLGETPROGRAMINFOLOGPROC) GLUtils::getProcAddress("glGetProgramInfoLog");
	if (!glGetProgramInfoLog_func)
		return false;
	glGetShaderInfoLog_func = (PFNGLGETSHADERINFOLOGPROC) GLUtils::getProcAddress("glGetShaderInfoLog");
	if (!glGetShaderInfoLog_func)
		return false;
	glGetProgramiv_func = (PFNGLGETPROGRAMIVPROC) GLUtils::getProcAddress("glGetProgramiv");
	if (!glGetProgramiv_func)
		return false;
	glGetShaderiv_func = (PFNGLGETSHADERIVPROC) GLUtils::getProcAddress("glGetShaderiv");
	if (!glGetShaderiv_func)
		return false;
	glGetShaderSource_func = (PFNGLGETSHADERSOURCEPROC) GLUtils::getProcAddress("glGetShaderSource");
	if (!glGetShaderSource_func)
		return false;
	glGetUniformLocation_func = (PFNGLGETUNIFORMLOCATIONPROC) GLUtils::getProcAddress("glGetUniformLocation");
	if (!glGetUniformLocation_func)
		return false;
	glGetUniformfv_func = (PFNGLGETUNIFORMFVPROC) GLUtils::getProcAddress("glGetUniformfv");
	if (!glGetUniformfv_func)
		return false;
	glIsProgram_func = (PFNGLISPROGRAMPROC) GLUtils::getProcAddress("glIsProgram");
	if (!glIsProgram_func)
		return false;
	glIsShader_func = (PFNGLISSHADERPROC) GLUtils::getProcAddress("glIsShader");
	if (!glIsShader_func)
		return false;
	glLinkProgram_func = (PFNGLLINKPROGRAMPROC) GLUtils::getProcAddress("glLinkProgram");
	if (!glLinkProgram_func)
		return false;
	glShaderSource_func = (PFNGLSHADERSOURCEPROC) GLUtils::getProcAddress("glShaderSource");
	if (!glShaderSource_func)
		return false;
	glUniform1i_func = (PFNGLUNIFORM1IPROC) GLUtils::getProcAddress("glUniform1i");
	if (!glUniform1i_func)
		return false;
	glUniform1fv_func = (PFNGLUNIFORM1FVPROC) GLUtils::getProcAddress("glUniform1fv");
	if (!glUniform1fv_func)
		return false;
	glUniform2fv_func = (PFNGLUNIFORM2FVPROC) GLUtils::getProcAddress("glUniform2fv");
	if (!glUniform2fv_func)
		return false;
	glUniform3fv_func = (PFNGLUNIFORM3FVPROC) GLUtils::getProcAddress("glUniform3fv");
	if (!glUniform3fv_func)
		return false;
	glUniform4fv_func = (PFNGLUNIFORM3FVPROC) GLUtils::getProcAddress("glUniform4fv");
	if (!glUniform4fv_func)
		return false;
	glUniformMatrix2fv_func = (PFNGLUNIFORMMATRIX2FVPROC) GLUtils::getProcAddress("glUniformMatrix2fv");
	if (!glUniformMatrix2fv_func)
		return false;
	glUniformMatrix3fv_func = (PFNGLUNIFORMMATRIX3FVPROC) GLUtils::getProcAddress("glUniformMatrix3fv");
	if (!glUniformMatrix3fv_func)
		return false;
	glUniformMatrix4fv_func = (PFNGLUNIFORMMATRIX4FVPROC) GLUtils::getProcAddress("glUniformMatrix4fv");
	if (!glUniformMatrix4fv_func)
		return false;
	glUseProgram_func = (PFNGLUSEPROGRAMPROC) GLUtils::getProcAddress("glUseProgram");
	if (!glUseProgram_func)
		return false;
	glVertexAttrib1f_func = (PFNGLVERTEXATTRIB1FPROC) GLUtils::getProcAddress("glVertexAttrib1f");
	if (!glVertexAttrib1f_func)
		return false;
	glVertexAttrib2f_func = (PFNGLVERTEXATTRIB2FPROC) GLUtils::getProcAddress("glVertexAttrib2f");
	if (!glVertexAttrib2f_func)
		return false;
	glVertexAttrib3f_func = (PFNGLVERTEXATTRIB3FPROC) GLUtils::getProcAddress("glVertexAttrib3f");
	if (!glVertexAttrib3f_func)
		return false;
	glVertexAttrib4f_func = (PFNGLVERTEXATTRIB4FPROC) GLUtils::getProcAddress("glVertexAttrib4f");
	if (!glVertexAttrib4f_func)
		return false;

	return true;
}


void
GLSLTest::setupTextures(void)
{
	GLubyte teximage0[16][16][4];
	GLubyte teximage1[8][8][4];
	GLubyte teximage2[4][4][4];
	GLubyte teximage3D[16][16][16][4];
	GLint i, j, k;

	//  +-------+-------+
	//  | blue  | white |
	//  +-------+-------+
	//  | red   | green |
	//  +-------+-------+
	for (i = 0; i < 16; i++) {
		for (j = 0; j < 16; j++) {
			if (i < 8) {
				// bottom half
				if (j < 8) {
					// red
					teximage0[i][j][0] = 255;
					teximage0[i][j][1] = 0;
					teximage0[i][j][2] = 0;
					teximage0[i][j][3] = 255;
				}
				else {
					// green
					teximage0[i][j][0] = 0;
					teximage0[i][j][1] = 255;
					teximage0[i][j][2] = 0;
					teximage0[i][j][3] = 255;
				}
			}
			else {
				// top half
				if (j < 8) {
					// blue
					teximage0[i][j][0] = 0;
					teximage0[i][j][1] = 0;
					teximage0[i][j][2] = 255;
					teximage0[i][j][3] = 255;
				}
				else {
					// white
					teximage0[i][j][0] = 255;
					teximage0[i][j][1] = 255;
					teximage0[i][j][2] = 255;
					teximage0[i][j][3] = 255;
				}
			}
		}
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, teximage0);

	// level 1: same colors, half intensity
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			teximage1[i][j][0] = teximage0[i*2][j*2][0] / 2;
			teximage1[i][j][1] = teximage0[i*2][j*2][1] / 2;
			teximage1[i][j][2] = teximage0[i*2][j*2][2] / 2;
			teximage1[i][j][3] = teximage0[i*2][j*2][3] / 2;
		}
	}
	glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 8, 8, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, teximage1);

	// level 2: 1/4 intensity
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			teximage2[i][j][0] = teximage0[i*4][j*4][0] / 4;
			teximage2[i][j][1] = teximage0[i*4][j*4][1] / 4;
			teximage2[i][j][2] = teximage0[i*4][j*4][2] / 4;
			teximage2[i][j][3] = teximage0[i*4][j*4][3] / 4;
		}
	}
	glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA, 4, 4, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, teximage2);

	// level 3, 4: don't care
	glTexImage2D(GL_TEXTURE_2D, 3, GL_RGBA, 2, 2, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, teximage0);
	glTexImage2D(GL_TEXTURE_2D, 4, GL_RGBA, 1, 1, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, teximage0);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	// 1D texture: just bottom row of the 2D texture
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 16, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, teximage0);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// 3D texture: 2D texture, depth = 1
	for (i = 0; i < 16; i++) {
		for (j = 0; j < 16; j++) {
			for (k = 0; k < 16; k++) {
				if (i < 8) {
					teximage3D[i][j][k][0] = teximage0[j][k][0];
					teximage3D[i][j][k][1] = teximage0[j][k][1];
					teximage3D[i][j][k][2] = teximage0[j][k][2];
					teximage3D[i][j][k][3] = teximage0[j][k][3];
				}
				else {
					// back half: half intensity
					teximage3D[i][j][k][0] = teximage0[j][k][0] / 2;
					teximage3D[i][j][k][1] = teximage0[j][k][1] / 2;
					teximage3D[i][j][k][2] = teximage0[j][k][2] / 2;
					teximage3D[i][j][k][3] = teximage0[j][k][3] / 2;
				}
			}
		}
	}

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 16, 16, 16, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, teximage3D);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}


bool
GLSLTest::setup(void)
{
	// check that we have OpenGL 2.0
	const char *version = (const char *) glGetString(GL_VERSION);
	if (version[0] != '2' || version[1] != '.') {
		env->log << "OpenGL 2.0 not supported\n";
		return false;
	}

	if (!getFunctions()) {
		env->log << "Unable to get pointer to an OpenGL 2.0 API function\n";
		return false;
	}

	setupTextures();

	// load program inputs
	glColor4fv(VertColor);

	// other GL state
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, Diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, VertColor);

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

        // Some tests request a looser tolerance:
        // XXX a factor of 4 may be too much...
        for (int i = 0; i < 5; i++)
                looseTolerance[i] = 4.0 * tolerance[i];

	return true;
}


void
GLSLTest::reportFailure(const char *programName,
				   const GLfloat expectedColor[4],
				   const GLfloat actualColor[4] ) const
{
	env->log << "FAILURE:\n";
	env->log << "  Shader test: " << programName << "\n";
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
GLSLTest::reportZFailure(const char *programName,
				  GLfloat expectedZ, GLfloat actualZ) const
{
	env->log << "FAILURE:\n";
	env->log << "  Program: " << programName << "\n";
	env->log << "  Expected Z: " << expectedZ << "\n";
	env->log << "  Observed Z: " << actualZ << "\n";
}



// Compare actual and expected colors
bool
GLSLTest::equalColors(const GLfloat act[4], const GLfloat exp[4], int flags) const
{
        const GLfloat *tol;
        if (flags & FLAG_LOOSE)
                tol = looseTolerance;
        else
                tol = tolerance;
	if ((fabsf(act[0] - exp[0]) > tol[0] && exp[0] != DONT_CARE_COLOR) ||
	    (fabsf(act[1] - exp[1]) > tol[1] && exp[1] != DONT_CARE_COLOR) ||
	    (fabsf(act[2] - exp[2]) > tol[2] && exp[2] != DONT_CARE_COLOR) ||
	    (fabsf(act[3] - exp[3]) > tol[3] && exp[3] != DONT_CARE_COLOR))
		return false;
	else
		return true;
}


bool
GLSLTest::equalDepth(GLfloat z0, GLfloat z1) const
{
	if (fabsf(z0 - z1) > tolerance[4])
		return false;
	else
		return true;
}


GLuint
GLSLTest::loadAndCompileShader(GLenum target, const char *str)
{
	GLint stat;
	GLuint shader;

	shader = glCreateShader_func(target);
	glShaderSource_func(shader, 1,
			    (const GLchar **) &str, NULL);
	glCompileShader_func(shader);
	glGetShaderiv_func(shader, GL_COMPILE_STATUS, &stat);
	if (!stat) {
		GLchar log[1000];
		GLsizei len;
		glGetShaderInfoLog_func(shader, 1000, &len, log);
		if (target == GL_FRAGMENT_SHADER)
			env->log << "Fragment Shader error: ";
		else
			env->log << "Vertex Shader error: ";
		env->log << log;
		return 0;
	}
	return shader;
}


bool
GLSLTest::testProgram(const ShaderProgram &p)
{
	const GLfloat r = 0.62; // XXX draw 16x16 pixel quad
	GLuint fragShader = 0, vertShader = 0, program;
	GLint u1, utex1d, utex2d, utex3d;

	if (p.fragShaderString) {
		fragShader = loadAndCompileShader(GL_FRAGMENT_SHADER,
						  p.fragShaderString);
	}
	if (p.vertShaderString) {
		vertShader = loadAndCompileShader(GL_VERTEX_SHADER,
						  p.vertShaderString);
	}
	if (!fragShader && !vertShader) {
		// must have had a compilation errror
		return false;
	}

	program = glCreateProgram_func();
	if (fragShader)
		glAttachShader_func(program, fragShader);
	if (vertShader)
		glAttachShader_func(program, vertShader);
	glLinkProgram_func(program);

	// check link
	{
		GLint stat;
		glGetProgramiv_func(program, GL_LINK_STATUS, &stat);
		if (!stat) {
			GLchar log[1000];
			GLsizei len;
			glGetProgramInfoLog_func(program, 1000, &len, log);
			env->log << "FAILURE:\n";
			env->log << "  Shader test: " << p.name << "\n";
			env->log << "  Link error: ";
			env->log << log;
			return false;
		}
	}

	glUseProgram_func(program);

	// load uniform vars
	u1 = glGetUniformLocation_func(program, "uniform1");
	if (u1 >= 0)
		glUniform4fv_func(u1, 1, Uniform1);

	utex1d = glGetUniformLocation_func(program, "tex1d");
	if (utex1d >= 0)
		glUniform1i_func(utex1d, 0);  // bind to tex unit 0

	utex2d = glGetUniformLocation_func(program, "tex2d");
	if (utex2d >= 0)
		glUniform1i_func(utex2d, 0);  // bind to tex unit 0

	utex3d = glGetUniformLocation_func(program, "tex3d");
	if (utex3d >= 0)
		glUniform1i_func(utex3d, 0);  // bind to tex unit 0


	// to avoid potential issue with undefined result.depth.z
	if (p.expectedZ == DONT_CARE_Z)
		glDisable(GL_DEPTH_TEST);
	else
		glEnable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBegin(GL_POLYGON);
	glTexCoord2f(0, 0);  glVertex2f(-r, -r);
	glTexCoord2f(1, 0);  glVertex2f( r, -r);
	glTexCoord2f(1, 1);  glVertex2f( r,  r);
	glTexCoord2f(0, 1);  glVertex2f(-r,  r);
	glEnd();

	// read a pixel from lower-left corder of rendered quad
	GLfloat pixel[4];
	glReadPixels(windowSize / 2 - 2, windowSize / 2 - 2, 1, 1,
		     GL_RGBA, GL_FLOAT, pixel);
	if (0) // debug
           printf("%s: Expect: %.3f %.3f %.3f %.3f  found: %.3f %.3f %.3f %.3f\n",
                  p.name,
                  p.expectedColor[0], p.expectedColor[1],
                  p.expectedColor[2], p.expectedColor[3], 
                  pixel[0], pixel[1], pixel[2], pixel[3]);

	if (!equalColors(pixel, p.expectedColor, p.flags)) {
		reportFailure(p.name, p.expectedColor, pixel);
		// XXX clean-up
		return false;
	}

	if (p.expectedZ != DONT_CARE_Z) {
		GLfloat z;
		// read z at center of quad
		glReadPixels(windowSize / 2, windowSize / 2, 1, 1,
			     GL_DEPTH_COMPONENT, GL_FLOAT, &z);
		if (!equalDepth(z, p.expectedZ)) {
			reportZFailure(p.name, p.expectedZ, z);
			// XXX clean-up
			return false;
		}
	}

	if (0) // debug
	   printf("%s passed\n", p.name);

	// clean-up
	if (fragShader)
		glDeleteShader_func(fragShader);
	if (vertShader)
		glDeleteShader_func(vertShader);
	glDeleteProgram_func(program);

	return true;
}


void
GLSLTest::runOne(MultiTestResult &r, Window &w)
{
	(void) w;
	if (!setup()) {
		r.pass = false;
		return;
	}

	for (int i = 0; Programs[i].name; i++) {
		if (!testProgram(Programs[i])) {
			r.numFailed++;
		}
		else {
			r.numPassed++;
		}
	}
	r.pass = (r.numFailed == 0);
}


// The test object itself:
GLSLTest glslTest("glsl1", "window, rgb, z",
		  "",  // no extension filter (we'll test for version 2.x during setup)
		  "GLSL test 1: test basic Shading Language functionality.\n"
		  );



} // namespace GLEAN
