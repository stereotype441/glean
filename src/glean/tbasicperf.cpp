// BEGIN_COPYRIGHT -*- linux-c -*-
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




// tbasicperf.cpp:  implementation of example class for basic tests

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
#include "tbasicperf.h"
#include "misc.h"
#include "perf.h"

namespace {
class MyPerf : public GLEAN::Perf {
public:
	int seconds;

	void preop()  { glFinish(); }
	void op()     { sleep(seconds); }
	void postop() { glFinish(); }

	MyPerf() { seconds = 1; }
};


// Complex results helper functions

void
logStats1(GLEAN::BasicPerfTest::Result& r, GLEAN::Environment* env) {
        env->log << "\tAverage = "
		 << r.tr.timeAvg
		 << "\tRange = ["
		 << r.tr.timeLow
		 << ", "
		 << r.tr.timeHigh
		 << "]\n";
} // logStats1

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
doComparison(const GLEAN::TimeResult& oldR,
             const GLEAN::TimeResult& newR,
             GLEAN::DrawingSurfaceConfig* config,
             bool& same,
             const string& name,
             GLEAN::Environment* env,
	     const char *title)
{
	if (newR.timeLow < oldR.timeLow) {
		double percent = (100.0
				  * (oldR.timeLow - newR.timeLow)
				  / newR.timeLow + 0.5);
		if (percent >= 5) {
			diffHeader(same, name, config, env);
			env->log << '\t'
				 << env->options.db1Name
				 << " may be "
				 << percent
				 << "% faster on "
				 << title
				 << '\n';
		}
        }
	if (newR.timeHigh > oldR.timeHigh) {
		int percent = static_cast<int>(100.0
					       * (newR.timeHigh -oldR.timeHigh)
					       / oldR.timeHigh + 0.5);
		if (percent >= 5) {
			diffHeader(same, name, config, env);
			env->log << '\t'
				 << env->options.db2Name
				 << " may be "
				 << percent
				 << "% faster on "
				 << title
				 << '\n';
		}
        }
} // doComparison
	
}

namespace GLEAN {
const int WIN_SIZE         = 100; // Maximum window width/height.

///////////////////////////////////////////////////////////////////////////////
// Constructors/Destructor:
///////////////////////////////////////////////////////////////////////////////
BasicPerfTest::BasicPerfTest(const char* aName, const char* aFilter,
			     const char* anExtensionList,
			     const char* aDescription):
	Test(aName), filter(aFilter), extensions(anExtensionList),
	description(aDescription) {
} // BasicPerfTest::BasicPerfTest()

BasicPerfTest::BasicPerfTest(const char* aName, const char* aFilter,
			     const char* aDescription):
    	Test(aName), filter(aFilter), extensions(0),
	description(aDescription) {
} // BasicPerfTest::BasicPerfTest()

BasicPerfTest::~BasicPerfTest() {
} // BasicPerfTest::~BasicPerfTest

///////////////////////////////////////////////////////////////////////////////
// run: run tests, save results in a vector and in the results file
///////////////////////////////////////////////////////////////////////////////
void
BasicPerfTest::run(Environment& environment) {
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
		vector<DrawingSurfaceConfig*>configs(f.filter(ws.surfConfigs));

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
BasicPerfTest::compare(Environment& environment) {
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
			env->log << name
				 << ":  NOTE no matching config for "
				 << (*newP)->config->conciseDescription()
				 << '\n';
		else
			compareOne(*(oldR[c]), **newP);
	}

	// Get rid of the results; we don't need them for future comparisons.
	for (vector<Result*>::iterator np = newR.begin(); np < newR.end();
	     ++np)
		delete *np;
	for (vector<Result*>::iterator op = oldR.begin(); op < oldR.end();
	     ++op)
		delete *op;
}

///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////
void
BasicPerfTest::runOne(Result& r, Window &w) {
	MyPerf perf;

	perf.calibrate();
	vector<float> m;
	for (int i = 0; i < 5; i++) {
		env->quiesce();
		double t = perf.time();
		w.swap();	// So user can see something
		m.push_back(t);
	}
	sort(m.begin(), m.end());
	r.tr.timeAvg  = (m[1] + m[2] + m[3]) / 3.0;
	r.tr.timeLow  = m[1];
	r.tr.timeHigh = m[3];
	r.pass        = true;
	logStats(r, env);
} // BasicPerfTest::runOne

///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
BasicPerfTest::compareOne(Result& oldR, Result& newR) {
	bool same = true;

	doComparison(oldR.tr, newR.tr, newR.config, same, name, env, "sleep");

	if (same && env->options.verbosity) {
		env->log << name
			 << ": SAME "
			 << newR.config->conciseDescription()
			 << "\n\t"
			 << env->options.db2Name
			 << " test time falls within the"
			 << " valid measurement range of\n\t"
			 << env->options.db1Name
			 << " test time.\n";
	}
	if (env->options.verbosity) {
		env->log << env->options.db1Name << ':';
		logStats(oldR, env);
		env->log << env->options.db2Name << ':';
		logStats(newR, env);
	}
} // BasicPerfTest::compareOne

///////////////////////////////////////////////////////////////////////////////
// logDescription:  Print description on the log file, according to the
//	current verbosity level.
///////////////////////////////////////////////////////////////////////////////
void
BasicPerfTest::logDescription() {
	if (env->options.verbosity)
		env->log <<
"----------------------------------------------------------------------\n"
			 << description
			 << '\n';
} // BasicPerfTest::logDescription

///////////////////////////////////////////////////////////////////////////////
// Result I/O functions:
///////////////////////////////////////////////////////////////////////////////
void
BasicPerfTest::Result::put(ostream& s) const {
	s << config->canonicalDescription() << '\n' << pass << '\n';
	tr.put(s);
} // BasicPerfTest::Result::put

bool
BasicPerfTest::Result::get(istream& s) {
	SkipWhitespace(s);
	string configDesc;
	if (!getline(s, configDesc))
		return false;
	config = new DrawingSurfaceConfig(configDesc);
	s >> pass;
	tr.get(s);
	return s.good();
} // BasicPerfTest::Result::get

vector<BasicPerfTest::Result*>
BasicPerfTest::getResults(istream& s) {
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
} // BasicPerfTest::getResults

void
BasicPerfTest::logStats(BasicPerfTest::Result& r, GLEAN::Environment* env) {
        env->log << name
                 << ":  "
                 << (r.pass ? "PASS " : "FAIL ")
                 << r.config->conciseDescription()
                 << '\n';
        logStats1(r, env);
} // OrthoPosPoints::logStats

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
BasicPerfTest basicPerfTest("basicPerf", "window",
	"This trivial test simply verifies the internal support for basic\n"
	"tests.  It is run on every OpenGL-capable drawing surface\n"
	"configuration that supports creation of a window.\n");

} // namespace GLEAN
