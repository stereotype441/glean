// BEGIN_COPYRIGHT -*- glean -*-

/*
 * Copyright © 2006 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *	Wei Wang <wei.z.wang@intel.com>
 *
 */


// toccluqry.h:  Test basic ARB_occlusion_query support.

#ifndef __toccluqry_h__
#define __toccluqry_h__

#include "tmultitest.h"

namespace GLEAN {

class OccluQryTest: public MultiTest {
    public:
	OccluQryTest(const char* testName, const char* filter,
		    const char *prereqs, const char* description):
	    MultiTest(testName, filter, prereqs, description) {
	}

	virtual void runOne(MultiTestResult& r, Window& w);
   protected:
	int conformOQ_GetObjivAval_multi1(GLuint id);
	int conformOQ_GetObjivAval_multi2();
	int conformOQ_GetQry_CnterBit();
	int conformOQ_Begin_unused_id();
	int conformOQ_EndAfter(GLuint id);
	int conformOQ_GenIn(GLuint id);
	int conformOQ_BeginIn(GLuint id);
	int conformOQ_DeleteIn(GLuint id);
	int conformOQ_GetObjAvalIn(GLuint id);
	int conformOQ_GetObjResultIn(GLuint id);
	int conformOQ_GetObjivAval(GLuint id);
	int conformOQ_Gen_Delete(unsigned int id_n);
	int conformOQ_IsIdZero(void);
	int conformOQ_BeginIdZero(void);
   private:
	void gen_box(GLfloat left, GLfloat right, GLfloat top, GLfloat btm);
	GLuint find_unused_id();
	int chk_ext( void );
	void reportPassFail(MultiTestResult &r, int pass, const char *msg) const;
};

} // namespace GLEAN

#endif
