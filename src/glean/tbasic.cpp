// BEGIN_COPYRIGHT -*- linux-c -*-
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




// tbasic.cpp:  implementation of example class for basic tests

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
#include "tbasic.h"
#include "misc.h"

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////
void
BaseTest<BasicResult>::runOne(BasicResult& r) {
	r.pass = true;
} // BasicTest::runOne

void
BaseTest<BasicResult>::logOne(BasicResult& r) {
	env->log << name
		 << (r.pass? ":  PASS ": ":  FAIL ")
		 << r.config->conciseDescription()
		 << '\n';
}

///////////////////////////////////////////////////////////////////////////////
// compareOne:  Compare results for a single test case
///////////////////////////////////////////////////////////////////////////////
void
BaseTest<BasicResult>::compareOne(BasicResult& oldR, BasicResult& newR) {
	if (oldR.pass == newR.pass) {
		if (env->options.verbosity)
			env->log << name << ":  SAME "
				<< newR.config->conciseDescription() << '\n'
				<< (oldR.pass? "\tBoth PASS\n": "\tBoth FAIL\n")
				;
	} else {
		env->log << name << ":  DIFF "
			<< newR.config->conciseDescription() << '\n'
			<< '\t' << env->options.db1Name
			<<	(oldR.pass? " PASS, ": " FAIL, ")
			<< env->options.db2Name
			<<	(newR.pass? " PASS\n": " FAIL\n");
	}
} // BasicTest::compareOne

///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
BasicTest basicTest("basic", "window",
	"This trivial test simply verifies the internal support for basic\n"
	"tests.  It is run on every OpenGL-capable drawing surface\n"
	"configuration that supports creation of a window.\n");

} // namespace GLEAN
