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


#include "tpixelformats.h"
#include <cassert>
#include <cmath>


// Set to 1 to help debug test failures:
#define DEBUG 0


namespace GLEAN {


struct NameTokenComps {
	const char *Name;
	GLenum Token;
	GLuint Components;
};


static const NameTokenComps Types[] =
{
	{ "GL_UNSIGNED_BYTE", GL_UNSIGNED_BYTE, 0 },
	{ "GL_BYTE", GL_BYTE, 0 },
	{ "GL_UNSIGNED_INT", GL_UNSIGNED_INT, 0 },
	{ "GL_SHORT", GL_SHORT, 0 },
	{ "GL_UNSIGNED_SHORT", GL_UNSIGNED_SHORT, 0 },
	{ "GL_INT", GL_INT, 0 },
	{ "GL_FLOAT", GL_FLOAT, 0 },
#ifdef GL_ARB_half_float_pixel
	{ "GL_HALF", GL_HALF_FLOAT_ARB, 0 },
#endif
	{ "GL_UNSIGNED_INT_8_8_8_8", GL_UNSIGNED_INT_8_8_8_8, 4 },
	{ "GL_UNSIGNED_INT_8_8_8_8_REV", GL_UNSIGNED_INT_8_8_8_8_REV, 4 },
	{ "GL_UNSIGNED_INT_10_10_10_2", GL_UNSIGNED_INT_10_10_10_2, 4 },
	{ "GL_UNSIGNED_INT_2_10_10_10_REV", GL_UNSIGNED_INT_2_10_10_10_REV, 4 },
	{ "GL_UNSIGNED_SHORT_5_5_5_1", GL_UNSIGNED_SHORT_5_5_5_1, 4 },
	{ "GL_UNSIGNED_SHORT_1_5_5_5_REV", GL_UNSIGNED_SHORT_1_5_5_5_REV, 4 },
	{ "GL_UNSIGNED_SHORT_4_4_4_4", GL_UNSIGNED_SHORT_4_4_4_4, 4 },
	{ "GL_UNSIGNED_SHORT_4_4_4_4_REV", GL_UNSIGNED_SHORT_4_4_4_4_REV, 4 },
	{ "GL_UNSIGNED_SHORT_5_6_5", GL_UNSIGNED_SHORT_5_6_5, 3 },
	{ "GL_UNSIGNED_SHORT_5_6_5_REV", GL_UNSIGNED_SHORT_5_6_5_REV, 3 },
	{ "GL_UNSIGNED_BYTE_3_3_2", GL_UNSIGNED_BYTE_3_3_2, 3 },
	{ "GL_UNSIGNED_BYTE_2_3_3_REV", GL_UNSIGNED_BYTE_2_3_3_REV, 3 }
};

#define NUM_TYPES (sizeof(Types) / sizeof(Types[0]))


static const NameTokenComps Formats[] =
{
	{ "GL_RGBA", GL_RGBA, 4 },
	{ "GL_BGRA", GL_BGRA, 4 },
	{ "GL_RGB", GL_RGB, 3 },
	{ "GL_BGR", GL_BGR, 3 },
#ifdef GL_EXT_abgr
	{ "GL_ABGR_EXT", GL_ABGR_EXT, 4 }
#endif
};

#define NUM_FORMATS (sizeof(Formats) / sizeof(Formats[0]))


static const NameTokenComps InternalFormats[] =
{
	{ "glDrawPixels", 0, 4 },  // special case for glDrawPixels

	{ "3", 3, 3 },
	{ "GL_RGB", GL_RGB, 3 },
	{ "GL_R3_G3_B2", GL_R3_G3_B2, 3 },
	{ "GL_RGB4", GL_RGB4, 3 },
	{ "GL_RGB5", GL_RGB5, 3 },
	{ "GL_RGB8", GL_RGB8, 3 },
	{ "GL_RGB10", GL_RGB10, 3 },
	{ "GL_RGB12", GL_RGB12, 3 },
	{ "GL_RGB16", GL_RGB16, 3 },
#ifdef GL_EXT_texture_sRGB
	{ "GL_SRGB_EXT", GL_SRGB_EXT, 3 },
	{ "GL_SRGB8_EXT", GL_SRGB8_EXT, 3 },
#endif

	{ "GL_RGBA", GL_RGBA, 4 },
	{ "4", 4, 4 },
	{ "GL_RGBA2", GL_RGBA2, 4 },
	{ "GL_RGBA4", GL_RGBA4, 4 },
	{ "GL_RGB5_A1", GL_RGB5_A1, 4 },
	{ "GL_RGBA8", GL_RGBA8, 4 },
	{ "GL_RGB10_A2", GL_RGB10_A2, 4 },
	{ "GL_RGBA12", GL_RGBA12, 4 },
	{ "GL_RGBA16", GL_RGBA16, 4 },
#ifdef GL_EXT_texture_sRGB
	{ "GL_SRGB_ALPHA_EXT", GL_SRGB_ALPHA_EXT, 4 },
	{ "GL_SRGB8_ALPHA8_EXT", GL_SRGB8_ALPHA8_EXT, 4 },
#endif
	// XXX maybe add ALPHA, LUMINANCE, INTENSITY formats someday
	// XXX maybe add compressed formats too...
};

#define NUM_INT_FORMATS (sizeof(InternalFormats) / sizeof(InternalFormats[0]))



// Return four bitmasks indicating which bits correspond to the
// 1st, 2nd, 3rd and 4th components in a packed datatype.
// Set all masks to zero for non-packed types.
static void
ComponentMasks(GLenum datatype, GLuint masks[4])
{
	switch (datatype) {
	case GL_UNSIGNED_BYTE:
	case GL_BYTE:
	case GL_UNSIGNED_SHORT:
	case GL_SHORT:
	case GL_UNSIGNED_INT:
	case GL_INT:
	case GL_FLOAT:
#ifdef GL_ARB_half_float_pixel
	case GL_HALF_FLOAT_ARB:
#endif
		masks[0] =
		masks[1] =
		masks[2] =
		masks[3] = 0x0;
		break;
	case GL_UNSIGNED_INT_8_8_8_8:
		masks[0] = 0xff000000;
		masks[1] = 0x00ff0000;
		masks[2] = 0x0000ff00;
		masks[3] = 0x000000ff;
		break;
	case GL_UNSIGNED_INT_8_8_8_8_REV:
		masks[0] = 0x000000ff;
		masks[1] = 0x0000ff00;
		masks[2] = 0x00ff0000;
		masks[3] = 0xff000000;
		break;
	case GL_UNSIGNED_INT_10_10_10_2:
		masks[0] = 0xffc00000;
		masks[1] = 0x003ff000;
		masks[2] = 0x00000ffc;
		masks[3] = 0x00000003;
		break;
	case GL_UNSIGNED_INT_2_10_10_10_REV:
		masks[0] = 0x000003ff;
		masks[1] = 0x000ffc00;
		masks[2] = 0x3ff00000;
		masks[3] = 0xc0000000;
		break;
	case GL_UNSIGNED_SHORT_5_5_5_1:
		masks[0] = 0xf800;
		masks[1] = 0x07c0;
		masks[2] = 0x003e;
		masks[3] = 0x0001;
		break;
	case GL_UNSIGNED_SHORT_1_5_5_5_REV:
		masks[0] = 0x001f;
		masks[1] = 0x03e0;
		masks[2] = 0x7c00;
		masks[3] = 0x8000;
		break;
	case GL_UNSIGNED_SHORT_4_4_4_4:
		masks[0] = 0xf000;
		masks[1] = 0x0f00;
		masks[2] = 0x00f0;
		masks[3] = 0x000f;
		break;
	case GL_UNSIGNED_SHORT_4_4_4_4_REV:
		masks[0] = 0x000f;
		masks[1] = 0x00f0;
		masks[2] = 0x0f00;
		masks[3] = 0xf000;
		break;
	case GL_UNSIGNED_SHORT_5_6_5:
		masks[0] = 0xf800;
		masks[1] = 0x07e0;
		masks[2] = 0x001f;
		masks[3] = 0;
		break;
	case GL_UNSIGNED_SHORT_5_6_5_REV:
		masks[0] = 0x001f;
		masks[1] = 0x07e0;
		masks[2] = 0xf800;
		masks[3] = 0;
		break;
	case GL_UNSIGNED_BYTE_3_3_2:
		masks[0] = 0xe0;
		masks[1] = 0x1c;
		masks[2] = 0x03;
		masks[3] = 0;
		break;
	case GL_UNSIGNED_BYTE_2_3_3_REV:
		masks[0] = 0x07;
		masks[1] = 0x38;
		masks[2] = 0xc0;
		masks[3] = 0;
		break;
	default:
		abort();
	}
}


// Return four values indicating the ordering of the Red, Green, Blue and
// Alpha components for the given image format.
// For example: GL_BGRA = {2, 1, 0, 3}.
static void
ComponentPositions(GLenum format, GLint pos[4])
{
	switch (format) {
	case GL_RGBA:
		pos[0] = 0;
		pos[1] = 1;
		pos[2] = 2;
		pos[3] = 3;
		break;
	case GL_BGRA:
		pos[0] = 2;
		pos[1] = 1;
		pos[2] = 0;
		pos[3] = 3;
		break;
	case GL_RGB:
		pos[0] = 0;
		pos[1] = 1;
		pos[2] = 2;
		pos[3] = -1;
		break;
	case GL_BGR:
		pos[0] = 2;
		pos[1] = 1;
		pos[2] = 0;
		pos[3] = -1;
		break;
#ifdef GL_EXT_abgr
	case GL_ABGR_EXT:
		pos[0] = 3;
		pos[1] = 2;
		pos[2] = 1;
		pos[3] = 0;
		break;
#endif
	default:
		abort();
	}
}


// Return number components in the given datatype.  This is 3 or 4 for
// packed types and zero for non-packed types
// Ex: GL_UNSIGNED_SHORT_5_5_5_1 = 4
// Ex: GL_INT = 0
static int
NumberOfComponentsInPackedType(GLenum datatype)
{
	for (unsigned i = 0; i < NUM_TYPES; i++) {
		if (Types[i].Token == datatype)
			return Types[i].Components;
	}
	abort();
}


static int
IsPackedType(GLenum datatype)
{
	return NumberOfComponentsInPackedType(datatype) > 0;
}


// Return number components in the given image format.
// Ex: GL_BGR = 3
static int
NumberOfComponentsInFormat(GLenum format)
{
	for (unsigned i = 0; i < NUM_FORMATS; i++) {
		if (Formats[i].Token == format)
			return Formats[i].Components;
	}
	abort();
}


static int
NumberOfComponentsInInternalFormat(GLint intFormat)
{
	for (unsigned i = 0; i < NUM_INT_FORMATS; i++) {
		if (InternalFormats[i].Token == (unsigned) intFormat)
			return InternalFormats[i].Components;
	}
	abort();
}


// Return size, in bytes, of given datatype.
static int
SizeofType(GLenum datatype)
{
	switch (datatype) {
	case GL_UNSIGNED_INT_10_10_10_2:
	case GL_UNSIGNED_INT_2_10_10_10_REV:
	case GL_UNSIGNED_INT_8_8_8_8:
	case GL_UNSIGNED_INT_8_8_8_8_REV:
	case GL_UNSIGNED_INT:
	case GL_INT:
	case GL_FLOAT:
#ifdef GL_ARB_half_float_pixel
	case GL_HALF_FLOAT_ARB:
#endif
		return 4;
	case GL_UNSIGNED_SHORT_5_5_5_1:
	case GL_UNSIGNED_SHORT_1_5_5_5_REV:
	case GL_UNSIGNED_SHORT_4_4_4_4:
	case GL_UNSIGNED_SHORT_4_4_4_4_REV:
	case GL_UNSIGNED_SHORT_5_6_5:
	case GL_UNSIGNED_SHORT_5_6_5_REV:
	case GL_UNSIGNED_SHORT:
	case GL_SHORT:
		return 2;
	case GL_UNSIGNED_BYTE_3_3_2:
	case GL_UNSIGNED_BYTE_2_3_3_REV:
	case GL_UNSIGNED_BYTE:
	case GL_BYTE:
		return 1;
	default:
		abort();
	}
}


// Check if the given image format and datatype are compatible.
// Also check for types/formats defined by GL extensions here.
bool
PixelFormatsTest::CompatibleFormatAndType(GLenum format, GLenum datatype) const
{
	// Special case: GL_BGR can't be used with packed types!
	// This has to do with putting the most color bits in red and green,
	// not blue.
	if (format == GL_BGR && IsPackedType(datatype))
		return false;

#ifdef GL_ARB_half_float_pixel
	if (format == GL_HALF_FLOAT_ARB && !haveHalfFloat)
		return false;
#endif

#ifdef GL_EXT_abgr
	if (format == GL_ABGR_EXT && !haveABGR)
		return false;
#endif

	const int formatComps = NumberOfComponentsInFormat(format);
	const int typeComps = NumberOfComponentsInPackedType(datatype);
	return formatComps == typeComps || typeComps == 0;
}


bool
PixelFormatsTest::SupportedIntFormat(GLint intFormat) const
{
#ifdef GL_EXT_texture_sRGB
	if (intFormat == GL_SRGB_EXT && !haveSRGB)
		return false;
#endif
	return true;
}


// Determine if the ith pixel is in the upper-right quadrant of the
// rectangle of size 'width' x 'height'.
static bool
IsUpperRight(int i, int width, int height)
{
	const int y = i / width, x = i % width;
	return (x >= width / 2 && y >= height / 2);
}



// Create an image buffer and fill it so that a single image channel is
// the max value (1.0) while the other channels are zero.  For example,
// if fillComponent==2 and we're filling a four-component image, the
// pixels will be (0, 0, max, 0).
//
// We always leave the upper-right quadrant black/zero.  This is to help
// detect any image conversion issues related to stride, packing, etc.
static GLubyte *
MakeImage(int width, int height, GLenum format, GLenum type,
		  int fillComponent)
{
	assert(fillComponent < 4);

	if (IsPackedType(type)) {
		const int bpp = SizeofType(type);
		GLubyte *image = new GLubyte [width * height * bpp];
		GLuint masks[4];
		int pos[4];
		int i;

		ComponentMasks(type, masks);
		ComponentPositions(format, pos);

		const GLuint value = masks[fillComponent];

		switch (bpp) {
		case 1:
			for (i = 0; i < width * height; i++) {
				if (IsUpperRight(i, width, height))
					image[i] = 0;
				else
					image[i] = (GLubyte) value;
			}
			break;
		case 2:
			{
				GLushort *image16 = (GLushort *) image;
				for (i = 0; i < width * height; i++) {
					if (IsUpperRight(i, width, height))
						image16[i] = 0;
					else
						image16[i] = (GLushort) value;
				}
			}
			break;
		case 4:
			{
				GLuint *image32 = (GLuint *) image;
				for (i = 0; i < width * height; i++) {
					if (IsUpperRight(i, width, height))
						image32[i] = 0;
					else
						image32[i] = (GLuint) value;
				}
			}
			break;
		default:
			abort();
		}

		return image;
	}
	else {
		const int comps = NumberOfComponentsInFormat(format);
		const int bpp = comps * SizeofType(type);
		assert(bpp > 0);
		GLubyte *image = new GLubyte [width * height * bpp];
		int i;

		switch (type) {
		case GL_UNSIGNED_BYTE:
			for (i = 0; i < width * height * comps; i++) {
				if (i % comps == fillComponent && !IsUpperRight(i/comps, width, height))
					image[i] = 0xff;
				else
					image[i] = 0x0;
			}
			break;
		case GL_BYTE:
			{
				GLbyte *b = (GLbyte *) image;
				for (i = 0; i < width * height * comps; i++) {
					if (i % comps == fillComponent && !IsUpperRight(i/comps, width, height))
						b[i] = 0x7f;
					else
						b[i] = 0x0;
				}
			}
			break;
		case GL_UNSIGNED_SHORT:
			{
				GLushort *us = (GLushort *) image;
				for (i = 0; i < width * height * comps; i++) {
					if (i % comps == fillComponent && !IsUpperRight(i/comps, width, height))
						us[i] = 0xffff;
					else
						us[i] = 0x0;
				}
			}
			break;
		case GL_SHORT:
			{
				GLshort *s = (GLshort *) image;
				for (i = 0; i < width * height * comps; i++) {
					if (i % comps == fillComponent && !IsUpperRight(i/comps, width, height))
						s[i] = 0x7fff;
					else
						s[i] = 0x0;
				}
			}
			break;
		case GL_UNSIGNED_INT:
			{
				GLuint *ui = (GLuint *) image;
				for (i = 0; i < width * height * comps; i++) {
					if (i % comps == fillComponent && !IsUpperRight(i/comps, width, height))
						ui[i] = 0xffffffff;
					else
						ui[i] = 0x0;
				}
			}
			break;
		case GL_INT:
			{
				GLint *in = (GLint *) image;
				for (i = 0; i < width * height * comps; i++) {
					if (i % comps == fillComponent && !IsUpperRight(i/comps, width, height))
						in[i] = 0x7fffffff;
					else
						in[i] = 0x0;
				}
			}
			break;
		case GL_FLOAT:
			{
				GLfloat *f = (GLfloat *) image;
				for (i = 0; i < width * height * comps; i++) {
					if (i % comps == fillComponent && !IsUpperRight(i/comps, width, height))
						f[i] = 1.0;
					else
						f[i] = 0.0;
				}
			}
			break;
#ifdef GL_ARB_half_float_pixel
		case GL_HALF_FLOAT_ARB:
			{
				GLhalfARB *f = (GLhalfARB *) image;
				for (i = 0; i < width * height * comps; i++) {
					if (i % comps == fillComponent && !IsUpperRight(i/comps, width, height))
						f[i] = 0x3c00;  /* == 1.0 */
					else
						f[i] = 0;
				}
			}
			break;
#endif
		default:
			abort();
		}
		return image;
	}
}


bool
PixelFormatsTest::CheckError(const char *where) const
{
	GLint err = glGetError();
	if (err) {
		char msg[1000];
		sprintf(msg, "GL Error: %s (0x%x) in %s\n",
				gluErrorString(err), err, where);
		env->log << msg;
		return true;
	}
	return false;
}


// Draw the given image, either as a texture quad or glDrawPixels.
// Return true for success, false if GL error detected.
bool
PixelFormatsTest::DrawImage(int width, int height,
							GLenum format, GLenum type, GLint intFormat,
							const GLubyte *image)
{
	if (intFormat) {
		glEnable(GL_TEXTURE_2D);
		glViewport(0, 0, width, height);
		glTexImage2D(GL_TEXTURE_2D, 0, intFormat, width, height, 0,
					 format, type, image);
		if (CheckError("glTexImage2D"))
			return false;
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 0);  glVertex2f(-1, -1);
		glTexCoord2f(1, 0);  glVertex2f(1, -1);
		glTexCoord2f(1, 1);  glVertex2f(1, 1);
		glTexCoord2f(0, 1);  glVertex2f(-1, 1);
		glEnd();
		glDisable(GL_TEXTURE_2D);
	}
	else {
		// glDrawPixels
		glDrawPixels(width, height, format, type, image);
		if (CheckError("glDrawPixels"))
			return false;
	}
	return true;
}


static bool
ColorsEqual(const GLubyte img[4], const GLubyte expected[4])
{
	const int tolerance = 1;
	if ((abs(img[0] - expected[0]) > tolerance) ||
		(abs(img[1] - expected[1]) > tolerance) ||
		(abs(img[2] - expected[2]) > tolerance) ||
		(abs(img[3] - expected[3]) > tolerance)) {
		return false;
	}
	else {
		return true;
	}
}


// Read framebuffer and check that region [width x height] is the expected
// solid color, except the upper-right quadrant will always be black/zero.
// colorChan: 0 = red, 1 = green, 2 = blue, 3 = alpha.
// format is the color format we're testing.
bool
PixelFormatsTest::CheckRendering(int width, int height, int colorChan, GLenum format, GLint intFormat) const
{
	const int comps = NumberOfComponentsInFormat(format);
	const int intComps = NumberOfComponentsInInternalFormat(intFormat);
	const int checkAlpha = alphaBits > 0 && comps == 4 && intComps == 4;
	GLubyte *image = new GLubyte [width * height * 4];
	GLboolean ok = 1;
	GLubyte expected[4];
	int i;

	assert(colorChan >= 0 && colorChan < 4);

	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image);
	for (i = 0; i < width * height; i += 4) {
		// determine which pixel color we're looking for
		expected[0] =
		expected[1] =
		expected[2] =
		expected[3] = 0;
		if (!IsUpperRight(i/4, width, height)) {
			expected[colorChan] = 0xff;
		}
		if (!checkAlpha) {
			expected[3] = 0xff;
		}

		// do the color check
		if (!ColorsEqual(image+i, expected)) {
			ok = false;
#if DEBUG
			char msg[1000];
			sprintf(msg, "for color %d at (%d,%d):  expected: 0x%x 0x%x 0x%x 0x%x  "
				"got: 0x%x 0x%x 0x%x 0x%x\n", colorChan, i/width, i%width,
				expected[0], expected[1], expected[2], expected[3],
				image[i + 0], image[i + 1], image[i + 2], image[i + 3]);
			env->log << msg;
#endif
			break;
		}
	}
	delete [] image;
	return ok;
}



// Exercise a particular combination of image format, type and internal
// texture format.
// Return true for success, false for failure.
bool
PixelFormatsTest::TestCombination(GLenum format, GLenum type, GLint intFormat)
{
	const int width = 16;
	const int height = 16;

	int colorPos[4];
	ComponentPositions(format, colorPos);

	for (int color = 0; color < 4; color++) {
		if (colorPos[color] >= 0) {
			// make test incoming image
			const int comp = colorPos[color];
			GLubyte *image = MakeImage(width, height, format, type, comp);

			// render with image (texture / glDrawPixels)
			bool ok = DrawImage(width, height, format, type, intFormat, image);

			if (ok) {
				// check rendering
				ok = CheckRendering(width, height, color, format, intFormat);
			}

			delete [] image;

			if (!ok) {
				return false;
			}
		}
	}

	return true;
}


// Per visual setup.
void
PixelFormatsTest::setup(void)
{
	haveHalfFloat = GLUtils::haveExtensions("GL_ARB_half_float_pixel");
	haveABGR = GLUtils::haveExtensions("GL_EXT_abgr");
	haveSRGB = GLUtils::haveExtensions("GL_EXT_texture_sRGB");

	glGetIntegerv(GL_ALPHA_BITS, &alphaBits);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glDrawBuffer(GL_FRONT);
	glReadBuffer(GL_FRONT);
}



// Test all possible image formats, types and internal texture formats.
// Result will indicate number of passes and failures.
void
PixelFormatsTest::runOne(PixelFormatsResult &r, Window &w)
{
	(void) w;  // silence warning

	setup();

	r.numPassed = 0;
	r.numFailed = 0;

	for (unsigned formatIndex = 0; formatIndex < NUM_FORMATS; formatIndex++) {
		for (unsigned typeIndex = 0; typeIndex < NUM_TYPES; typeIndex++) {

			if (CompatibleFormatAndType(Formats[formatIndex].Token,
										Types[typeIndex].Token)) {

				for (unsigned intFormat = 0; intFormat < NUM_INT_FORMATS; intFormat++) {

					if (!SupportedIntFormat(InternalFormats[intFormat].Token))
						continue;

#if DEBUG
					env->log << "testing:\n";
					env->log << "  Format:    " << Formats[formatIndex].Name << "\n";
					env->log << "  Type:      " << Types[typeIndex].Name << "\n";
					env->log << "  IntFormat: " << InternalFormats[intFormat].Name << "\n";

#endif
					bool ok = TestCombination(Formats[formatIndex].Token,
																		Types[typeIndex].Token,
																		InternalFormats[intFormat].Token);

					if (!ok) {
						// report error
						env->log << name << " Failure:\n";
						env->log << "  Format: " << Formats[formatIndex].Name << "\n";
						env->log << "  Type: " << Types[typeIndex].Name << "\n";
						env->log << "  Internal Format: " << InternalFormats[intFormat].Name << "\n";

						r.numFailed++;
					}
					else {
						r.numPassed++;
					}
				}
			}
		}
	}

	if (r.numFailed > 0)
		r.pass = false;
	else
		r.pass = true;
}


void
PixelFormatsTest::logOne(PixelFormatsResult &r)
{
	logPassFail(r);
	env->log << ": "
			 << r.numPassed << " combinations passed, "
			 << r.numFailed << " combinations failed.\n";
	logConcise(r);
}


void
PixelFormatsTest::compareOne(PixelFormatsResult &oldR,
			    PixelFormatsResult &newR)
{
	if (oldR.pass && newR.pass) {
		if (oldR.numPassed != newR.numPassed ||
			oldR.numFailed != newR.numFailed) {
			env->log << "Different results: passed: "
					 << oldR.numPassed
					 << " vs. "
					 << newR.numPassed
					 << "  failed: "
					 << oldR.numFailed
					 << " vs. "
					 << newR.numFailed
					 << "\n";
		}
	}
}


void
PixelFormatsResult::putresults(ostream &s) const
{
	s << pass;
	s << numPassed;
	s << numFailed;
}


bool
PixelFormatsResult::getresults(istream &s)
{
	s >> pass;
	s >> numPassed;
	s >> numFailed;
	return s.good();
}


// The test object itself:
PixelFormatsTest pixelFormatsTest("pixelFormats", "window, rgb",
				"",
	"Test that all the various pixel formats/types (like\n"
	"GL_BGRA/GL_UNSIGNED_SHORT_4_4_4_4_REV) operate correctly.\n"
	"Test both glTexImage and glDrawPixels.\n"
	"For textures, also test all the various internal texture formats.\n"
	"Well over a thousand combinations are possible!\n"
	);




} // namespace GLEAN
