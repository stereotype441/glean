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




// tblend.h:  Test blending functions.

#ifndef __tblend_h__
#define __tblend_h__

#include "test.h"

class DrawingSurfaceConfig;		// Forward reference.
class GLEAN::Window;

namespace GLEAN {

class BlendFuncTest: public Test {
    public:
	BlendFuncTest(const char* testName, const char* filter,
		const char* description);
	virtual ~BlendFuncTest();

	const char* filter;		// Drawing surface configuration filter.
	const char* description;	// Verbose description of test.

	virtual void run(Environment& env);	// Run test, save results.

	virtual void compare(Environment& env);
					// Compare two previous runs.

	// Class for a single test result.  All basic tests have a
	// drawing surface configuration, plus other information
	// that's specific to the test.
	class Result: public Test::Result {
	    public:
		DrawingSurfaceConfig* config;
		struct PartialResult {
			GLenum src;	// Source blend factor.
			GLenum dst;	// Destination blend factor.
			float rbErr;	// Max readback error, in bits.
			float blErr;	// Max blend error, in bits.
		};
		vector<PartialResult> results;

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

}; // class BlendFuncTest

} // namespace GLEAN

#endif // __tblend_h__
