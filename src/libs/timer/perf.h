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




// perf.h:  Simple benchmark timing utilities based on timer.h
// Modified from timer.h by Rickard E. (Rik) Faith <faith@valinux.com>

// Timer objects provide a framework for measuring the rate at which an
// operation can be performed.


#ifndef __perf_h__
#define __perf_h__


namespace GLEAN {

class Perf {
	double overhead;	// Overhead (in seconds) of initial op,
				// final op, and timer access.

	double chooseRunTime();	// Select a runtime that will reduce random
				// timing error to an acceptable level.

    public:
	virtual void preop()  = 0;
	virtual void op()     = 0;
	virtual void postop() = 0;
	void         calibrate();
	double       time();
	double       getClock();    // Get wall-clock time, in seconds
	double       waitForTick(); // Wait for next clock tick; return time

	
	Perf() {overhead = 0.0;}
}; // class Perf

} // namespace GLEAN

#endif // __perf_h__
