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


// ttexcombine.cpp:  Test the GL_EXT_texture_env_combine extension
// Author: Brian Paul (brianp@valinux.com)  September 2000
//
// GL_EXT_texture_env_dot3 extension test
// Author: Gareth Hughes (gareth@valinux.com)  January 2001
//
// The challenge with testing this extension is dealing with combinatorial
// explosion.  There are 16 state variables in this extension:
//
// GL_COMBINE_RGB_EXT which has 5 possible values
// GL_COMBINE_ALPHA_EXT which has 5 possible values
// GL_SOURCE0_RGB_EXT which has 4 possible values
// GL_SOURCE1_RGB_EXT which has 4 possible values
// GL_SOURCE2_RGB_EXT which has 4 possible values
// GL_SOURCE0_ALPHA_EXT which has 4 possible values
// GL_SOURCE1_ALPHA_EXT which has 4 possible values
// GL_SOURCE2_ALPHA_EXT which has 4 possible values
// GL_OPERAND0_RGB_EXT which has 4 possible values
// GL_OPERAND1_RGB_EXT which has 4 possible values
// GL_OPERAND2_RGB_EXT which has 2 possible values
// GL_OPERAND0_ALPHA_EXT which has 2 possible values
// GL_OPERAND1_ALPHA_EXT which has 2 possible values
// GL_OPERAND2_ALPHA_EXT which has 1 possible value
// GL_RGB_SCALE_EXT which has 3 possible values
// GL_ALPHA_SCALE which has 3 possible values
//
// The product of those values is 117,964,800.  And that's just for one
// texture unit!  If we wanted to fully exercise N texture units we'd
// need to run 117,964,800 ^ N tests!  Ideally we'd also like to test
// with a number of different fragment, texenv and texture colors.
// Clearly we can't test everything.
// 
// So, we've partitioned the combination space into subsets defined
// by the ReplaceParams[], AddParams[], InterpolateParams[], etc arrays.
// For multitexture, we do an even more limited set of tests:  testing
// all permutations of the 5 combine modes on all texture units.
//
// In the future we might look at programs that use the combine
// extension to see which mode combination are important to them and
// put them into this test.
//

#include "ttexcombine.h"
#include <stdio.h>
#include <cmath>

#define CLAMP(VAL, MIN, MAX)	\
	((VAL) < (MIN) ? (MIN) : ((VAL) > (MAX) ? (MAX) : (VAL)))

#define COPY3(DST, SRC)		\
{				\
	(DST)[0] = (SRC)[0];	\
	(DST)[1] = (SRC)[1];	\
	(DST)[2] = (SRC)[2];	\
}

#define COPY4(DST, SRC)		\
{				\
	(DST)[0] = (SRC)[0];	\
	(DST)[1] = (SRC)[1];	\
	(DST)[2] = (SRC)[2];	\
	(DST)[3] = (SRC)[3];	\
}


namespace GLEAN {

//
// These objects define the space of tex-env combinations that we exercise.
// Each array element is { state-var, { list of possible values, 0 } }.
//

TexCombineTest::test_param TexCombineTest::ReplaceParams[] = {
	{ GL_COMBINE_RGB_EXT, { GL_REPLACE, 0 } },
	{ GL_COMBINE_ALPHA_EXT, { GL_REPLACE, 0 } },
	{ GL_SOURCE0_RGB_EXT, { GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0 } },
	{ GL_SOURCE0_ALPHA_EXT, { GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0 } },
	{ GL_OPERAND0_RGB_EXT, { GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_OPERAND0_ALPHA_EXT, { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_RGB_SCALE_EXT, { 1, 2, 4, 0 } },
	{ GL_ALPHA_SCALE, { 1, 2, 4, 0 } },
	{ 0, { 0, 0, 0, 0, 0 } }
};

TexCombineTest::test_param TexCombineTest::AddParams[] = {
	{ GL_COMBINE_RGB_EXT, { GL_ADD, 0 } },
	{ GL_COMBINE_ALPHA_EXT, { GL_ADD, 0 } },
	{ GL_SOURCE0_RGB_EXT, { GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0 } },
	{ GL_SOURCE1_RGB_EXT, { GL_TEXTURE, GL_CONSTANT_EXT, GL_PREVIOUS_EXT, 0 } },
	{ GL_SOURCE0_ALPHA_EXT, { GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0 } },
	{ GL_SOURCE1_ALPHA_EXT, { GL_TEXTURE, GL_CONSTANT_EXT, GL_PREVIOUS_EXT, 0 } },
	{ GL_OPERAND0_RGB_EXT, { GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_OPERAND1_RGB_EXT, { GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_OPERAND0_ALPHA_EXT, { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_OPERAND1_ALPHA_EXT, { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_RGB_SCALE_EXT, { 1, 2, 4, 0 } },
	{ GL_ALPHA_SCALE, { 1, 2, 4, 0 } },
	{ 0, { 0, 0, 0, 0, 0 } }
};

TexCombineTest::test_param TexCombineTest::ModulateParams[] = {
	{ GL_COMBINE_RGB_EXT, { GL_MODULATE, 0 } },
	{ GL_COMBINE_ALPHA_EXT, { GL_MODULATE, 0 } },
	{ GL_SOURCE0_RGB_EXT, { GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, 0 } },
	{ GL_SOURCE1_RGB_EXT, { GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0 } },
	{ GL_SOURCE0_ALPHA_EXT, { GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, 0 } },
	{ GL_SOURCE1_ALPHA_EXT, { GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0 } },
	{ GL_OPERAND0_RGB_EXT, { GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_OPERAND1_RGB_EXT, { GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_OPERAND0_ALPHA_EXT, { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_OPERAND1_ALPHA_EXT, { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_RGB_SCALE_EXT, { 1, 2, 4, 0 } },
	{ GL_ALPHA_SCALE, { 1, 2, 4, 0 } },
	{ 0, { 0, 0, 0, 0, 0 } }
};

TexCombineTest::test_param TexCombineTest::AddSignedParams[] = {
	{ GL_COMBINE_RGB_EXT, { GL_ADD_SIGNED_EXT, 0 } },
	{ GL_COMBINE_ALPHA_EXT, { GL_ADD_SIGNED_EXT, 0 } },
	{ GL_SOURCE0_RGB_EXT, { GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, 0 } },
	{ GL_SOURCE1_RGB_EXT, { GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0 } },
	{ GL_SOURCE0_ALPHA_EXT, { GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, 0 } },
	{ GL_SOURCE1_ALPHA_EXT, { GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0 } },
	{ GL_OPERAND0_RGB_EXT, { GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_OPERAND1_RGB_EXT, { GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_OPERAND0_ALPHA_EXT, { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_OPERAND1_ALPHA_EXT, { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_RGB_SCALE_EXT, { 1, 2, 4, 0 } },
	{ GL_ALPHA_SCALE, { 1, 2, 4, 0 } },
	{ 0, { 0, 0, 0, 0, 0 } }
};

TexCombineTest::test_param TexCombineTest::InterpolateParams[] = {
	{ GL_COMBINE_RGB_EXT, { GL_INTERPOLATE_EXT, 0 } },
	{ GL_COMBINE_ALPHA_EXT, { GL_INTERPOLATE_EXT, 0 } },
	{ GL_SOURCE0_RGB_EXT, { GL_TEXTURE, GL_PRIMARY_COLOR_EXT, 0 } },
	{ GL_SOURCE1_RGB_EXT, { GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0 } },
	{ GL_SOURCE2_RGB_EXT, { GL_TEXTURE, GL_PRIMARY_COLOR_EXT, 0 } },
	{ GL_SOURCE0_ALPHA_EXT, { GL_TEXTURE, GL_PRIMARY_COLOR_EXT, 0 } },
	{ GL_SOURCE1_ALPHA_EXT, { GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0 } },
	{ GL_SOURCE2_ALPHA_EXT, { GL_TEXTURE, GL_PRIMARY_COLOR_EXT, 0 } },
	{ GL_OPERAND0_RGB_EXT, { GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_OPERAND1_RGB_EXT, { GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_OPERAND2_RGB_EXT, { GL_SRC_ALPHA, 0 } },
	{ GL_OPERAND0_ALPHA_EXT, { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_OPERAND1_ALPHA_EXT, { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_OPERAND2_ALPHA_EXT, { GL_SRC_ALPHA, 0 } },
	{ GL_RGB_SCALE_EXT, { 1, 4, 0 } },
	{ GL_ALPHA_SCALE, { 1, 2, 0 } },
	{ 0, { 0, 0, 0, 0, 0 } }
};

TexCombineTest::test_param TexCombineTest::Dot3RGBParams[] = {
	{ GL_COMBINE_RGB_EXT, { GL_DOT3_RGB_EXT, 0 } },
	{ GL_COMBINE_ALPHA_EXT, { GL_MODULATE, 0 } },
	{ GL_SOURCE0_RGB_EXT, { GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, 0 } },
	{ GL_SOURCE1_RGB_EXT, { GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0 } },
	{ GL_SOURCE0_ALPHA_EXT, { GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, 0 } },
	{ GL_SOURCE1_ALPHA_EXT, { GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0 } },
	{ GL_OPERAND0_RGB_EXT, { GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_OPERAND1_RGB_EXT, { GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_OPERAND0_ALPHA_EXT, { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_OPERAND1_ALPHA_EXT, { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_RGB_SCALE_EXT, { 1, 2, 4, 0 } },
	{ GL_ALPHA_SCALE, { 1, 2, 4, 0 } },
	{ 0, { 0, 0, 0, 0, 0 } }
};

TexCombineTest::test_param TexCombineTest::Dot3RGBAParams[] = {
	{ GL_COMBINE_RGB_EXT, { GL_DOT3_RGBA_EXT, 0 } },
	{ GL_COMBINE_ALPHA_EXT, { GL_MODULATE, 0 } },
	{ GL_SOURCE0_RGB_EXT, { GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, 0 } },
	{ GL_SOURCE1_RGB_EXT, { GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0 } },
	{ GL_SOURCE0_ALPHA_EXT, { GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, 0 } },
	{ GL_SOURCE1_ALPHA_EXT, { GL_TEXTURE, GL_CONSTANT_EXT, GL_PRIMARY_COLOR_EXT, GL_PREVIOUS_EXT, 0 } },
	{ GL_OPERAND0_RGB_EXT, { GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_OPERAND1_RGB_EXT, { GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_OPERAND0_ALPHA_EXT, { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_OPERAND1_ALPHA_EXT, { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 0 } },
	{ GL_RGB_SCALE_EXT, { 1, 2, 4, 0 } },
	{ GL_ALPHA_SCALE, { 1, 2, 4, 0 } },
	{ 0, { 0, 0, 0, 0, 0 } }
};


static void
problem(const char *s) {
	cerr << "Problem in combine():" << s << "\n";
}


//
// Set machine parameters to default values.
//
void
TexCombineTest::ResetMachine(glmachine &machine) {
	for (int u = 0; u < MAX_TEX_UNITS; u++) {
		machine.COMBINE_RGB[u] = GL_MODULATE;
		machine.COMBINE_ALPHA[u] = GL_MODULATE;
		machine.SOURCE0_RGB[u] = GL_TEXTURE;
		machine.SOURCE1_RGB[u] = GL_PREVIOUS_EXT;
		machine.SOURCE2_RGB[u] = GL_CONSTANT_EXT;
		machine.SOURCE0_ALPHA[u] = GL_TEXTURE;
		machine.SOURCE1_ALPHA[u] = GL_PREVIOUS_EXT;
		machine.SOURCE2_ALPHA[u] = GL_CONSTANT_EXT;
		machine.OPERAND0_RGB[u] = GL_SRC_COLOR;
		machine.OPERAND1_RGB[u] = GL_SRC_COLOR;
		machine.OPERAND2_RGB[u] = GL_SRC_ALPHA;
		machine.OPERAND0_ALPHA[u] = GL_SRC_ALPHA;
		machine.OPERAND1_ALPHA[u] = GL_SRC_ALPHA;
		machine.OPERAND2_ALPHA[u] = GL_SRC_ALPHA;
		machine.RGB_SCALE[u] = 1.0;
		machine.ALPHA_SCALE[u] = 1.0;
	}
}


//
// This computes the expected texcombine result for one texture unit.
//
void
TexCombineTest::ComputeTexCombine(const glmachine &machine, int texUnit,
				const GLfloat prevColor[4],
				GLfloat result[4]) const {
	GLfloat term0[4], term1[4], term2[4], dot;
	const GLfloat *fragColor = machine.FragColor;
	const GLfloat *constColor = machine.EnvColor[texUnit];
	const GLfloat *texColor = machine.TexColor[texUnit];

	switch (machine.SOURCE0_RGB[texUnit]) {
	case GL_PRIMARY_COLOR_EXT:
		COPY3(term0, fragColor);
		break;
	case GL_TEXTURE:
		COPY3(term0, texColor);
		break;
	case GL_CONSTANT_EXT:
		COPY3(term0, constColor);
		break;
	case GL_PREVIOUS_EXT:
		COPY3(term0, prevColor);
		break;
	default:
		problem("bad rgbSource0");
		return;
	}

	switch (machine.SOURCE0_ALPHA[texUnit]) {
	case GL_PRIMARY_COLOR_EXT:
		term0[3] = fragColor[3];
		break;
	case GL_TEXTURE:
		term0[3] = texColor[3];
		break;
	case GL_CONSTANT_EXT:
		term0[3] = constColor[3];
		break;
	case GL_PREVIOUS_EXT:
		term0[3] = prevColor[3];
		break;
	default:
		problem("bad alphaSource0");
		return;
	}

	switch (machine.SOURCE1_RGB[texUnit]) {
	case GL_PRIMARY_COLOR_EXT:
		COPY3(term1, fragColor);
		break;
	case GL_TEXTURE:
		COPY3(term1, texColor);
		break;
	case GL_CONSTANT_EXT:
		COPY3(term1, constColor);
		break;
	case GL_PREVIOUS_EXT:
		COPY3(term1, prevColor);
		break;
	default:
		problem("bad rgbSource1");
		return;
	}

	switch (machine.SOURCE1_ALPHA[texUnit]) {
	case GL_PRIMARY_COLOR_EXT:
		term1[3] = fragColor[3];
		break;
	case GL_TEXTURE:
		term1[3] = texColor[3];
		break;
	case GL_CONSTANT_EXT:
		term1[3] = constColor[3];
		break;
	case GL_PREVIOUS_EXT:
		term1[3] = prevColor[3];
		break;
	default:
		problem("bad alphaSource1");
		return;
	}

	switch (machine.SOURCE2_RGB[texUnit]) {
	case GL_PRIMARY_COLOR_EXT:
		COPY3(term2, fragColor);
		break;
	case GL_TEXTURE:
		COPY3(term2, texColor);
		break;
	case GL_CONSTANT_EXT:
		COPY3(term2, constColor);
		break;
	case GL_PREVIOUS_EXT:
		COPY3(term2, prevColor);
		break;
	default:
		problem("bad rgbSource2");
		return;
	}

	switch (machine.SOURCE2_ALPHA[texUnit]) {
	case GL_PRIMARY_COLOR_EXT:
		term2[3] = fragColor[3];
		break;
	case GL_TEXTURE:
		term2[3] = texColor[3];
		break;
	case GL_CONSTANT_EXT:
		term2[3] = constColor[3];
		break;
	case GL_PREVIOUS_EXT:
		term2[3] = prevColor[3];
		break;
	default:
		problem("bad alphaSource2");
		return;
	}

	switch (machine.OPERAND0_RGB[texUnit]) {
	case GL_SRC_COLOR:
		// no change
		break;
	case GL_ONE_MINUS_SRC_COLOR:
		term0[0] = 1.0 - term0[0];
		term0[1] = 1.0 - term0[1];
		term0[2] = 1.0 - term0[2];
		break;
	case GL_SRC_ALPHA:
		term0[0] = term0[3];
		term0[1] = term0[3];
		term0[2] = term0[3];
		break;
	case GL_ONE_MINUS_SRC_ALPHA:
		term0[0] = 1.0 - term0[3];
		term0[1] = 1.0 - term0[3];
		term0[2] = 1.0 - term0[3];
		break;
	default:
		problem("bad rgbOperand0");
		return;
	}

	switch (machine.OPERAND0_ALPHA[texUnit]) {
	case GL_SRC_ALPHA:
		// no change
		break;
	case GL_ONE_MINUS_SRC_ALPHA:
		term0[3] = 1.0 - term0[3];
		break;
	default:
		problem("bad alphaOperand0");
		return;
	}

	switch (machine.OPERAND1_RGB[texUnit]) {
	case GL_SRC_COLOR:
		// no change
		break;
	case GL_ONE_MINUS_SRC_COLOR:
		term1[0] = 1.0 - term1[0];
		term1[1] = 1.0 - term1[1];
		term1[2] = 1.0 - term1[2];
		break;
	case GL_SRC_ALPHA:
		term1[0] = term1[3];
		term1[1] = term1[3];
		term1[2] = term1[3];
		break;
	case GL_ONE_MINUS_SRC_ALPHA:
		term1[0] = 1.0 - term1[3];
		term1[1] = 1.0 - term1[3];
		term1[2] = 1.0 - term1[3];
		break;
	default:
		problem("bad rgbOperand1");
		return;
	}

	switch (machine.OPERAND1_ALPHA[texUnit]) {
	case GL_SRC_ALPHA:
		// no change
		break;
	case GL_ONE_MINUS_SRC_ALPHA:
		term1[3] = 1.0 - term1[3];
		break;
	default:
		problem("bad alphaOperand1");
		return;
	}

	switch (machine.OPERAND2_RGB[texUnit]) {
	case GL_SRC_ALPHA:
		term2[0] = term2[3];
		term2[1] = term2[3];
		term2[2] = term2[3];
		break;
	default:
		problem("bad rgbOperand2");
		return;
	}

	switch (machine.OPERAND2_ALPHA[texUnit]) {
	case GL_SRC_ALPHA:
		// no change
		break;
	default:
		problem("bad alphaOperand2");
		return;
	}

	// Final combine
	switch (machine.COMBINE_RGB[texUnit]) {
	case GL_REPLACE:
		COPY3(result, term0);
		break;
	case GL_MODULATE:
		result[0] = term0[0] * term1[0];
		result[1] = term0[1] * term1[1];
		result[2] = term0[2] * term1[2];
		break;
	case GL_ADD:
		result[0] = term0[0] + term1[0];
		result[1] = term0[1] + term1[1];
		result[2] = term0[2] + term1[2];
		break;
	case GL_ADD_SIGNED_EXT:
		result[0] = term0[0] + term1[0] - 0.5;
		result[1] = term0[1] + term1[1] - 0.5;
		result[2] = term0[2] + term1[2] - 0.5;
		break;
	case GL_INTERPOLATE_EXT:
		result[0] = term0[0] * term2[0] + term1[0] * (1.0 - term2[0]);
		result[1] = term0[1] * term2[1] + term1[1] * (1.0 - term2[1]);
		result[2] = term0[2] * term2[2] + term1[2] * (1.0 - term2[2]);
		break;
	case GL_DOT3_RGB_EXT:
	case GL_DOT3_RGBA_EXT:
		dot = ((term0[0] - 0.5) * (term1[0] - 0.5) +
		       (term0[1] - 0.5) * (term1[1] - 0.5) +
		       (term0[2] - 0.5) * (term1[2] - 0.5));
		result[0] = dot;
		result[1] = dot;
		result[2] = dot;
		break;
	default:
		problem("bad rgbCombine");
		return;
	}

	switch (machine.COMBINE_ALPHA[texUnit]) {
	case GL_REPLACE:
		result[3] = term0[3];
		break;
	case GL_MODULATE:
		result[3] = term0[3] * term1[3];
		break;
	case GL_ADD:
		result[3] = term0[3] + term1[3];
		break;
	case GL_ADD_SIGNED_EXT:
		result[3] = term0[3] + term1[3] - 0.5;
		break;
	case GL_INTERPOLATE_EXT:
		result[3] = term0[3] * term2[3] + term1[3] * (1.0 - term2[3]);
		break;
	default:
		problem("bad alphaCombine");
		return;
	}

	if (machine.COMBINE_RGB[texUnit] == GL_DOT3_RGBA_EXT) {
	   result[3] = result[0];
	}

	   
	// scaling
	// GH: Remove this crud when the ARB extension is done.  It
	// most likely won't have this scale factor restriction.
	switch (machine.COMBINE_RGB[texUnit]) {
	case GL_DOT3_RGB_EXT:
	case GL_DOT3_RGBA_EXT:
	   result[0] *= 4.0;
	   result[1] *= 4.0;
	   result[2] *= 4.0;
	   break;
	default:
	   result[0] *= machine.RGB_SCALE[texUnit];
	   result[1] *= machine.RGB_SCALE[texUnit];
	   result[2] *= machine.RGB_SCALE[texUnit];
	   break;
	}
	switch (machine.COMBINE_RGB[texUnit]) {
	case GL_DOT3_RGBA_EXT:
	   result[3] *= 4.0;
	   break;
	default:
	   result[3] *= machine.ALPHA_SCALE[texUnit];
	   break;
	}

	// final clamping
	result[0] = CLAMP(result[0], 0.0, 1.0);
	result[1] = CLAMP(result[1], 0.0, 1.0);
	result[2] = CLAMP(result[2], 0.0, 1.0);
	result[3] = CLAMP(result[3], 0.0, 1.0);
}


//
// Return string for an enum value.
//
const char *
EnumString(GLenum pname)
{
	static char s[100];
	switch (pname) {
	case GL_COMBINE_RGB_EXT:
		return "GL_COMBINE_RGB_EXT";
	case GL_COMBINE_ALPHA_EXT:
		return "GL_COMBINE_ALPHA_EXT";
	case GL_REPLACE:
		return "GL_REPLACE";
	case GL_MODULATE:
		return "GL_MODULATE";
	case GL_ADD:
		return "GL_ADD";
	case GL_ADD_SIGNED_EXT:
		return "GL_ADD_SIGNED_EXT";
	case GL_INTERPOLATE_EXT:
		return "GL_INTERPOLATE_EXT";
	case GL_DOT3_RGB_EXT:
		return "GL_DOT3_RGB_EXT";
	case GL_DOT3_RGBA_EXT:
		return "GL_DOT3_RGBA_EXT";
	case GL_TEXTURE:
		return "GL_TEXTURE";
	case GL_CONSTANT_EXT:
		return "GL_CONSTANT_EXT";
	case GL_PRIMARY_COLOR_EXT:
		return "GL_PRIMARY_COLOR_EXT";
	case GL_PREVIOUS_EXT:
		return "GL_PREVIOUS_EXT";
	case GL_SRC_COLOR:
		return "GL_SRC_COLOR";
	case GL_ONE_MINUS_SRC_COLOR:
		return "GL_ONE_MINUS_SRC_COLOR";
	case GL_SRC_ALPHA:
		return "GL_SRC_ALPHA";
	case GL_ONE_MINUS_SRC_ALPHA:
		return "GL_ONE_MINUS_SRC_ALPHA";
	default:
		sprintf(s, "0x%04x", pname);
		return s;
	}
}


//
// Print current values of all machine state vars.
// Used when reporting failures.
//
void
TexCombineTest::PrintMachineState(const glmachine &machine) const {

	env->log << "\tCurrent combine state:\n";
	for (int u = 0; u < machine.NumTexUnits; u++) {
		env->log << "\tTexture Unit " << u << ":\n";
		env->log << "\tGL_COMBINE_RGB_EXT = "
			 << EnumString(machine.COMBINE_RGB[u]) << "\n";
		env->log << "\tGL_COMBINE_ALPHA_EXT = "
			 << EnumString(machine.COMBINE_ALPHA[u]) << "\n";
		env->log << "\tGL_SOURCE0_RGB_EXT = "
			 << EnumString(machine.SOURCE0_RGB[u]) << "\n";
		env->log << "\tGL_SOURCE1_RGB_EXT = "
			 << EnumString(machine.SOURCE1_RGB[u]) << "\n";
		env->log << "\tGL_SOURCE2_RGB_EXT = "
			 << EnumString(machine.SOURCE2_RGB[u]) << "\n";
		env->log << "\tGL_SOURCE0_ALPHA_EXT = "
			 << EnumString(machine.SOURCE0_ALPHA[u]) << "\n";
		env->log << "\tGL_SOURCE1_ALPHA_EXT = "
			 << EnumString(machine.SOURCE1_ALPHA[u]) << "\n";
		env->log << "\tGL_SOURCE2_ALPHA_EXT = "
			 << EnumString(machine.SOURCE2_ALPHA[u]) << "\n";
		env->log << "\tGL_OPERAND0_RGB_EXT = "
			 << EnumString(machine.OPERAND0_RGB[u]) << "\n";
		env->log << "\tGL_OPERAND1_RGB_EXT = "
			 << EnumString(machine.OPERAND1_RGB[u]) << "\n";
		env->log << "\tGL_OPERAND2_RGB_EXT = "
			 << EnumString(machine.OPERAND2_RGB[u]) << "\n";
		env->log << "\tGL_OPERAND0_ALPHA_EXT = "
			 << EnumString(machine.OPERAND0_ALPHA[u]) << "\n";
		env->log << "\tGL_OPERAND1_ALPHA_EXT = "
			 << EnumString(machine.OPERAND1_ALPHA[u]) << "\n";
		env->log << "\tGL_OPERAND2_ALPHA_EXT = "
			 << EnumString(machine.OPERAND2_ALPHA[u]) << "\n";
		env->log << "\tGL_RGB_SCALE_EXT = "
			 << machine.RGB_SCALE[u] << "\n";
		env->log << "\tGL_ALPHA_SCALE = "
			 << machine.ALPHA_SCALE[u] << "\n";
                env->log << "\tFragment Color = "
                         << machine.FragColor[0] << ", "
                         << machine.FragColor[1] << ", "
                         << machine.FragColor[2] << ", "
                         << machine.FragColor[3] << "\n";
                env->log << "\tTex Env Color = "
                         << machine.EnvColor[u][0] << ", "
                         << machine.EnvColor[u][1] << ", "
                         << machine.EnvColor[u][2] << ", "
                         << machine.EnvColor[u][3] << "\n";
                env->log << "\tTexture Color = "
                         << machine.TexColor[u][0] << ", "
                         << machine.TexColor[u][1] << ", "
                         << machine.TexColor[u][2] << ", "
                         << machine.TexColor[u][3] << "\n";
	}
}


//
// Print an error report.
//
void
TexCombineTest::ReportFailure(const glmachine &machine,
	const GLfloat expected[4],
	const GLfloat rendered[4],
	BasicResult& r) {

	env->log << name << ":  FAIL "
		 << r.config->conciseDescription() << '\n'
		 << "\texpected "
		 << expected[0] << ", "
		 << expected[1] << ", "
		 << expected[2] << ", "
		 << expected[3] << ", got "
		 << rendered[0] << ", "
		 << rendered[1] << ", "
		 << rendered[2] << ", "
		 << rendered[3] << "\n";
	PrintMachineState(machine);
}


//
// Examine a set of test params and compute the number of possible
// state combinations.
//
int
TexCombineTest::CountTestCombinations(const test_param testParams[]) const {

	int numTests = 1;
	for (int t = 0; testParams[t].target; t++) {
		int values = 0;
		for (int val = 0; testParams[t].validValues[val]; val++) {
			values++;
		}
		numTests *= values;
	}
	return numTests;
}


//
// Setup the actual GL state and our internal simulated GL state.
//
void
TexCombineTest::TexEnv(glmachine &machine, int texUnit,
	GLenum target, GLenum value) {

	if (machine.NumTexUnits > 1)
		p_glActiveTextureARB(GL_TEXTURE0_ARB + texUnit);

	glTexEnvi(GL_TEXTURE_ENV, target, value);

	switch (target) {
	case GL_COMBINE_RGB_EXT:
		machine.COMBINE_RGB[texUnit] = value;
		break;
	case GL_COMBINE_ALPHA_EXT:
		machine.COMBINE_ALPHA[texUnit] = value;
		break;
	case GL_SOURCE0_RGB_EXT:
		machine.SOURCE0_RGB[texUnit] = value;
		break;
	case GL_SOURCE1_RGB_EXT:
		machine.SOURCE1_RGB[texUnit] = value;
		break;
	case GL_SOURCE2_RGB_EXT:
		machine.SOURCE2_RGB[texUnit] = value;
		break;
	case GL_SOURCE0_ALPHA_EXT:
		machine.SOURCE0_ALPHA[texUnit] = value;
		break;
	case GL_SOURCE1_ALPHA_EXT:
		machine.SOURCE1_ALPHA[texUnit] = value;
		break;
	case GL_SOURCE2_ALPHA_EXT:
		machine.SOURCE2_ALPHA[texUnit] = value;
		break;
	case GL_OPERAND0_RGB_EXT:
		machine.OPERAND0_RGB[texUnit] = value;
		break;
	case GL_OPERAND1_RGB_EXT:
		machine.OPERAND1_RGB[texUnit] = value;
		break;
	case GL_OPERAND2_RGB_EXT:
		machine.OPERAND2_RGB[texUnit] = value;
		break;
	case GL_OPERAND0_ALPHA_EXT:
		machine.OPERAND0_ALPHA[texUnit] = value;
		break;
	case GL_OPERAND1_ALPHA_EXT:
		machine.OPERAND1_ALPHA[texUnit] = value;
		break;
	case GL_OPERAND2_ALPHA_EXT:
		machine.OPERAND2_ALPHA[texUnit] = value;
		break;
	case GL_RGB_SCALE_EXT:
		machine.RGB_SCALE[texUnit] = value;
		break;
	case GL_ALPHA_SCALE:
		machine.ALPHA_SCALE[texUnit] = value;
		break;
	}
}


//
// Make the glTexEnv calls to setup one particular set of test parameters
// from <testParams>.
// <testNum> must be between 0 and CountTestCombinations(testParams)-1.
//
void
TexCombineTest::SetupTestEnv(struct glmachine &machine, int texUnit,
	int testNum, const struct test_param testParams[]) {

	int divisor = 1;
	for (int t = 0; testParams[t].target; t++) {
		int numValues = 0;
		for (int val = 0; testParams[t].validValues[val]; val++) {
			numValues++;
		}
		int v = (testNum / divisor) % numValues;
		GLenum target = testParams[t].target;
		GLenum value = testParams[t].validValues[v];
		TexEnv(machine, texUnit, target, value);
		divisor *= numValues;
	}
}


//
// Set the fragment, texenv (constant), and texture colors for all the
// machine's texture units.
//
void
TexCombineTest::SetupColors(glmachine &machine) {

	static const GLfloat fragColor[4] = { 0.25, 0.5,  0.75, 1.0 };
	static const GLfloat envColors[][4] = {
		{ 0.5, 0.5, 1.0, 1.0 },
		{ 0.5, 1.0, 0.5, 1.0 },
		{ 1.0, 0.5, 0.5, 1.0 },
		{ 1.0, 1.0, 1.0, 0.5 }
	};
	static const GLfloat texColors[][4] = {
		{ 1.00, 0.75, 0.25, 1.0 },
		{ 0.25, 1.00, 0.25, 1.0 },
		{ 0.25, 0.25, 1.00, 1.0 },
		{ 0.50, 0.50, 0.50, 0.5 }
	};

	COPY4(machine.FragColor, fragColor);
	glColor4fv(fragColor);

	for (int u = 0; u < machine.NumTexUnits; u++) {
		if (machine.NumTexUnits > 1)
			p_glActiveTextureARB(GL_TEXTURE0_ARB + u);
		glEnable(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
		machine.EnvColor[u][0] = envColors[u % 4][0];
		machine.EnvColor[u][1] = envColors[u % 4][1];
		machine.EnvColor[u][2] = envColors[u % 4][2];
		machine.EnvColor[u][3] = envColors[u % 4][3];
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR,
			envColors[u % 4]);

		machine.TexColor[u][0] = texColors[u % 4][0];
		machine.TexColor[u][1] = texColors[u % 4][1];
		machine.TexColor[u][2] = texColors[u % 4][2];
		machine.TexColor[u][3] = texColors[u % 4][3];

                /* Make a 4x4 solid color texture */
		{
			GLfloat image[16][4];
			int i;
			for (i = 0; i < 16; i++) {
				image[i][0] = texColors[u % 4][0];
				image[i][1] = texColors[u % 4][1];
				image[i][2] = texColors[u % 4][2];
				image[i][3] = texColors[u % 4][3];
			}
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
				     4, 4, 0, GL_RGBA, GL_FLOAT, image);
#if 0 // Debug
			GLfloat check[16][4];
			GLint r, g, b, a;
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
						 GL_TEXTURE_RED_SIZE, &r);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
						 GL_TEXTURE_GREEN_SIZE, &g);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
						 GL_TEXTURE_BLUE_SIZE, &b);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
						 GL_TEXTURE_ALPHA_SIZE, &a);
			printf("Texture bits: %d %d %d %d\n", r, g, b, a);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT,
				      check);
			for (i = 0;i < 16; i++) {
				printf("%2d: %4f %4f %4f %4f  %4f %4f %4f %4f\n", i,
				       image[i][0], image[i][1],
				       image[i][2], image[i][3],
				       check[i][0], check[i][1],
				       check[i][2], check[i][3]);
			}
#endif
                }
	}
}


//
// Test texenv-combine with a single texture unit.
//
bool
TexCombineTest::RunSingleTextureTest(glmachine &machine,
	const test_param testParams[], BasicResult &r, Window& w) {

	assert(machine.NumTexUnits == 1);
	SetupColors(machine);

	const int numTests = CountTestCombinations(testParams);
	//printf("Testing %d combinations\n", numTests);

	for (int test = 0; test < numTests; test++) {
		// 0. Setup state
		ResetMachine(machine);      
		SetupTestEnv(machine, 0, test, testParams);

		// 1. Render with OpenGL
		GLfloat renderedResult[4];
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 0);  glVertex2f(-1.0, -1.0);
		glTexCoord2f(1, 0);  glVertex2f( 1.0, -1.0);
		glTexCoord2f(1, 1);  glVertex2f( 1.0,  1.0);
		glTexCoord2f(0, 1);  glVertex2f(-1.0,  1.0);
		glEnd();
		glReadPixels(0, 0, 1, 1, GL_RGBA, GL_FLOAT, renderedResult);
		w.swap();

		// 2. Compute expected result
		GLfloat expected[4];
		ComputeTexCombine(machine, 0, machine.FragColor, expected);

		// 3. Compare rendered result to expected result
		const GLfloat epsilon = 1.0 / 32.0;
		const GLfloat dr = fabs(expected[0] - renderedResult[0]);
		const GLfloat dg = fabs(expected[1] - renderedResult[1]);
		const GLfloat db = fabs(expected[2] - renderedResult[2]);
		//const GLfloat da = fabs(expected[3] - renderedResult[3]);
		if (dr > epsilon || dg > epsilon || db > epsilon) {
			ReportFailure(machine, expected, renderedResult, r);
#if 0 // Debug
			// For debugging, printing the state of the previous
			// test is useful to see what's changed when we've
			// failed a test but passed the previous one.
			printf("single test %d failed\n", test);
			if (test > 0) {
				printf("prev test:\n");
				SetupTestEnv(machine, 0, test - 1, testParams);
				PrintMachineState(machine);
			}
#endif
			return false;
		}
#if 0 // Debug
                else {
			printf("PASSED test %d!\n", test);
			env->log << "\texpected "
				 << expected[0] << ", "
				 << expected[1] << ", "
				 << expected[2] << ", "
				 << expected[3] << ", got "
				 << renderedResult[0] << ", "
				 << renderedResult[1] << ", "
				 << renderedResult[2] << ", "
				 << renderedResult[3] << "\n";
			// PrintMachineState(machine);
                }
#endif
	}
	return true;
}



//
// For each texture unit, test each texenv-combine mode.
// That's 5 ^ NumTexUnits combinations.
//
int
TexCombineTest::CountMultiTextureTestCombinations(const glmachine &machine) const {

	int numTests = 1;
	for (int i = 0; i < machine.NumTexUnits; i++)
		numTests *= 5;

	return numTests;
}


//
// Test texenv-combine with multiple texture units.
//
bool
TexCombineTest::RunMultiTextureTest(glmachine &machine, BasicResult &r,
    Window& w) {

	static const GLenum combineModes[7] = {
		GL_REPLACE,
		GL_ADD,
		GL_ADD_SIGNED_EXT,
		GL_MODULATE,
		GL_INTERPOLATE_EXT,
		GL_DOT3_RGB_EXT,
		GL_DOT3_RGBA_EXT
	};

	const int numTests = CountMultiTextureTestCombinations(machine);
	//printf("Testing %d multitexture combinations\n", numTests);

	SetupColors(machine);
	for (int testNum = 0; testNum < numTests; testNum++) {
		// 0. Set up texture units
		ResetMachine(machine);
		int divisor = 1;
		for (int u = 0; u < machine.NumTexUnits; u++) {
			const int m = (testNum / divisor) % 5;
			const GLenum mode = combineModes[m];

			// Set GL_COMBINE_RGB_EXT and GL_COMBINE_ALPHA_EXT
			TexEnv(machine, u, GL_COMBINE_RGB_EXT, mode);
			TexEnv(machine, u, GL_COMBINE_ALPHA_EXT, mode);
			TexEnv(machine, u, GL_SOURCE0_RGB_EXT, GL_PREVIOUS_EXT);
			TexEnv(machine, u, GL_SOURCE1_RGB_EXT, GL_PREVIOUS_EXT);
			TexEnv(machine, u, GL_SOURCE2_RGB_EXT, GL_TEXTURE);
			TexEnv(machine, u, GL_SOURCE0_ALPHA_EXT, GL_PREVIOUS_EXT);
			TexEnv(machine, u, GL_SOURCE1_ALPHA_EXT, GL_PREVIOUS_EXT);
			TexEnv(machine, u, GL_SOURCE2_ALPHA_EXT, GL_TEXTURE);
			TexEnv(machine, u, GL_OPERAND0_RGB_EXT, GL_SRC_COLOR);
			TexEnv(machine, u, GL_OPERAND1_RGB_EXT, GL_ONE_MINUS_SRC_COLOR);
			TexEnv(machine, u, GL_OPERAND2_RGB_EXT, GL_SRC_ALPHA);
			TexEnv(machine, u, GL_OPERAND0_ALPHA_EXT, GL_SRC_ALPHA);
			TexEnv(machine, u, GL_OPERAND1_ALPHA_EXT, GL_ONE_MINUS_SRC_ALPHA);
			TexEnv(machine, u, GL_OPERAND2_ALPHA_EXT, GL_SRC_ALPHA);
			TexEnv(machine, u, GL_RGB_SCALE_EXT, 1);
			TexEnv(machine, u, GL_ALPHA_SCALE, 1);

			//printf("texenv%d = %s  ", u, EnumString(mode));
			divisor *= 5;
		}
		//printf("\n");

		// 1. Render with OpenGL
		GLfloat renderedResult[4];
		glBegin(GL_POLYGON);
		for (int u = 0; u < machine.NumTexUnits; u++)
			p_glMultiTexCoord2fARB(GL_TEXTURE0_ARB + u, 0, 0);
		glVertex2f(-1.0, -1.0);
		for (int u = 0; u < machine.NumTexUnits; u++)
			p_glMultiTexCoord2fARB(GL_TEXTURE0_ARB + u, 1, 0);
		glVertex2f( 1.0, -1.0);
		for (int u = 0; u < machine.NumTexUnits; u++)
			p_glMultiTexCoord2fARB(GL_TEXTURE0_ARB + u, 1, 1);
		glVertex2f( 1.0,  1.0);
		for (int u = 0; u < machine.NumTexUnits; u++)
			p_glMultiTexCoord2fARB(GL_TEXTURE0_ARB + u, 0, 1);
		glVertex2f(-1.0,  1.0);
		glEnd();
		glReadPixels(0, 0, 1, 1, GL_RGBA, GL_FLOAT, renderedResult);
		w.swap();

                // 2. Compute expected result
		GLfloat prevColor[4];
		GLfloat expected[4];
		for (int u = 0; u < machine.NumTexUnits; u++) {
			if (u == 0) {
				COPY4(prevColor, machine.FragColor);
			} else {
				COPY4(prevColor, expected);
			}
			ComputeTexCombine(machine, u, prevColor, expected);
		}

		// 3. Compare rendered result to expected result
		const GLfloat epsilon = 1.0 / 32.0;
		const GLfloat dr = fabs(expected[0] - renderedResult[0]);
		const GLfloat dg = fabs(expected[1] - renderedResult[1]);
		const GLfloat db = fabs(expected[2] - renderedResult[2]);
		//const GLfloat da = fabs(expected[3] - renderedResult[3]);
		if (dr > epsilon || dg > epsilon || db > epsilon) {
			ReportFailure(machine, expected, renderedResult, r);
			printf("multitex test %d failed\n", testNum);
#if 0 // Debug
			if (test > 0) {
				printf("prev test:\n");
				SetupTestEnv(machine, 0, test - 1, testParams);
				PrintMachineState(machine);
			}
#endif
                        
			return false;
		}
	}
	return true;
}


///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////

// XXX should we run a number of individual tests instead?
void
TexCombineTest::runOne(BasicResult& r, Window& w) {
	// Grab pointers to the extension functions.  It's safe to use
	// these without testing them because we already know that we
	// won't be invoked except on contexts that support the
	// extension.
	p_glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)
		(GLUtils::getProcAddress("glActiveTextureARB"));
	p_glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)
		(GLUtils::getProcAddress("glMultiTexCoord2fARB"));

	// Test the availability of the DOT3 extenstion
	haveDot3 = GLUtils::haveExtensions("GL_EXT_texture_env_dot3");

        // We'll only render a 4-pixel polygon
	glViewport(0, 0, 2, 2);

	Machine.NumTexUnits = 1;

	// Do single texture unit tests first.
	bool passed = RunSingleTextureTest(Machine, ReplaceParams, r, w);
	if (passed)
		passed = RunSingleTextureTest(Machine, AddParams, r, w);
	if (passed)
		passed = RunSingleTextureTest(Machine, AddSignedParams, r, w);
	if (passed)
		passed = RunSingleTextureTest(Machine, ModulateParams, r, w);
	if (passed)
		passed = RunSingleTextureTest(Machine, InterpolateParams, r, w);
	if (passed && haveDot3)
		passed = RunSingleTextureTest(Machine, Dot3RGBParams, r, w);
	if (passed && haveDot3)
		passed = RunSingleTextureTest(Machine, Dot3RGBAParams, r, w);

	// Now do some multi-texture tests
	if (passed) {
		glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &Machine.NumTexUnits);
		if (Machine.NumTexUnits > 1) {
			// six texture units is enough to test
			if (Machine.NumTexUnits > 6)
				Machine.NumTexUnits = 6;
			passed = RunMultiTextureTest(Machine, r, w);
		}
	}
	r.pass = passed;
} // TexCombineTest::runOne

void
TexCombineTest::logOne(BasicResult& r) {
	if (r.pass) {
		logPassFail(r);
		logConcise(r);
		env->log << "\tTested "
			<< CountTestCombinations(ReplaceParams)
			<< " GL_REPLACE combinations\n";
		env->log << "\tTested "
			<< CountTestCombinations(AddParams)
			<< " GL_ADD combinations\n";
		env->log << "\tTested "
			<< CountTestCombinations(AddSignedParams)
			<< " GL_ADD_SIGNED_EXT combinations\n";
		env->log << "\tTested "
			<< CountTestCombinations(ModulateParams)
			<< " GL_MODULATE combinations\n";
		env->log << "\tTested "
			<< CountTestCombinations(InterpolateParams)
			<< " GL_INTERPOLATE_EXT combinations\n";
		if (haveDot3) {
			env->log << "\tTested "
				 << CountTestCombinations(Dot3RGBParams)
				 << " GL_DOT3_RGB_EXT combinations\n";
			env->log << "\tTested "
				 << CountTestCombinations(Dot3RGBAParams)
				 << " GL_DOT3_RGBA_EXT combinations\n";
		}
		env->log << "\tTested "
			<< CountMultiTextureTestCombinations(Machine)
			<< " multitexture combinations\n";
	}
} // TexCombineTest::logOne


///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
TexCombineTest texCombTest("texCombine", "window, rgb",

	"GL_EXT_texture_env_combine verification test.\n"
	"We only test a subset of all possible texture env combinations\n"
	"because there's simply too many to exhaustively test them all.\n");


} // namespace GLEAN
