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


// tpgos.h:  Example class for pgos tests
//
// glPolygonOffset test code donated to the glean project by
// Angus Dorbie <dorbie@sgi.com>, Thu, 07 Sep 2000 04:13:45 -0700
//
// glPolygonOffset test code integrated into glean framework by
// Rickard E. (Rik) Faith <faith@precisioninsight.com>, October 2000

// This class (derived from Test) provides a framework for a large set
// of correctness tests that should be portable (in the sense that
// they don't contain OS- or window-system-specific code).

// Each pgos test includes a drawing surface filter string.  The test
// will be run on all the drawing surface configurations that are
// selected by the filter, and one result structure will be generated
// for each such configuration.
//
// Each pgos test may also include an extension filter string.  The test
// will only be run on contexts that support all the listed extensions.
// Extension names in the string may be separated with non alphanumerics;
// whitespace and commas are used by convention.

// When comparing two runs, the drawing surface configurations are
// used to select plausible matches among the results.


#ifndef __tpgos_h__
#define __tpgos_h__

#include "test.h"

class DrawingSurfaceConfig;		// Forward reference.

namespace GLEAN {

// Auxiliary struct for holding a glPolygonOffset result
struct POResult {
	static const int maxtests = 100;
	int              testcount;

	struct {
		float factor;
		float units;
		float near;
		float far;
		bool  punchthrough;
		float punchthrough_translation;
		bool  badEdge;
		bool  badMiddle;
	} data[maxtests];
	
	POResult() {
		testcount = 0;
		for (int i = 0; i < maxtests; i++) {
			data[i].factor                   = 0.0;
			data[i].units                    = 0.0;
			data[i].near                     = 0.0;
			data[i].far                      = 0.0;
			data[i].punchthrough             = false;
			data[i].punchthrough_translation = 0.0;
			data[i].badEdge                  = false;
			data[i].badMiddle                = false;
		}
	}
	
	void put(ostream& s) const {
		s << testcount;
		for (int i = 0; i < testcount; i++) {
			s << ' ' << data[i].factor
			  << ' ' << data[i].units
			  << ' ' << data[i].near
			  << ' ' << data[i].far
			  << ' ' << data[i].punchthrough
			  << ' ' << data[i].punchthrough_translation
			  << ' ' << data[i].badEdge
			  << ' ' << data[i].badMiddle;
		}
		s << '\n';
	}
	
	void get(istream& s) {
		s >> testcount;
		for (int i = 0; i < testcount; i++) {
			s >> data[i].factor
			  >> data[i].units
			  >> data[i].near
			  >> data[i].far
			  >> data[i].punchthrough
			  >> data[i].punchthrough_translation
			  >> data[i].badEdge
			  >> data[i].badMiddle;
		}
	}

	void addtest(float factor, float units, float near, float far,
		     bool punchthrough, float punchthrough_translation,
		     bool badEdge, bool badMiddle) {
		if (testcount + 1 >= maxtests) return; /* ERROR! */
		data[testcount].factor       = factor;
		data[testcount].units        = units;
		data[testcount].near         = near;
		data[testcount].far          = far;
		data[testcount].punchthrough = punchthrough;
		data[testcount].punchthrough_translation
			= punchthrough_translation;
		data[testcount].badEdge      = badEdge;
		data[testcount].badMiddle    = badMiddle;
		++testcount;
	}
};


class PgosTest: public Test {
public:
	PgosTest(const char* testName, const char* filter,
		 const char* description);
	PgosTest(const char* testName, const char* filter,
		 const char* extensions, const char* description);
	virtual ~PgosTest();
	
	const char* filter;	// Drawing surface configuration filter.
	const char* extensions;	// Required extensions.
	const char* description; // Verbose description of test.

	virtual void run(Environment& env);	// Run test, save results.
	
	virtual void compare(Environment& env); // Compare two previous runs.

	// Class for a single test result.  All pgos tests have a
	// drawing surface configuration, plus other information
	// that's specific to the test.
	class Result: public Test::Result {
	public:
		DrawingSurfaceConfig* config;
		bool pass;

		POResult po;
		
		virtual void put(ostream& s) const;
		virtual bool get(istream& s);
		
		Result() { }
		virtual ~Result() { }
	};
	
	vector<Result*> results;
	
	virtual void runOne(Result& r, GLEAN::Window& w);
	virtual void compareOne(Result& oldR, Result& newR);
	virtual vector<Result*> getResults(istream& s);
	
	void logDescription();
	void logStats(Result& r, GLEAN::Environment* env);
	
}; // class PgosTest

} // namespace GLEAN

#endif // __tpgos_h__
