// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 2001  Allen Akin   All Rights Reserved.
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


// treadpix.h:  ReadPixels tests.

#ifndef __treadpix_h__
#define __treadpix_h__

#include "tbase.h"

namespace GLEAN {

class ReadPixSanityResult: public BaseResult {
public:
	bool pass;

	bool passRGBA;
	int xRGBA;
	int yRGBA;
	float errRGBA;
	GLfloat expectedRGBA[4];
	GLfloat actualRGBA[4];

	bool passDepth;
	int xDepth;
	int yDepth;
	float errDepth;
	GLdouble expectedDepth;
	GLdouble actualDepth;

	bool passStencil;
	int xStencil;
	int yStencil;
	GLuint expectedStencil;
	GLuint actualStencil;

	bool passIndex;
	int xIndex;
	int yIndex;
	GLuint expectedIndex;
	GLuint actualIndex;

	ReadPixSanityResult() {
		pass = true;

		passRGBA = true;
		xRGBA = yRGBA = 0;
		errRGBA = 0.0;
		expectedRGBA[0] = expectedRGBA[1] = expectedRGBA[2]
			= expectedRGBA[3] = 0.0;
		actualRGBA[0] = actualRGBA[1] = actualRGBA[2]
			= actualRGBA[3] = 0.0;
			
		passDepth = true;
		xDepth = yDepth = 0;
		errDepth = 0.0;
		expectedDepth = 0.0;
		actualDepth = 0.0;

		passStencil = true;
		xStencil = yStencil = 0;
		expectedStencil = 0;
		actualStencil = 0;

		passIndex = true;
		xIndex = yIndex = 0;
		expectedIndex = 0;
		actualIndex = 0;
	}
	
	void putresults(ostream& s) const {
		s
		  << pass << '\n'

		  << passRGBA << '\n'
		  << xRGBA << ' ' << yRGBA << '\n'
		  << errRGBA << '\n'
		  << expectedRGBA[0] << ' ' << expectedRGBA[1] << ' '
		  	<< expectedRGBA[2] << ' ' << expectedRGBA[3] << '\n'
		  << actualRGBA[0] << ' ' << actualRGBA[1] << ' '
		  	<< actualRGBA[2] << ' ' << actualRGBA[3] << '\n'

		  << passDepth << '\n'
		  << xDepth << ' ' << yDepth << '\n'
		  << errDepth << '\n'
		  << setprecision(16)
		  << expectedDepth << '\n'
		  << actualDepth << '\n'

		  << passStencil << '\n'
		  << xStencil << ' ' << yStencil << '\n'
		  << expectedStencil << '\n'
		  << actualStencil << '\n'

		  << passIndex << '\n'
		  << xIndex << ' ' << yIndex << '\n'
		  << expectedIndex << '\n'
		  << actualIndex << '\n'
		  ;
	}
	
	bool getresults(istream& s) {
		s >> pass

		  >> passRGBA
		  >> xRGBA >> yRGBA
		  >> errRGBA
		  >> expectedRGBA[0] >> expectedRGBA[1] >> expectedRGBA[2]
		  	>> expectedRGBA[3]
		  >> actualRGBA[0] >> actualRGBA[1] >> actualRGBA[2]
		  	>> actualRGBA[3]

		  >> passDepth
		  >> xDepth >> yDepth
		  >> errDepth
		  >> expectedDepth
		  >> actualDepth

		  >> passStencil
		  >> xStencil >> yStencil
		  >> expectedStencil
		  >> actualStencil

		  >> passIndex
		  >> xIndex >> yIndex
		  >> expectedIndex
		  >> actualIndex
		  ;
		return s.good();
	}
};

#define READPIX_SANITY_WIN_SIZE 32

class ReadPixSanityTest: public BaseTest<ReadPixSanityResult> {
public:
	GLEAN_CLASS_WH(ReadPixSanityTest, ReadPixSanityResult,
		READPIX_SANITY_WIN_SIZE, READPIX_SANITY_WIN_SIZE);

	void summarize(char* label, bool oldPass, bool newPass);
}; // class ReadPixSanityTest

} // namespace GLEAN

#endif // __treadpix_h__
