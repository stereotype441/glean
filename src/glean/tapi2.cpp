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

// tapi2.h:  Test OpenGL 2.x API functions/features
// Brian Paul  9 March 2007

#define GL_GLEXT_PROTOTYPES

#include "tapi2.h"
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
static PFNGLUNIFORM1FVPROC glUniform1fv_func = NULL;
static PFNGLUNIFORM2FVPROC glUniform2fv_func = NULL;
static PFNGLUNIFORM3FVPROC glUniform3fv_func = NULL;
static PFNGLUNIFORM4FVPROC glUniform4fv_func = NULL;
static PFNGLUNIFORM1FPROC glUniform1f_func = NULL;
static PFNGLUNIFORM2FPROC glUniform2f_func = NULL;
static PFNGLUNIFORM3FPROC glUniform3f_func = NULL;
static PFNGLUNIFORM4FPROC glUniform4f_func = NULL;
static PFNGLUNIFORM1IPROC glUniform1i_func = NULL;
static PFNGLUNIFORM2IPROC glUniform2i_func = NULL;
static PFNGLUNIFORM3IPROC glUniform3i_func = NULL;
static PFNGLUNIFORM4IPROC glUniform4i_func = NULL;
static PFNGLUNIFORM1IVPROC glUniform1iv_func = NULL;
static PFNGLUNIFORM2IVPROC glUniform2iv_func = NULL;
static PFNGLUNIFORM3IVPROC glUniform3iv_func = NULL;
static PFNGLUNIFORM4IVPROC glUniform4iv_func = NULL;
static PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv_func = NULL;
static PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv_func = NULL;
static PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv_func = NULL;
static PFNGLUSEPROGRAMPROC glUseProgram_func = NULL;
static PFNGLVERTEXATTRIB1FPROC glVertexAttrib1f_func = NULL;
static PFNGLVERTEXATTRIB2FPROC glVertexAttrib2f_func = NULL;
static PFNGLVERTEXATTRIB3FPROC glVertexAttrib3f_func = NULL;
static PFNGLVERTEXATTRIB4FPROC glVertexAttrib4f_func = NULL;

static PFNGLSTENCILOPSEPARATEPROC glStencilOpSeparate_func = NULL;
static PFNGLSTENCILFUNCSEPARATEPROC glStencilFuncSeparate_func = NULL;
static PFNGLSTENCILMASKSEPARATEPROC glStencilMaskSeparate_func = NULL;

static PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate_func = NULL;
static PFNGLDRAWBUFFERSPROC glDrawBuffers_func = NULL;


#define FLAG_LOOSE 0x1
#define DONT_CARE_COLOR -1.0

#define PRIMARY_R 0.25
#define PRIMARY_G 0.75
#define PRIMARY_B 0.5
#define PRIMARY_A 0.25

#define UNIFORM1 {1.0, 0.25, 0.75, 0.0 }  // don't change!


static const GLfloat PrimaryColor[4] = { PRIMARY_R, PRIMARY_G,
					 PRIMARY_B, PRIMARY_A };

static const GLfloat Uniform1[4] = UNIFORM1;




// Get ptrs to 2.0 API functions.
bool
API2Test::getFunctions_2_0(void)
{
	// shading language
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
	glUniform1f_func = (PFNGLUNIFORM1FPROC) GLUtils::getProcAddress("glUniform1f");
	if (!glUniform1f_func)
		return false;
	glUniform2f_func = (PFNGLUNIFORM2FPROC) GLUtils::getProcAddress("glUniform2f");
	if (!glUniform2f_func)
		return false;
	glUniform3f_func = (PFNGLUNIFORM3FPROC) GLUtils::getProcAddress("glUniform3f");
	if (!glUniform3f_func)
		return false;
	glUniform4f_func = (PFNGLUNIFORM4FPROC) GLUtils::getProcAddress("glUniform4f");
	if (!glUniform4f_func)
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
	glUniform1i_func = (PFNGLUNIFORM1IPROC) GLUtils::getProcAddress("glUniform1i");
	if (!glUniform1i_func)
		return false;
	glUniform2i_func = (PFNGLUNIFORM2IPROC) GLUtils::getProcAddress("glUniform2i");
	if (!glUniform2i_func)
		return false;
	glUniform3i_func = (PFNGLUNIFORM3IPROC) GLUtils::getProcAddress("glUniform3i");
	if (!glUniform3i_func)
		return false;
	glUniform4i_func = (PFNGLUNIFORM4IPROC) GLUtils::getProcAddress("glUniform4i");
	if (!glUniform4i_func)
		return false;
	glUniform1iv_func = (PFNGLUNIFORM1IVPROC) GLUtils::getProcAddress("glUniform1iv");
	if (!glUniform1iv_func)
		return false;
	glUniform2iv_func = (PFNGLUNIFORM2IVPROC) GLUtils::getProcAddress("glUniform2iv");
	if (!glUniform2iv_func)
		return false;
	glUniform3iv_func = (PFNGLUNIFORM3IVPROC) GLUtils::getProcAddress("glUniform3iv");
	if (!glUniform3iv_func)
		return false;
	glUniform4iv_func = (PFNGLUNIFORM4IVPROC) GLUtils::getProcAddress("glUniform4iv");
	if (!glUniform4iv_func)
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

	// stencil
	glStencilOpSeparate_func = (PFNGLSTENCILOPSEPARATEPROC) GLUtils::getProcAddress("glStencilOpSeparate");
	if (!glStencilOpSeparate_func)
		return false;
	glStencilFuncSeparate_func = (PFNGLSTENCILFUNCSEPARATEPROC) GLUtils::getProcAddress("glStencilFuncSeparate");
	if (!glStencilFuncSeparate_func)
		return false;
	glStencilMaskSeparate_func = (PFNGLSTENCILMASKSEPARATEPROC) GLUtils::getProcAddress("glStencilMaskSeparate");
	if (!glStencilMaskSeparate_func)
		return false;

	glBlendEquationSeparate_func = (PFNGLBLENDEQUATIONSEPARATEPROC) GLUtils::getProcAddress("glBlendEquationSeparate");
	if (!glBlendEquationSeparate_func)
		return false;

	glDrawBuffers_func = (PFNGLDRAWBUFFERSPROC) GLUtils::getProcAddress("glDrawBuffers");
	if (!glDrawBuffers_func)
		return false;

	return true;
}


bool
API2Test::setup(void)
{
	// check that we have OpenGL 2.0
	const char *version = (const char *) glGetString(GL_VERSION);
	if (version[0] != '2' || version[1] != '.') {
		env->log << "OpenGL 2.0 not supported\n";
		return false;
	}

	if (!getFunctions_2_0()) {
		env->log << "Unable to get pointer to an OpenGL 2.0 API function\n";
		return false;
	}

	// load program inputs
	glColor4fv(PrimaryColor);

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
API2Test::reportFailure(const char *msg) const
{
	env->log << "FAILURE: " << msg << "\n";
}

void
API2Test::reportFailure(const char *msg, GLenum target) const
{
	env->log << "FAILURE: " << msg;
	if (target == GL_FRAGMENT_SHADER)
		env->log << " (fragment)";
	else
		env->log << " (vertex)";
	env->log << "\n";
}



// Compare actual and expected colors
bool
API2Test::equalColors(const GLfloat act[4], const GLfloat exp[4], int flags) const
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
API2Test::equalDepth(GLfloat z0, GLfloat z1) const
{
	if (fabsf(z0 - z1) > tolerance[4])
		return false;
	else
		return true;
}


// Render test quad w/ current shader program, return RGBA color of quad
void
API2Test::renderQuad(GLfloat *pixel) const
{
	const GLfloat r = 0.62; // XXX draw 16x16 pixel quad

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBegin(GL_POLYGON);
	glTexCoord2f(0, 0);  glVertex2f(-r, -r);
	glTexCoord2f(1, 0);  glVertex2f( r, -r);
	glTexCoord2f(1, 1);  glVertex2f( r,  r);
	glTexCoord2f(0, 1);  glVertex2f(-r,  r);
	glEnd();

	// read a pixel from lower-left corder of rendered quad
	glReadPixels(windowSize / 2 - 2, windowSize / 2 - 2, 1, 1,
		     GL_RGBA, GL_FLOAT, pixel);
}


GLuint
API2Test::loadAndCompileShader(GLenum target, const char *text)
{
	GLint stat, val;
	GLuint shader = glCreateShader_func(target);
	if (!shader) {
		reportFailure("glCreateShader failed (fragment)");
		return 0;
	}
	glShaderSource_func(shader, 1,
			    (const GLchar **) &text, NULL);
	glCompileShader_func(shader);
	glGetShaderiv_func(shader, GL_COMPILE_STATUS, &stat);
	if (!stat) {
		reportFailure("glShaderSource or glCompileShader failed", target);
		return 0;
	}
        if (!glIsShader_func(shader)) {
		reportFailure("glIsShader failed (fragment)");
		return false;
	}
	glGetShaderiv_func(shader, GL_SHADER_TYPE, &val);
	if (val != (GLint) target) {
		reportFailure("glGetShaderiv(GL_SHADER_TYPE) failed", target);
		return 0;
	}
	glGetShaderiv_func(shader, GL_COMPILE_STATUS, &val);
	if (val != GL_TRUE) {
		reportFailure("glGetShaderiv(GL_COMPILE_STATUS) failed", target);
		return 0;
	}
	glGetShaderiv_func(shader, GL_SHADER_SOURCE_LENGTH, &val);
        // Note: some OpenGLs return a 1-char shorter length than strlen(text)
	if (abs(val - (int) strlen(text)) > 1) {
		reportFailure("glGetShaderiv(GL_SHADER_SOURCE_LENGTH) failed", target);
		return 0;
	}
	return shader;
}


bool
API2Test::testShaderObjectFuncs(void)
{
	static const char *vertShaderText =
		"void main() { \n"
		"   gl_Position = ftransform(); \n"
		"} \n";
	static const char *fragShaderText =
		"void main() { \n"
		"   gl_FragColor = vec4(1.0, 0.5, 0.25, 0.0); \n"
		"} \n";
	GLuint vertShader, fragShader, program;
	GLint stat, val;

	vertShader = loadAndCompileShader(GL_VERTEX_SHADER, vertShaderText);
	if (!vertShader)
		return false;
	fragShader = loadAndCompileShader(GL_FRAGMENT_SHADER, fragShaderText);
	if (!fragShader)
		return false;


	program = glCreateProgram_func();
	if (!fragShader) {
		reportFailure("glCreateProgram failed");
		return false;
	}
	glAttachShader_func(program, fragShader);
	glAttachShader_func(program, vertShader);
	glLinkProgram_func(program);
	glGetProgramiv_func(program, GL_LINK_STATUS, &stat);
	if (!stat) {
		reportFailure("glLinkProgram failed");
		return false;
	}
	glUseProgram_func(program);

	glGetIntegerv(GL_CURRENT_PROGRAM, &val);
	if (val != (GLint) program) {
		reportFailure("glGetInteger(GL_CURRENT_PROGRAM) failed");
		return false;
	}

        stat = glGetError();
        if (stat) {
		reportFailure("OpenGL error detected in testShaderFuncs");
		return false;
	}

        if (!glIsProgram_func(program)) {
		reportFailure("glIsProgram failed");
		return false;
	}

	GLuint objects[2];
	GLsizei count;
	glGetProgramiv_func(program, GL_ATTACHED_SHADERS, &val);
	if (val != 2) {
		reportFailure("glGetProgramiv(GL_ATTACHED_SHADERS) failed");
		return false;
	}
	glGetAttachedShaders_func(program, 2, &count, objects);
	if (count != 2) {
		reportFailure("glGetAttachedShaders failed (wrong count)");
		return false;
	}
	if (objects[0] != vertShader && objects[1] != vertShader) {
		reportFailure("glGetAttachedShaders failed (vertex shader missing)");
		return false;
	}
	if (objects[0] != fragShader && objects[1] != fragShader) {
		reportFailure("glGetAttachedShaders failed (fragment shader missing)");
		return false;
	}

	glUseProgram_func(0);
	glDeleteShader_func(vertShader);
	glGetShaderiv(vertShader, GL_DELETE_STATUS, &stat);
	if (stat != GL_TRUE) {
		reportFailure("Incorrect shader delete status)");
		return false;
	}
	glDeleteShader_func(fragShader);
	glDeleteProgram_func(program);
	if (glIsProgram(program)) {
		reportFailure("glIsProgram(deleted program) failed");
		return false;
	}

	return true;
}


bool
API2Test::testUniformfFuncs(void)
{
	static const char *fragShaderText =
		"uniform float uf1; \n"
		"uniform vec2 uf2; \n"
		"uniform vec3 uf3; \n"
		"uniform vec4 uf4; \n"
		"void main() { \n"
		"   gl_FragColor = vec4(uf1, uf2.y, uf3.z, uf4.w); \n"
		"} \n";
	GLuint fragShader, program;
	GLint uf1, uf2, uf3, uf4;
	GLfloat value[4];

	fragShader = loadAndCompileShader(GL_FRAGMENT_SHADER, fragShaderText);
	if (!fragShader) {
		return false;
	}
	program = glCreateProgram_func();
	if (!fragShader) {
		reportFailure("glCreateProgram (uniform test) failed");
		return false;
	}
	glAttachShader_func(program, fragShader);
	glLinkProgram_func(program);
	glUseProgram_func(program);

	uf1 = glGetUniformLocation_func(program, "uf1");
	if (uf1 < 0) {
		reportFailure("glGetUniform \"uf1\" failed");
		return false;
	}
	uf2 = glGetUniformLocation_func(program, "uf2");
	if (uf2 < 0) {
		reportFailure("glGetUniform \"uf2\" failed");
		return false;
	}
	uf3 = glGetUniformLocation_func(program, "uf3");
	if (uf3 < 0) {
		reportFailure("glGetUniform \"uf3\" failed");
		return false;
	}
	uf4 = glGetUniformLocation_func(program, "uf4");
	if (uf4 < 0) {
		reportFailure("glGetUniform \"uf4\" failed");
		return false;
	}


	GLfloat pixel[4], expected[4];

	// Test glUniform[1234]f()
	expected[0] = 0.1;
	expected[1] = 0.2;
	expected[2] = 0.3;
	expected[3] = 0.4;
	glUniform1f(uf1, expected[0]);
	glUniform2f(uf2, 0.0, expected[1]);
	glUniform3f(uf3, 0.0, 0.0, expected[2]);
	glUniform4f(uf4, 0.0, 0.0, 0.0, expected[3]);
	renderQuad(pixel);
	if (!equalColors(pixel, expected, 0)) {
		reportFailure("glUniform[1234]f failed");
		printf("%f %f %f %f\n", pixel[0], pixel[1], pixel[2], pixel[3]);
		return false;
	}

	// Test glUniform[1234]fv()
	GLfloat u[4];
	expected[0] = 0.9;
	expected[1] = 0.8;
	expected[2] = 0.7;
	expected[3] = 0.6;
	u[0] = expected[0];
	glUniform1fv(uf1, 1, u);
	u[0] = 0.0;  u[1] = expected[1];
	glUniform2fv(uf2, 1, u);
	u[0] = 0.0;  u[1] = 0.0;  u[2] = expected[2];
	glUniform3fv(uf3, 1, u);
	u[0] = 0.0;  u[1] = 0.0;  u[2] = 0.0;  u[3] = expected[3];
	glUniform4fv(uf4, 1, u);
	renderQuad(pixel);
	if (!equalColors(pixel, expected, 0)) {
		reportFailure("glUniform[1234]f failed");
		return false;
	}

	// Test glGetUniformfv
	glUniform4fv(uf4, 1, expected);
	glGetUniformfv(program, uf4, value);
	if (value[0] != expected[0] ||
	    value[1] != expected[1] ||
	    value[2] != expected[2] ||
	    value[3] != expected[3]) {
		reportFailure("glGetUniformfv failed");
		return false;
	}

	return true;
}


bool
API2Test::testUniformiFuncs(void)
{
	static const char *fragShaderText =
		"uniform int ui1; \n"
		"uniform ivec2 ui2; \n"
		"uniform ivec3 ui3; \n"
		"uniform ivec4 ui4; \n"
		"void main() { \n"
		"   gl_FragColor = vec4(ui1, ui2.y, ui3.z, ui4.w) * 0.1; \n"
		"} \n";
	GLuint fragShader, program;
	GLint ui1, ui2, ui3, ui4;

	fragShader = loadAndCompileShader(GL_FRAGMENT_SHADER, fragShaderText);
	if (!fragShader) {
		return false;
	}
	program = glCreateProgram_func();
	if (!fragShader) {
		reportFailure("glCreateProgram (uniform test) failed");
		return false;
	}
	glAttachShader_func(program, fragShader);
	glLinkProgram_func(program);
	glUseProgram_func(program);

	ui1 = glGetUniformLocation_func(program, "ui1");
	if (ui1 < 0) {
		reportFailure("glGetUniform \"ui1\" failed");
		return false;
	}
	ui2 = glGetUniformLocation_func(program, "ui2");
	if (ui2 < 0) {
		reportFailure("glGetUniform \"ui2\" failed");
		return false;
	}
	ui3 = glGetUniformLocation_func(program, "ui3");
	if (ui3 < 0) {
		reportFailure("glGetUniform \"ui3\" failed");
		return false;
	}
	ui4 = glGetUniformLocation_func(program, "ui4");
	if (ui4 < 0) {
		reportFailure("glGetUniform \"ui4\" failed");
		return false;
	}

	GLfloat pixel[4], expected[4];
	GLint expectedInt[4];

	// Test glUniform[1234]f()
	expectedInt[0] = 1;
	expectedInt[1] = 2;
	expectedInt[2] = 3;
	expectedInt[3] = 4;
	expected[0] = 0.1;
	expected[1] = 0.2;
	expected[2] = 0.3;
	expected[3] = 0.4;
	glUniform1i(ui1, expectedInt[0]);
	glUniform2i(ui2, 0, expectedInt[1]);
	glUniform3i(ui3, 0, 0, expectedInt[2]);
	glUniform4i(ui4, 0, 0, 0, expectedInt[3]);
	renderQuad(pixel);
	if (!equalColors(pixel, expected, 0)) {
		reportFailure("glUniform[1234]i failed");
		printf("%f %f %f %f\n", pixel[0], pixel[1], pixel[2], pixel[3]);
		return false;
	}

	// Test glUniform[1234]fv()
	GLint u[4];
	expectedInt[0] = 9;
	expectedInt[1] = 8;
	expectedInt[2] = 7;
	expectedInt[3] = 6;
	expected[0] = 0.9;
	expected[1] = 0.8;
	expected[2] = 0.7;
	expected[3] = 0.6;
	u[0] = expectedInt[0];
	glUniform1iv(ui1, 1, u);
	u[0] = 0;  u[1] = expectedInt[1];
	glUniform2iv(ui2, 1, u);
	u[0] = 0;  u[1] = 0;  u[2] = expectedInt[2];
	glUniform3iv(ui3, 1, u);
	u[0] = 0;  u[1] = 0;  u[2] = 0;  u[3] = expectedInt[3];
	glUniform4iv(ui4, 1, u);
	renderQuad(pixel);
	if (!equalColors(pixel, expected, 0)) {
		reportFailure("glUniform[1234]i failed");
		printf("%f %f %f %f\n", pixel[0], pixel[1], pixel[2], pixel[3]);
		return false;
	}

	return true;
}



bool
API2Test::testStencilFuncSeparate(void)
{
	GLint val;

	glStencilFuncSeparate_func(GL_FRONT, GL_LEQUAL, 12, 0xf);
	glStencilFuncSeparate_func(GL_BACK, GL_GEQUAL, 13, 0xe);

	glGetIntegerv(GL_STENCIL_BACK_FUNC, &val);
	if (val != GL_GEQUAL) {
		reportFailure("GL_STENCIL_BACK_FUNC query returned wrong value");
		return false;
	}

	glGetIntegerv(GL_STENCIL_FUNC, &val);
	if (val != GL_LEQUAL) {
		reportFailure("GL_STENCIL_FUNC (front) query returned wrong value");
		return false;
	}

	glGetIntegerv(GL_STENCIL_BACK_REF, &val);
	if (val != 13) {
		reportFailure("GL_STENCIL_BACK_REF query returned wrong value");
		return false;
	}

	glGetIntegerv(GL_STENCIL_REF, &val);
	if (val != 12) {
		reportFailure("GL_STENCIL_REF (front) query returned wrong value");
		return false;
	}

	glGetIntegerv(GL_STENCIL_BACK_VALUE_MASK, &val);
	if (val != 0xe) {
		reportFailure("GL_STENCIL_BACK_VALUE_MASK query returned wrong value");
		return false;
	}

	glGetIntegerv(GL_STENCIL_VALUE_MASK, &val);
	if (val != 0xf) {
		reportFailure("GL_STENCIL_VALUE_MASK (front) query returned wrong value");
		return false;
	}

	return true;
}


bool
API2Test::testStencilOpSeparate(void)
{
	GLint val;

	// face, fail, zfail, zpass
	glStencilOpSeparate_func(GL_FRONT, GL_INVERT, GL_ZERO, GL_INCR);
	glStencilOpSeparate_func(GL_BACK, GL_INCR, GL_KEEP, GL_REPLACE);

	glGetIntegerv(GL_STENCIL_BACK_FAIL, &val);
	if (val != GL_INCR) {
		reportFailure("GL_STENCIL_BACK_FAIL query returned wrong value");
		return false;
	}

	glGetIntegerv(GL_STENCIL_FAIL, &val);
	if (val != GL_INVERT) {
		reportFailure("GL_STENCIL_FAIL (front) query returned wrong value");
		return false;
	}

	glGetIntegerv(GL_STENCIL_BACK_PASS_DEPTH_FAIL, &val);
	if (val != GL_KEEP) {
		reportFailure("GL_STENCIL_BACK_PASS_DEPTH_FAIL query returned wrong value");
		return false;
	}

	glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, &val);
	if (val != GL_ZERO) {
		reportFailure("GL_STENCIL_PASS_DEPTH_FAIL (front) query returned wrong value");
		return false;
	}

	glGetIntegerv(GL_STENCIL_BACK_PASS_DEPTH_PASS, &val);
	if (val != GL_REPLACE) {
		reportFailure("GL_STENCIL_BACK_PASS_DEPTH_PASS query returned wrong value");
		return false;
	}

	glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, &val);
	if (val != GL_INCR) {
		reportFailure("GL_STENCIL_PASS_DEPTH_PASS (front) query returned wrong value");
		return false;
	}

	return true;
}


bool
API2Test::testStencilMaskSeparate(void)
{
	GLint val;

	// face, fail, zfail, zpass
	glStencilMaskSeparate_func(GL_BACK, 0xa);
	glStencilMaskSeparate_func(GL_FRONT, 0xb);

	glGetIntegerv(GL_STENCIL_BACK_WRITEMASK, &val);
	if (val != 0xa) {
		reportFailure("GL_STENCIL_BACK_WRITEMASK query returned wrong value");
		return false;
	}

	glGetIntegerv(GL_STENCIL_WRITEMASK, &val);
	if (val != 0xb) {
		reportFailure("GL_STENCIL_WRITEMASK (front) query returned wrong value");
		return false;
	}

	return true;
}


bool
API2Test::testBlendEquationSeparate(void)
{
	GLint val;

	glBlendEquationSeparate_func(GL_MAX, GL_FUNC_SUBTRACT);

	glGetIntegerv(GL_BLEND_EQUATION, &val);
	if (val != GL_MAX) {
		reportFailure("GL_BLEND_EQUATION (rgb) query returned wrong value");
		return false;
	}

	glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &val);
	if (val != GL_FUNC_SUBTRACT) {
		reportFailure("GL_BLEND_EQUATION (rgb) query returned wrong value");
		return false;
	}

	return true;
}


bool
API2Test::testDrawBuffers(void)
{
	const int MAX = 8;
	GLint maxBuf = -1, i, n, val;
	GLenum buffers[MAX];
	GLint initDrawBuffer;

	glGetIntegerv(GL_DRAW_BUFFER, &initDrawBuffer);

	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxBuf);
	if (maxBuf < 1) {
		reportFailure("GL_MAX_DRAW_BUFFERS query failed");
		return false;
	}

	n = maxBuf < MAX ? maxBuf : MAX;
	assert(n > 0);
	for (i = 0; i < n; i++) {
		buffers[i] = (i & 1) ? GL_FRONT_LEFT : GL_BACK_LEFT;
	}
	glDrawBuffers_func(n, buffers);

	for (i = 0; i < n; i++) {
		glGetIntegerv(GL_DRAW_BUFFER0 + i, &val);
		if (val != (GLint) buffers[i]) {
			reportFailure("glDrawBuffers failed");
			return false;
		}
	}

	// restore
	glDrawBuffer(initDrawBuffer);

	return true;
}


// Run all the subtests, incrementing numPassed, numFailed
void
API2Test::runSubTests(MultiTestResult &r)
{
	static TestFunc funcs[] = {
		&GLEAN::API2Test::testStencilFuncSeparate,
		&GLEAN::API2Test::testStencilOpSeparate,
		&GLEAN::API2Test::testStencilMaskSeparate,
		&GLEAN::API2Test::testBlendEquationSeparate,
		&GLEAN::API2Test::testDrawBuffers,
		&GLEAN::API2Test::testShaderObjectFuncs,
		&GLEAN::API2Test::testUniformfFuncs,
		&GLEAN::API2Test::testUniformiFuncs,
		NULL
	};

	for (int i = 0; funcs[i]; i++)
		if ((this->*funcs[i])())
			r.numPassed++;
		else
			r.numFailed++;
}


void
API2Test::runOne(MultiTestResult &r, Window &w)
{
	(void) w;

	if (!setup()) {
		r.pass = false;
		return;
	}

	runSubTests(r);

	r.pass = (r.numFailed == 0);
}


// The test object itself:
API2Test api2Test("api2", "window, rgb, z, db",
		  "",  // no extension filter (we'll test for version 2.x during setup)
		  "API2 test: check that OpenGL 2.x API functions work.\n"
		  );



} // namespace GLEAN
