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


// ttexgen.h:  Basic test of GL texture coordinate generation.
// Author: Brian Sharp (brian@maniacal.org)  December 2000


#ifndef __ttexgen_h__
#define __ttexgen_h__

#include "tbasic.h"

class DrawingSurfaceConfig;		// Forward reference.

namespace GLEAN {

class TexgenTest: public BasicTest {
    public:
	TexgenTest(const char* testName, const char* filter,
		const char* description);
	virtual ~TexgenTest();

	virtual void runOne(Result& r);

    private:
	void FailMessage(Result &r, const std::string& texgenMode, const std::string& colorMismatch) const;
	bool compareColors(GLfloat* color0, GLfloat* color1, std::string& failureInfo) const;
        bool verifyCheckers(GLfloat* pixels, GLfloat* upperLeftColor, GLfloat* upperRightColor, std::string& failureInfo) const;

}; // class TexgenTest

} // namespace GLEAN

#endif // __ttexgen_h__
