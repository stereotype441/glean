// BEGIN_COPYRIGHT -*- linux-c -*-
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




// tbasicperf.h:  Example class for basic performance tests

// This class provides a framework for performance tests that should
// be portable (in the sense that they don't contain OS- or
// window-system-specific code).

// Each basic perf test includes a drawing surface filter string.  The
// test will be run on all the drawing surface configurations that are
// selected by the filter, and one result structure will be generated
// for each such configuration.
//
// Each basic perf test may also include an extension filter string.
// The test will only be run on contexts that support all the listed
// extensions.  Extension names in the string may be separated with non
// alphanumerics; whitespace and commas are used by convention.

// When comparing two runs, the drawing surface configurations are
// used to select plausible matches among the results.

// To customize this to benchmark a particular function, see
// tbasicperf.cpp.


#ifndef __tbasicperf_h__
#define __tbasicperf_h__

#include "test.h"

class DrawingSurfaceConfig;		// Forward reference.

namespace GLEAN {

struct TimeResult {
	double timeAvg, timeLow, timeHigh;
	
	TimeResult() {
		timeAvg = timeLow = timeHigh = 0.0;
	}

	void put(ostream& s) const {
		s << timeAvg << ' ' << timeLow << ' ' << timeHigh << '\n';
	}

	void get(istream& s) {
		s >> timeAvg >> timeLow >> timeHigh;
	}
};

class BasicPerfTest: public Test {
public:
	BasicPerfTest(const char* testName, const char* filter,
		      const char* description);
	BasicPerfTest(const char* testName, const char* filter,
		      const char* extensions, const char* description);
	virtual ~BasicPerfTest();
	
	const char* filter;	 // Drawing surface configuration filter.
	const char* extensions;	 // Required extensions.
	const char* description; // Verbose description of test.

	virtual void run(Environment& env);	// Run test, save results.

	virtual void compare(Environment& env); // Compare two previous runs.

	// Class for a single test result.  All basic tests have a
	// drawing surface configuration, plus other information
	// that's specific to the test.
	class Result: public Test::Result {
	public:
		DrawingSurfaceConfig* config;

		bool       pass;
		TimeResult tr;

		virtual void put(ostream& s) const;
		virtual bool get(istream& s);

		Result() { }
		virtual ~Result() { }
	};

	vector<Result*> results;
	
	virtual void runOne(Result& r, Window &w);
	virtual void compareOne(Result& oldR, Result& newR);
	virtual vector<Result*> getResults(istream& s);

	void logDescription();
	void logStats(Result& r, GLEAN::Environment* env);

}; // class BasicPerfTest

} // namespace GLEAN

#endif // __tbasicperf_h__
