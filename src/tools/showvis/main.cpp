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




#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <string>
#include "glwrap.h"
#include "dsconfig.h"
#include "dsfilt.h"

char* mandatoryArg(int argc, char* argv[], int i);
void usage(char* appName);

///////////////////////////////////////////////////////////////////////////////
// showvis - display list of visuals (X11 Visuals, GLX FBConfigs, or
//	Win32 PixelFormatDescriptors) matching user-supplied criteria
///////////////////////////////////////////////////////////////////////////////
int
main(int argc, char* argv[]) {
	string criteria;
	bool canonical = false;

#	if defined(__X11__)
		char* displayName = NULL;
#	endif

	for (int i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "--help")) {
			usage(argv[0]);
		} else if (!strcmp(argv[i], "-c")
		    || !strcmp(argv[i], "--canonical")) {
			canonical = true;
#	    if defined(__X11__)
		} else if (!strcmp(argv[i], "-display")) {
			++i;
			displayName = mandatoryArg(argc, argv, i);
#	    endif
		} else {
			// Concatenate all non-option arguments, separating
			// them with commas, to form a single string of
			// criteria:
			if (criteria.size())
				criteria += ",";
			criteria += argv[i];
		}
	}

	// By default, display all visuals:
	if (criteria.size() == 0)
		criteria = "1";


#	if defined(__X11__)
#		if defined(GLX_VERSION_1_3)
			// XXX Need to write GLX 1.3 FBConfig code
			// Yeah, yeah, it shouldn't be hard.  I just don't
			// have any way to test it yet.
			goto legacyMethod;
#		endif

legacyMethod:
		Display* dpy = XOpenDisplay(displayName);

		if (!dpy) {
			cerr << "can't open display " << XDisplayName(displayName) << "\n";
			exit(2);
		}

		// Get the list of raw XVisualInfo structures:
		XVisualInfo t;
		t.screen = DefaultScreen(dpy);
		int n;
		XVisualInfo* vi =
			XGetVisualInfo(dpy, VisualScreenMask, &t, &n);

		// Construct a vector of DrawingSurfaceConfigs corresponding
		// to the XVisualInfo structures that indicate they support
		// OpenGL:
		vector<GLEAN::DrawingSurfaceConfig*> glxv;
		for (int i = 0; i < n; ++i) {
			int supportsOpenGL;
			glXGetConfig(dpy, &vi[i], GLX_USE_GL, &supportsOpenGL);
			if (supportsOpenGL)
				glxv.push_back(new GLEAN::DrawingSurfaceConfig
					(dpy, &vi[i]));
		}

		// Create a configuration filter and apply it:
		try {
			GLEAN::DrawingSurfaceFilter f(criteria);
			vector<GLEAN::DrawingSurfaceConfig*> v(f.filter(glxv));

			for(vector<GLEAN::DrawingSurfaceConfig*>::const_iterator
			    p = v.begin(); p < v.end(); ++p)
				cout << (canonical? (*p)->canonicalDescription()
					: (*p)->conciseDescription())
					<< "\n";

			exit (v.size() == 0);
		}
		catch (GLEAN::DrawingSurfaceFilter::Syntax e) {
			cerr << "Syntax error:\n'" << criteria << "'\n";
			for (int i = 0; i < e.position; ++i)
				cerr << ' ';
			cerr << "^ " << e.err << "\n";
			exit(2);
		}


#	elif defined(__WIN__)
		HDC hDC = GetDC(GetDesktopWindow());

		int n = DescribePixelFormat(hDC,0,sizeof(PIXELFORMATDESCRIPTOR),0);
	
		PIXELFORMATDESCRIPTOR *pfd = new PIXELFORMATDESCRIPTOR[n+1];
		
		// Construct a vector of DrawingSurfaceConfigs corresponding
		// to all available pixelformats
		vector<GLEAN::DrawingSurfaceConfig*> glxv;
		for (int j = 1;j <= n;j++) {
			DescribePixelFormat(hDC,j,sizeof(PIXELFORMATDESCRIPTOR),&pfd[j]);
			glxv.push_back(new GLEAN::DrawingSurfaceConfig(j,&pfd[j]));
		}

		ReleaseDC(GetDesktopWindow(),hDC);

		// Create a configuration filter and apply it:
		try {
			GLEAN::DrawingSurfaceFilter f(criteria);
			vector<GLEAN::DrawingSurfaceConfig*> v(f.filter(glxv));

			for(vector<GLEAN::DrawingSurfaceConfig*>::const_iterator
			    p = v.begin(); p < v.end(); ++p)
				cout << (canonical? (*p)->canonicalDescription()
					: (*p)->conciseDescription())
					<< "\n";

			exit (v.size() == 0);
		}
		catch (GLEAN::DrawingSurfaceFilter::Syntax e) {
			cerr << "Syntax error:\n'" << criteria << "'\n";
			for (int i = 0; i < e.position; ++i)
				cerr << ' ';
			cerr << "^ " << e.err << "\n";
			exit(2);
		}

		return 0;
		
#	elif defined(__AGL__)
		GDHandle	mainGD;
		GLint		attrib2[] 	= { AGL_RGBA, AGL_DOUBLEBUFFER, AGL_ACCELERATED, 16, AGL_NONE };
		GLint		attrib[] 	= { AGL_RGBA, AGL_DOUBLEBUFFER, AGL_DEPTH_SIZE, 32, AGL_NONE };
		GLint		attrib3[] 	= { AGL_RGBA, AGL_DOUBLEBUFFER, AGL_NONE };
		GLint       swAttrib[] 	= { AGL_RGBA, AGL_RENDERER_ID, AGL_RENDERER_GENERIC_ID, AGL_DOUBLEBUFFER, AGL_DEPTH_SIZE, 16, AGL_NONE };
		AGLPixelFormat	pf; 
	
		mainGD = GetMainDevice();
	
		// Construct a vector of DrawingSurfaceConfigs corresponding to the
		// returned pixel formats
		vector<GLEAN::DrawingSurfaceConfig*> glpf;
	
		pf = aglChoosePixelFormat(&mainGD, 1, attrib2);
		if (pf != NULL) glpf.push_back(new GLEAN::DrawingSurfaceConfig (1, pf));

/*
		pf = aglChoosePixelFormat(&mainGD, 1, attrib2);
		if (pf != NULL) glpf.push_back(new GLEAN::DrawingSurfaceConfig (2, pf));
		pf = aglChoosePixelFormat(&mainGD, 1, attrib3);
		if (pf != NULL) glpf.push_back(new GLEAN::DrawingSurfaceConfig (3, pf));
		pf = aglChoosePixelFormat(&mainGD, 1, swAttrib);
		if (pf != NULL) 
			glpf.push_back(new GLEAN::DrawingSurfaceConfig (4, pf));
		else
		{
			swAttrib[2] = 0x30300;
			pf = aglChoosePixelFormat(&mainGD, 1, swAttrib);
			if (pf != NULL) 
				glpf.push_back(new GLEAN::DrawingSurfaceConfig (4, pf));
			else
			{
				swAttrib[2] = 0x30200;
				pf = aglChoosePixelFormat(&mainGD, 1, swAttrib);
				if (pf != NULL) glpf.push_back(new GLEAN::DrawingSurfaceConfig (4, pf));
			}
		}
*/
		// Create a configuration filter and apply it:
		try {
			GLEAN::DrawingSurfaceFilter f(criteria);
			vector<GLEAN::DrawingSurfaceConfig*> v(f.filter(glpf));

			for(vector<GLEAN::DrawingSurfaceConfig*>::const_iterator
			    p = v.begin(); p < v.end(); ++p)
				cout << (canonical? (*p)->canonicalDescription()
					: (*p)->conciseDescription())
					<< "\n";

			exit (v.size() == 0);
		}
		catch (GLEAN::DrawingSurfaceFilter::Syntax e) {
			cerr << "Syntax error:\n'" << criteria << "'\n";
			for (int i = 0; i < e.position; ++i)
				cerr << ' ';
			cerr << "^ " << e.err << "\n";
			exit(2);
		}

		return 0;
		
#	endif
}

///////////////////////////////////////////////////////////////////////////////
// mandatoryArg - fetch a required argument from the command line;
//	if it isn't present, print a usage message and die.
///////////////////////////////////////////////////////////////////////////////

char*
mandatoryArg(int argc, char* argv[], int i) {
	if (i < argc && argv[i][0] != '-')
		return argv[i];
	usage(argv[0]);
	/*NOTREACHED*/
	return 0;
} // mandatoryArg

///////////////////////////////////////////////////////////////////////////////
// usage - print usage message, then die
///////////////////////////////////////////////////////////////////////////////
void
usage(char* appName) {
	cerr << "Usage:  " << appName << "[options] {selection-expression}\n"
"\n"
"options:\n"
#if defined(__X11__)
"       -display X11-display-name  # select X11 display to use\n"
#elif defined(__WIN__)
#endif
"       --help                     # display usage information\n"
"       (-c|--canonical)           # display visuals in canonical form,\n"
"                                  # rather than abbreviated form\n"
"\n"
"selection-expression:\n"
"       Selection expressions are C expressions involving the usual\n"
"       arithmetic and logical operators, plus variables that correspond\n"
"       to attributes of OpenGL-capable visuals (pixel formats).\n"
"       Operators are:\n"
"               ! -(unary) + - * / % == != < <= > >= && || ( )\n"
"       Variables are:\n"
"               r             red channel size\n"
"               g             green channel size\n"
"               b             blue channel size\n"
"               a             alpha channel size\n"
"               rgb           minimum of r, g, and b\n"
"               rgba          minimum of r, g, b, and a\n"
"               ci            color index channel size\n"
"               accumr        accumulation buffer red channel size\n"
"               accumg        accumulation buffer green channel size\n"
"               accumb        accumulation buffer blue channel size\n"
"               accuma        accumulation buffer alpha channel size\n"
"               accumrgb      minimum of accumr, accumg, and accumb\n"
"               accumrgba     minimum of accumr, accumg, accumb, and accuma\n"
"               aux           number of auxiliary color buffers\n"
"               db            nonzero if visual is double-buffered\n"
"               sb            nonzero if visual is single-buffered\n"
"               id            system-dependent ID number of visual\n"
"               fbcid         framebuffer configuration ID (GLX 1.3 only)\n"
"               level         <0 if underlay; ==0 if main; >0 if overlay\n"
"               main          nonzero if visual is in main planes\n"
"               overlay       nonzero if visual is in overlay planes\n"
"               underlay      nonzero if visual is in underlay planes\n"
"               mono          nonzero if visual is monoscopic\n"
"               stereo        nonzero if visual is stereoscopic\n"
"               ms            number of multisamples\n"
"               s             stencil buffer size\n"
"               z             depth (Z) buffer size\n"
"               fast          nonzero if visual is accelerated\n"
"               conformant    nonzero if visual conforms to OpenGL spec\n"
"               transparent   nonzero if visual has a transparent pixel value\n"
"               transr        transparent pixel red value (RGBA visuals only)\n"
"               transg        transparent pixel green value\n"
"               transb        transparent pixel blue value\n"
"               transa        transparent pixel alpha value\n"
"               transci       transparent pixel color index value (CI visuals\n"
"                             only)\n"
"               window        nonzero if visual can be used to create windows\n"
#if defined(__X11__)
"               pixmap        nonzero if visual can be used to create pixmaps\n"
#if defined(GLX_VERSION_1_3)
"               pbuffer       nonzero if visual can be used to create pbuffers\n"
#endif
#endif
"               glonly        nonzero if visual can be used only for OpenGL\n"
"                             rendering\n"
"\n"
"       Selection expressions may also include sort keys, which consist of\n"
"       ``max'' or ``min'' followed by a variable.  When multiple visuals\n"
"       meeting the selection criteria are found, those with the largest\n"
"       (for max) or smallest (for min) value of the given variable are\n"
"       listed first.\n"
"\n"
"       Finally, multiple selection expressions may be provided as separate\n"
"       arguments or separated by commas; in either case, only visuals that\n"
"       meet all the selection expressions are returned.\n"
"\n"
"       Exit status is 0 (normal) if visuals were found, 1 if no matching\n"
"       visuals were found, and 2 if any error occurred\n"
"\n"
"Examples:\n"
"       " << appName << "\n"
"               display all OpenGL-capable visuals\n"
"       " << appName << " db rgb\n"
"               display all double-buffered RGB or RGBA visuals\n"
"       " << appName << " 'r != g || g != b'\n"
"               display all visuals for which the color channels are not all\n"
"               the same size\n"
"       " << appName << " 'max rgba, fast, conformant, accumrgba >= 2*rgba'\n"
"               display all visuals that are accelerated and OpenGL\n"
"               conformant, and for which the accumulation buffer is at\n"
"               least twice as deep as the normal color buffer, sorted with\n"
"               the deepest color buffer first\n"
"       " << appName << " 'r%2 || g%2 || b%2 '\n"
"               display all visuals for which at least one color channel\n"
"               has an odd size\n"
"       " << appName << " 'min ci'\n"
"               display color index visuals, smallest first\n";

	exit(2);
}
