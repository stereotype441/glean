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




// tbinding.cpp:  Test functions in the window-system binding

#ifdef __UNIX__
#include <unistd.h>
#endif

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include "dsconfig.h"
#include "dsfilt.h"
#include "dsurf.h"
#include "winsys.h"
#include "environ.h"
#include "rc.h"
#include "glutils.h"
#include "rand.h"
#include "stats.h"
#include "image.h"
#include "tbinding.h"
#include "misc.h"

namespace {

const int drawingSize = 64;

bool
makeCurrentOK(GLEAN::DrawingSurfaceConfig& config) {
	using namespace GLEAN;
	float expected[4];
	glClear(GL_COLOR_BUFFER_BIT);
	glGetFloatv(GL_COLOR_CLEAR_VALUE, expected);
	Image probe(1, 1, GL_RGBA, GL_FLOAT);
	probe.read(drawingSize/2, drawingSize/2);
	const float* actual = reinterpret_cast<float*>(probe.pixels());
	double maxError = ErrorBits(fabs(expected[0] - actual[0]), config.r);
	maxError = max(maxError,
		ErrorBits(fabs(expected[1] - actual[1]), config.g));
	maxError = max(maxError,
		ErrorBits(fabs(expected[2] - actual[2]), config.b));
	return maxError <= 1.0;
} // makeCurrentOK

} // anonymous namespace

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// Constructor/Destructor:
///////////////////////////////////////////////////////////////////////////////
MakeCurrentTest::MakeCurrentTest(const char* aName, const char* aFilter,
    const char* aDescription):
    	Test(aName), filter(aFilter), description(aDescription) {
} // MakeCurrentTest::MakeCurrentTest()

MakeCurrentTest::~MakeCurrentTest() {
} // MakeCurrentTest::~MakeCurrentTest

///////////////////////////////////////////////////////////////////////////////
// run: run tests, save results in a vector and in the results file
///////////////////////////////////////////////////////////////////////////////
void
MakeCurrentTest::run(Environment& environment) {
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
MakeCurrentTest::compare(Environment& environment) {
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
MakeCurrentTest::runOne(Result& r, Window& w) {

	DrawingSurfaceConfig& config = *(r.config);
	WindowSystem& ws = env->winSys;

	// The rendering contexts to be used:
	vector<RenderingContext*> rcs;
	// Short descriptions of the rendering contexts:
	vector<const char*> descriptions;
	// Complete record of rendering contexts made "current" during
	// the test:
	vector<int> testSequence;

	RandomBitsDouble rRand(config.r, 712105);
	RandomBitsDouble gRand(config.g, 63230);
	RandomBitsDouble bRand(config.b, 912167);

	// Create rendering contexts to be used with the test window.
	// Note that the first context (at index 0) is always the
	// null context.

	rcs.push_back(0);
	descriptions.push_back("Null context");
	ws.makeCurrent();
	testSequence.push_back(static_cast<int>(rcs.size()) - 1);

	rcs.push_back(new RenderingContext(env->winSys, config, 0, true));
	descriptions.push_back("Direct-rendering context");
	ws.makeCurrent(*rcs.back(), w);
	testSequence.push_back(static_cast<int>(rcs.size()) - 1);
	glDisable(GL_DITHER);
	glClearColor(rRand.next(), gRand.next(), bRand.next(), 1.0);
	if (!makeCurrentOK(config))
		goto failed;

	rcs.push_back(new RenderingContext(env->winSys, config, 0, false));
	descriptions.push_back("Indirect-rendering context");
	ws.makeCurrent(*rcs.back(), w);
	testSequence.push_back(static_cast<int>(rcs.size()) - 1);
	glDisable(GL_DITHER);
	glClearColor(rRand.next(), gRand.next(), bRand.next(), 1.0);
	if (!makeCurrentOK(config))
		goto failed;

	// Now run through all the pairs of rendering contexts, making
	// them current in sequence and checking that rendering looks
	// correct.  Don't worry about the redundant sequences; we want
	// to check those, too!

	int i;
	for (i = 0; i < static_cast<int>(rcs.size()); ++i)
		for (int j = 0; j < static_cast<int>(rcs.size()); ++j) {
			testSequence.push_back(i);
			if (rcs[i] == 0)
				ws.makeCurrent();
			else {
				ws.makeCurrent(*rcs[i], w);
				if (!makeCurrentOK(config))
					goto failed;
			}
			testSequence.push_back(j);
			if (rcs[j] == 0)
				ws.makeCurrent();
			else {
				ws.makeCurrent(*rcs[j], w);
				if (!makeCurrentOK(config))
					goto failed;
			}
		}

	env->log << name << ":  PASS "
		<< config.conciseDescription() << '\n';
	r.pass = true;
	goto cleanup;

failed:
	// The braces are to appease MSVC's broken for-loop variable declaration scope
	// semantics and its tendency to complain about skipped variable initialization.
	// Without the braces it complains that k's initialization can be skipped
	// by jumping to "cleanup".
	{
	  env->log << name << ":  FAIL "
		   << config.conciseDescription() << '\n';
	  env->log << "\tSequence of MakeCurrent operations was:\n";
	  for (int k = 0; k < static_cast<int>(testSequence.size()); ++k)
	    env->log << "\t\t" << descriptions[testSequence[k]] << '\n';
	  r.pass = false;
	}

cleanup:
	for (i = 0; i < static_cast<int>(rcs.size()); ++i)
		if (rcs[i])
			delete rcs[i];

} // MakeCurrentTest::runOne

///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
MakeCurrentTest::compareOne(Result& oldR, Result& newR) {

	if (oldR.pass == newR.pass) {
		if (env->options.verbosity)
			env->log << name << ": SAME "
				<< newR.config->conciseDescription() << '\n';
	} else {
		env->log << name << ": DIFF "
			<< newR.config->conciseDescription() << '\n';
		env->log << '\t' << env->options.db1Name << ' '
			<< (oldR.pass? "PASS": "FAIL") << '\n';
		env->log << '\t' << env->options.db2Name << ' '
			<< (newR.pass? "PASS": "FAIL") << '\n';

	}
} // MakeCurrentTest::compareOne

///////////////////////////////////////////////////////////////////////////////
// logDescription:  Print description on the log file, according to the
//	current verbosity level.
///////////////////////////////////////////////////////////////////////////////
void
MakeCurrentTest::logDescription() {
	if (env->options.verbosity)
		env->log <<
"----------------------------------------------------------------------\n"
		<< description << '\n';
} // MakeCurrentTest::logDescription

///////////////////////////////////////////////////////////////////////////////
// Result I/O functions:
///////////////////////////////////////////////////////////////////////////////
void
MakeCurrentTest::Result::put(ostream& s) const {
	s << config->canonicalDescription() << '\n';
	s << pass << '\n';
} // MakeCurrentTest::Result::put

bool
MakeCurrentTest::Result::get(istream& s) {
	SkipWhitespace(s);
	string configDesc;
	if (!getline(s, configDesc))
		return false;
	config = new DrawingSurfaceConfig(configDesc);

	s >> pass;

	return s.good();
} // MakeCurrentTest::Result::get

vector<MakeCurrentTest::Result*>
MakeCurrentTest::getResults(istream& s) {
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
} // MakeCurrentTest::getResults

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
MakeCurrentTest makeCurrentTest("makeCurrent", "window, rgb",

	"This test sanity-checks the ability to use multiple rendering\n"
	"contexts.  It creates several contexts with differing\n"
	"characteristics (e.g., some are direct-rendering and some\n"
	"are indirect-rendering, if the window system binding supports\n"
	"that distinction).  Then it runs through all pairs of contexts,\n"
	"making each one \"current\" in turn and verifying that simple\n"
	"rendering succeeds.\n"

	);

} // namespace GLEAN
