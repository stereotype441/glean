// BEGIN_COPYRIGHT -*- linux-c -*-
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

// tpgos.cpp:  implementation of example class for pgos tests
//
// glPolygonOffset test code donated to the glean project by
// Angus Dorbie <dorbie@sgi.com>, Thu, 07 Sep 2000 04:13:45 -0700
//
// glPolygonOffset test code integrated into glean framework by
// Rickard E. (Rik) Faith <faith@valinux.com>, October 2000

#ifdef __UNIX__
#include <unistd.h>
#endif

#include <iostream>
#include <fstream>
#include "dsconfig.h"
#include "dsfilt.h"
#include "dsurf.h"
#include "winsys.h"
#include "environ.h"
#include "rc.h"
#include "glutils.h"
#include "tpgos.h"
#include "misc.h"
#include "image.h"

namespace {
const int WIN_SIZE         = 100; // Maximum window width/height.
const int DISP_LIST_SPHERE = 1;

static void
create_lists(void)
{
	float a, b;
	int face;
	// These subdiv numbers must be unequal to test the offset.
	int subdiv1 = 50;
	float microres1;
	float x1, y1, z1;
	float x2, y2, z2;
	float scale;

	x1 = y1 = z1 = x2 = y2 = z2 = 0.0;
	
	microres1 = (float)(1.0/(double)subdiv1);

	// Generate faces of sphere by normalizing cube with face
	// subdivision.
	glNewList(DISP_LIST_SPHERE, GL_COMPILE);
	for(face = 0; face < 6; face++) {
		for(a = -1.0f; a < 0.999f; a+= microres1) {
			glBegin(GL_TRIANGLE_STRIP);
			for(b = -1.0f; b < 1.001f; b+= microres1) {
				switch(face) {
				case 0:
					x1 = a; y1 =  b; z1 = 1.0f;
					x2 = a+microres1; y2 =  b; z2 = 1.0f;
					break;
				case 1:
					x1 = a; y1 =  b; z1 = -1.0f;
					x2 = a+microres1; y2 =  b; z2 = -1.0f;
					break;
				case 2:
					x1 = 1.0f; y1 =  b; z1 = a;
					x2 = 1.0f; y2 =  b; z2 = a+microres1;
					break;
				case 3:
					x1 = -1.0f; y1 =  b; z1 = a;
					x2 = -1.0f; y2 =  b; z2 = a+microres1;
					break;
				case 4:
					x1 = a; y1 =  1.0f; z1 = b;
					x2 = a+microres1; y2 =  1.0f; z2 = b;
					break;
				case 5:
					x1 = a; y1 =  -1.0f; z1 = b;
					x2 = a+microres1; y2 =  -1.0f; z2 = b;
					break;
				}
				scale = sqrtf(x1 * x1 + y1 * y1 + z1 * z1);
				x1 /= scale; y1 /=  scale; z1 /=  scale;
				scale = sqrtf(x2 * x2 + y2 * y2 + z2 * z2);
				x2 /=  scale; y2 /=  scale; z2 /=  scale;
				
				glVertex3f(x1, y1, z1);
				glVertex3f(x2, y2, z2);
			}
			glEnd();
		}
	}
	glEndList();
} // create_lists

static void
draw_spheres(float factor, float units,
	     bool punchthrough_mode, float displace)
	     
{
	float Cred[] =   {1.0f, 0.0f, 0.0f, 1.0f};
	float Cgreen[] = {0.0f, 1.0f, 0.0f, 1.0f};

	if(punchthrough_mode) {
		// Test for punchthrough.
		glColor3fv(Cgreen);
		glPushMatrix();
		glTranslatef(0.0f, 0.0f, displace);
		glCallList(DISP_LIST_SPHERE);
		glPopMatrix();
		glColor3fv(Cred);
	} else {
		// Test for zfighting.
		glColor3fv(Cred);
		glCallList(DISP_LIST_SPHERE);
		glColor3fv(Cgreen);
	}
	
	glRotatef(45.0f, 1.0, 0.0f, 0.0f);
	glRotatef(45.0f, 0.0, 1.0f, 0.0f);
	glPolygonOffset(factor, units);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glCallList(DISP_LIST_SPHERE);
	glDisable(GL_POLYGON_OFFSET_FILL);
	
	glFinish();
} // draw_spheres

static void
project_and_draw(float factor, float units,
		 float near, float far,
		 bool punchthrough_mode, float punchthrough_translation)
{
	float translate_away, frust_edge;

	// Compute reasonable translation to the back of the frustum.
	translate_away = far - 2.0f - far * 0.1f;
	
	// Compute frustum edge to include sphere (radius = 1) by
	// projecting sphere radius plus a smidgen onto near clip.
	// Should leave a few pixels around the sphere.
	frust_edge = near * 1.05/(translate_away - punchthrough_translation);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-frust_edge, frust_edge, -frust_edge, frust_edge, near, far);
	
	// Translate to far clip and draw.
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	
	glTranslatef(0.0f, 0.0f, -translate_away);
	draw_spheres(factor, units,
		     punchthrough_mode, punchthrough_translation);
	
	glPopMatrix();
} // project_and_draw

static void
verify(GLEAN::Window& w, bool& badEdge, bool& badMiddle)
{
	bool      passed;
	const int edge = 10;
	badEdge        = false;
	badMiddle      = false;
	
        GLEAN::Image img(WIN_SIZE, WIN_SIZE, GL_RGB, GL_UNSIGNED_BYTE);
        img.read(0, 0);
        w.swap();       // Give the user something to watch.

	GLubyte* row0 = reinterpret_cast<GLubyte*>(img.pixels());
        GLubyte* row1 = row0 + img.rowSizeInBytes();

	GLubyte* row = row1;
        for (int i = 0; i < WIN_SIZE-1; ++i) {
		bool     found_center = false;
                GLubyte* p            = row;
		enum { left_b, left_e, center, righte, rightb } state = left_b;

                for (int j = 0; j < WIN_SIZE-1; ++j) {
			enum { gry, red, grn } color          = gry;

			if (p[0] > 64 && p[1] < 64 && p[2] < 64) color = red;
			if (p[0] < 64 && p[1] > 64 && p[2] < 64) color = grn;

			passed = true;
			
			switch (state) {
			case left_b: // Left grey border
				switch (color) {
				case gry: state = left_b;               break;
				case red: state = left_e;               break;
				case grn: state = center;               break;
				}
				break;
			case left_e: // Left red edge
				switch (color) {
				case gry: state = rightb;               break;
				case red: state = left_e;               break;
				case grn: state = center;               break;
				}
				break;
			case center:
				switch (color) {
				case gry: state = rightb;               break;
				case red: state = righte;               break;
				case grn: state = center;               break;
				}
				break;
			case righte:
				switch (color) {
				case gry: state = rightb;               break;
				case red: state = righte;               break;
				case grn: state = center; passed=false; break;
				}
				break;
			case rightb:
				switch (color) {
				case gry: state = rightb;               break;
				case red: state = righte; passed=false; break;
				case grn: state = center; passed=false; break;
				}
				break;
			}

			if (state == center) found_center = true;

			if (!passed) {
				if (i < edge || i > WIN_SIZE-edge
				    || j < edge || j > WIN_SIZE-edge)
					badEdge = true;
				else
					badMiddle = true;
			}

#if 0
			// Must pass env when debugging.
			env->log << i << ","
				 << j << ": "
				 << int(p[0]) << ","
				 << int(p[1]) << ","
				 << int(p[2]) << "=> "
				 << passed << "\n";
#endif

                        p += 3;
                }
		if (i > edge && i <= WIN_SIZE - edge && !found_center)
			badMiddle = true;

                row += img.rowSizeInBytes();
        }
} // verify

// Complex results helper functions

void
doTitle(const GLEAN::POResult& r, int t, GLEAN::Environment* env)
{
	if (t >= r.testcount) {
		env->log << "Test " << t << " (out of range)";
		return;
	}
	
	env->log << "Test " << t
		 << " (f=" << r.data[t].factor
		 << ",u=" << r.data[t].units << ')'
		 << " (n=" << r.data[t].near << ",f=" << r.data[t].far << ')';
	if (r.data[t].punchthrough)
		env->log << " PT("
			 << r.data[t].punchthrough_translation << ')';
	else
		env->log << " ZF";
} // doTitle

void
logStats1(GLEAN::POResult& r, int t, GLEAN::Environment* env) {
        env->log << '\t';
	doTitle(r, t, env);
	env->log << ": ";
	if (r.data[t].badEdge || r.data[t].badMiddle) {
		env->log << (r.data[t].badEdge   ? " Edge.": "")
			 << (r.data[t].badMiddle ? " Middle.\n": "\n");
	} else {
		env->log << " PASS.\n";
	}
} // logStats1

void
diffHeader(bool& same, const string& name,
	   GLEAN::DrawingSurfaceConfig* config, GLEAN::Environment* env) {
        if (same) {
                same = false;
                env->log << name << ":  DIFF "
			 << config->conciseDescription() << '\n';
        }
} // diffHeader

void
failHeader(bool& pass, const string& name,
	   GLEAN::DrawingSurfaceConfig* config, GLEAN::Environment* env) {
        if (pass) {
                pass = false;
                env->log << name << ":  FAIL "
			 << config->conciseDescription() << '\n';
        }
} // failHeader

void
doComparison(const GLEAN::POResult& oldR,
	     const GLEAN::POResult& newR,
	     GLEAN::DrawingSurfaceConfig* config,
	     bool& same,
	     const string& name,
	     GLEAN::Environment* env)
{
	int count = (oldR.testcount < newR.testcount
		     ? oldR.testcount
		     : newR.testcount);

	for (int i = 0; i < count; i++) {
		if (newR.data[i].badEdge != oldR.data[i].badEdge) {
			diffHeader(same, name, config, env);
			env->log << '\t' << env->options.db1Name << ' ';
			doTitle(oldR, i, env);
			env->log << ": "
				 << (oldR.data[i].badEdge ? "" : "NO")
				 << " failure at edge\n";
			env->log << '\t' << env->options.db2Name << ' ';
			doTitle(newR, i, env);
			env->log << ": "
				 << (newR.data[i].badEdge ? "" : "NO")
				 << " failure at edge\n";
		}
		if (newR.data[i].badMiddle != oldR.data[i].badMiddle) {
			diffHeader(same, name, config, env);
			env->log << '\t' << env->options.db1Name << ' ';
			doTitle(oldR, i, env);
			env->log << ": "
				 << (oldR.data[i].badMiddle ? "" : " NO")
				 << " failure in middle\n";
			env->log << '\t' << env->options.db2Name << ' ';
			doTitle(newR, i, env);
			env->log << ": "
				 << (newR.data[i].badMiddle ? "" : " NO")
				 << " failure in middle\n";
		}
	}
} // doComparison


} // anonymous namespace

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// Constructors/Destructor:
///////////////////////////////////////////////////////////////////////////////
PgosTest::PgosTest(const char* aName, const char* aFilter,
		   const char* anExtensionList, const char* aDescription):
    	Test(aName), filter(aFilter), extensions(anExtensionList),
	description(aDescription) {
} // PgosTest::PgosTest()

PgosTest::PgosTest(const char* aName, const char* aFilter,
		   const char* aDescription):
    	Test(aName), filter(aFilter), extensions(0),
	description(aDescription) {
} // PgosTest::PgosTest()

PgosTest::~PgosTest() {
} // PgosTest::~PgosTest

///////////////////////////////////////////////////////////////////////////////
// run: run tests, save results in a vector and in the results file
///////////////////////////////////////////////////////////////////////////////
void
PgosTest::run(Environment& environment) {
	// Guard against multiple invocations:
	if (hasRun)
		return;

	// Set up environment for use by other functions:
	env = &environment;

	// Document the test in the log, if requested:
	logDescription();

	// Compute results and make them available to subsequent tests:
	WindowSystem& ws = env->winSys;
	try {
		// Open the results file:
		OutputStream os(*this);

		// Select the drawing configurations for testing:
		DrawingSurfaceFilter f(filter);
		vector<DrawingSurfaceConfig*>configs(f.filter(ws.surfConfigs));

		// Test each config:
		for (vector<DrawingSurfaceConfig*>::const_iterator
			     p = configs.begin(); p < configs.end(); ++p) {
			Window w(ws, **p, WIN_SIZE, WIN_SIZE);
			RenderingContext rc(ws, **p);
			if (!ws.makeCurrent(rc, w))
				;	// XXX need to throw exception here

			// Check for all prerequisite extensions.  Note
			// that this must be done after the rendering
			// context has been created and made current!
			if (!GLUtils::haveExtensions(extensions))
				continue;

			// Create a result object and run the test:
			Result* r = new Result();
			r->config = *p;
			runOne(*r, w);
			
			// Save the result locally and in the results file:
			results.push_back(r);
			r->put(os);
		}
	}
	catch (DrawingSurfaceFilter::Syntax e) {
		env->log << "Syntax error in test's drawing-surface selection"
			"criteria:\n'" << filter << "'\n";
		for (int i = 0; i < e.position; ++i)
			env->log << ' ';
		env->log << "^ " << e.err << '\n';
	}
	catch (RenderingContext::Error) {
		env->log << "Could not create a rendering context\n";
	}

	env->log << '\n';

	// Note that we've completed the run:
	hasRun = true;
}

void
PgosTest::compare(Environment& environment) {
	// Save the environment for use by other member functions:
	env = &environment;

	// Display the description if needed:
	logDescription();

	// Read results from previous runs:
	Input1Stream is1(*this);
	vector<Result*> oldR(getResults(is1));
	Input2Stream is2(*this);
	vector<Result*> newR(getResults(is2));
	
	// Construct a vector of surface configurations from the old run.
	// (Later we'll find the best match in this vector for each config
	// in the new run.)
	vector<DrawingSurfaceConfig*> oldConfigs;
	for (vector<Result*>::const_iterator p = oldR.begin(); p < oldR.end();
	     ++p)
		oldConfigs.push_back((*p)->config);

	// Compare results:
	for (vector<Result*>::const_iterator newP = newR.begin();
	     newP < newR.end(); ++newP) {

	    	// Find the drawing surface config that most closely matches
		// the config for this result:
		int c = (*newP)->config->match(oldConfigs);

		// If there was a match, compare the results:
		if (c < 0)
			env->log << name
				 << ":  NOTE no matching config for "
				 << (*newP)->config->conciseDescription()
				 << '\n';
		else
			compareOne(*(oldR[c]), **newP);
	}

	// Get rid of the results; we don't need them for future comparisons.
	for (vector<Result*>::iterator np = newR.begin(); np < newR.end();
	     ++np)
		delete *np;
	for (vector<Result*>::iterator op = oldR.begin(); op < oldR.end();
	     ++op)
		delete *op;
}

///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////
void
PgosTest::runOne(Result& r, GLEAN::Window& w) {
	int depthbits[4];
	
	glDepthFunc(GL_LESS); // This is the right test, especially
                              // for checking bias.
	glDepthRange(0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	glGetIntegerv(GL_DEPTH_BITS, depthbits);

#if 0
	// Could base punchthrough on depth precision...
	if(depthbits[0] > 22) {
	}
#endif

	create_lists();

	glClearColor(0.5,0.5,0.5,0);
	
        // Clear both front and back buffers and swap, to avoid confusing
        // this test with results of the previous test:
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        w.swap();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (int i = 0; /**/; i++) {
		// Test for three main cases of near and far values:
		//      (near,far) = case 1: (1.0,   100.0)
		//                   case 2: (1.0,  1000.0)
		//                   case 3: (1.5, 10000.0)
		//
		// For the punchtrhough tests, reasonable translation values
		// are selected.  This is fairly arbitrary, but the test
		// should be 'soft'.  Setting the near close to the distant
		// far clip is quite demanding on punchthrough for most
		// implementations.  Punchthrough failure on case 1 is worse
		// than failure on case 2 is worse than failure on case 3.
		//
		// Failure in the middle of the sphere is worse than failure
		// at the edges.
		//
		// Some red fragments may be correctly visible on the VERY
		// edge of the sphere.
		//
		// Future enhancement: vary the punchthrough_translation
		// based on visual bits obtained.
		struct {
			bool valid;
			float factor, units, near, far;
			bool  pt;
			float ptt;
		} params[] = {
			// Add new tests at the BOTTOM
			{ true,  0.0, -1.0, 1.0,   100.0, false, 0.0 },
			{ true,  0.0, -1.0, 1.0,   100.0, false, 0.0 },
			{ true,  0.0, -1.0, 1.0,  1000.0, false, 0.0 },
			{ true,  0.0, -1.0, 1.5, 10000.0, false, 0.0 },
			{ true, -1.0, -1.0, 1.0,   100.0, false, 0.0 },
			{ true, -1.0, -1.0, 1.0,  1000.0, false, 0.0 },
			{ true, -1.0, -1.0, 1.5, 10000.0, false, 0.0 },
			{ true, -1.0, -1.0, 1.0,   100.0, true,  0.1 },
			{ true, -1.0, -1.0, 1.0,  1000.0, true,  1.0 },
			{ true, -1.0, -1.0, 1.5, 10000.0, true, 15.0 },
			{ false, 0, 0, 0, 0, false, 0 } // Must be last entry!
		};
		if (!params[i].valid) break; // End of list

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		project_and_draw(params[i].factor, params[i].units,
				 params[i].near, params[i].far,
				 params[i].pt, params[i].ptt);

		bool badEdge   = false;
		bool badMiddle = false;
		
		verify(w, badEdge, badMiddle);
		
		r.po.addtest(params[i].factor, params[i].units,
			     params[i].near, params[i].far,
			     params[i].pt, params[i].ptt,
			     badEdge, badMiddle);

	}
	
	logStats(r, env);
} // PgosTest::runOne

///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
PgosTest::compareOne(Result& oldR, Result& newR) {
	bool same = true;

	doComparison(oldR.po, newR.po, newR.config, same, name, env);

	if (env->options.verbosity) {
		if (same)
			env->log << name
				 << ":  SAME "
				 << newR.config->conciseDescription()
				 << "\n";
		else
			env->log << name
				 << ":  DIFF "
				 << newR.config->conciseDescription()
				 << "\n";
	} else if (!same) {
		env->log << name
			 << ":  DIFF "
			 << newR.config->conciseDescription()
			 << '\n'
			 << '\t'
			 << env->options.db1Name
			 << (oldR.pass ? " PASS, " : " FAIL, ")
			 << env->options.db2Name
			 << (newR.pass ? " PASS\n" : " FAIL\n");
	}
	
	if (oldR.po.testcount > newR.po.testcount)
		env->log << "\tWARNING: "
			 << "First test has more subtests than second test\n";
	if (oldR.po.testcount < newR.po.testcount)
		env->log << "\tWARNING: "
			 << "Second test has more subtests than first test\n";

	if (env->options.verbosity) {
		env->log << env->options.db1Name << ':';
		logStats(oldR, env);
		env->log << env->options.db2Name << ':';
		logStats(newR, env);
	}
} // PgosTest::compareOne

///////////////////////////////////////////////////////////////////////////////
// logDescription:  Print description on the log file, according to the
//	current verbosity level.
///////////////////////////////////////////////////////////////////////////////
void
PgosTest::logDescription() {
	if (env->options.verbosity)
		env->log <<
"----------------------------------------------------------------------\n"
			 << description
			 << '\n';
} // PgosTest::logDescription

///////////////////////////////////////////////////////////////////////////////
// Result I/O functions:
///////////////////////////////////////////////////////////////////////////////
void
PgosTest::Result::put(ostream& s) const {
	s << config->canonicalDescription() << '\n';
	po.put(s);
} // PgosTest::Result::put

bool
PgosTest::Result::get(istream& s) {
	SkipWhitespace(s);
	string configDesc;
	if (!getline(s, configDesc))
		return false;
	config = new DrawingSurfaceConfig(configDesc);
	po.get(s);

	return s.good();
} // PgosTest::Result::get

vector<PgosTest::Result*>
PgosTest::getResults(istream& s) {
	vector<Result*> v;
	while (s.good()) {
		Result* r = new Result();
		if (r->get(s))
			v.push_back(r);
		else {
			delete r;
			break;
		}
	}

	return v;
} // PgosTest::getResults

void
PgosTest::logStats(PgosTest::Result& r, GLEAN::Environment* env) {
	bool pass = true;
	for (int i = 0; i < r.po.testcount; i++)
		if (r.po.data[i].badMiddle || r.po.data[i].badEdge)
			pass = false;
	env->log << name
		 << ":  "
		 << (pass ? "PASS " : "FAIL ")
		 << r.config->conciseDescription()
		 << '\n';
	for (int i = 0; i < r.po.testcount; i++) logStats1(r.po, i, env);
} // OrthoPosPoints::logStats

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
PgosTest
pgosTest("pgos", "window",
	 "This test verifies glPolygonOffset.  It is run on every\n"
	 "OpenGL-capable drawing surface configuration that supports\n"
	 "creation of a window.\n\n"
	 "When factor=0 or -1 and units=-1, all the zfighting tests should\n"
	 "pass.  Some red fragments may be correctly visible on the very\n"
	 "edge of the sphere -- these fragments are not reported as an\n"
	 "error.  Failure in the middle of the sphere is worse than failure\n"
	 "at the edges.\n\n"
         "The tests for punchthrough are more demanding, especially when\n"
	 "the near clip is close to the distant far clip.  Punchthrough\n"
	 "failure on the first case (near=1,far=100) is worse than on the\n"
	 "second case (near=1,far=1000) is worse than on the third case\n"
	 "(near=1.5,far=10000).\n"
	);

} // namespace GLEAN
