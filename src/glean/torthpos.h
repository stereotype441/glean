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

// torthpos.h:  Test positioning of primitives in orthographic projection.
// Apps that require precise 2D rasterization depend on these semantics.

#ifndef __torthpos_h__
#define __torthpos_h__

#include "tbase.h"

namespace {
const int drawingSize = 256;
const int windowSize = drawingSize + 2;	// one-pixel border on all sides
}

namespace GLEAN {

// Auxiliary struct for holding a test result:
class OPResult: public BaseResult {
public:
	bool hasGaps;		// true if gaps between prims were detected
	bool hasOverlaps;	// true if overlaps were detected
	bool hasBadEdges;	// true if edge-conditions were incorrect
	bool pass;		// not written to log file (why?)

	OPResult() {
		pass = false;
		hasGaps = hasOverlaps = hasBadEdges = false;
	}
	
	void putresults(ostream& s) const {
		s << hasGaps
		  << ' ' << hasOverlaps
		  << ' ' << hasBadEdges
		  << '\n';
	}
	
	bool getresults(istream& s) {
		s >> hasGaps >> hasOverlaps >> hasBadEdges;
		return s.good();
	}
};

class OrthoPosPoints: public BaseTest<OPResult> {
public:
	static const char * const title = "immediate-mode points";
	GLEAN_CLASS_WH(OrthoPosPoints, OPResult, windowSize, windowSize);
}; // class OrthoPosPoints

class OrthoPosVLines: public BaseTest<OPResult> {
public:
	static const char * const title = "immediate-mode vertical lines";
	GLEAN_CLASS_WH(OrthoPosVLines, OPResult, windowSize, windowSize);
}; // class OrthoVLines

class OrthoPosHLines: public BaseTest<OPResult> {
public:
	static const char * const title = "immeditate-mode horizontal lines";
	GLEAN_CLASS_WH(OrthoPosHLines, OPResult, windowSize, windowSize);
}; // class OrthoPosHLines

class OrthoPosTinyQuads: public BaseTest<OPResult> {
public:
	static const char * const title = "immediate-mode 1x1 quads";
	GLEAN_CLASS_WH(OrthoPosTinyQuads, OPResult, windowSize, windowSize);
}; // class OrthoPosTinyQuads

class OrthoPosRandRects: public BaseTest<OPResult> {
public:
	static char * const title = "immediate-mode axis-aligned rectangles";
	GLEAN_CLASS_WH(OrthoPosRandRects, OPResult, windowSize, windowSize);
}; // class OrthoPosRandRects

class OrthoPosRandTris: public BaseTest<OPResult> {
public:
	static const char * const title ="immediate-mode triangles";
	GLEAN_CLASS_WH(OrthoPosRandTris, OPResult, windowSize, windowSize);
}; // class OrthoPosRandTris

} // namespace GLEAN

#endif // __torthpos_h__
