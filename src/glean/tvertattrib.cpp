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


// tvertattrib.cpp:  Test vertex attribute functions.
//
// Indexed vertex attributes may either alias with conventional attributes
// or name a separate set of generic attributes.  The following extensions/
// versions are tested (and whether aliasing is allowed):
//   GL_NV_vertex_program (aliasing required)
//   GL_ARB_vertex_program (aliasing optional)
//   GL_ARB_vertex_shader (aliasing required)
//   OpenGL 2.0 (aliasing required)
//
// If either GL_ARB_vertex_shader or OpenGL 2.0 is supported, that means
// aliasing is required for GL_ARB_vertex_program.
//
// To do: We should test all of the glVertexAttrib variations, not just
// the glVertexAttrib4fv[ARB/NV] functions.
//
// Author: Brian Paul (brian@tungstengraphics.com)  October 2004


#include "tvertattrib.h"
#include "glutils.h"

namespace GLEAN {


void
VertAttribTest::FailMessage(BasicResult &r, const char *msg, const char *ext) const
{
	// record the failure
	r.pass = false;

	// print the message
	env->log << name << ":  FAIL "
		 << r.config->conciseDescription() << '\n';
	env->log << "\t" << msg << " (Testing " << ext << ")\n";
}


bool
VertAttribTest::TestAttribs(BasicResult &r,
			    const char *extensionName,
			    PFNGLVERTEXATTRIB4FVARBPROC attrib4fv,
			    PFNGLGETVERTEXATTRIBFVARBPROC getAttribfv,
			    Aliasing aliasing,
			    int numAttribs)
{
	static const GLfloat ref[4] = { 1.0F, 2.0F, 3.0F, 4.0F };
	static const GLfloat zero[4] = { 0.0F, 0.0F, 0.0F, 0.0F };

	// set a couple conventional attribs for later tests
	glNormal3f(-1.0F, -2.0F, -3.0F);
	glTexCoord4f(-1.0F, -2.0F, -3.0F, -4.0F);

	for (int i = 1; i < numAttribs; i++) {
		// set attribs
		for (int j = 1; j < numAttribs; j++) {
			if (j == i) {
				attrib4fv(j, ref);
			}
			else {
				attrib4fv(j, zero);
			}
		}
		// test attribs
		for (int j = 1; j < numAttribs; j++) {
			const GLfloat *expected;
			if (j == i) {
				expected = ref;
			}
			else {
				expected = zero;
			}
			GLfloat v[4];
			getAttribfv(j, GL_CURRENT_VERTEX_ATTRIB_ARB, v);
			if (v[0] != expected[0] ||
			    v[1] != expected[1] ||
			    v[2] != expected[2] ||
			    v[3] != expected[3]) {
				char msg[1000];
				sprintf(msg, "Vertex Attribute %d is %g, %g, %g, %g but expected %g, %g, %g, %g",
					j, v[0], v[1], v[2], v[3],
					expected[0], expected[1], expected[2], expected[3]);
				FailMessage(r, msg, extensionName);
				return false;
			}
		}

		if (aliasing == REQUIRED) {
			// spot check a few aliased attribs
			GLfloat v[4];
			if (i == 2) {
				glGetFloatv(GL_CURRENT_NORMAL, v);
				if (v[0] != ref[0] ||
				    v[1] != ref[1] ||
				    v[2] != ref[2]) {
					FailMessage(r, "Setting attribute 2 did not update GL_CURRENT_NORMAL", extensionName);
					return false;
				}
			}
			if (i == 8) {
				glGetFloatv(GL_CURRENT_TEXTURE_COORDS, v);
				if (v[0] != ref[0] ||
				    v[1] != ref[1] ||
				    v[2] != ref[2] ||
				    v[3] != ref[3]) {
					FailMessage(r, "Setting attribute 8 did not update GL_CURRENT_TEXTURE_COORDS", extensionName);
					return false;
				}
			}
		}
		else if (aliasing == DISALLOWED) {
			// spot check a few non-aliased attribs
			GLfloat v[4];
			glGetFloatv(GL_CURRENT_NORMAL, v);
			if (v[0] != 1.0F ||
			    v[1] != 1.0F ||
			    v[2] != 1.0F) {
				FailMessage(r, "GL_CURRENT_NORMAL was errantly set by a glVertexAttrib call", extensionName);
				return false;
			}
			glGetFloatv(GL_CURRENT_TEXTURE_COORDS, v);
			if (v[0] != 1.0F ||
			    v[1] != 1.0F ||
			    v[2] != 1.0F) {
				FailMessage(r, "GL_CURRENT_TEXTURE_COORDS was errantly set by a glVertexAttrib call", extensionName);
				return false;
			}
		}
	}

	// all passed
	return true;
}


// Test the GL_NV_vertex_program functions
// Return true if pass, false if fail
bool
VertAttribTest::TestNVfuncs(BasicResult &r)
{
#ifdef GL_NV_vertex_program
	PFNGLVERTEXATTRIB4FVNVPROC attrib4fv;
	PFNGLGETVERTEXATTRIBFVNVPROC getAttribfv;
	const GLint numAttribs = 16;

	attrib4fv = (PFNGLVERTEXATTRIB4FVNVPROC) GLUtils::getProcAddress("glVertexAttrib4fvNV");
	assert(attrib4fv);

	getAttribfv = (PFNGLGETVERTEXATTRIBFVNVPROC) GLUtils::getProcAddress("glGetVertexAttribfvNV");
	assert(getAttribfv);

	return TestAttribs(r, "GL_NV_vertex_program", attrib4fv, getAttribfv,
			   REQUIRED, numAttribs);
#else
	(void) r;
	return true;
#endif
}


// Test the GL_ARB_vertex_program/shader functions
// Return true if pass, false if fail
bool
VertAttribTest::TestARBfuncs(BasicResult &r, bool shader)
{
#if defined(GL_ARB_vertex_program) || defined(GL_ARB_vertex_shader)
	PFNGLVERTEXATTRIB4FVARBPROC attrib4fv;
	PFNGLGETVERTEXATTRIBFVARBPROC getAttribfv;
	GLint numAttribs;

	attrib4fv = (PFNGLVERTEXATTRIB4FVARBPROC) GLUtils::getProcAddress("glVertexAttrib4fvARB");
	assert(attrib4fv);

	getAttribfv = (PFNGLGETVERTEXATTRIBFVARBPROC) GLUtils::getProcAddress("glGetVertexAttribfvARB");
	assert(getAttribfv);

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS_ARB, &numAttribs);
	assert(numAttribs > 0);

	if (shader) {
		return TestAttribs(r, "GL_ARB_vertex_shader",
				   attrib4fv, getAttribfv,
				   REQUIRED, numAttribs);
	}
	else {
		// GL_ARB_vertex_program:
		// Determine if attribute aliasing is allowed
		Aliasing aliasing;
		if (GLUtils::haveExtension("GL_ARB_vertex_shader")) {
			aliasing = DISALLOWED;
		}
		else {
			// check for OpenGL 2.x support
			char *vers = (char *) glGetString(GL_VERSION);
			if (vers[0] == '2' && vers[1] == '.') {
				aliasing = DISALLOWED;
			}
			else {
				aliasing = OPTIONAL;
			}
		}
		return TestAttribs(r, "GL_ARB_vertex_program",
				   attrib4fv, getAttribfv,
				   aliasing, numAttribs);
	}
#else
	(void) r;
	return true;
#endif
}


// Test the OpenGL 2.x glVertexAttrib functions
// Return true if pass, false if fail
bool
VertAttribTest::Test20funcs(BasicResult &r)
{
#ifdef GL_VERSION_2_0
	PFNGLVERTEXATTRIB4FVPROC attrib4fv;
	PFNGLGETVERTEXATTRIBFVPROC getAttribfv;
	GLint numAttribs;

	attrib4fv = (PFNGLVERTEXATTRIB4FVPROC) GLUtils::getProcAddress("glVertexAttrib4fv");
	assert(attrib4fv);

	getAttribfv = (PFNGLGETVERTEXATTRIBFVPROC) GLUtils::getProcAddress("glGetVertexAttribfv");
	assert(getAttribfv);

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numAttribs);
	assert(numAttribs > 0);

	return TestAttribs(r, "OpenGL 2.x", attrib4fv, getAttribfv,
			   DISALLOWED, numAttribs);
#else
	(void) r;
	return true;
#endif
}


void
VertAttribTest::runOne(BasicResult& r, Window&)
{
	r.pass = true;
#ifdef GL_NV_vertex_program
	if (GLUtils::haveExtension("GL_NV_vertex_program")) {
		bool p = TestNVfuncs(r);
		if (!p)
			return;
	}
#endif
#ifdef GL_ARB_vertex_program
	if (GLUtils::haveExtension("GL_ARB_vertex_program")) {
		bool p = TestARBfuncs(r, false);
		if (!p)
			return;
	}
#endif
#ifdef GL_ARB_vertex_shader
	if (GLUtils::haveExtension("GL_ARB_vertex_shader")) {
		bool p = TestARBfuncs(r, true);
		if (!p)
			return;
	}
#endif
#ifdef GL_VERSION_2_0
	const char *vers = (const char *) glGetString(GL_VERSION);
	if (vers[0] == '2' && vers[1] == '.') {
		bool p = Test20funcs(r);
		if (!p)
			return;
	}
#endif
}


void
VertAttribTest::logOne(BasicResult& r)
{
	if (r.pass) {
		logPassFail(r);
		logConcise(r);
	}
}


// Instantiate this test object
VertAttribTest vertAttribTest("vertattrib", "window, rgb",
	"Verify that glVertexAttribNV, glVertexAttribARB, and glVertexAttrib\n"
	"all work correctly.\n");


} // namespace GLEAN
