// BEGIN_COPYRIGHT -*- linux-c -*-
// 
// Copyright (C) 1999,2000  Allen Akin   All Rights Reserved.
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




// tbasic.h:  Example class for basic tests

// This class (derived from BaseTest) provides a framework for a large set
// of correctness tests that should be portable (in the sense that
// they don't contain OS- or window-system-specific code).

// Each basic test includes a drawing surface filter string.  The test
// will be run on all the drawing surface configurations that are
// selected by the filter, and one result structure will be generated
// for each such configuration.
//
// Each basic test may also include an extension filter string.  The test
// will only be run on contexts that support all the listed extensions.
// Extension names in the string may be separated with non alphanumerics;
// whitespace and commas are used by convention.

// When comparing two runs, the drawing surface configurations are
// used to select plausible matches among the results.


#ifndef __tbasic_h__
#define __tbasic_h__

#include "tbase.h"

namespace GLEAN {

class BasicResult: public BaseResult {
public:
	bool pass;

	void putresults(ostream& s) const { s << pass << '\n'; }
	bool getresults(istream& s) { s >> pass; return s.good(); }
};

#if 1
class BasicTest: public BaseTest<BasicResult> {
public:
	BasicTest(const char* aName, const char* aFilter,
		  const char* aDescription):
		BaseTest(aName, aFilter, aDescription) {
	}
	BasicTest(const char* aName, const char* aFilter,
		  const char* anExtensionList,
		  const char* aDescription):
		BaseTest(aName, aFilter, anExtensionList, aDescription) {
	}
	virtual ~BasicTest() {}

#if 0
	virtual void runOne(BasicResult& r);
	virtual void compareOne(BasicResult& oldR, BasicResult& newR);
	virtual void logOne(BasicResult& r);
#endif
};
#else
BaseTest<BasicResult> BasicTest;
#endif

} // namespace GLEAN

#endif // __tbasic_h__
