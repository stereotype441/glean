// BEGIN_COPYRIGHT
// 
// Copyright (C) 1999,2000  Allen Akin   All Rights Reserved.
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



// geomutil.cpp:  frequently-used geometric operations

#include "geomutil.h"
#include "rand.h"
#include <algorithm>
#include <cmath>
#include <float.h>

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// RandomMesh2D:  Generate 2D array with fixed boundaries but interior points
//	that have been perturbed randomly.
///////////////////////////////////////////////////////////////////////////////
RandomMesh2D::RandomMesh2D(float minX, float maxX, int xPoints,
    float minY, float maxY, int yPoints,
    RandomDouble& rand) {
    	m = new float[xPoints * yPoints * 2];
	rowLength = xPoints;

	for (int iy = 0; iy < yPoints; ++iy) {
		double sum = 0.0;
		int ix;
		for (ix = 0; ix < xPoints; ++ix) {
			(*this)(iy, ix)[0] = sum;
			sum += rand.next();
		}
		double scale = (maxX - minX) / (*this)(iy, xPoints - 1)[0];
		for (ix = 0; ix < xPoints; ++ix) {
			float* x = (*this)(iy, ix) + 0;
			*x = scale * *x + minX;
		}
	}

	for (int ix = 0; ix < xPoints; ++ix) {
		double sum = 0.0;
		int iy;
		for (iy = 0; iy < yPoints; ++iy) {
			(*this)(iy, ix)[1] = sum;
			sum += rand.next();
		}
		double scale = (maxY - minY) / (*this)(yPoints - 1, ix)[1];
		for (iy = 0; iy < yPoints; ++iy) {
			float* y = (*this)(iy, ix) + 1;
			*y = scale * *y + minY;
		}
	}
} // RandomMesh2D::RandomMesh2D

RandomMesh2D::~RandomMesh2D() {
	delete[] m;
} // RandomMesh2D::~RandomMesh2D



///////////////////////////////////////////////////////////////////////////////
// SpiralStrip2D: Generate (x,y) vertices for a triangle strip of arbitrary
//	length.  The triangles are of approximately equal size, and arranged
//	in a spiral so that a reasonably large number of triangles can be
//	packed into a small screen area.
///////////////////////////////////////////////////////////////////////////////
SpiralStrip2D::SpiralStrip2D(int nPoints, float minX, float maxX,
    float minY, float maxY) {

	// Most of the complexity of this code results from attempting
	// to keep the triangles approximately equal in area.
	//
	// Conceptually, we construct concentric rings whose inner and
	// outer radii differ by a constant.  We then split each ring
	// (at theta == 0), and starting from the point of the split
	// gradually increase both the inner and outer radii so that
	// when we've wrapped all the way around the ring (to theta ==
	// 2*pi), the inner radius now matches the original outer
	// radius.  We then repeat the process with the next ring
	// (working from innermost to outermost) until we've
	// accumulated enough vertices to satisfy the caller's
	// requirements.
	//
	// Finally, we scale and offset all the points so that the
	// resulting spiral fits comfortably within the rectangle
	// provided by the caller.

	// Set up the array of vertices:
	v = new float[2 * nPoints];
	float* lastV = v + 2 * nPoints;

	// Initialize the ring parameters:
	double innerRadius = 4.0;
	double ringWidth = 1.0;
	double segLength = 1.0;

	float* pV = v;
	while (pV < lastV) {
		// Each ring consists of segments.  We'll make the arc
		// length of each segment that lies on the inner
		// radius approximately equal to segLength, but we'll
		// adjust it to ensure there are an integral number of
		// equal-sized segments in the ring.
		int nSegments = static_cast<int>
			(6.2831853 * innerRadius / segLength + 0.5);

		double dTheta = 6.2831853 / nSegments;
		double dRadius = ringWidth / nSegments;

		double theta = 0.0;
		for (int i = 0; i < nSegments; ++i) {
			double c = cos(theta);
			double s = sin(theta);

			*pV++ = innerRadius * c;
			*pV++ = innerRadius * s;
			if (pV >= lastV)
				break;

			*pV++ = (innerRadius + ringWidth) * c;
			*pV++ = (innerRadius + ringWidth) * s;
			if (pV >= lastV)
				break;

			theta += dTheta;
			innerRadius += dRadius;
		}
	}

	// Find the bounding box for the spiral:
	float lowX = FLT_MAX;
	float highX = - FLT_MAX;
	float lowY = FLT_MAX;
	float highY = -FLT_MAX;
	for (pV = v; pV < lastV; pV += 2) {
		lowX = min(lowX, pV[0]);
		highX = max(highX, pV[0]);
		lowY = min(lowY, pV[1]);
		highY = max(highY, pV[1]);
	}

	// Find scale and offset to map the spiral into the bounds supplied
	// by our caller, with a little bit of margin around the edges:
	lowX -= ringWidth;
	highX += ringWidth;
	lowY -= ringWidth;
	highY += ringWidth;
	float scaleX = (maxX - minX) / (highX - lowX);
	float offsetX = minX - scaleX * lowX;
	float scaleY = (maxY - minY) / (highY - lowY);
	float offsetY = minY - scaleY * lowY;

	// Finally scale and offset the constructed vertices so that
	// they fit in the caller-supplied rectangle:
	for (pV = v; pV < lastV; pV += 2) {
		pV[0] = scaleX * pV[0] + offsetX;
		pV[1] = scaleY * pV[1] + offsetY;
	}
} // SpiralStrip2D::SpiralStrip2D

SpiralStrip2D::~SpiralStrip2D() {
	delete[] v;
} // SpiralStrip2D::~SpiralStrip2D




///////////////////////////////////////////////////////////////////////////////
// SpiralTri2D:  Generate (x,y) vertices for a set of independent triangles,
//	arranged in spiral fashion as in SpiralStrip2D
///////////////////////////////////////////////////////////////////////////////
SpiralTri2D::SpiralTri2D(int nTris, float minX, float maxX,
    float minY, float maxY) {
	SpiralStrip2D ts(nTris + 2, minX, maxX, minY, maxY);
	const int nVertices = 3 * nTris;
	v = new float[2 * nVertices];

	float* pTris = v;
	float* pStrip = ts(0);
	bool front = false;	// winding order alternates in strip
	for (int i = 0; i < nTris; ++i) {
		if (front) {
			pTris[0] = pStrip[0];
			pTris[1] = pStrip[1];

			pTris[2] = pStrip[2];
			pTris[3] = pStrip[3];

			pTris[4] = pStrip[4];
			pTris[5] = pStrip[5];
		} else {
			pTris[0] = pStrip[0];
			pTris[1] = pStrip[1];

			pTris[2] = pStrip[4];
			pTris[3] = pStrip[5];

			pTris[4] = pStrip[2];
			pTris[5] = pStrip[3];
		}

		front = !front;
		pTris += 6;
		pStrip += 2;
	}
} // SpiralTri2D::SpiralTri2D

SpiralTri2D::~SpiralTri2D() {
	delete[] v;
} // SpiralTri2D::~SpiralTri2D

} // namespace GLEAN
