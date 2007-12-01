// BEGIN_COPYRIGHT -*- glean -*-
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

#include "tbase.h"

namespace GLEAN {

#define drawingSize 32          // We will check each pair of blend factors
				// for each pixel in a square image of this
				// dimension, so if you make it too large,
				// the tests may take quite a while to run.
#define windowSize (drawingSize + 2)

class BlendFuncResult: public BaseResult {
public:
	bool pass;	// not written to log file

	struct PartialResult {
		GLenum srcRGB, srcA;	// Source blend factor.
		GLenum dstRGB, dstA;	// Destination blend factor.
		GLenum opRGB, opA;	// Operator (add, sub, min, max)
		GLfloat constColor[4];
		float rbErr;	// Max readback error, in bits.
		float blErr;	// Max blend error, in bits.
	};
	vector<PartialResult> results;
	
	virtual void putresults(ostream& s) const;
	virtual bool getresults(istream& s);
};

class BlendFuncTest: public BaseTest<BlendFuncResult> {
public:
	GLEAN_CLASS_WH(BlendFuncTest, BlendFuncResult,
		       windowSize, windowSize);

private:
	struct runFactorsResult {
		float readbackErrorBits;
		float blendErrorBits;
	};

	runFactorsResult runFactors(GLenum srcFactorRGB, GLenum srcFactorA,
				    GLenum dstFactorRGB, GLenum dstFactorA,
				    GLenum opRGB, GLenum opA,
				    const GLfloat constantColor[4],
				    GLEAN::DrawingSurfaceConfig& config,
				    GLEAN::Environment& env);

	bool runCombo(BlendFuncResult& r, Window& w,
		      BlendFuncResult::PartialResult p,
		      GLEAN::Environment& env);

	bool equalMode(const BlendFuncResult::PartialResult &r1,
		       const BlendFuncResult::PartialResult &r2) const;

	void printMode(const BlendFuncResult::PartialResult &r) const;

	bool haveSepFunc;
	bool haveBlendEquation;
	bool haveBlendEquationSep;
	bool haveBlendColor;

}; // class BlendFuncTest

} // namespace GLEAN

#endif // __tblend_h__
