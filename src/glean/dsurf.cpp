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




// dsurf.cpp:  implementation of drawing surface utilities

#include <iostream>
#include <algorithm>
#include "dsurf.h"
#include "dsconfig.h"
#include "winsys.h"

namespace {

#if defined(__X11__)

Colormap
ChooseColormap(Display* dpy, XVisualInfo* vi) {
	// We could be polite here and search for a standard colormap,
	// but the normal mode of operation should be that glean is
	// running alone, so there doesn't seem to be much point in sharing.

	return XCreateColormap(dpy, RootWindow(dpy, vi->screen),
		vi->visual, AllocNone);
} // ChooseColormap

#endif

} // anonymous namespace

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////
DrawingSurface::DrawingSurface(WindowSystem& ws, DrawingSurfaceConfig& c) {
	// Link back to enclosing window system, so as to simplify bookkeeping:
	winSys = &ws;
	ws.surfaces.push_back(this);

	// Save pointer to configuration information:
	config = &c;
} // DrawingSurface::DrawingSurface

Window::Window(WindowSystem& ws, DrawingSurfaceConfig& c, int w, int h):
    DrawingSurface(ws, c) {

#if defined(__X11__)

#if defined(GLX_VERSION_1_3)

#error "XXX Need GLX 1.3 window-creation code"

#else

	// XXX There's basically no error-handling code here.
	// See XErrorHandler().

	// Create the window:
	XSetWindowAttributes xswa;
	xswa.background_pixmap = None;
	xswa.border_pixel = 0;
	xswa.colormap = ChooseColormap(winSys->dpy, config->vi);
	xswa.event_mask = StructureNotifyMask;
	xWindow = XCreateWindow(winSys->dpy,
		RootWindow(winSys->dpy, config->vi->screen),
		0, 0, w, h,
		0,
		config->vi->depth,
		InputOutput,
		config->vi->visual,
		CWBackPixmap|CWBorderPixel|CWColormap|CWEventMask,
		&xswa);

	// Set attributes for the benefit of the window manager:
	XSizeHints sizeHints;
	sizeHints.width = w;
	sizeHints.height = h;
	sizeHints.x = 10;
	sizeHints.y = 10;
	sizeHints.flags = USSize | USPosition;
	XSetStandardProperties(winSys->dpy, xWindow, "glean", "glean",
		None, 0, 0, &sizeHints);

	// Map the window and wait for it to appear:
	XMapWindow(winSys->dpy, xWindow);
	XEvent event;
	for (;;) {
		XNextEvent(winSys->dpy, &event);
		if (event.type == MapNotify && event.xmap.window == xWindow)
			break;
	}

#endif

#elif defined(__WIN__)
	// XXX There's basically no error-handling code here.
	// create the window
	RECT r;
	int style = WS_POPUP | WS_CAPTION | WS_BORDER;

	r.left = 50;
	r.top = 50;
	r.right = r.left + w;
	r.bottom = r.top + h;
	AdjustWindowRect(&r,style,FALSE);
		
	hWindow = CreateWindow("glean","glean",
		style | WS_VISIBLE,
		r.left,r.top,r.right - r.left,r.bottom - r.top,
		NULL, NULL,
		GetModuleHandle(NULL),
		NULL);

    if (!hWindow)
        return;

	hDC = GetDC(hWindow);

	PIXELFORMATDESCRIPTOR pfd;
	SetPixelFormat(hDC,config->pfdID,&pfd);
#endif
} // Window::Window

///////////////////////////////////////////////////////////////////////////////
// Destructors
///////////////////////////////////////////////////////////////////////////////

void
DrawingSurface::commonDestructorCode() {
	remove(winSys->surfaces.begin(), winSys->surfaces.end(), this);
} // DrawingSurface::commonDestructorCode

Window::~Window() {

#if defined(__X11__)
	XDestroyWindow(winSys->dpy, xWindow);
#elif defined(__WIN__)
	ReleaseDC(hWindow,hDC);
	DestroyWindow(hWindow);
#endif

} // Window::~Window

///////////////////////////////////////////////////////////////////////////////
// Utilities
///////////////////////////////////////////////////////////////////////////////
void
Window::swap() {
#   if defined(__X11__)
	glXSwapBuffers(winSys->dpy, xWindow);
#   elif defined(__WIN__)
	SwapBuffers(hDC);
#   endif
} // Window::swap

#if defined(__WIN__)

///////////////////////////////////////////////////////////////////////////////
// Window procedure
///////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK 
Window::WindowProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch (message)
    {
		default :
			return DefWindowProc(hwnd, message, wParam, lParam);

	}

	return FALSE;
}

#endif

} // namespace GLEAN
