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




// timer.cpp:  Implementation of simple benchmark timer utilities.

// This particular implementation is derived from the one in libpdb, part
// of the isfast library for OpenGL.

// XXXWIN as of 5/8/99:  The code for Windows timing is taken from
// Michael Gold's implementation of libpdb.  I've probably introduced
// some bugs in the translation, unfortunately.

#include "timer.h"

#if defined(__UNIX__)
#    include <sys/time.h>	// for gettimeofday, used by getClock
#elif defined(__MS__)
#    include <windows.h>
#    include <sys/types.h>
#    include <sys/timeb.h>	// for _ftime(), used by getClock
#endif

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// calibrate:  Determine overhead of measurement, initialization routine,
//	and finalization routine
///////////////////////////////////////////////////////////////////////////////
void
Timer::calibrate(void (*initialize)(), void (*finalize)()) {
	double runTime = chooseRunTime();

	if (initialize)
		(*initialize)();

	long reps = 0;
	double current;
	double start = waitForTick();
	do {
		if (finalize)
			(*finalize)();
		++reps;
	} while ((current = getClock()) < start + runTime);

	overhead = (current - start) / (double) reps;
} // Timer::calibrate

///////////////////////////////////////////////////////////////////////////////
// chooseRunTime:  Select an appropriate runtime for benchmarks.
//	By running for at least 10000 ticks, and attempting to keep timing
//	accurate to one tick, we hope to make our results repeatable.
//	(ignoring all the other stuff that might be going on in	the system,
//	of course).  Long runs reduce the effect of measurement error, but
//	short runs reduce the chance that some other process on the system
//	will steal time.
///////////////////////////////////////////////////////////////////////////////
double
Timer::chooseRunTime() {
	double start = getClock();
	double finish;

	// Wait for next tick:
	while ((finish = getClock()) == start)
		;
	
	// Run for 10000 ticks, clamped to [0.1 sec, 5.0 sec]:
	double runTime = 10000.0 * (finish - start);
	if (runTime < 0.1)
		runTime = 0.1;
	else if (runTime > 5.0)
		runTime = 5.0;

	return runTime;
}

///////////////////////////////////////////////////////////////////////////////
// getClock - get current wall-clock time (expressed in seconds)
///////////////////////////////////////////////////////////////////////////////
double
Timer::getClock() {
#if defined(__MS__)
	static int once = 1;
	static double freq;

	if (once) {
	    LARGE_INTEGER fr;
	    freq = (double) (QueryPerformanceFrequency(&fr) ?
			     1.0 / fr.QuadPart : 0);
	    once = 0;
	}

	// Use high-resolution counter, if available
	if (freq) {
	    LARGE_INTEGER pc;
	    QueryPerformanceCounter(&pc);
	    return freq * (double) pc.QuadPart;
	} else {
	    struct _timeb t;

	    _ftime(&t);

	    return (double) t.time + (double) t.millitm * 1E-3;
	}
#elif defined(__UNIX__)
	struct timeval t;

	// XXX gettimeofday is different on SysV, if I remember correctly
	gettimeofday(&t, 0);

	return (double) t.tv_sec + (double) t.tv_usec * 1E-6;
#endif
}

///////////////////////////////////////////////////////////////////////////////
// waitForTick:  wait for beginning of next system clock tick; return the time.
///////////////////////////////////////////////////////////////////////////////
double
Timer::waitForTick() {
	double start;
	double current;

	start = getClock();

	// Wait for next tick:
	while ((current = getClock()) == start)
		;

	// Start timing:
	return current;
}

///////////////////////////////////////////////////////////////////////////////
// time:  measure time (in seconds) to perform caller's operation
///////////////////////////////////////////////////////////////////////////////
double
Timer::time(
    void (*initialize)(),
    void (*operation)(),
    void (*finalize)()) {

	if (!operation)
		return 0.0;

	// Select a run time that's appropriate for our timer resolution:
	double runTime = chooseRunTime();

	// Measure successively larger batches of operations until we find
	// one that's long enough to meet our runtime target:
	long reps = 1;
	double start;
	double current;
	for (;;) {
		if (initialize)
			(*initialize)();

		start = waitForTick();

		for (long i = reps; i > 0; --i)
			(*operation)();

		if (finalize)
			(*finalize)();

		current = getClock();
		if (current >= start + runTime + overhead)
			break;

		// Try to reach runtime target in one fell swoop:
		long newReps;
		if (current > start + overhead)
			newReps = static_cast<long> (reps *
			       (0.5 + runTime / (current - start - overhead)));
		else
			newReps = reps * 2;
		if (newReps == reps)
			reps += 1;
		else
			reps = newReps;
		}

	// Subtract overhead to determine the final operation rate:
	return (current - start - overhead) / reps;
} // Timer::time

} // namespace GLEAN
