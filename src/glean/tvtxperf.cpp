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
C4UB_N3F_V3F* c4ub_n3f_v3f;


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
callDList() {
	glCallList(dList);
} // callDList

void
logStats(GLEAN::ColoredLitPerf::Result& r, GLEAN::Environment* env) {
	env->log << "\tImmediate-mode independent triangle rate = "
		<< r.imTriTps << " tri/sec.\n"
		<< "\t\tRange of valid measurements = ["
		<< r.imTriTpsLow << ", " << r.imTriTpsHigh << "]\n";
	env->log << "\t\tImage sanity check "
		<< (r.imTriImageOK? "passed\n": "failed\n");
	env->log << "\tDisplay-list independent triangle rate = "
		<< r.dlTriTps << " tri/sec.\n"
		<< "\t\tRange of valid measurements = ["
		<< r.dlTriTpsLow << ", " << r.dlTriTpsHigh << "]\n";
	env->log << "\t\tImage sanity check "
		<< (r.dlTriImageOK? "passed\n": "failed\n");
	env->log << "\t\tImage consistency check "
		<< (r.dlTriImageMatch? "passed\n": "failed\n");
} // logStats

void
diffHeader(bool& same, string& name, GLEAN::DrawingSurfaceConfig* config,
    GLEAN::Environment* env) {
	if (same) {
		same = false;
		env->log << name << ":  DIFF "
			<< config->conciseDescription() << '\n';
	}
} // diffHeader

void
failHeader(bool& pass, string& name, GLEAN::DrawingSurfaceConfig* config,
    GLEAN::Environment* env) {
	if (pass) {
		pass = false;
		env->log << name << ":  FAIL "
			<< config->conciseDescription() << '\n';
	}
} // failHeader


typedef void (*TIME_FUNC) ();

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
	Image imTriImage(drawingSize, drawingSize, GL_RGB, GL_UNSIGNED_BYTE);
	Image testImage(drawingSize, drawingSize, GL_RGB, GL_UNSIGNED_BYTE);
	Image::Registration imageReg;
	bool passed = true;

	// Make colors deterministic, so we can check them easily if we
	// choose:
	RGBCodedID colorGen(r.config->r, r.config->g, r.config->b);
	int IDModulus = colorGen.maxID() + 1;

	// We need to minimize the number of pixels per triangle, so that
	// we're measuring vertex-processing rate rather than fill rate.
	// However, we'd also like to guarantee that every triangle covers
	// at least one pixel, so that we can confirm drawing actually took
	// place.  As a compromise, we'll choose a number of triangles that
	// yields approximately 5 pixels per triangle.
	// We're drawing a filled spiral that approximates a circle, so
	// pi * (drawingSize/2)**2 / nTris = 5 implies...
	const int nTris = static_cast<int>
		(((3.14159 / 4.0) * drawingSize * drawingSize) / 5.0 + 0.5);
	nVertices = nTris * 3;
	const int lastID = min(IDModulus - 1, nTris - 1);

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

	glFrontFace(GL_CW);
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

	glDrawBuffer(GL_BACK);
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

	vector<float> measurements;

	////////////////////////////////////////////////////////////
	// Immediate-mode independent triangles
	////////////////////////////////////////////////////////////
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	w.swap();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	measurements.resize(0);
	for (int i1 = 0; i1 < 5; ++i1) {
		env->quiesce();
		double t = time.time(static_cast<TIME_FUNC>(glFinish),
			static_cast<TIME_FUNC>(coloredLit_imIndTri),
			static_cast<TIME_FUNC>(glFinish));
		w.swap();	// So the user can see something happening.
		measurements.push_back(nTris / t);
	}
	sort(measurements.begin(), measurements.end());
	r.imTriTps = measurements[2];
	r.imTriTpsLow = measurements[1];
	r.imTriTpsHigh = measurements[3];

	// Read back the image, verify that every triangle was drawn:
	imTriImage.read(0, 0);
	if (colorGen.allPresent(imTriImage, 0, lastID))
		r.imTriImageOK = true;
	else {
		failHeader(passed, name, r.config, env);
		env->log << "\tImmediate-mode independent triangle rendering is missing\n"
			<< "\t\tsome triangles.\n";
		r.imTriImageOK = false;
	}

	////////////////////////////////////////////////////////////
	// Display-listed independent triangles
	////////////////////////////////////////////////////////////
	dList = glGenLists(1);
	glNewList(dList, GL_COMPILE);
		coloredLit_imIndTri();
	glEndList();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	w.swap();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	measurements.resize(0);
	for (int i2 = 0; i2 < 5; ++i2) {
		env->quiesce();
		double t = time.time(static_cast<TIME_FUNC>(glFinish),
			static_cast<TIME_FUNC>(callDList),
			static_cast<TIME_FUNC>(glFinish));
		w.swap();
		measurements.push_back(nTris / t);
	}
	glDeleteLists(dList, 1);

	sort(measurements.begin(), measurements.end());
	r.dlTriTps = measurements[2];
	r.dlTriTpsLow = measurements[1];
	r.dlTriTpsHigh = measurements[3];

	// Verify that the image is the same as that produced by
	// rendering independent triangles:
	testImage.read(0, 0);
	if (colorGen.allPresent(testImage, 0, lastID))
		r.dlTriImageOK = true;
	else {
		failHeader(passed, name, r.config, env);
		env->log << "\tDisplay-listed independent triangle rendering is missing\n"
			<< "\t\tsome triangles.\n";
		r.dlTriImageOK = false;
	}
	imageReg = testImage.reg(imTriImage);
	if (imageReg.stats[0].max()
	    + imageReg.stats[1].max()
	    + imageReg.stats[2].max() != 0.0) {
		failHeader(passed, name, r.config, env);
		env->log << "\tDisplay-listed independent triangle rendering\n"
			<< "\t\tyielded different image than immediate-mode\n"
			<< "\t\tindependent triangle rendering.\n";
		r.dlTriImageMatch = false;
	} else
		r.dlTriImageMatch = true;

	delete[] c4ub_n3f_v3f;

	if (passed)
		env->log << name << ":  PASS "
			<< r.config->conciseDescription() << '\n';
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
	if (newR.imTriTps < oldR.imTriTpsLow) {
		diffHeader(same, name, newR.config, env);
		env->log << '\t' << env->options.db1Name
			<< " may have higher immediate-mode "
			"independent triangle performance.\n";
	}
	if (newR.imTriTps > oldR.imTriTpsHigh) {
		diffHeader(same, name, newR.config, env);
		env->log << '\t' << env->options.db2Name
			<< " may have higher immediate-mode "
			"independent triangle performance.\n";
	}
	if (newR.imTriImageOK != oldR.imTriImageOK) {
		diffHeader(same, name, newR.config, env);
		env->log << '\t' << env->options.db1Name << " image check "
			<< (oldR.imTriImageOK? "passed\n": "failed\n");
		env->log << '\t' << env->options.db2Name << " image check "
			<< (newR.imTriImageOK? "passed\n": "failed\n");
	}

	if (newR.dlTriTps < oldR.dlTriTpsLow) {
		diffHeader(same, name, newR.config, env);
		env->log << '\t' << env->options.db1Name
			<< " may have higher display-list "
			"independent triangle performance.\n";
	}
	if (newR.dlTriTps > oldR.dlTriTpsHigh) {
		diffHeader(same, name, newR.config, env);
		env->log << '\t' << env->options.db2Name
			<< " may have higher display-list "
			"independent triangle performance.\n";
	}
	if (newR.dlTriImageOK != oldR.dlTriImageOK) {
		diffHeader(same, name, newR.config, env);
		env->log << '\t' << env->options.db1Name << " image check "
			<< (oldR.dlTriImageOK? "passed\n": "failed\n");
		env->log << '\t' << env->options.db2Name << " image check "
			<< (newR.dlTriImageOK? "passed\n": "failed\n");
	}
	if (newR.dlTriImageMatch != oldR.dlTriImageMatch) {
		diffHeader(same, name, newR.config, env);
		env->log << '\t' << env->options.db1Name << " image compare "
			<< (oldR.dlTriImageMatch? "passed\n": "failed\n");
		env->log << '\t' << env->options.db2Name << " image compare "
			<< (newR.dlTriImageMatch? "passed\n": "failed\n");
	}

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

	s << imTriTps << ' ' << imTriTpsLow << ' ' << imTriTpsHigh
		<< imTriImageOK << '\n';
	s << dlTriTps << ' ' << dlTriTpsLow << ' ' << dlTriTpsHigh
		<< dlTriImageOK << dlTriImageMatch << '\n';
} // ColoredLitPerf::Result::put

bool
ColoredLitPerf::Result::get(istream& s) {
	SkipWhitespace(s);
	string configDesc;
	if (!getline(s, configDesc))
		return false;
	config = new DrawingSurfaceConfig(configDesc);

	s >> imTriTps >> imTriTpsLow >> imTriTpsHigh >> imTriImageOK;
	s >> dlTriTps >> dlTriTpsLow >> dlTriTpsHigh >> dlTriImageOK
		>> dlTriImageMatch;
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
