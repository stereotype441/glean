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




// tbasic.cpp:  implementation of example class for basic tests

#ifdef __UNIX__
#include <unistd.h>
#endif

#include <iostream>
#include <fstream>
#include "dsconfig.h"
#include "dsfilt.h"
#include "dsurf.h"
#include "winsys.h"
#include "environ.h"
#include "rc.h"
#include "glutils.h"
#include "tbasic.h"
#include "misc.h"

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// Constructors/Destructor:
///////////////////////////////////////////////////////////////////////////////
BasicTest::BasicTest(const char* aName, const char* aFilter,
    const char* anExtensionList, const char* aDescription):
    	Test(aName), filter(aFilter), extensions(anExtensionList),
	description(aDescription) {
} // BasicTest::BasicTest()

BasicTest::BasicTest(const char* aName, const char* aFilter,
    const char* aDescription):
    	Test(aName), filter(aFilter), extensions(0),
	description(aDescription) {
} // BasicTest::BasicTest()

BasicTest::~BasicTest() {
} // BasicTest::~BasicTest

///////////////////////////////////////////////////////////////////////////////
// run: run tests, save results in a vector and in the results file
///////////////////////////////////////////////////////////////////////////////
void
BasicTest::run(Environment& environment) {
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
			Window w(ws, **p, 258, 258);
			RenderingContext rc(ws, **p);
			if (!ws.makeCurrent(rc, w))
				;	// XXX need to throw exception here

			// Check for all prerequisite extensions.  Note
			// that this must be done after the rendering
			// context has been created and made current!
			if (!GLUtils::haveExtensions(extensions))
				continue;

			// Create a result object and run the test:
			Result* r = new Result();
			r->config = *p;
			runOne(*r);

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
BasicTest::compare(Environment& environment) {
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
BasicTest::runOne(Result& r) {
	r.pass = true;
	env->log << name << (r.pass? ":  PASS ": ":  FAIL ")
		<< r.config->conciseDescription() << '\n';
} // BasicTest::runOne

///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
BasicTest::compareOne(Result& oldR, Result& newR) {
	if (oldR.pass == newR.pass) {
		if (env->options.verbosity)
			env->log << name << ":  SAME "
				<< newR.config->conciseDescription() << '\n'
				<< (oldR.pass? "\tBoth PASS\n": "\tBoth FAIL\n")
				;
	} else {
		env->log << name << ":  DIFF "
			<< newR.config->conciseDescription() << '\n'
			<< '\t' << env->options.db1Name
			<<	(oldR.pass? " PASS, ": " FAIL, ")
			<< env->options.db2Name
			<<	(newR.pass? " PASS\n": " FAIL\n");
	}
} // BasicTest::compareOne

///////////////////////////////////////////////////////////////////////////////
// logDescription:  Print description on the log file, according to the
//	current verbosity level.
///////////////////////////////////////////////////////////////////////////////
void
BasicTest::logDescription() {
	if (env->options.verbosity)
		env->log <<
"----------------------------------------------------------------------\n"
		<< description << '\n';
} // BasicTest::logDescription

///////////////////////////////////////////////////////////////////////////////
// Result I/O functions:
///////////////////////////////////////////////////////////////////////////////
void
BasicTest::Result::put(ostream& s) const {
	s << config->canonicalDescription() << '\n' << pass << '\n';
} // BasicTest::Result::put

bool
BasicTest::Result::get(istream& s) {
	SkipWhitespace(s);
	string configDesc;
	if (!getline(s, configDesc))
		return false;
	config = new DrawingSurfaceConfig(configDesc);
	s >> pass;
	return s.good();
} // BasicTest::Result::get

vector<BasicTest::Result*>
BasicTest::getResults(istream& s) {
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
} // BasicTest::getResults

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
BasicTest basicTest("basic", "window",
	"This trivial test simply verifies the internal support for basic\n"
	"tests.  It is run on every OpenGL-capable drawing surface\n"
	"configuration that supports creation of a window.\n");

} // namespace GLEAN
