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




// tchgperf.cpp:  Some basic tests of attribute-change performance.
#ifdef __UNIX__
#include <unistd.h>
#endif

#include <iostream>
#include <fstream>
#include <algorithm>
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
#include "tchgperf.h"
#include "misc.h"


namespace {

const int drawingSize = 128;	// must be power-of-2, 128 or greater
GLEAN::Image redImage(64, 64, GL_RGB, GL_UNSIGNED_BYTE, 1.0, 0.0, 0.0, 0.0);
GLuint redTex;
GLEAN::Image greenImage(64, 64, GL_RGB, GL_UNSIGNED_BYTE, 0.0, 1.0, 0.0, 0.0);
GLuint greenTex;

int nPoints;
float* vertices;
float* texCoords;

void
noBindDraw() {
	int rowSize = 2 * nPoints;
	for (int y = 0; y < nPoints - 1; ++y) {
		float* t0 = texCoords + y * rowSize;
		float* v0 = vertices + y * rowSize;
		for (int x = 0; x < nPoints - 1; ++x) {
			float* t1 = t0 + rowSize;
			float* t2 = t1 + 2;
			float* t3 = t0 + 2;
			float* v1 = v0 + rowSize;
			float* v2 = v1 + 2;
			float* v3 = v0 + 2;

			glBegin(GL_TRIANGLES);
				glTexCoord2fv(t0);
				glVertex2fv(v0);
				glTexCoord2fv(t1);
				glVertex2fv(v1);
				glTexCoord2fv(t2);
				glVertex2fv(v2);
			glEnd();
			glBegin(GL_TRIANGLES);
				glTexCoord2fv(t2);
				glVertex2fv(v2);
				glTexCoord2fv(t3);
				glVertex2fv(v3);
				glTexCoord2fv(t0);
				glVertex2fv(v0);
			glEnd();

			t0 += 2;
			v0 += 2;
		}
	}
} // noBindDraw

void
bindDraw() {
	int rowSize = 2 * nPoints;
	for (int y = 0; y < nPoints - 1; ++y) {
		float* v0 = vertices + y * rowSize;
		float* t0 = texCoords + y * rowSize;
		for (int x = 0; x < nPoints - 1; ++x) {
			float* t1 = t0 + rowSize;
			float* t2 = t1 + 2;
			float* t3 = t0 + 2;
			float* v1 = v0 + rowSize;
			float* v2 = v1 + 2;
			float* v3 = v0 + 2;

			glBindTexture(GL_TEXTURE_2D, redTex);
			glBegin(GL_TRIANGLES);
				glTexCoord2fv(t0);
				glVertex2fv(v0);
				glTexCoord2fv(t1);
				glVertex2fv(v1);
				glTexCoord2fv(t2);
				glVertex2fv(v2);
			glEnd();

			glBindTexture(GL_TEXTURE_2D, greenTex);
			glBegin(GL_TRIANGLES);
				glTexCoord2fv(t2);
				glVertex2fv(v2);
				glTexCoord2fv(t3);
				glVertex2fv(v3);
				glTexCoord2fv(t0);
				glVertex2fv(v0);
			glEnd();

			t0 += 2;
			v0 += 2;
		}
	}
} // noBindDraw

void
logStats(GLEAN::TexBindPerf::Result& r, GLEAN::Environment* env) {
	env->log << "\tApproximate texture binding time = " << r.bindTime
		<< " microseconds.\n\tRange of valid measurements = ["
		<< r.lowerBound << ", " << r.upperBound << "]\n";
} // logStats

} // anonymous namespace

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// Constructor/Destructor:
///////////////////////////////////////////////////////////////////////////////
TexBindPerf::TexBindPerf(const char* aName, const char* aFilter,
    const char* aDescription):
    	Test(aName), filter(aFilter), description(aDescription) {
} // TexBindPerf::TexBindPerf()

TexBindPerf::~TexBindPerf() {
} // TexBindPerf::~TexBindPerf

///////////////////////////////////////////////////////////////////////////////
// run: run tests, save results in a vector and in the results file
///////////////////////////////////////////////////////////////////////////////
void
TexBindPerf::run(Environment& environment) {
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

		// Test each config:
		for (vector<DrawingSurfaceConfig*>::const_iterator
		    p = configs.begin(); p < configs.end(); ++p) {
			Window w(ws, **p, drawingSize + 2, drawingSize + 2);
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
TexBindPerf::compare(Environment& environment) {
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

typedef void (*TIME_FUNC) ();

void
TexBindPerf::runOne(Result& r, Window& w) {
	Timer time;
	time.calibrate((TIME_FUNC)glFinish, (TIME_FUNC) glFinish);

	glGenTextures(1, &redTex);
	glBindTexture(GL_TEXTURE_2D, redTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	redImage.makeMipmaps(GL_RGB);

	glGenTextures(1, &greenTex);
	glBindTexture(GL_TEXTURE_2D, greenTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	greenImage.makeMipmaps(GL_RGB);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	GLUtils::useScreenCoords(drawingSize + 2, drawingSize + 2);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_TEXTURE_2D);
	glColor4f(1.0, 1.0, 1.0, 1.0);

	nPoints = drawingSize / 2;	// Yields 1-pixel triangles.

	RandomDouble vRand(142857);
	RandomMesh2D v(1.0, drawingSize, nPoints, 1.0, drawingSize, nPoints,
		vRand);
	vertices = v(0, 0);

	RandomDouble tRand(314159);
	RandomMesh2D t(0.0, 1.0, nPoints, 0.0, 1.0, nPoints, tRand);
	texCoords = t(0, 0);

	int nTris = (nPoints - 1) * (nPoints - 1) / 2;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	vector<float> measurements;
	for (int i = 0; i < 5; ++i) {
		env->quiesce();
		double tBind = time.time((TIME_FUNC)glFinish, (TIME_FUNC) bindDraw, (TIME_FUNC) glFinish);
		w.swap();	// So the user can see something happening.

		env->quiesce();
		double tNoBind = time.time((TIME_FUNC)glFinish, (TIME_FUNC)noBindDraw, (TIME_FUNC)glFinish);
		w.swap();

		double bindTime = 1E6 * (tBind - tNoBind) / nTris;
		if (bindTime < 0.0) {
			// This can happen if the system isn't quiescent;
			// some process sneaks in and takes wall-clock time
			// when ``noBindDraw'' is running.  Just flush it
			// and try again.  (Note:  You really shouldn't be
			// running timing tests on a system where other
			// processes are active!)
			--i;
			continue;
		}

		measurements.push_back(bindTime);
	}

	sort(measurements.begin(), measurements.end());
	r.bindTime = measurements[2];
	r.lowerBound = measurements[1];
	r.upperBound = measurements[3];

	env->log << name << ":  PASS "
		<< r.config->conciseDescription() << '\n';
	logStats(r, env);
} // TexBindPerf::runOne

///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
TexBindPerf::compareOne(Result& oldR, Result& newR) {
	if (oldR.lowerBound < newR.bindTime && newR.bindTime < oldR.upperBound
	 && newR.lowerBound < oldR.bindTime && oldR.bindTime < newR.upperBound){
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
			<< ((oldR.bindTime < newR.bindTime)?
				env->options.db1Name: env->options.db2Name)
			<< " appears to have higher performance.\n";
	}
	if (env->options.verbosity) {
		env->log << env->options.db1Name << ':';
		logStats(oldR, env);
		env->log << env->options.db2Name << ':';
		logStats(newR, env);
	}
} // TexBindPerf::compareOne

///////////////////////////////////////////////////////////////////////////////
// logDescription:  Print description on the log file, according to the
//	current verbosity level.
///////////////////////////////////////////////////////////////////////////////
void
TexBindPerf::logDescription() {
	if (env->options.verbosity)
		env->log <<
"----------------------------------------------------------------------\n"
		<< description << '\n';
} // TexBindPerf::logDescription

///////////////////////////////////////////////////////////////////////////////
// Result I/O functions:
///////////////////////////////////////////////////////////////////////////////
void
TexBindPerf::Result::put(ostream& s) const {
	s << config->canonicalDescription() << '\n';

	s << bindTime << ' ' << lowerBound << ' ' << upperBound << '\n';
} // TexBindPerf::Result::put

bool
TexBindPerf::Result::get(istream& s) {
	SkipWhitespace(s);
	string configDesc;
	if (!getline(s, configDesc))
		return false;
	config = new DrawingSurfaceConfig(configDesc);

	s >> bindTime >> lowerBound >> upperBound;
	return s.good();
} // TexBindPerf::Result::get

vector<TexBindPerf::Result*>
TexBindPerf::getResults(istream& s) {
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
} // TexBindPerf::getResults

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
TexBindPerf texBindPerfTest("texBindPerf", "window, rgb, z",

	"This test makes a rough estimate of the cost of a glBindTexture()\n"
	"operation, expressed in microseconds.\n"
	"\n"
	"Since the apparent cost of a texture bind is dependent on many\n"
	"factors (including the fraction of the texture map that's actually\n"
	"used for drawing, on machines that cache textures; texture map\n"
	"size; texel format; etc.), a general-purpose test can only estimate\n"
	"it.  In this test we do so by drawing random triangles of very\n"
	"small size, and reporting simple statistics concerning the cost.\n");


} // namespace GLEAN
