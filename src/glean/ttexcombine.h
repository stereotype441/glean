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


// ttexcombing.h:  Test the GL_EXT_texture_env_combine extension
// Author: Brian Paul (brianp@valinux.com)  September 2000


#ifndef __ttexcombine_h__
#define __ttexcombine_h__

#include "tbasic.h"

class DrawingSurfaceConfig;		// Forward reference.

namespace GLEAN {

#define MAX_TEX_UNITS 4

class TexCombineTest: public BasicTest {
    public:
	TexCombineTest(const char* testName, const char* filter,
		const char* description);

	virtual void runOne(Result& r);

    private:
	// Our model of GL machine state
	struct glmachine {
		GLenum COMBINE_RGB[MAX_TEX_UNITS];
		GLenum COMBINE_ALPHA[MAX_TEX_UNITS];
		GLenum SOURCE0_RGB[MAX_TEX_UNITS];
		GLenum SOURCE1_RGB[MAX_TEX_UNITS];
		GLenum SOURCE2_RGB[MAX_TEX_UNITS];
		GLenum SOURCE0_ALPHA[MAX_TEX_UNITS];
		GLenum SOURCE1_ALPHA[MAX_TEX_UNITS];
		GLenum SOURCE2_ALPHA[MAX_TEX_UNITS];
		GLenum OPERAND0_RGB[MAX_TEX_UNITS];
		GLenum OPERAND1_RGB[MAX_TEX_UNITS];
		GLenum OPERAND2_RGB[MAX_TEX_UNITS];
		GLenum OPERAND0_ALPHA[MAX_TEX_UNITS];
		GLenum OPERAND1_ALPHA[MAX_TEX_UNITS];
		GLenum OPERAND2_ALPHA[MAX_TEX_UNITS];
		GLfloat RGB_SCALE[MAX_TEX_UNITS];
		GLfloat ALPHA_SCALE[MAX_TEX_UNITS];
		GLfloat FragColor[4];                  // fragment color
		GLfloat EnvColor[MAX_TEX_UNITS][4];    // texture env color
		GLfloat TexColor[MAX_TEX_UNITS][4];    // texture image color
		int NumTexUnits;
	};

	// describes possible state combinations
	struct test_param {
		GLenum target;
		GLenum validValues[6];
	};

	glmachine Machine;
	static test_param ReplaceParams[];
	static test_param ModulateParams[];
	static test_param AddParams[];
	static test_param AddSignedParams[];
	static test_param InterpolateParams[];
	static test_param MultitexParams[];

	void ResetMachine(glmachine &machine);
	void ComputeTexCombine(const glmachine &machine, int texUnit,
		const GLfloat prevColor[4], GLfloat result[4]) const;
	void PrintMachineState(const glmachine &machine) const;
	void ReportFailure(const glmachine &machine, const GLfloat expected[4],
		const GLfloat rendered[4], Result &r);
	void TexEnv(glmachine &machine, int texUnit, GLenum target,
		GLenum value);
	void SetupTestEnv(glmachine &machine, int texUnit, int testNum,
		const test_param testParams[]);
	void SetupColors(struct glmachine &machine);
	int CountTestCombinations(const test_param testParams[]) const;
	bool RunSingleTextureTest(glmachine &machine,
		const test_param testParams[], Result &r);
        int CountMultiTextureTestCombinations(const glmachine &machine) const;
	bool RunMultiTextureTest(glmachine &machine, Result &r);

}; // class TexCombineTest

} // namespace GLEAN

#endif // __ttexcombine_h__