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




// tgetstr.cpp:  implementation of OpenGL glGetString() tests
#ifdef __UNIX__
#include <unistd.h>
#endif

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include "dsconfig.h"
#include "dsfilt.h"
#include "dsurf.h"
#include "winsys.h"
#include "environ.h"
#include "rc.h"
#include "lex.h"
#include "glutils.h"
#include "tgetstr.h"
#include "misc.h"

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// Constructor/Destructor:
///////////////////////////////////////////////////////////////////////////////
GetStringTest::GetStringTest(const char* aName, const char* aFilter,
    const char* aDescription):
    	Test(aName), filter(aFilter), description(aDescription) {
} // GetStringTest::GetStringTest()

GetStringTest::~GetStringTest() {
} // GetStringTest::~GetStringTest

///////////////////////////////////////////////////////////////////////////////
// run: run tests, save results in a vector and in the results file
///////////////////////////////////////////////////////////////////////////////
void
GetStringTest::run(Environment& environment) {
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
GetStringTest::compare(Environment& environment) {
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
GetStringTest::runOne(Result& r) {
	r.vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
	r.renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
	r.version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
	r.extensions = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
	env->log << name << ":  PASS " << r.config->conciseDescription()<<'\n';
	if (env->options.verbosity) {
		env->log << "\tvendor:     " << r.vendor << '\n';
		env->log << "\trenderer:   " << r.renderer << '\n';
		env->log << "\tversion:    " << r.version << '\n';
		env->log << "\textensions: " << r.extensions << '\n';
	}
} // GetStringTest::runOne

///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
GetStringTest::compareOne(Result& oldR, Result& newR) {
	if (oldR.vendor == newR.vendor && oldR.renderer == newR.renderer
	 && oldR.version == newR.version && oldR.extensions == newR.extensions){
		if (env->options.verbosity)
			env->log << name << ":  SAME " <<
				newR.config->conciseDescription() << '\n';
	} else {
		env->log << name <<  ":  DIFF "
			<< newR.config->conciseDescription() << '\n';
		if (oldR.vendor != newR.vendor) {
			env->log << '\t' << env->options.db1Name
				<< " vendor: " << oldR.vendor;
			env->log << '\t' << env->options.db2Name
				<< " vendor: " << newR.vendor;
		}
		if (oldR.renderer != newR.renderer) {
			env->log << '\t' << env->options.db1Name
				<< " renderer: " << oldR.renderer;
			env->log << '\t' << env->options.db2Name
				<< " renderer: " << newR.renderer;
		}
		if (oldR.version != newR.version) {
			env->log << '\t' << env->options.db1Name
				<< " version: " << oldR.version;
			env->log << '\t' << env->options.db2Name
				<< " version: " << newR.version;
		}
		if (oldR.extensions != newR.extensions) {
			vector<string> oldExts;
			Lex oldLex(oldR.extensions.c_str());
			for (;;) {
				oldLex.next();
				if (oldLex.token == Lex::ID)
					oldExts.push_back(oldLex.id);
				else
					break;
			}
			sort(oldExts.begin(), oldExts.end());

			vector<string> newExts;
			Lex newLex(newR.extensions.c_str());
			for (;;) {
				newLex.next();
				if (newLex.token == Lex::ID)
					newExts.push_back(newLex.id);
				else
					break;
			}
			sort(newExts.begin(), newExts.end());

			vector<string> d(max(oldExts.size(), newExts.size()));
			vector<string>::iterator dEnd;

			dEnd = set_difference(oldExts.begin(), oldExts.end(),
				newExts.begin(), newExts.end(), d.begin());
			if (dEnd != d.begin()) {
				env->log << "\tExtensions in " <<
					env->options.db1Name << " but not in "
					<< env->options.db2Name << ":\n";
				for (vector<string>::iterator p = d.begin();
				    p != dEnd; ++p)
					env->log << "\t\t" << *p << '\n';
			}

			dEnd = set_difference(newExts.begin(), newExts.end(),
				oldExts.begin(), oldExts.end(), d.begin());
			if (dEnd != d.begin()) {
				env->log << "\tExtensions in " <<
					env->options.db2Name << " but not in "
					<< env->options.db1Name << ":\n";
				for (vector<string>::iterator p = d.begin();
				    p != dEnd; ++p)
					env->log << "\t\t" << *p << '\n';
			}

			dEnd = set_intersection(newExts.begin(), newExts.end(),
				oldExts.begin(), oldExts.end(), d.begin());
			if (dEnd != d.begin()) {
				env->log << "\tExtensions in both " <<
					env->options.db2Name << " and in "
					<< env->options.db1Name << ":\n";
				for (vector<string>::iterator p = d.begin();
				    p != dEnd; ++p)
					env->log << "\t\t" << *p << '\n';
			}
		}
	}
} // GetStringTest::compareOne

///////////////////////////////////////////////////////////////////////////////
// logDescription:  Print description on the log file, according to the
//	current verbosity level.
///////////////////////////////////////////////////////////////////////////////
void
GetStringTest::logDescription() {
	if (env->options.verbosity)
		env->log <<
"----------------------------------------------------------------------\n"
		<< description << '\n';
} // GetStringTest::logDescription

///////////////////////////////////////////////////////////////////////////////
// Result I/O functions:
///////////////////////////////////////////////////////////////////////////////
void
GetStringTest::Result::put(ostream& s) const {
	s << config->canonicalDescription() << '\n'
		<< vendor << '\n'
		<< renderer << '\n'
		<< version << '\n'
		<< extensions << '\n';
} // GetStringTest::Result::put

bool
GetStringTest::Result::get(istream& s) {
	SkipWhitespace(s);
	string configDesc;
	if (!getline(s, configDesc))
		return false;
	config = new DrawingSurfaceConfig(configDesc);
	getline(s, vendor);
	getline(s, renderer);
	getline(s, version);
	getline(s, extensions);
	return s.good();
} // GetStringTest::Result::get

vector<GetStringTest::Result*>
GetStringTest::getResults(istream& s) {
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
} // GetStringTest::getResults

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
GetStringTest getStringTest("getString", "window",
	"This test checks the contents of the strings returned by\n"
	"glGetString():  the vendor name, renderer name, version, and\n"
	"extensions.  It is run on every OpenGL-capable drawing surface\n"
	"configuration that supports creation of a window.\n");

} // namespace GLEAN
