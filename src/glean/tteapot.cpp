// BEGIN_COPYRIGHT
// 
// Copyright (C) 2000  Adam Haberlach   All Rights Reserved.
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
#include "tteapot.h"
#include "misc.h"
#include "timer.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

namespace GLEAN {

float depthOfView = 30.0;
float zRatio = 10.0;

	float position[] = {0.0, 3.0, 3.0, 0.0};
	float position1[] = {-3.0, -3.0, 3.0, 0.0};
	float position2[] = {3.0, 0.0, 0.0, 0.0};
	float local_view[] = {0.0,0.0};
	float ambient[] = {0.1745, 0.03175, 0.03175};
	float diffuse[] = {0.61424, 0.10136, 0.10136};
	float specular[] = {0.727811, 0.626959, 0.626959};
//	rgb_color bg_black = {0,0,0,255};

enum lights {
	lightNone = 0,
	lightWhite,
	lightYellow,
	lightRed,
	lightBlue,
	lightGreen
};


float white[3] = {1.0,1.0,1.0};
float dimWhite[3] = {0.25,0.25,0.25};
float black[3] = {0.0,0.0,0.0};
float foggy[3] = {0.4,0.4,0.4};
float blue[3] = {0.0,0.0,1.0};
float dimBlue[3] = {0.0,0.0,0.5};
float yellow[3] = {1.0,1.0,0.0};
float dimYellow[3] = {0.5,0.5,0.0};
float green[3] = {0.0,1.0,0.0};
float dimGreen[3] = {0.0,0.5,0.0};
float red[3] = {1.0,0.0,0.0};

float *bgColor = black;

struct light {
	float *ambient;
	float *diffuse;
	float *specular;
};

light lights[] = {
	{NULL,NULL,NULL},
	{dimWhite,white,white},
	{dimWhite,yellow,yellow},
	{dimWhite,red,red},
	{dimWhite,blue,blue},
	{dimWhite,green,green}
};

struct material {
	float ambient[3],diffuse[3],specular[3];
};

float *colors[] =
{
	NULL,white,yellow,blue,red,green
};


material materials[] = {
	// Null
	{
		{0.1745, 0.03175, 0.03175},
		{0.61424, 0.10136, 0.10136},
		{0.727811, 0.626959, 0.626959}
	},
	// White
	{
		{0.1745, 0.1745, 0.1745},
		{0.61424, 0.61424, 0.61424},
		{0.727811, 0.727811, 0.727811}
	},
	// Yellow
	{
		{0.1745, 0.1745, 0.03175},
		{0.61424, 0.61424, 0.10136},
		{0.727811, 0.727811, 0.626959}
	},
	// Blue
	{
		{0.03175, 0.03175, 0.1745},
		{0.10136, 0.10136, 0.61424},
		{0.626959, 0.626959, 0.727811}
	},
	// Red
	{
		{0.1745, 0.03175, 0.03175},
		{0.61424, 0.10136, 0.10136},
		{0.727811, 0.626959, 0.626959}
	},
	// Green
	{
		{0.03175, 0.1745, 0.03175},
		{0.10136, 0.61424, 0.10136},
		{0.626959, 0.727811, 0.626959}
	},
};

///////////////////////////////////////////////////////////////////////////////
// Constructor/Destructor:
///////////////////////////////////////////////////////////////////////////////
TeapotTest::TeapotTest(const char* aName, const char* aFilter, const char* aDescription):
    Test(aName), filter(aFilter), description(aDescription),
	triangles(100,100),
	qs(50,50),
	fWidth(300),
	fHeight(315),
	fInitFailed(false)

 {

	float maxp=0;
	int numPt,numTri;

	FILE *f = fopen("teapot.data","r");
	if (!f) {
		fInitFailed = true;
		return;
	}
	fscanf(f,"%d",&numPt);

//	cout << numPt << " Points: ";
	vertexArrayData = new point[numPt];
	for (int i=0;i<numPt;i++) {
//		cout << ".";
		point p;
		fscanf(f,"%f %f %f %f %f %f",
			&p.x,
			&p.y,
			&p.z,
			&p.nx,
			&p.ny,
			&p.nz);
		if (fabs(p.x) > maxp)
			maxp = fabs(p.x);
		if (fabs(p.y) > maxp)
			maxp = fabs(p.y);
		if (fabs(p.z) > maxp)
			maxp = fabs(p.z);
		vertexArrayData[i] = p;
	};
	for (int i=0;i<numPt;i++) {
		vertexArrayData[i].x /= maxp;
		vertexArrayData[i].y /= maxp;
		vertexArrayData[i].z /= maxp;
	};
//	cout << "\n";

		
	fscanf(f,"%d",&numTri);
//	cout << numTri << " Triangles: ";

	int tpts=0;
	for (int i=0;i<numTri;i++) {
//		cout << ".";
		tri t;
		fscanf(f,"%d %d %d",
			&t.p1,
			&t.p2,
			&t.p3);
		triangles.add(t);
		tpts+=3;
	};

	int qpts=4;
	int qp[1024];
	quadStrip q;
	q.pts = qp;
	q.numpts = 4;
	q.pts[2] = triangles[0].p1;
	q.pts[0] = triangles[0].p2;
	q.pts[1] = triangles[0].p3;
	q.pts[3] = triangles[1].p3;

	for (int i=2;i<numTri;i+=2) {
		if ((triangles[i-1].p1 == triangles[i].p2) &&
			(triangles[i-1].p3 == triangles[i].p3)) {
			q.pts[q.numpts++] = triangles[i+1].p1;
			q.pts[q.numpts++] = triangles[i+1].p3;
			qpts+=2;
		} else {
			int *np = (int*)malloc(sizeof(int)*q.numpts);
			memcpy(np,qp,q.numpts*sizeof(int));
			quadStrip nqs;
			nqs.numpts = q.numpts;
			nqs.pts = np;
			qs.add(nqs);
		
			qpts+=4;
			q.numpts = 4;
			q.pts[2] = triangles[i].p1;
			q.pts[0] = triangles[i].p2;
			q.pts[1] = triangles[i].p3;
			q.pts[3] = triangles[i+1].p3;
		};
	};

	int *np = (int*)malloc(sizeof(int)*q.numpts);
	memcpy(np,qp,q.numpts*sizeof(int));
	quadStrip nqs;
	nqs.numpts = q.numpts;
	nqs.pts = np;
	qs.add(nqs);
	fclose(f);
} // TeapotTest::TeapotTest()

TeapotTest::~TeapotTest() {
} // TeapotTest::~TeapotTest()


void
TeapotTest::run(Environment& environment) {
	// Guard against multiple invocations:
	if (hasRun)
		return;

	if (fInitFailed) {
		printf("Couldn't start test (do you have teapot.data in the current directory?\n");
		return;
	}

	// Set up environment for use by other functions:
	env = &environment;

	// Document the test in the log, if requested:
	logDescription();

	// Compute results and make them available to subsequent tests:
	WindowSystem& ws = env->winSys;
	try {
		// Open the results file:
		OutputStream os(*this);

		DrawingSurfaceFilter f(filter);
		vector<DrawingSurfaceConfig*> configs(f.filter(ws.surfConfigs));

		// Test each config:
		for (vector<DrawingSurfaceConfig*>::const_iterator
		    p = configs.begin(); p < configs.end(); ++p) {
			Window w(ws, **p, fWidth, fHeight);
			RenderingContext rc(ws, **p);
			if (!ws.makeCurrent(rc, w))
				printf("Couldn't capture window\n");
				;	// XXX need to throw exception here

			// Create a result object and run the test:
			Result *r = new Result;
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
TeapotTest::compare(Environment& environment) {
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
			env->log << name << ":  NOTE no matching config for " <<
				(*newP)->config->conciseDescription() << '\n';
		else
			compareOne(*(oldR[c]), **newP);
	}

	// Get rid of the results; we don't need them for future comparisons.
	for (vector<Result*>::iterator op = newR.begin(); op < newR.end(); ++op)
		delete *op;
	for (vector<Result*>::iterator np = oldR.begin(); np < oldR.end(); ++np)
		delete *np;
}

///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////
void
TeapotTest::runOne(Result& res, GLEAN::Window& w) {

	glCullFace(GL_BACK);
	glDepthFunc(GL_LESS);

	
//	glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, local_view);

	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lights[lightWhite].specular);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,lights[lightWhite].diffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT,lights[lightWhite].ambient);

	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT1, GL_POSITION, position2);
	glLightfv(GL_LIGHT1, GL_SPECULAR, lights[lightBlue].specular);
	glLightfv(GL_LIGHT1, GL_DIFFUSE,lights[lightBlue].diffuse);
	glLightfv(GL_LIGHT1, GL_AMBIENT,lights[lightBlue].ambient);
		
	glFrontFace(GL_CW);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);

//	glEnable(GL_AUTO_NORMAL);
//	glEnable(GL_NORMALIZE);
	
	glMaterialf(GL_FRONT, GL_SHININESS, 0.6*128.0);

	glClearColor(bgColor[0],bgColor[1],bgColor[2], 1.0);
	glColor3f(1.0, 1.0, 1.0);
		
	glViewport(0, 0, (GLint)fWidth, (GLint)fHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	const float scale=1.0;

	glOrtho(-scale, scale, -scale, scale, -scale*depthOfView, scale*depthOfView);
////////////////////////////////// End of Viewport Set-up /////////////////////

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	int color = 4;
	float c[3][4];
	c[0][0] = materials[color].ambient[0];
	c[0][1] = materials[color].ambient[1];
	c[0][2] = materials[color].ambient[2];
	c[1][0] = materials[color].diffuse[0];
	c[1][1] = materials[color].diffuse[1];
	c[1][2] = materials[color].diffuse[2];
	c[2][0] = materials[color].specular[0];
	c[2][1] = materials[color].specular[1];
	c[2][2] = materials[color].specular[2];
	
	const int solidity = 0;
	float alpha;
	if (solidity == 0)
		alpha = 1.0;
	else if (solidity == 1)
		alpha = 0.95;
	else if (solidity == 2)
		alpha = 0.6;
	c[0][3] = c[1][3] = c[2][3] = alpha;

	if (solidity != 0) {
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glDisable(GL_CULL_FACE);
	} else {
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
	}
	
	glMaterialfv(GL_FRONT, GL_AMBIENT, c[0]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, c[1]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, c[2]);

///////////////////////// End of materials set-up //////////////////////

	glInterleavedArrays( GL_N3F_V3F, 0, vertexArrayData );
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );


	Timer tTimer;
	double start = tTimer.getClock();

	const int startX = 0;
	const int endX = 360;

	for (int rotX=0; rotX < 360; rotX++) {
		glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT );
		glPushMatrix();
		glRotatef(rotX, 1.0,0.0,0.0);
		glRotatef(rotX, 0.0,1.0,0.0);

		for (int i=0;i<qs.num_items;i++) {
			glBegin(GL_QUAD_STRIP);
			for (int j=0;j<qs[i].numpts;j++) {
				glArrayElement( qs[i].pts[j] );
			};
			glEnd();
		}
		w.swap();
		glPopMatrix();
	}
	
	double finish = tTimer.getClock();
	
	res.fTps = (endX - startX) / (finish - start);
	res.pass = true;

	env->log << name << (res.pass? ":  PASS ": ":  FAIL ")
		<< "Teapots/Sec: " << res.fTps << "\n"
		<< res.config->conciseDescription() << '\n';

} // BasicTest::runOne

///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
TeapotTest::compareOne(Result& oldR, Result& newR) {
	if (oldR.pass == newR.pass) {
		if (env->options.verbosity)
			env->log << name << ":  SAME "
				<< newR.config->conciseDescription() << '\n'
				<< (oldR.pass? "\tBoth PASS\n": "\tBoth FAIL\n")
				<< "\tTeapots Comparison: " << oldR.fTps << " vs. " << newR.fTps 
				;
	} else {
		env->log << name << ":  DIFF "
			<< newR.config->conciseDescription() << '\n'
			<< '\t' << env->options.db1Name
			<<	(oldR.pass? " PASS, ": " FAIL, ")
			<< env->options.db2Name
			<<	(newR.pass? " PASS\n": " FAIL\n")
			<< "\tTeapots Comparison: " << oldR.fTps << " vs. " << newR.fTps
			;
	}
} // BasicTest::compareOne

///////////////////////////////////////////////////////////////////////////////
// logDescription:  Print description on the log file, according to the
//	current verbosity level.
///////////////////////////////////////////////////////////////////////////////
void
TeapotTest::logDescription() {
	if (env->options.verbosity)
		env->log <<
"----------------------------------------------------------------------\n"
		<< description << '\n';
} // BasicTest::logDescription

///////////////////////////////////////////////////////////////////////////////
// Result I/O functions:
///////////////////////////////////////////////////////////////////////////////
void
TeapotTest::Result::put(ostream& s) const {
	s << config->canonicalDescription() << '\n' << pass << '\n';
	
	s << fTps << '\n';
} // BasicTest::Result::put

bool
TeapotTest::Result::get(istream& s) {
	SkipWhitespace(s);
	string configDesc;
	if (!getline(s, configDesc))
		return false;
	config = new DrawingSurfaceConfig(configDesc);
	s >> pass;
	s >> fTps;
		
	return s.good();
} // BasicTest::Result::get

vector<TeapotTest::Result*>
TeapotTest::getResults(istream& s) {
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
} // BasicTest::getResults


///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
TeapotTest teapotTest("Teapot", "window",
	"This test simply displays a teapot, rotates it, and attempts to\n"
	"determine the frame/sec the pipeline can generate\n");

} // namespace GLEAN
