// BEGIN_COPYRIGHT -*- glean -*-
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

#include "tbase.h"

class DrawingSurfaceConfig;		// Forward reference.

namespace GLEAN {

class BasicPerfResult: public BaseResult {
public:
	bool   pass;
	double timeAvg, timeLow, timeHigh;
	
	BasicPerfResult() {
		timeAvg = timeLow = timeHigh = 0.0;
	}

	void putresults(ostream& s) const {
		s << pass
		  << ' ' << timeAvg
		  << ' ' << timeLow
		  << ' ' << timeHigh
		  << '\n';
	}

	bool getresults(istream& s) {
		s >> pass >> timeAvg >> timeLow >> timeHigh;
		return s.good();
	}
};

class BasicPerfTest: public BaseTest<BasicPerfResult> {
public:
	GLEAN_CLASS(BasicPerfTest, BasicPerfResult);
	void logStats(BasicPerfResult& r);
}; // class BasicPerfTest

} // namespace GLEAN

#endif // __tbasicperf_h__
