// BEGIN_COPYRIGHT
// 
// Copyright (C) 2000  Allen Akin   All Rights Reserved.
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




// tvtxperf.cpp:  Test performance of various ways to specify vertex data

#ifdef __UNIX__
#include <unistd.h>
#endif

#include <iostream>
#include <fstream>
#include <algorithm>
#include "codedid.h"
#include "dsconfig.h"
#include "dsfilt.h"
#include "dsurf.h"
#include "winsys.h"
#include "environ.h"
#include "rc.h"
#include "glutils.h"
#include "geomutil.h"
#include "timer.h"
#include "rand.h"
#include "image.h"
#include "tvtxperf.h"
#include "misc.h"


namespace {

const int drawingSize = 256;

// The current Timer code requires that any parameters to the function
// being timed must be passed through global variables.  This probably
// should be changed, but in the meantime, here are the types and globals
// used to pass geometry to the various benchmark functions.

struct C4UB_N3F_V3F {
	GLubyte c[4];
	GLfloat n[3];
	GLfloat v[3];
};

int nVertices;
int dList;
GLuint* indices;
C4UB_N3F_V3F* c4ub_n3f_v3f;


typedef void (*TIME_FUNC)();

void
coloredLit_imIndTri() {
	const C4UB_N3F_V3F* p = c4ub_n3f_v3f;
	glBegin(GL_TRIANGLES);
		// Assume that the data is complete, thus allowing us
		// to unroll 3X and do one tri per iteration rather than
		// one vertex.
		for (int i = nVertices / 3; i; --i) {
			glColor4ubv(p[0].c);
			glNormal3fv(p[0].n);
			glVertex3fv(p[0].v);
			glColor4ubv(p[1].c);
			glNormal3fv(p[1].n);
			glVertex3fv(p[1].v);
			glColor4ubv(p[2].c);
			glNormal3fv(p[2].n);
			glVertex3fv(p[2].v);
			p += 3;
		}
	glEnd();
} // coloredLit_imIndTri

void
coloredLit_imTriStrip() {
	glBegin(GL_TRIANGLE_STRIP);

	// Duff's device.  Yes, this is legal C (and C++).
	// See Stroustrup, 3rd ed., p. 141
	const C4UB_N3F_V3F* p = c4ub_n3f_v3f;
	int n = (nVertices + 3) >> 2;
	switch (nVertices & 0x3) {
		case 0:	do {
				glColor4ubv(p->c);
				glNormal3fv(p->n);
				glVertex3fv(p->v);
				++p;
		case 3:
				glColor4ubv(p->c);
				glNormal3fv(p->n);
				glVertex3fv(p->v);
				++p;
		case 2:
				glColor4ubv(p->c);
				glNormal3fv(p->n);
				glVertex3fv(p->v);
				++p;
		case 1:
				glColor4ubv(p->c);
				glNormal3fv(p->n);
				glVertex3fv(p->v);
				++p;
			} while (--n > 0);
	}

	glEnd();
} // coloredLit_imTriStrip

void
coloredLit_daIndTri() {
	glDrawArrays(GL_TRIANGLES, 0, nVertices);
} // coloredLit_daIndTri

void
coloredLit_daTriStrip() {
	glDrawArrays(GL_TRIANGLE_STRIP, 0, nVertices);
} // coloredLit_daTriStrip

void
coloredLit_deIndTri() {
	glDrawElements(GL_TRIANGLES, nVertices, GL_UNSIGNED_INT, indices);
} // coloredLit_deIndTri

void
coloredLit_deTriStrip() {
	glDrawElements(GL_TRIANGLE_STRIP, nVertices, GL_UNSIGNED_INT, indices);
} // coloredLit_deTriStrip

void
callDList() {
	glCallList(dList);
} // callDList

void
measure(GLEAN::Timer& time, TIME_FUNC function, GLEAN::Environment* env,
    GLEAN::Window& w, int nTris, GLEAN::VPResult& r) {
	// Clear both front and back buffers and swap, to avoid confusing
	// this test with results of the previous test:
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	w.swap();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Make several measurements, to reduce influence of random factors:
	const int nMeasurements = 5;	// Must be >= 3
	vector<double> measurements;
	for (int i = 0; i < nMeasurements; ++i) {
		env->quiesce();
		double t = time.time(static_cast<TIME_FUNC>(glFinish),
			function,
			static_cast<TIME_FUNC>(glFinish));
		w.swap();	// give the user something to watch
		measurements.push_back(nTris / t);
	}

	// Throw out the fastest and slowest measurement, and return
	// the arithmetic mean, fastest, and slowest of those that remain:
	sort(measurements.begin(), measurements.end());
	double sum = 0.0;
	for (int j = 1; j < nMeasurements - 1; ++j)
		sum += measurements[j];
	r.tps = sum / (nMeasurements - 2.0);
	r.tpsLow = measurements[1];
	r.tpsHigh = measurements[nMeasurements - 2];
} // measure

void
logStats1(const char* title, GLEAN::VPResult& r,
    GLEAN::Environment* env) {
	env->log << '\t' << title << " rate = "
		<< r.tps << " tri/sec.\n"
		<< "\t\tRange of valid measurements = ["
		<< r.tpsLow << ", " << r.tpsHigh << "]\n"
		<< "\t\tImage sanity check "
		<< (r.imageOK? "passed\n": "failed\n")
		<< "\t\tImage consistency check "
		<< (r.imageMatch? "passed\n": "failed\n");

} // logStats1

void
logStats(GLEAN::ColoredLitPerf::Result& r, GLEAN::Environment* env) {
	logStats1("Immediate-mode independent triangle", r.imTri, env);
	logStats1("Display-listed independent triangle", r.dlTri, env);
	logStats1("DrawArrays independent triangle", r.daTri, env);
	logStats1("Locked DrawArrays independent triangle", r.ldaTri, env);
	logStats1("DrawElements independent triangle", r.deTri, env);
	logStats1("Locked DrawElements independent triangle", r.ldeTri, env);
	logStats1("Immediate-mode triangle strip", r.imTS, env);
	logStats1("Display-listed triangle strip", r.dlTS, env);
	logStats1("DrawArrays triangle strip", r.daTS, env);
	logStats1("Locked DrawArrays triangle strip", r.ldaTS, env);
	logStats1("DrawElements triangle strip", r.deTS, env);
	logStats1("Locked DrawElements triangle strip", r.ldeTS, env);
} // logStats

void
diffHeader(bool& same, const string& name,
    GLEAN::DrawingSurfaceConfig* config, GLEAN::Environment* env) {
	if (same) {
		same = false;
		env->log << name << ":  DIFF "
			<< config->conciseDescription() << '\n';
	}
} // diffHeader

void
failHeader(bool& pass, const string& name,
    GLEAN::DrawingSurfaceConfig* config, GLEAN::Environment* env) {
	if (pass) {
		pass = false;
		env->log << name << ":  FAIL "
			<< config->conciseDescription() << '\n';
	}
} // failHeader

void
doComparison(const GLEAN::VPResult& oldR,
    const GLEAN::VPResult& newR,
    GLEAN::DrawingSurfaceConfig* config,
    bool& same, const string& name, GLEAN::Environment* env,
    const char* title) {
	if (newR.tps < oldR.tpsLow) {
		int percent = static_cast<int>(
			100.0 * (oldR.tps - newR.tps) / newR.tps + 0.5);
		diffHeader(same, name, config, env);
		env->log << '\t' << env->options.db1Name
			<< " may be " << percent << "% faster on "
			<< title << " drawing.\n";
	}
	if (newR.tps > oldR.tpsHigh) {
		int percent = static_cast<int>(
			100.0 * (newR.tps - oldR.tps) / oldR.tps + 0.5);
		diffHeader(same, name, config, env);
		env->log << '\t' << env->options.db2Name
			<< " may be " << percent << "% faster on "
			<< title << " drawing.\n";
	}
	if (newR.imageOK != oldR.imageOK) {
		diffHeader(same, name, config, env);
		env->log << '\t' << env->options.db1Name << " image check "
			<< (oldR.imageOK? "passed\n": "failed\n");
		env->log << '\t' << env->options.db2Name << " image check "
			<< (newR.imageOK? "passed\n": "failed\n");
	}
	if (newR.imageMatch != oldR.imageMatch) {
		diffHeader(same, name, config, env);
		env->log << '\t' << env->options.db1Name << " image compare "
			<< (oldR.imageMatch? "passed\n": "failed\n");
		env->log << '\t' << env->options.db2Name << " image compare "
			<< (newR.imageMatch? "passed\n": "failed\n");
	}
} // doComparison

bool
imagesDiffer(GLEAN::Image& testImage, GLEAN::Image& goldenImage) {
	GLEAN::Image::Registration imageReg(testImage.reg(goldenImage));
	return (imageReg.stats[0].max()
	    + imageReg.stats[1].max()
	    + imageReg.stats[2].max()) != 0.0;
} // imagesDiffer

void
missingSome(GLEAN::Environment* env, const char* title) {
	env->log << '\t' << title << " rendering is missing\n"
			<< "\t\tsome triangles.\n";
} // missingSome

void
theyDiffer(GLEAN::Environment* env, const char* title) {
	env->log << '\t' << title << " image differs from\n"
		<< "\t\tthe reference image.\n";
} // theyDiffer

void
verify(GLEAN::Image& testImage, GLEAN::RGBCodedID& colorGen,
    int firstID, int lastID, GLEAN::Image& refImage,
    bool& passed, string& name, GLEAN::DrawingSurfaceConfig* config,
    GLEAN::VPResult& res, GLEAN::Environment* env, const char* title) {

	// Verify that the entire range of RGB coded identifiers is
	// present in the image.  (This is an indicator that all triangles
	// were actually drawn.)
	testImage.read(0, 0);
	if (!colorGen.allPresent(testImage, firstID, lastID)) {
		failHeader(passed, name, config, env);
		missingSome(env, title);
		res.imageOK = false;
	}

	// Verify that the test image is the same as the reference image.
	if (imagesDiffer(testImage, refImage)) {
		failHeader(passed, name, config, env);
		theyDiffer(env, title);
		res.imageMatch = false;
	}
} // verify

} // anonymous namespace

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// Constructor/Destructor:
///////////////////////////////////////////////////////////////////////////////
ColoredLitPerf::ColoredLitPerf(const char* aName, const char* aFilter,
    const char* aDescription):
    	Test(aName), filter(aFilter), description(aDescription) {
} // ColoredLitPerf::ColoredLitPerf()

ColoredLitPerf::~ColoredLitPerf() {
} // ColoredLitPerf::~ColoredLitPerf

///////////////////////////////////////////////////////////////////////////////
// run: run tests, save results in a vector and in the results file
///////////////////////////////////////////////////////////////////////////////
void
ColoredLitPerf::run(Environment& environment) {
	// Guard against multiple invocations:
	if (hasRun)
		return;

	// Set up environment for use by other functions:
	env = &environment;

	// Document the test in the log, if requested:
	logDescription();

	// Compute results and make them available to subsequent tests:
	WindowSystem& ws = env->winSys;
	try {
		// Open the results file:
		OutputStream os(*this);

		// Select the drawing configurations for testing:
		DrawingSurfaceFilter f(filter);
		vector<DrawingSurfaceConfig*> configs(f.filter(ws.surfConfigs));

		// Test only one config, otherwise this would take forever.
		if (configs.size()) {
			vector<DrawingSurfaceConfig*>::const_iterator
			    p = configs.begin();
			Window w(ws, **p, drawingSize, drawingSize);
			RenderingContext rc(ws, **p);
			if (!ws.makeCurrent(rc, w))
				;	// XXX need to throw exception here

			// Create a result object and run the test:
			Result* r = new Result();
			r->config = *p;
			runOne(*r, w);

			// Save the result locally and in the results file:
			results.push_back(r);
			r->put(os);
		}
	}
	catch (DrawingSurfaceFilter::Syntax e) {
		env->log << "Syntax error in test's drawing-surface selection"
			"criteria:\n'" << filter << "'\n";
		for (int i = 0; i < e.position; ++i)
			env->log << ' ';
		env->log << "^ " << e.err << '\n';
	}
	catch (RenderingContext::Error) {
		env->log << "Could not create a rendering context\n";
	}

	env->log << '\n';

	// Note that we've completed the run:
	hasRun = true;
}

void
ColoredLitPerf::compare(Environment& environment) {
	// Save the environment for use by other member functions:
	env = &environment;

	// Display the description if needed:
	logDescription();

	// Read results from previous runs:
	Input1Stream is1(*this);
	vector<Result*> oldR(getResults(is1));
	Input2Stream is2(*this);
	vector<Result*> newR(getResults(is2));

	// Construct a vector of surface configurations from the old run.
	// (Later we'll find the best match in this vector for each config
	// in the new run.)
	vector<DrawingSurfaceConfig*> oldConfigs;
	for (vector<Result*>::const_iterator p = oldR.begin(); p < oldR.end();
	    ++p)
		oldConfigs.push_back((*p)->config);

	// Compare results:
	for (vector<Result*>::const_iterator newP = newR.begin();
	    newP < newR.end(); ++newP) {

	    	// Find the drawing surface config that most closely matches
		// the config for this result:
		int c = (*newP)->config->match(oldConfigs);

		// If there was a match, compare the results:
		if (c < 0)
			env->log << name << ":  NOTE no matching config for " <<
				(*newP)->config->conciseDescription() << '\n';
		else
			compareOne(*(oldR[c]), **newP);
	}

	// Get rid of the results; we don't need them for future comparisons.
	for (vector<Result*>::iterator np = newR.begin(); np < newR.end(); ++np)
		delete *np;
	for (vector<Result*>::iterator op = oldR.begin(); op < oldR.end(); ++op)
		delete *op;
}

///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////

void
ColoredLitPerf::runOne(Result& r, Window& w) {
	PFNGLLOCKARRAYSEXTPROC glLockArraysEXT = 0;
	PFNGLUNLOCKARRAYSEXTPROC glUnlockArraysEXT = 0;
	if (GLUtils::haveExtension("GL_EXT_compiled_vertex_array")) {
		glLockArraysEXT = reinterpret_cast<PFNGLLOCKARRAYSEXTPROC>
			(GLUtils::getProcAddress("glLockArraysEXT"));
		glUnlockArraysEXT = reinterpret_cast<PFNGLUNLOCKARRAYSEXTPROC>
			(GLUtils::getProcAddress("glUnlockArraysEXT"));
	}

	Image imTriImage(drawingSize, drawingSize, GL_RGB, GL_UNSIGNED_BYTE);
	Image testImage(drawingSize, drawingSize, GL_RGB, GL_UNSIGNED_BYTE);
	bool passed = true;

	// Make colors deterministic, so we can check them:
	RGBCodedID colorGen(r.config->r, r.config->g, r.config->b);
	int IDModulus = colorGen.maxID() + 1;

	// We need to minimize the number of pixels per triangle, so that
	// we're measuring vertex-processing rate rather than fill rate.
	// However, we'd also like to guarantee that every triangle covers
	// at least one pixel, so that we can confirm drawing actually took
	// place.  As a compromise, we'll choose a number of triangles that
	// yields approximately 3 pixels per triangle.
	// We're drawing a filled spiral that approximates a circular area,
	// so pi * (drawingSize/2)**2 / nTris = 3 implies...
	const int nTris = static_cast<int>
		(((3.14159 / 4.0) * drawingSize * drawingSize) / 3.0 + 0.5);
	nVertices = nTris * 3;
	int lastID = min(IDModulus - 1, nTris - 1);

	c4ub_n3f_v3f = new C4UB_N3F_V3F[nVertices];
	SpiralTri2D it(nTris, 0, drawingSize, 0, drawingSize);
	int k = 0;
	for (int j = 0; j < nTris; ++j) {
		float* t = it(j);
		GLubyte r, g, b;
		colorGen.toRGB(j % IDModulus, r, g, b);

		c4ub_n3f_v3f[k+0].c[0] = r;
		c4ub_n3f_v3f[k+0].c[1] = g;
		c4ub_n3f_v3f[k+0].c[2] = b;
		c4ub_n3f_v3f[k+0].c[3] = 0xFF;
		c4ub_n3f_v3f[k+0].n[0] = 0.0;
		c4ub_n3f_v3f[k+0].n[1] = 0.0;
		c4ub_n3f_v3f[k+0].n[2] = 1.0;
		c4ub_n3f_v3f[k+0].v[0] = t[0];
		c4ub_n3f_v3f[k+0].v[1] = t[1];
		c4ub_n3f_v3f[k+0].v[2] = 0.0;

		c4ub_n3f_v3f[k+1].c[0] = r;
		c4ub_n3f_v3f[k+1].c[1] = g;
		c4ub_n3f_v3f[k+1].c[2] = b;
		c4ub_n3f_v3f[k+1].c[3] = 0xFF;
		c4ub_n3f_v3f[k+1].n[0] = 0.0;
		c4ub_n3f_v3f[k+1].n[1] = 0.0;
		c4ub_n3f_v3f[k+1].n[2] = 1.0;
		c4ub_n3f_v3f[k+1].v[0] = t[2];
		c4ub_n3f_v3f[k+1].v[1] = t[3];
		c4ub_n3f_v3f[k+1].v[2] = 0.0;

		c4ub_n3f_v3f[k+2].c[0] = r;
		c4ub_n3f_v3f[k+2].c[1] = g;
		c4ub_n3f_v3f[k+2].c[2] = b;
		c4ub_n3f_v3f[k+2].c[3] = 0xFF;
		c4ub_n3f_v3f[k+2].n[0] = 0.0;
		c4ub_n3f_v3f[k+2].n[1] = 0.0;
		c4ub_n3f_v3f[k+2].n[2] = 1.0;
		c4ub_n3f_v3f[k+2].v[0] = t[4];
		c4ub_n3f_v3f[k+2].v[1] = t[5];
		c4ub_n3f_v3f[k+2].v[2] = 0.0;

		k += 3;
	}

	indices = new GLuint[nVertices];
	for (k = 0; k < nVertices; ++k)
		indices[k] = k;

	GLUtils::useScreenCoords(drawingSize, drawingSize);

	// Diffuse white light at infinity, behind the eye:
	GLUtils::Light light(0);
	light.ambient(0, 0, 0, 0);
	light.diffuse(1, 1, 1, 0);
	light.specular(0, 0, 0, 0);
	light.position(0, 0, 1, 0);
	light.spotCutoff(180);
	light.constantAttenuation(1);
	light.linearAttenuation(0);
	light.quadraticAttenuation(0);
	light.enable();

	GLUtils::LightModel lm;
	lm.ambient(0, 0, 0, 0);
	lm.localViewer(false);
	lm.twoSide(false);
	lm.colorControl(GL_SINGLE_COLOR);

	glFrontFace(GL_CCW);
	glEnable(GL_NORMALIZE);
	GLUtils::Material mat;
	mat.ambient(0, 0, 0, 1);
	mat.ambientAndDiffuse(1, 1, 1, 1);
	mat.specular(0, 0, 0, 1);
	mat.emission(0, 0, 0, 1);
	mat.shininess(0);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	glEnable(GL_LIGHTING);

	glDisable(GL_FOG);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_STENCIL_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_DITHER);
	glDisable(GL_COLOR_LOGIC_OP);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glDisable(GL_POLYGON_STIPPLE);
	glDisable(GL_POLYGON_OFFSET_FILL);

	glShadeModel(GL_FLAT);

	Timer time;
	time.calibrate(static_cast<TIME_FUNC>(glFinish),
		static_cast<TIME_FUNC>(glFinish));

	////////////////////////////////////////////////////////////
	// Immediate-mode independent triangles
	////////////////////////////////////////////////////////////
	measure(time, static_cast<TIME_FUNC>(coloredLit_imIndTri), env, w,
		nTris, r.imTri);
	imTriImage.read(0, 0);
	verify(testImage, colorGen, 0, lastID, imTriImage,
		passed, name, r.config, r.imTri, env,
		"Immediate-mode independent triangle");

	////////////////////////////////////////////////////////////
	// Display-listed independent triangles
	////////////////////////////////////////////////////////////
	dList = glGenLists(1);
	glNewList(dList, GL_COMPILE);
		coloredLit_imIndTri();
	glEndList();
	measure(time, static_cast<TIME_FUNC>(callDList), env, w, nTris,
		r.dlTri);
	glDeleteLists(dList, 1);
	verify(testImage, colorGen, 0, lastID, imTriImage,
		passed, name, r.config, r.dlTri, env,
		"Display-listed independent triangle");

	////////////////////////////////////////////////////////////
	// DrawArrays on independent triangles
	////////////////////////////////////////////////////////////
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(c4ub_n3f_v3f[0]),
		c4ub_n3f_v3f[0].c);
	glEnableClientState(GL_COLOR_ARRAY);
	glNormalPointer(GL_FLOAT, sizeof(c4ub_n3f_v3f[0]),
		c4ub_n3f_v3f[0].n);
	glEnableClientState(GL_NORMAL_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(c4ub_n3f_v3f[0]),
		c4ub_n3f_v3f[0].v);
	glEnableClientState(GL_VERTEX_ARRAY);

	measure(time, static_cast<TIME_FUNC>(coloredLit_daIndTri), env, w,
		nTris, r.daTri);
	verify(testImage, colorGen, 0, lastID, imTriImage,
		passed, name, r.config, r.daTri, env,
		"DrawArrays independent triangle");

	////////////////////////////////////////////////////////////
	// Locked DrawArrays on independent triangles
	//	XXX This is probably unrealistically favorable to
	//	locked arrays.
	////////////////////////////////////////////////////////////
	if (glLockArraysEXT)
		glLockArraysEXT(0, nVertices);
	measure(time, static_cast<TIME_FUNC>(coloredLit_daIndTri), env, w,
		nTris, r.ldaTri);
	if (glUnlockArraysEXT)
		glUnlockArraysEXT();
	if (!glLockArraysEXT)
		r.ldaTri.tps = r.ldaTri.tpsLow = r.ldaTri.tpsHigh = 0.0;
	verify(testImage, colorGen, 0, lastID, imTriImage,
		passed, name, r.config, r.ldaTri, env,
		"Locked DrawArrays independent triangle");

	////////////////////////////////////////////////////////////
	// DrawElements on independent triangles
	////////////////////////////////////////////////////////////
	measure(time, static_cast<TIME_FUNC>(coloredLit_deIndTri), env, w,
		nTris, r.deTri);
	verify(testImage, colorGen, 0, lastID, imTriImage,
		passed, name, r.config, r.deTri, env,
		"DrawElements independent triangle");

	////////////////////////////////////////////////////////////
	// Locked DrawElements on independent triangles
	////////////////////////////////////////////////////////////
	if (glLockArraysEXT)
		glLockArraysEXT(0, nVertices);
	measure(time, static_cast<TIME_FUNC>(coloredLit_deIndTri), env, w,
		nTris, r.ldeTri);
	if (glUnlockArraysEXT)
		glUnlockArraysEXT();
	if (!glLockArraysEXT)
		r.ldeTri.tps = r.ldeTri.tpsLow = r.ldeTri.tpsHigh = 0.0;
	verify(testImage, colorGen, 0, lastID, imTriImage,
		passed, name, r.config, r.ldeTri, env,
		"Locked DrawElements independent triangle");


	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	delete[] c4ub_n3f_v3f;
	delete[] indices;

	// Now we test triangle strips, rather than independent triangles.

	nVertices = nTris + 2;
	lastID = min(IDModulus - 1, nTris - 1);

	c4ub_n3f_v3f = new C4UB_N3F_V3F[nVertices];
	SpiralStrip2D is(nVertices, 0, drawingSize, 0, drawingSize);
	for (int j2 = 0; j2 < nVertices; ++j2) {
		float* t = is(j2);
		GLubyte r, g, b;
		// Take care to get the correct color on the provoking vertex:
		colorGen.toRGB((j2 - 2) % IDModulus, r, g, b);

		c4ub_n3f_v3f[j2].c[0] = r;
		c4ub_n3f_v3f[j2].c[1] = g;
		c4ub_n3f_v3f[j2].c[2] = b;
		c4ub_n3f_v3f[j2].c[3] = 0xFF;
		c4ub_n3f_v3f[j2].n[0] = 0.0;
		c4ub_n3f_v3f[j2].n[1] = 0.0;
		c4ub_n3f_v3f[j2].n[2] = 1.0;
		c4ub_n3f_v3f[j2].v[0] = t[0];
		c4ub_n3f_v3f[j2].v[1] = t[1];
		c4ub_n3f_v3f[j2].v[2] = 0.0;
	}

	indices = new GLuint[nVertices];
	for (int j3 = 0; j3 < nVertices; ++j3)
		indices[j3] = j3;

	////////////////////////////////////////////////////////////
	// Immediate-mode triangle strips
	////////////////////////////////////////////////////////////
	measure(time, static_cast<TIME_FUNC>(coloredLit_imTriStrip), env, w,
		nTris, r.imTS);
	verify(testImage, colorGen, 0, lastID, imTriImage,
		passed, name, r.config, r.imTS, env,
		"Immediate-mode triangle strip");

	////////////////////////////////////////////////////////////
	// Display-listed triangle strips
	////////////////////////////////////////////////////////////
	dList = glGenLists(1);
	glNewList(dList, GL_COMPILE);
		coloredLit_imTriStrip();
	glEndList();
	measure(time, static_cast<TIME_FUNC>(callDList), env, w, nTris,
		r.dlTS);
	glDeleteLists(dList, 1);
	verify(testImage, colorGen, 0, lastID, imTriImage,
		passed, name, r.config, r.dlTS, env,
		"Display-listed triangle strip");

	////////////////////////////////////////////////////////////
	// DrawArrays on triangle strips
	////////////////////////////////////////////////////////////
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(c4ub_n3f_v3f[0]),
		c4ub_n3f_v3f[0].c);
	glEnableClientState(GL_COLOR_ARRAY);
	glNormalPointer(GL_FLOAT, sizeof(c4ub_n3f_v3f[0]),
		c4ub_n3f_v3f[0].n);
	glEnableClientState(GL_NORMAL_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(c4ub_n3f_v3f[0]),
		c4ub_n3f_v3f[0].v);
	glEnableClientState(GL_VERTEX_ARRAY);

	measure(time, static_cast<TIME_FUNC>(coloredLit_daTriStrip), env, w,
		nTris, r.daTS);
	verify(testImage, colorGen, 0, lastID, imTriImage,
		passed, name, r.config, r.daTS, env,
		"DrawArrays triangle strip");

	////////////////////////////////////////////////////////////
	// Locked DrawArrays on triangle strips
	////////////////////////////////////////////////////////////
	if (glLockArraysEXT)
		glLockArraysEXT(0, nVertices);
	measure(time, static_cast<TIME_FUNC>(coloredLit_daTriStrip), env, w,
		nTris, r.ldaTS);
	if (glUnlockArraysEXT)
		glUnlockArraysEXT();
	if (!glLockArraysEXT)
		r.ldaTS.tps = r.ldaTS.tpsLow = r.ldaTS.tpsHigh = 0.0;
	verify(testImage, colorGen, 0, lastID, imTriImage,
		passed, name, r.config, r.ldaTS, env,
		"Locked DrawArrays triangle strip");

	////////////////////////////////////////////////////////////
	// DrawElements on triangle strips
	////////////////////////////////////////////////////////////
	measure(time, static_cast<TIME_FUNC>(coloredLit_deTriStrip), env, w,
		nTris, r.deTS);
	verify(testImage, colorGen, 0, lastID, imTriImage,
		passed, name, r.config, r.deTS, env,
		"DrawElements triangle strip");

	////////////////////////////////////////////////////////////
	// Locked DrawElements on triangle strips
	////////////////////////////////////////////////////////////
	if (glLockArraysEXT)
		glLockArraysEXT(0, nVertices);
	measure(time, static_cast<TIME_FUNC>(coloredLit_deTriStrip), env, w,
		nTris, r.ldeTS);
	if (glUnlockArraysEXT)
		glUnlockArraysEXT();
	if (!glLockArraysEXT)
		r.ldeTS.tps = r.ldeTS.tpsLow = r.ldeTS.tpsHigh = 0.0;
	verify(testImage, colorGen, 0, lastID, imTriImage,
		passed, name, r.config, r.ldeTS, env,
		"Locked DrawElements triangle strip");


	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	delete[] c4ub_n3f_v3f;
	delete[] indices;

	if (passed)
		env->log << name << ":  PASS "
			<< r.config->conciseDescription() << '\n';
	else
		env->log << '\n';
	logStats(r, env);
env->log
<< "\tTHIS TEST IS UNDER DEVELOPMENT; THE RESULTS ARE NOT YET USABLE.\n";
} // ColoredLitPerf::runOne

///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
ColoredLitPerf::compareOne(Result& oldR, Result& newR) {
	bool same = true;

	doComparison(oldR.imTri, newR.imTri, newR.config, same, name,
		env, "immediate-mode independent triangle");
	doComparison(oldR.dlTri, newR.dlTri, newR.config, same, name,
		env, "display-listed independent triangle");
	doComparison(oldR.daTri, newR.daTri, newR.config, same, name,
		env, "DrawArrays independent triangle");
	doComparison(oldR.ldaTri, newR.ldaTri, newR.config, same, name,
		env, "Locked DrawArrays independent triangle");
	doComparison(oldR.deTri, newR.deTri, newR.config, same, name,
		env, "DrawElements independent triangle");
	doComparison(oldR.ldeTri, newR.ldeTri, newR.config, same, name,
		env, "Locked DrawElements independent triangle");
	doComparison(oldR.imTS, newR.imTS, newR.config, same, name,
		env, "immediate-mode triangle strip");
	doComparison(oldR.dlTS, newR.dlTS, newR.config, same, name,
		env, "display-listed triangle strip");
	doComparison(oldR.daTS, newR.daTS, newR.config, same, name,
		env, "DrawArrays triangle strip");
	doComparison(oldR.ldaTS, newR.ldaTS, newR.config, same, name,
		env, "Locked DrawArrays triangle strip");
	doComparison(oldR.deTS, newR.deTS, newR.config, same, name,
		env, "DrawElements triangle strip");
	doComparison(oldR.ldeTS, newR.ldeTS, newR.config, same, name,
		env, "Locked DrawElements triangle strip");

	if (same && env->options.verbosity) {
		env->log << name << ":  SAME "
			<< newR.config->conciseDescription()
			<< "\n\tEach test time falls within the "
			"valid measurement range of the\n"
			"\tother test time; both have the same"
			"image comparison results.\n";
	}

	if (env->options.verbosity) {
		env->log << env->options.db1Name << ':';
		logStats(oldR, env);
		env->log << env->options.db2Name << ':';
		logStats(newR, env);
	}
} // ColoredLitPerf::compareOne

///////////////////////////////////////////////////////////////////////////////
// logDescription:  Print description on the log file, according to the
//	current verbosity level.
///////////////////////////////////////////////////////////////////////////////
void
ColoredLitPerf::logDescription() {
	if (env->options.verbosity)
		env->log <<
"----------------------------------------------------------------------\n"
		<< description << '\n';
} // ColoredLitPerf::logDescription

///////////////////////////////////////////////////////////////////////////////
// Result I/O functions:
///////////////////////////////////////////////////////////////////////////////
void
ColoredLitPerf::Result::put(ostream& s) const {
	s << config->canonicalDescription() << '\n';
	imTri.put(s);
	dlTri.put(s);
	daTri.put(s);
	ldaTri.put(s);
	deTri.put(s);
	ldeTri.put(s);
	imTS.put(s);
	dlTS.put(s);
	daTS.put(s);
	ldaTS.put(s);
	deTS.put(s);
	ldeTS.put(s);
} // ColoredLitPerf::Result::put

bool
ColoredLitPerf::Result::get(istream& s) {
	SkipWhitespace(s);
	string configDesc;
	if (!getline(s, configDesc))
		return false;
	config = new DrawingSurfaceConfig(configDesc);
	imTri.get(s);
	dlTri.get(s);
	daTri.get(s);
	ldaTri.get(s);
	deTri.get(s);
	ldeTri.get(s);
	imTS.get(s);
	dlTS.get(s);
	daTS.get(s);
	ldaTS.get(s);
	deTS.get(s);
	ldeTS.get(s);

	return s.good();
} // ColoredLitPerf::Result::get

vector<ColoredLitPerf::Result*>
ColoredLitPerf::getResults(istream& s) {
	vector<Result*> v;
	while (s.good()) {
		Result* r = new Result();
		if (r->get(s))
			v.push_back(r);
		else {
			delete r;
			break;
		}
	}

	return v;
} // ColoredLitPerf::getResults

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
ColoredLitPerf coloredLitPerfTest("coloredLitPerf", "window, rgb, z, fast",

	"This test examines rendering performance for colored, lit\n"
	"triangles.  It checks several different ways to specify the\n"
	"vertex data in order to determine which is fastest.  The test\n"
	"result is performance measured in triangles per second for\n"
	"each of the various vertex specification methods.\n"

	);


} // namespace GLEAN
