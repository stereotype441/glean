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




// trgbtris.cpp:  example image-based test to show use of TIFF images
#if defined __UNIX__
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
#include "rand.h"
#include "stats.h"
#include "image.h"
#include "trgbtris.h"
#include "misc.h"

namespace {

const int drawingSize = 64;	// Don't make this too large!
				// Uncompressed TIFF images demand a lot of
				// disk space and network bandwidth.

void
logStats(GLEAN::BasicStats& stats, GLEAN::Environment* env) {
	env->log << "\t\tmin = " << stats.min() << ", max = " << stats.max()
		<< "\n\t\tmean = " << stats.mean() << ", standard deviation = "
		<< stats.deviation() << '\n';
}

} // anonymous namespace

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// Constructor/Destructor:
///////////////////////////////////////////////////////////////////////////////
RGBTriStripTest::RGBTriStripTest(const char* aName, const char* aFilter,
    const char* aDescription):
    	Test(aName), filter(aFilter), description(aDescription) {
} // RGBTriStripTest::RGBTriStripTest()

RGBTriStripTest::~RGBTriStripTest() {
} // RGBTriStripTest::~RGBTriStripTest

///////////////////////////////////////////////////////////////////////////////
// run: run tests, save results in a vector and in the results file
///////////////////////////////////////////////////////////////////////////////
void
RGBTriStripTest::run(Environment& environment) {
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
			r->imageNumber = 1 + (p - configs.begin());
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
RGBTriStripTest::compare(Environment& environment) {
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
RGBTriStripTest::runOne(Result& r, Window& w) {
	GLUtils::useScreenCoords(drawingSize + 2, drawingSize + 2);

	int nPoints = 20;	// Exact value doesn't really matter.
	RandomDouble vRand(142857);
	RandomMesh2D v(1.0, drawingSize, nPoints, 1.0, drawingSize, nPoints,
		vRand);

	RandomDouble cRand(271828);

	glClear(GL_COLOR_BUFFER_BIT);
	glShadeModel(GL_SMOOTH);

	for (int row = 0; row < nPoints - 1; ++row) {
		glBegin(GL_TRIANGLE_STRIP);
			for (int col = 0; col < nPoints; ++col) {
				float r = cRand.next();
				float g = cRand.next();
				float b = cRand.next();
				glColor3f(r, g, b);
				glVertex2fv(v(row, col));
				r = cRand.next();
				g = cRand.next();
				b = cRand.next();
				glColor3f(r, g, b);
				glVertex2fv(v(row + 1, col));
			}
		glEnd();
	}
	w.swap();

	Image image(drawingSize + 2, drawingSize + 2, GL_RGB, GL_FLOAT);
	image.read(0, 0);	// Invoke glReadPixels to read the image.
	image.writeTIFF(env->imageFileName(name, r.imageNumber));

	env->log << name << ":  NOTE "
		<< r.config->conciseDescription() << '\n'
		<< "\tImage number " << r.imageNumber << '\n';
	if (env->options.verbosity)
		env->log <<
		   "\tThis test does not check its result.  Please view\n"
		   "\tthe image to verify that the result is correct, or\n"
		   "\tcompare it to a known-good result from a different\n"
		   "\trun of glean.\n";
} // RGBTriStripTest::runOne

///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
RGBTriStripTest::compareOne(Result& oldR, Result& newR) {
	// Fetch the old and new images:
	Image oldI;
	oldI.readTIFF(env->image1FileName(name, oldR.imageNumber));
	Image newI;
	newI.readTIFF(env->image2FileName(name, newR.imageNumber));

	// Register the images, and gather statistics about the differences
	// for each color channel:
	Image::Registration reg(oldI.reg(newI));

	// Compute worst-case tolerance (1 LSB in the shallowest drawing
	// surface configuration) for each color channel:
	double rTol = 1.0 / (1 << min(oldR.config->r, newR.config->r));
	double gTol = 1.0 / (1 << min(oldR.config->g, newR.config->g));
	double bTol = 1.0 / (1 << min(oldR.config->b, newR.config->b));

	// We'll conclude that the images are the ``same'' if the maximum
	// absolute error is no more than 1 LSB (in the shallowest config):
	if (reg.stats[0].max() <= rTol && reg.stats[1].max() <= gTol
	 && reg.stats[2].max() <= bTol) {
		if (env->options.verbosity) {
			env->log << name << ": SAME "
				<< newR.config->conciseDescription() << '\n';
			if (reg.stats[0].max() == 0 && reg.stats[1].max() == 0
			 && reg.stats[1].max() == 0)
				env->log << "\tImages are exactly equal\n";
			else
				env->log << "\tImages are approximately equal\n";
		}
	} else {
		env->log << name << ":  DIFF "
			<< newR.config->conciseDescription() << '\n'
			<< "\tDifference exceeds 1 LSB in at least one "
			   "color channel\n";
	}
	if (env->options.verbosity) {
		env->log << "\tred:\n";
		logStats(reg.stats[0], env);
		env->log << "\tgreen:\n";
		logStats(reg.stats[1], env);
		env->log << "\tblue:\n";
		logStats(reg.stats[2], env);
	}
} // RGBTriStripTest::compareOne

///////////////////////////////////////////////////////////////////////////////
// logDescription:  Print description on the log file, according to the
//	current verbosity level.
///////////////////////////////////////////////////////////////////////////////
void
RGBTriStripTest::logDescription() {
	if (env->options.verbosity)
		env->log <<
"----------------------------------------------------------------------\n"
		<< description << '\n';
} // RGBTriStripTest::logDescription

///////////////////////////////////////////////////////////////////////////////
// Result I/O functions:
///////////////////////////////////////////////////////////////////////////////
void
RGBTriStripTest::Result::put(ostream& s) const {
	s << config->canonicalDescription() << '\n';

	s << imageNumber << '\n';
} // RGBTriStripTest::Result::put

bool
RGBTriStripTest::Result::get(istream& s) {
	SkipWhitespace(s);
	string configDesc;
	if (!getline(s, configDesc))
		return false;
	config = new DrawingSurfaceConfig(configDesc);

	s >> imageNumber;
	return s.good();
} // RGBTriStripTest::Result::get

vector<RGBTriStripTest::Result*>
RGBTriStripTest::getResults(istream& s) {
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
} // RGBTriStripTest::getResults

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
RGBTriStripTest rgbTriStripTest("rgbTriStrip", "window, rgb",

	"The best approach when designing a test is to make it\n"
	"self-checking; that is, the test itself should determine\n"
	"whether the image or other data structure that it produces is\n"
	"correct.  However, some tests are difficult to design in this\n"
	"way, and for some other tests (like stress tests or regression\n"
	"tests concerning previously-reported bugs) it may be\n"
	"unnecessary.  For such tests, glean provides mechanisms to\n"
	"save images and compare them to images generated from other\n"
	"runs.  This test simply exercises those mechanisms.\n");


} // namespace GLEAN
