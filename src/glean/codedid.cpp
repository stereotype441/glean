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



// codedid.h:  tool to map integer IDs into colors, and vice-versa

#include "codedid.h"
#include <algorithm>

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// RGBCodedID: Create an object that maps integer identification numbers
//	to RGB triples, and vice-versa
///////////////////////////////////////////////////////////////////////////////
RGBCodedID::RGBCodedID(int r, int g, int b) {
	rBits = min(8, r);	// 8 because we use GLubyte color values
	gBits = min(8, g);
	bBits = min(8, b);
	nsRBits = r - rBits;
	nsGBits = g - gBits;
	nsBBits = b - bBits;
	rMask = (1 << rBits) - 1;
	gMask = (1 << gBits) - 1;
	bMask = (1 << bBits) - 1;
} // RGBCodedID::RGBCodedID

RGBCodedID::~RGBCodedID() {
} // RGBCodedID::~RGBCodedID

///////////////////////////////////////////////////////////////////////////////
// maxID: Return the maximum allowable integer ID number
///////////////////////////////////////////////////////////////////////////////
int
RGBCodedID::maxID() const {
	return (1 << (rBits + gBits + bBits)) - 1;
} // RGBCodedID::maxID

///////////////////////////////////////////////////////////////////////////////
// toRGB: Convert integer ID number to RGB triple
///////////////////////////////////////////////////////////////////////////////
void
RGBCodedID::toRGB(int id, GLubyte& r, GLubyte& g, GLubyte& b) const {
	b = (id & bMask) << nsBBits;
	id >>= bBits;
	g = (id & gMask) << nsGBits;
	id >>= gBits;
	r = (id & rMask) << nsRBits;
} // RGBCodedID::toRGB

///////////////////////////////////////////////////////////////////////////////
// toID: Convert RGB triple to integer ID number
///////////////////////////////////////////////////////////////////////////////
int
RGBCodedID::toID(GLubyte r, GLubyte g, GLubyte b) const {
	int id = 0;
	id |= (r >> nsRBits) & rMask;
	id <<= gBits;
	id |= (g >> nsGBits) & gMask;
	id <<= bBits;
	id |= (b >> nsBBits) & bMask;
	return id;
} // RGBCodedID::toID

} // namespace GLEAN
