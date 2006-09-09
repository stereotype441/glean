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

// Brian Paul  September 2006

#ifndef __tpixelformats_h__
#define __tpixelformats_h__

#include "tbase.h"

namespace GLEAN {

#define windowSize 100

class PixelFormatsResult: public BaseResult
{
public:
	bool pass;
	int numPassed, numFailed;

	virtual void putresults(ostream& s) const;
	virtual bool getresults(istream& s);
};


class PixelFormatsTest: public BaseTest<PixelFormatsResult>
{
public:
	GLEAN_CLASS_WH(PixelFormatsTest, PixelFormatsResult,
		       windowSize, windowSize);

private:
	int alphaBits;
	bool haveHalfFloat;
	bool haveABGR;
	bool haveSRGB;

	bool CheckError(const char *where) const;

	bool CompatibleFormatAndType(GLenum format, GLenum datatype) const;

	bool SupportedIntFormat(GLint intFormat) const ;

	bool DrawImage(int width, int height,
				   GLenum format, GLenum type, GLint intFormat,
				   const GLubyte *image);

	bool CheckRendering(int width, int height, int color, GLenum format, GLint intFormat) const;

	bool TestCombination(GLenum format, GLenum type, GLint intFormat);

	void setup(void);
};

} // namespace GLEAN

#endif // __tpixelformats_h__

