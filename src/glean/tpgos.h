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

#include "tbase.h"

namespace GLEAN {

#define WIN_SIZE 100

// Auxiliary struct for holding a glPolygonOffset result
class POResult: public BaseResult {
public:
	bool pass; // not logged to file
	struct PartialResult {
		float factor;
		float units;
		float near;
		float far;
		bool  punchthrough;
		float punchthrough_translation;
		bool  badEdge;
		bool  badMiddle;
	};
	vector<PartialResult> data;
	
	void putresults(ostream& s) const {
		s << data.size() << '\n';
		for (vector<PartialResult>::const_iterator p = data.begin();
		     p != data.end(); ++p)
			s << ' ' << p->factor
			  << ' ' << p->units
			  << ' ' << p->near
			  << ' ' << p->far
			  << ' ' << p->punchthrough
			  << ' ' << p->punchthrough_translation
			  << ' ' << p->badEdge
			  << ' ' << p->badMiddle
			  << '\n';
	}
	
	bool getresults(istream& s) {
		int n;
		s >> n;
		for (int i = 0; i < n; ++i) {
			PartialResult p;
			s >> p.factor
			  >> p.units
			  >> p.near
			  >> p.far
			  >> p.punchthrough
			  >> p.punchthrough_translation
			  >> p.badEdge
			  >> p.badMiddle;
			data.push_back(p);
		}
		return s.good();
	}

	void addtest(float factor, float units, float near, float far,
		     bool punchthrough, float punchthrough_translation,
		     bool badEdge, bool badMiddle) {
		PartialResult p;
		p.factor                   = factor;
		p.units                    = units;
		p.near                     = near;
		p.far                      = far;
		p.punchthrough             = punchthrough;
		p.punchthrough_translation = punchthrough_translation;
		p.badEdge                  = badEdge;
		p.badMiddle                = badMiddle;
		data.push_back(p);
	}
};


class PgosTest: public BaseTest<POResult> {
public:
	GLEAN_CLASS_WH(PgosTest, POResult, WIN_SIZE, WIN_SIZE);
}; // class PgosTest

} // namespace GLEAN

#endif // __tpgos_h__
