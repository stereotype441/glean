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




// torthpos.h:  Test positioning of primitives in orthographic projection.
// Apps that require precise 2D rasterization depend on these semantics.

#ifndef __torthpos_h__
#define __torthpos_h__

#include "test.h"

class DrawingSurfaceConfig;		// Forward reference.
class GLEAN::Window;

namespace GLEAN {

// Auxiliary struct for holding a test result:
struct OPResult {
	bool hasGaps;		// true if gaps between prims were detected
	bool hasOverlaps;	// true if overlaps were detected
	bool hasBadEdges;	// true if edge-conditions were incorrect

	OPResult() {
		hasGaps = hasOverlaps = hasBadEdges = false;
	}
	void put(ostream& s) const {
		s << hasGaps
		   << ' ' << hasOverlaps
		   << ' ' << hasBadEdges
		   << '\n';
	}
	void get(istream& s) {
		s >> hasGaps >> hasOverlaps >> hasBadEdges;
	}
};

class OrthoPosPoints: public Test {
    public:
	OrthoPosPoints(const char* testName, const char* filter,
		const char* description);
	virtual ~OrthoPosPoints();

	const char* filter;		// Drawing surface configuration filter.
	const char* description;	// Verbose description of test.

	virtual void run(Environment& env);	// Run test, save results.

	virtual void compare(Environment& env);	// Compare two previous runs.

	// Class for a single test result.  All basic tests have a
	// drawing surface configuration, plus other information
	// that's specific to the test.
	class Result: public Test::Result {
	    public:
		DrawingSurfaceConfig* config;

		OPResult im;		// immediate-mode
		// VPResult dl;		// display-listed
		// VPResult da;		// DrawArrays
		// VPResult lda;	// Locked DrawArrays
		// VPResult de;		// DrawElements
		// VPResult lde;	// Locked DrawElements

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

}; // class OrthoPosPoints

class OrthoPosVLines: public Test {
    public:
	OrthoPosVLines(const char* testName, const char* filter,
		const char* description);
	virtual ~OrthoPosVLines();

	const char* filter;		// Drawing surface configuration filter.
	const char* description;	// Verbose description of test.

	virtual void run(Environment& env);	// Run test, save results.

	virtual void compare(Environment& env);	// Compare two previous runs.

	// Class for a single test result.  All basic tests have a
	// drawing surface configuration, plus other information
	// that's specific to the test.
	class Result: public Test::Result {
	    public:
		DrawingSurfaceConfig* config;

		OPResult im;		// immediate-mode

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

}; // class OrthoPosVLines

class OrthoPosHLines: public Test {
    public:
	OrthoPosHLines(const char* testName, const char* filter,
		const char* description);
	virtual ~OrthoPosHLines();

	const char* filter;		// Drawing surface configuration filter.
	const char* description;	// Verbose description of test.

	virtual void run(Environment& env);	// Run test, save results.

	virtual void compare(Environment& env);	// Compare two previous runs.

	// Class for a single test result.  All basic tests have a
	// drawing surface configuration, plus other information
	// that's specific to the test.
	class Result: public Test::Result {
	    public:
		DrawingSurfaceConfig* config;

		OPResult im;		// immediate-mode

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

}; // class OrthoPosHLines

class OrthoPosTinyQuads: public Test {
    public:
	OrthoPosTinyQuads(const char* testName, const char* filter,
		const char* description);
	virtual ~OrthoPosTinyQuads();

	const char* filter;		// Drawing surface configuration filter.
	const char* description;	// Verbose description of test.

	virtual void run(Environment& env);	// Run test, save results.

	virtual void compare(Environment& env);	// Compare two previous runs.

	// Class for a single test result.  All basic tests have a
	// drawing surface configuration, plus other information
	// that's specific to the test.
	class Result: public Test::Result {
	    public:
		DrawingSurfaceConfig* config;

		OPResult im;		// immediate-mode

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

}; // class OrthoPosTinyQuads

class OrthoPosRandRects: public Test {
    public:
	OrthoPosRandRects(const char* testName, const char* filter,
		const char* description);
	virtual ~OrthoPosRandRects();

	const char* filter;		// Drawing surface configuration filter.
	const char* description;	// Verbose description of test.

	virtual void run(Environment& env);	// Run test, save results.

	virtual void compare(Environment& env);	// Compare two previous runs.

	// Class for a single test result.  All basic tests have a
	// drawing surface configuration, plus other information
	// that's specific to the test.
	class Result: public Test::Result {
	    public:
		DrawingSurfaceConfig* config;

		OPResult im;		// immediate-mode

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

}; // class OrthoPosRandRects

class OrthoPosRandTris: public Test {
    public:
	OrthoPosRandTris(const char* testName, const char* filter,
		const char* description);
	virtual ~OrthoPosRandTris();

	const char* filter;		// Drawing surface configuration filter.
	const char* description;	// Verbose description of test.

	virtual void run(Environment& env);	// Run test, save results.

	virtual void compare(Environment& env);	// Compare two previous runs.

	// Class for a single test result.  All basic tests have a
	// drawing surface configuration, plus other information
	// that's specific to the test.
	class Result: public Test::Result {
	    public:
		DrawingSurfaceConfig* config;

		OPResult im;		// immediate-mode

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

}; // class OrthoPosRandTris

} // namespace GLEAN

#endif // __torthpos_h__
