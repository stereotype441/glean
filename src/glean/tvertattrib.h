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


#ifndef __tvertattrib_h__
#define __tvertattrib_h__

#include "tbasic.h"

namespace GLEAN {

class VertAttribTest: public BasicTest
{
public:
	VertAttribTest(const char* testName, const char* filter,
		       const char* description):
		BasicTest(testName, filter, description)
	{
	}

	virtual void runOne(BasicResult& r, Window& w);
	virtual void logOne(BasicResult& r);

private:
	enum Aliasing {
		REQUIRED,
		DISALLOWED,
		OPTIONAL
	};

	void FailMessage(BasicResult &r, const char *msg, const char *ext) const;
	bool TestAttribs(BasicResult &r,
			 const char *extensionName,
			 PFNGLVERTEXATTRIB4FVARBPROC attrib4fv,
			 PFNGLGETVERTEXATTRIBFVARBPROC getAttribfv,
			 Aliasing aliasing,
			 int numAttribs);
	bool TestNVfuncs(BasicResult &r);
	bool TestARBfuncs(BasicResult &r, bool shader);
	bool Test20funcs(BasicResult &r);
};

} // namespace GLEAN

#endif // __tvertattrib_h__
