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

// tfpexceptions.h:  Test for floating point exceptions caused by
// infinity, Nan, denormalized numbers, divide by zero, etc.
// Brian Paul  9 November 2005

#ifndef __tfpexceptions_h__
#define __tfpexceptions_h__

#include "tbasic.h"

namespace GLEAN {


#define windowSize 100


class FPExceptionsTest: public BasicTest
{
public:
	FPExceptionsTest(const char *testName,
                         const char *filter,
                         const char *extensions,
                         const char *description);

	virtual void runOne(BasicResult& r, Window& w);
	virtual void logOne(BasicResult& r);

private:
        enum Mode {
                MODE_INFINITY,
                MODE_NAN,
                MODE_DIVZERO,
                MODE_DENORM,
                MODE_OVERFLOW
        };

	void enableExceptions(bool enable);

        bool testVertices(Mode m);
        bool testTransformation(Mode m);
        bool testClipping(Mode m);
        bool testOverflow(void);

        void reportPassFail(bool pass, const char *msg) const;
        void setup(void);
};


} // namespace GLEAN

#endif // __tfpexceptions_h__
