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




// misc.cpp:  implementation of miscellaneous functions

#include <cctype>
#include "misc.h"

namespace GLEAN {


///////////////////////////////////////////////////////////////////////////////
// Utility routine to skip whitespace in streams
//	This is helpful when interleaving invocations of getline() and
//	operator>>.  In particular, after operator>> at the end of a line,
//	there may be whitespace (especially a newline) remaining; this may
//	confuse a subsequent invocation of getline().
///////////////////////////////////////////////////////////////////////////////
void
SkipWhitespace(istream& s) {
	char c;
	while (s.get(c)) {
		if (!isspace(c)) {
			s.putback(c);
			break;
		}
	}
} // SkipWhitespace


} // namespace GLEAN
