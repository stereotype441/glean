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

// treadpix.cpp:  implementation of ReadPixels tests

#include <cmath>
#include <iomanip>
#include "misc.h"
#include "rand.h"
#include "treadpix.h"


namespace {

void
checkRGBA(GLEAN::ReadPixSanityResult& r, GLEAN::Window& w) {
	using namespace GLEAN;
	DrawingSurfaceConfig& config = *r.config;
	RandomBitsDouble rRand(config.r, 1066);
	RandomBitsDouble gRand(config.g, 1492);
	RandomBitsDouble bRand(config.b, 1776);
	RandomBitsDouble aRand((config.a? config.a: 1), 1789);
	int thresh = 1;

	r.passRGBA = true;
	r.errRGBA = 0.0;
	for (int i = 0; i < 100 && r.passRGBA; ++i) {
		// Generate a random color and use it to clear the color buffer:
		float expected[4];
		expected[0] = rRand.next();
		expected[1] = gRand.next();
		expected[2] = bRand.next();
		expected[3] = aRand.next();
		glClearColor(expected[0],expected[1],expected[2],expected[3]);
		glClear(GL_COLOR_BUFFER_BIT);

		// If the color buffer doesn't have an alpha channel, then
		// the spec requires the readback value to be 1.0:
		if (!config.a)
			expected[3] = 1.0;

		// Read the buffer:
		GLfloat buf[READPIX_SANITY_WIN_SIZE][READPIX_SANITY_WIN_SIZE][4];
		glReadPixels(0, 0, READPIX_SANITY_WIN_SIZE,
			READPIX_SANITY_WIN_SIZE, GL_RGBA, GL_FLOAT, buf);

		// Now compute the error for each pixel, and record the
		// worst one we find:
		for (int y = 0; y < READPIX_SANITY_WIN_SIZE; ++y)
			for (int x = 0; x < READPIX_SANITY_WIN_SIZE; ++x) {
				GLfloat dr = abs(buf[y][x][0] - expected[0]);
				GLfloat dg = abs(buf[y][x][1] - expected[1]);
				GLfloat db = abs(buf[y][x][2] - expected[2]);
				GLfloat da = abs(buf[y][x][3] - expected[3]);
				double err =
				    max(ErrorBits(dr, config.r),
				    max(ErrorBits(dg, config.g),
				    max(ErrorBits(db, config.b),
					ErrorBits(da,
					   config.a? config.a: thresh+1))));
					   // The "thresh+1" fudge above is
					   // needed to force the error to
					   // be greater than the threshold
					   // in the case where there is no
					   // alpha channel.  Without it the
					   // error would be just equal to
					   // the threshold, and the test
					   // would spuriously pass.
				if (err > r.errRGBA) {
					r.xRGBA = x;
					r.yRGBA = y;
					r.errRGBA = err;
					for (int j = 0; j < 4; ++j) {
						r.expectedRGBA[j] = expected[j];
						r.actualRGBA[j] = buf[y][x][j];
					}
				}
			}

		if (r.errRGBA > thresh)
			r.passRGBA = false;
		w.swap();
	}
} // checkRGBA

void
checkDepth(GLEAN::ReadPixSanityResult& r, GLEAN::Window& w) {
	using namespace GLEAN;
	DrawingSurfaceConfig& config = *r.config;
	RandomDouble dRand(35798);
	int thresh = 1;

	r.passDepth = true;
	r.errDepth = 0.0;
	for (int i = 0; i < 100 && r.passDepth; ++i) {
		// Generate a random depth and use it to clear the depth buffer:
		GLdouble expected = dRand.next();
		glClearDepth(expected);
		glClear(GL_DEPTH_BUFFER_BIT);

		// Because glReadPixels won't return data of type GLdouble,
		// there's no straightforward portable way to deal with
		// integer depth buffers that are deeper than 32 bits or
		// floating-point depth buffers that have higher precision
		// than a GLfloat.  Since this is just a sanity check, we'll
		// use integer readback and settle for 32 bits at best.
		GLuint buf[READPIX_SANITY_WIN_SIZE][READPIX_SANITY_WIN_SIZE];
		glReadPixels(0, 0, READPIX_SANITY_WIN_SIZE,
			READPIX_SANITY_WIN_SIZE, GL_DEPTH_COMPONENT,
			GL_UNSIGNED_INT, buf);

		// Now compute the error for each pixel, and record the
		// worst one we find:
		for (int y = 0; y < READPIX_SANITY_WIN_SIZE; ++y)
			for (int x = 0; x < READPIX_SANITY_WIN_SIZE; ++x) {
				GLfloat dd = abs(buf[y][x]/4294967295.0
					- expected);
				double err = ErrorBits(dd, config.z);
				if (err > r.errDepth) {
					r.xDepth = x;
					r.yDepth = y;
					r.errDepth = err;
					r.expectedDepth = expected;
					r.actualDepth = buf[y][x]/4294967295.0;
				}
			}

		if (r.errDepth > thresh)
			r.passDepth = false;
		w.swap();
	}
} // checkDepth

void
checkStencil(GLEAN::ReadPixSanityResult& r, GLEAN::Window& w) {
	using namespace GLEAN;
	DrawingSurfaceConfig& config = *r.config;
	RandomBits sRand(config.s, 10101);

	r.passStencil = true;
	for (int i = 0; i < 100 && r.passStencil; ++i) {
		GLuint expected = sRand.next();
		glClearStencil(expected);
		glClear(GL_STENCIL_BUFFER_BIT);

		GLuint buf[READPIX_SANITY_WIN_SIZE][READPIX_SANITY_WIN_SIZE];
		glReadPixels(0, 0, READPIX_SANITY_WIN_SIZE,
			READPIX_SANITY_WIN_SIZE, GL_STENCIL_INDEX,
			GL_UNSIGNED_INT, buf);

		for (int y = 0; y < READPIX_SANITY_WIN_SIZE && r.passStencil;
		    ++y)
			for (int x = 0; x < READPIX_SANITY_WIN_SIZE; ++x)
				if (buf[y][x] != expected) {
					r.passStencil = false;
					r.xStencil = x;
					r.yStencil = y;
					r.expectedStencil = expected;
					r.actualStencil = buf[y][x];
					break;
				}

		w.swap();
	}
} // checkStencil

void
checkIndex(GLEAN::ReadPixSanityResult& r, GLEAN::Window& w) {
	using namespace GLEAN;
	DrawingSurfaceConfig& config = *r.config;
	RandomBits iRand(config.bufSize, 2);

	r.passIndex = true;
	for (int i = 0; i < 100 && r.passIndex; ++i) {
		GLuint expected = iRand.next();
		glClearIndex(expected);
		glClear(GL_COLOR_BUFFER_BIT);

		GLuint buf[READPIX_SANITY_WIN_SIZE][READPIX_SANITY_WIN_SIZE];
		glReadPixels(0, 0, READPIX_SANITY_WIN_SIZE,
			READPIX_SANITY_WIN_SIZE, GL_COLOR_INDEX,
			GL_UNSIGNED_INT, buf);

		for (int y = 0; y < READPIX_SANITY_WIN_SIZE && r.passIndex; ++y)
			for (int x = 0; x < READPIX_SANITY_WIN_SIZE; ++x)
				if (buf[y][x] != expected) {
					r.passIndex = false;
					r.xIndex = x;
					r.yIndex = y;
					r.expectedIndex = expected;
					r.actualIndex = buf[y][x];
					break;
				}

		w.swap();
	}
} // checkIndex

} // anonymous namespace

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////
void
ReadPixSanityTest::runOne(ReadPixSanityResult& r, GLEAN::Window& w) {

	// Many (if not most) other tests need to read the contents of
	// the framebuffer to determine if the correct image has been
	// drawn.  Obviously this is a waste of time if the basic
	// functionality of glReadPixels isn't working.
	//
	// This test does a "sanity" check of glReadPixels.  Using as
	// little of the GL as practicable, it writes a random value
	// in the framebuffer, reads it, and compares the value read
	// with the value written.

	glPixelStorei(GL_PACK_SWAP_BYTES, GL_FALSE);
	glPixelStorei(GL_PACK_LSB_FIRST, GL_FALSE);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_SKIP_ROWS, 0);
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	glPixelTransferi(GL_MAP_COLOR, GL_FALSE);
	glPixelTransferi(GL_MAP_STENCIL, GL_FALSE);
	glPixelTransferi(GL_INDEX_SHIFT, 0);
	glPixelTransferi(GL_INDEX_OFFSET, 0);
	glPixelTransferf(GL_RED_SCALE, 1.0);
	glPixelTransferf(GL_GREEN_SCALE, 1.0);
	glPixelTransferf(GL_BLUE_SCALE, 1.0);
	glPixelTransferf(GL_ALPHA_SCALE, 1.0);
	glPixelTransferf(GL_DEPTH_SCALE, 1.0);
	glPixelTransferf(GL_RED_BIAS, 0.0);
	glPixelTransferf(GL_GREEN_BIAS, 0.0);
	glPixelTransferf(GL_BLUE_BIAS, 0.0);
	glPixelTransferf(GL_ALPHA_BIAS, 0.0);
	glPixelTransferf(GL_DEPTH_BIAS, 0.0);

	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_DITHER);

	glIndexMask(~0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glStencilMask(~0);

	if (r.config->canRGBA)
		checkRGBA(r, w);
	if (r.config->z)
		checkDepth(r, w);
	if (r.config->s)
		checkStencil(r, w);
	
	r.pass = r.passRGBA & r.passDepth & r.passStencil & r.passIndex;
} // ReadPixSanityTest::runOne

///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
ReadPixSanityTest::compareOne(ReadPixSanityResult& oldR, ReadPixSanityResult& newR) {
	comparePassFail(oldR, newR);
	summarize("RGBA:    ", oldR.passRGBA, newR.passRGBA);
	summarize("Depth:   ", oldR.passDepth, newR.passDepth);
	summarize("Stencil: ", oldR.passStencil, newR.passStencil);
	summarize("Index:   ", oldR.passIndex, newR.passIndex);
	if (env->options.verbosity) {
		if (!oldR.passRGBA && !newR.passRGBA) {
			if (oldR.xRGBA != newR.xRGBA
			 || oldR.yRGBA != newR.yRGBA)
				env->log << "\tRGBA:    "
					<< env->options.db1Name
					<< " failed at ("
					<< oldR.xRGBA
					<< ", "
					<< oldR.yRGBA
					<< "); "
					<< env->options.db2Name
					<< " failed at ("
					<< newR.xRGBA
					<< ", "
					<< newR.yRGBA
					<< ").\n"
					;
			if (oldR.errRGBA != newR.errRGBA)
				env->log << "\tRGBA:    "
					<< env->options.db1Name
					<< " had "
					<< oldR.errRGBA
					<< " bits in error; "
					<< env->options.db2Name
					<< " had "
					<< newR.errRGBA
					<< " bits in error.\n"
					;
		}
		if (!oldR.passDepth && !newR.passDepth) {
			if (oldR.xDepth != newR.xDepth
			 || oldR.yDepth != newR.yDepth)
				env->log << "\tDepth:   "
					<< env->options.db1Name
					<< " failed at ("
					<< oldR.xDepth
					<< ", "
					<< oldR.yDepth
					<< "); "
					<< env->options.db2Name
					<< " failed at ("
					<< newR.xDepth
					<< ", "
					<< newR.yDepth
					<< ").\n"
					;
			if (oldR.errDepth != newR.errDepth)
				env->log << "\tDepth:   "
					<< env->options.db1Name
					<< " had "
					<< oldR.errDepth
					<< " bits in error; "
					<< env->options.db2Name
					<< " had "
					<< newR.errDepth
					<< " bits in error.\n"
					;
		}
		if (!oldR.passStencil && !newR.passStencil) {
			if (oldR.xStencil != newR.xStencil
			 || oldR.yStencil != newR.yStencil)
				env->log << "\tStencil: "
					<< env->options.db1Name
					<< " failed at ("
					<< oldR.xStencil
					<< ", "
					<< oldR.yStencil
					<< "); "
					<< env->options.db2Name
					<< " failed at ("
					<< newR.xStencil
					<< ", "
					<< newR.yStencil
					<< ").\n"
					;
		}
		if (!oldR.passIndex && !newR.passIndex) {
			if (oldR.xIndex != newR.xIndex
			 || oldR.yIndex != newR.yIndex)
				env->log << "\tIndex: "
					<< env->options.db1Name
					<< " failed at ("
					<< oldR.xIndex
					<< ", "
					<< oldR.yIndex
					<< "); "
					<< env->options.db2Name
					<< " failed at ("
					<< newR.xIndex
					<< ", "
					<< newR.yIndex
					<< ").\n"
					;
		}
	}
} // ReadPixSanityTest::compareOne

void
ReadPixSanityTest::summarize(char* label, bool oldPass, bool newPass) {
	if (oldPass == newPass) {
		if (env->options.verbosity)
			env->log << "\t"
				<< label
				<< "both "
				<< (oldPass? "passed": "failed")
				<< ".\n";
	} else {
		env->log << "\t"
			<< label
			<< env->options.db1Name
			<< " "
			<< (oldPass? "passed": "failed")
			<< "; "
			<< env->options.db2Name
			<< " "
			<< (newPass? "passed": "failed")
			<< ".\n"
			;
	}
} // ReadPixSanityTest::summarize

void
ReadPixSanityTest::logOne(ReadPixSanityResult& r) {
	logPassFail(r);
	logConcise(r);

	if (!r.passRGBA) {
		env->log << "\tRGB(A) worst-case error was "
			<< r.errRGBA << " bits at ("
			<< r.xRGBA << ", " << r.yRGBA << ")\n";
		env->log << "\t\texpected ("
			<< r.expectedRGBA[0] << ", "
			<< r.expectedRGBA[1] << ", "
			<< r.expectedRGBA[2] << ", "
			<< r.expectedRGBA[3] << ")\n\t\tgot ("
			<< r.actualRGBA[0] << ", "
			<< r.actualRGBA[1] << ", "
			<< r.actualRGBA[2] << ", "
			<< r.actualRGBA[3] << ")\n"
			;
	}
	if (!r.passDepth) {
		env->log << "\tDepth worst-case error was "
			<< r.errDepth << " bits at ("
			<< r.xDepth << ", " << r.yDepth << ")\n";
		env->log << "\t\texpected "
			<< r.expectedDepth
			<< "; got "
			<< r.actualDepth
			<< ".\n"
			;
	}
	if (!r.passStencil) {
		env->log << "\tStencil expected "
			<< r.expectedStencil
			<< "; got "
			<< r.actualStencil
			<< ".\n"
			;
	}
	if (!r.passIndex) {
		env->log << "\tIndex expected "
			<< r.expectedIndex
			<< "; got "
			<< r.actualIndex
			<< ".\n"
			;
	}
	if (env->options.verbosity) {
		if (r.config->canRGBA)
			env->log << "\tRGBA largest readback error was "
				<< r.errRGBA
				<< " bits\n";
		if (r.config->z)
			env->log << "\tDepth largest readback error was "
				<< r.errDepth
				<< " bits\n";
	}
} // ReadPixSanityTest::logOne

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
ReadPixSanityTest
readPixSanityTest("readPixSanity", "1",

	"This test performs a sanity check of glReadPixels, using as\n"
	"few other portions of the GL as possible.  If this test fails,\n"
	"it may be pointless to run other tests, since so many of them\n"
	"depend on reading the contents of the framebuffer to determine\n"
	"if they pass.\n"
	"\n"
	"The test works by using glClear to fill the framebuffer with a\n"
	"randomly-chosen value, reading the contents of the\n"
	"framebuffer, and comparing the actual contents with the\n"
	"expected contents.  RGB, RGBA, color index, stencil, and depth\n"
	"buffers (whichever are applicable to the current rendering\n"
	"context) are checked.  The test passes if the actual contents\n"
	"are within 1 LSB of the expected contents.\n"

	);

} // namespace GLEAN
