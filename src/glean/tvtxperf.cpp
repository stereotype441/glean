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
logStats(GLEAN::ColoredLitPerf::Result& r, GLEAN::Environment* env) {
	env->log << "\tImmediate-mode independent triangle rate = "
		<< r.imTriTps << " tri/sec.\n"
		<< "\t\tRange of valid measurements = ["
		<< r.imTriTpsLow << ", " << r.imTriTpsHigh << "]\n";
} // logStats

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

	glDisable(GL_DITHER);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	GLUtils::useScreenCoords(drawingSize, drawingSize);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	Timer time;
	time.calibrate(static_cast<TIME_FUNC>(glFinish),
		static_cast<TIME_FUNC>(glFinish));

	vector<float> measurements;
	for (int i = 0; i < 5; ++i) {
		env->quiesce();
		double t = time.time(static_cast<TIME_FUNC>(glFinish),
			static_cast<TIME_FUNC>(coloredLit_imIndTri),
			static_cast<TIME_FUNC>(glFinish));
		w.swap();	// So the user can see something happening.
		measurements.push_back(nTris / t);
	}

	delete[] c4ub_n3f_v3f;

	sort(measurements.begin(), measurements.end());
	r.imTriTps = measurements[2];
	r.imTriTpsLow = measurements[1];
	r.imTriTpsHigh = measurements[3];

	env->log << name << ":  PASS "
		<< r.config->conciseDescription() << '\n';
	logStats(r, env);
} // ColoredLitPerf::runOne

///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
ColoredLitPerf::compareOne(Result& oldR, Result& newR) {
	if (oldR.imTriTpsLow < newR.imTriTps && newR.imTriTps < oldR.imTriTpsHigh
	 && newR.imTriTpsLow < oldR.imTriTps && oldR.imTriTps < newR.imTriTpsHigh){
		if (env->options.verbosity)
			env->log << name << ":  SAME "
				<< newR.config->conciseDescription()
				<< "\n\tEach test time falls within the "
				"valid measurement range of the\n"
				"\tother test time.\n";
	} else {
		env->log << name << ":  DIFF "
			<< newR.config->conciseDescription() << '\n';
		env->log << '\t'
			<< ((oldR.imTriTps < newR.imTriTps)?
				env->options.db1Name: env->options.db2Name)
			<< " appears to have higher performance.\n";
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

	s << imTriTps << ' ' << imTriTpsLow << ' ' << imTriTpsHigh << '\n';
} // ColoredLitPerf::Result::put

bool
ColoredLitPerf::Result::get(istream& s) {
	SkipWhitespace(s);
	string configDesc;
	if (!getline(s, configDesc))
		return false;
	config = new DrawingSurfaceConfig(configDesc);

	s >> imTriTps >> imTriTpsLow >> imTriTpsHigh;
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
