// geomutil.cpp:  frequently-used geometric operations

#include "geomutil.h"
#include "rand.h"

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


} // namespace GLEAN
