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



// geomrend.h:  convenience object for rendering any geometry via
// a host of OpenGL paths: immediate mode (glVertex), vertex
// arrays with glDrawArrays, vertex arrays with glArrayElement,
// vertex arrays with glDrawElements, and any of the preceding
// methods stuffed in a display list.

#include "geomrend.h"
#include "rand.h"
#include "glutils.h"
#include <algorithm>
#include <cmath>
#include <float.h>
#include <assert.h>

#ifdef __WIN__
using namespace std;
#endif

typedef GLvoid (* GLLOCKARRAYSEXT_FUNCTYPE)(GLint, GLsizei);
typedef GLvoid (* GLUNLOCKARRAYSEXT_FUNCTYPE)();

namespace GLEAN {


// geomrend.h:  convenience object for rendering any geometry via
// a host of OpenGL paths: immediate mode (glVertex), vertex
// arrays with glDrawArrays, vertex arrays with glArrayElement,
// vertex arrays with glDrawElements, and any of the preceding
// methods stuffed in a display list.

// Functions for the helper class ArrayData, which stores the info about each parameter's data.
ArrayData::ArrayData()
{
    size = 0;
    type = GL_UNSIGNED_INT;
    stride = 0;
    pointer = 0;
}

void ArrayData::setData(GLint sizeIn, GLenum typeIn, GLsizei strideIn, const GLvoid* pointerIn)
{
    size = sizeIn;
    type = typeIn;
    stride = strideIn;
    pointer = pointerIn;
    if (stride == 0)
    {
        switch(type)
        {
            case GL_BYTE:           stride = size*sizeof(GLbyte); break;
            case GL_UNSIGNED_BYTE:  stride = size*sizeof(GLubyte); break;
            case GL_SHORT:          stride = size*sizeof(GLshort); break;
            case GL_UNSIGNED_SHORT: stride = size*sizeof(GLushort); break;
            case GL_INT:            stride = size*sizeof(GLint); break;
            case GL_UNSIGNED_INT:   stride = size*sizeof(GLuint); break;
            case GL_FLOAT:          stride = size*sizeof(GLfloat); break;
            case GL_DOUBLE:         stride = size*sizeof(GLdouble); break;
            default: assert(false);
        }
    }
}

// Only a default constructor.
GeomRenderer::GeomRenderer() : vertexData(), colorData(), texCoordData(), normalData()
{
    drawMethod = GLVERTEX_MODE;
    parameterBits = 0;
    compileArrays = false;

    indicesCount = 0;
    indicesType = GL_UNSIGNED_INT;
    indices = 0;

    arrayLength = 0;
}
    
// Used to set the method by which this GeomRenderer will pass the primitive data to the GL.
// Default is GLVERTEX_MODE.
void GeomRenderer::setDrawMethod(GeomRenderer::DrawMethod method)
{
    drawMethod = method;
}

GeomRenderer::DrawMethod GeomRenderer::getDrawMethod() const
{
    return drawMethod;
}

// Used to set the various parameters that are either enabled or disabled.  Example usage:
// to tell the GeomRenderer to pass vertex, color, and texcoord data, but not normals,
// call setParameterBits(COLOR_BIT | TEXTURE_COORD_BIT).  (Vertex data is implicitly enabled
// all the time.)  The default is that only vertex data is enabled.
void GeomRenderer::setParameterBits(GLuint bits)
{
    parameterBits = bits;
}

GLuint GeomRenderer::getParameterBits() const
{
    return parameterBits;
}

// Used to specify whether EXT_compiled_vertex_array should be used if present.  Default is false.
// If set to true, the arrays are kept unlocked and only locked just before rendering calls are issued.
// If you call setArraysCompiled(true) and the extension is not present, the function returns false
// and acts as though you had passed false in as the argument.
bool GeomRenderer::setArraysCompiled(bool compile)
{
    // Make sure we have the extension.
    if (!GLUtils::haveExtension("GL_EXT_compiled_vertex_array") && compile == true)
    {
        compileArrays = false;
        return false;
    }

    compileArrays = compile;
    return true;
}

bool GeomRenderer::getArraysCompiled() const
{
    return compileArrays;
}

// If you're using GLDRAWELEMENTS_MODE, GLARRAYELEMENT_MODE, or GLVERTEX_MODE, you need to give
// it the indices to pass into the GL.
void GeomRenderer::setVArrayIndices(GLuint count, GLenum type, const GLvoid* indicesIn)
{
    assert(type == GL_UNSIGNED_BYTE || type == GL_UNSIGNED_SHORT || type == GL_UNSIGNED_INT);

    indicesCount = count;
    indicesType = type;
    indices = indicesIn;
}

// This hands the actual primitive data to the GeomRenderer.  It holds onto these as pointers,
// rather than copying them, so don't delete the data until you're done with the GeomRenderer.
// These are prototypically equivalent to their respective GL calls, except that there's an extra
// argument on the front of the vertex function for how many elements are in the array (this is
// atomic; if you pass in 5, it means there are 5 vertices, not 5 floats or bytes or whatever).
// The lengths of all other arrays are assumed to be >= the size passed in for the vertex array.
void GeomRenderer::setVertexPointer(GLuint length, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
    arrayLength = length;
    vertexData.setData(size, type, stride, pointer);
}

void GeomRenderer::setColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
    colorData.setData(size, type, stride, pointer);
}

void GeomRenderer::setTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
    texCoordData.setData(size, type, stride, pointer);
}

void GeomRenderer::setNormalPointer(GLenum type, GLsizei stride, const GLvoid* pointer)
{
    normalData.setData(3, type, stride, pointer);
}

// Finally, the actual calls to do something with all this data.  You can either choose to render
// it given the configuration, or generate a display list of rendering it with the given
// configuration (uses GL_COMPILE mode to build the list).  Fails if insufficient data has
// been given (i.e. if you don't give it an array for an enabled parameter, if you don't
// give it an array of indices when it needs them).
bool GeomRenderer::renderPrimitives(GLenum mode)
{
    if (!isReadyToRender())
    {
        return false;
    }

    // Okay, different sections here depending on what we're doing.
    if (drawMethod == GLVERTEX_MODE)
    {
        glBegin(mode);
        for (int x=0; x<indicesCount; x++)
        {
            int directIndex = getIndex(x);
            if (parameterBits & COLOR_BIT) sendColor(directIndex);
            if (parameterBits & TEXTURE_COORD_BIT) sendTexCoord(directIndex);
            if (parameterBits & NORMAL_BIT) sendNormal(directIndex);
            sendVertex(directIndex);
        }
        glEnd();
    }
    // Otherwise it has something to do with arrays; set up the arrays.
    else
    {
        if (parameterBits & COLOR_BIT)
        { 
            glEnableClientState(GL_COLOR_ARRAY);
            glColorPointer(colorData.size, colorData.type, colorData.stride, colorData.pointer);
        }
        if (parameterBits & TEXTURE_COORD_BIT)
        { 
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(texCoordData.size, texCoordData.type, texCoordData.stride, texCoordData.pointer);
        }
        if (parameterBits & NORMAL_BIT)
        { 
            glEnableClientState(GL_NORMAL_ARRAY);
            glNormalPointer(normalData.type, normalData.stride, normalData.pointer);
        }
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(vertexData.size, vertexData.type, vertexData.stride, vertexData.pointer);

        // Should we lock?
        if (compileArrays)
        {
            ((GLLOCKARRAYSEXT_FUNCTYPE)GLUtils::getProcAddress("glLockArraysEXT"))(0, arrayLength);
        }

        // Okay, arrays configured; what exactly are we doing?
        if (drawMethod == GLARRAYELEMENT_MODE)
        {
            glBegin(mode);
            for (int x=0; x<indicesCount; x++)
            {
                glArrayElement(getIndex(x));
            }
            glEnd();
        }
        else if (drawMethod == GLDRAWARRAYS_MODE)
        {
            glDrawArrays(mode, 0, arrayLength);
        }
        else if (drawMethod == GLDRAWELEMENTS_MODE)
        {
            glDrawElements(mode, indicesCount, indicesType, indices);
        }

        // Done.  If we locked, unlock.
        if (compileArrays)
        {
            ((GLUNLOCKARRAYSEXT_FUNCTYPE)GLUtils::getProcAddress("glUnlockArraysEXT"))();
        }
    }

    return true;
}

bool GeomRenderer::generateDisplayList(GLenum mode, GLint& listHandleOut)
{
    if (!isReadyToRender())
    {
        return false;
    }

    listHandleOut = glGenLists(1);
    glNewList(listHandleOut, GL_COMPILE);
    assert(renderPrimitives(mode));
    glEndList();

    return true;
}

bool GeomRenderer::isReadyToRender()
{
    // Make sure we have vertex data.
    if (vertexData.pointer == 0) return false;

    // For the enabled parameters, make sure we have them, too.
    if ((parameterBits & COLOR_BIT        ) && (colorData.pointer    == 0)) return false;
    if ((parameterBits & TEXTURE_COORD_BIT) && (texCoordData.pointer == 0)) return false;
    if ((parameterBits & NORMAL_BIT       ) && (normalData.pointer   == 0)) return false;

    // If we need indices, we'd better have them.
    if ((drawMethod == GLVERTEX_MODE || 
         drawMethod == GLARRAYELEMENT_MODE || 
         drawMethod == GLDRAWELEMENTS_MODE) && indices == 0)
    {
        return false;
    }

    // Otherwise we're good to go!
    return true;
}

// This unpacks the indices depending on their format and returns the specified one.
GLuint GeomRenderer::getIndex(int indicesIndex)
{
    assert(indicesIndex >= 0 && indicesIndex < indexCount);

    switch (indicesType)
    {
        case GL_UNSIGNED_BYTE:
            return ((GLubyte*)indices)[indicesIndex];
            break;
            
        case GL_UNSIGNED_SHORT:
            return ((GLushort*)indices)[indicesIndex];
            break;

        case GL_UNSIGNED_INT:
            return ((GLuint*)indices)[indicesIndex];
            break;

        default:
            assert(false);
            break;
    }
    
    // It never gets here, but let's quell the compiler warning...
    return 0;
}

// I thought about making a lookup table for this, but it would involve an STL map of STL vectors
// and some weird function casts, so I'm doing it the naive way instead.
void GeomRenderer::sendVertex(GLuint vertexIndex)
{
    assert(vertexData.size >= 2 && vertexData.size <= 4);

    switch(vertexData.type)
    {
        case GL_SHORT:
            if (vertexData.size == 2) glVertex2sv((GLshort*)vertexData.pointer + vertexIndex*vertexData.stride);
            if (vertexData.size == 3) glVertex3sv((GLshort*)vertexData.pointer + vertexIndex*vertexData.stride);
            if (vertexData.size == 4) glVertex4sv((GLshort*)vertexData.pointer + vertexIndex*vertexData.stride);
            break;

        case GL_INT:
            if (vertexData.size == 2) glVertex2iv((GLint*)vertexData.pointer + vertexIndex*vertexData.stride);
            if (vertexData.size == 3) glVertex3iv((GLint*)vertexData.pointer + vertexIndex*vertexData.stride);
            if (vertexData.size == 4) glVertex4iv((GLint*)vertexData.pointer + vertexIndex*vertexData.stride);
            break;

        case GL_FLOAT:
            if (vertexData.size == 2) glVertex2fv((GLfloat*)vertexData.pointer + vertexIndex*vertexData.stride);
            if (vertexData.size == 3) glVertex3fv((GLfloat*)vertexData.pointer + vertexIndex*vertexData.stride);
            if (vertexData.size == 4) glVertex4fv((GLfloat*)vertexData.pointer + vertexIndex*vertexData.stride);
            break;

        case GL_DOUBLE:
            if (vertexData.size == 2) glVertex2dv((GLdouble*)vertexData.pointer + vertexIndex*vertexData.stride);
            if (vertexData.size == 3) glVertex3dv((GLdouble*)vertexData.pointer + vertexIndex*vertexData.stride);
            if (vertexData.size == 4) glVertex4dv((GLdouble*)vertexData.pointer + vertexIndex*vertexData.stride);
            break;
    }
}

void GeomRenderer::sendColor(GLuint colorIndex)
{
    assert(colorData.size == 3 || colorData.size == 4);

    switch(colorData.type)
    {
        case GL_BYTE:
            if (colorData.size == 3) glColor3bv((GLbyte*)colorData.pointer + colorIndex*colorData.stride);
            if (colorData.size == 4) glColor4bv((GLbyte*)colorData.pointer + colorIndex*colorData.stride);
            break;

        case GL_UNSIGNED_BYTE:
            if (colorData.size == 3) glColor3ubv((GLubyte*)colorData.pointer + colorIndex*colorData.stride);
            if (colorData.size == 4) glColor4ubv((GLubyte*)colorData.pointer + colorIndex*colorData.stride);
            break;

        case GL_SHORT:
            if (colorData.size == 3) glColor3sv((GLshort*)colorData.pointer + colorIndex*colorData.stride);
            if (colorData.size == 4) glColor4sv((GLshort*)colorData.pointer + colorIndex*colorData.stride);
            break;

        case GL_UNSIGNED_SHORT:
            if (colorData.size == 3) glColor3usv((GLushort*)colorData.pointer + colorIndex*colorData.stride);
            if (colorData.size == 4) glColor4usv((GLushort*)colorData.pointer + colorIndex*colorData.stride);
            break;

        case GL_INT:
            if (colorData.size == 3) glColor3iv((GLint*)colorData.pointer + colorIndex*colorData.stride);
            if (colorData.size == 4) glColor4iv((GLint*)colorData.pointer + colorIndex*colorData.stride);
            break;

        case GL_UNSIGNED_INT:
            if (colorData.size == 3) glColor3uiv((GLuint*)colorData.pointer + colorIndex*colorData.stride);
            if (colorData.size == 4) glColor4uiv((GLuint*)colorData.pointer + colorIndex*colorData.stride);
            break;

        case GL_FLOAT:
            if (colorData.size == 3) glColor3fv((GLfloat*)colorData.pointer + colorIndex*colorData.stride);
            if (colorData.size == 4) glColor4fv((GLfloat*)colorData.pointer + colorIndex*colorData.stride);
            break;

        case GL_DOUBLE:
            if (colorData.size == 3) glColor3dv((GLdouble*)colorData.pointer + colorIndex*colorData.stride);
            if (colorData.size == 4) glColor4dv((GLdouble*)colorData.pointer + colorIndex*colorData.stride);
            break;
    }
}

void GeomRenderer::sendTexCoord(GLuint texCoordIndex)
{
    assert(texCoordData.size >= 1 && texCoordData.size <= 4);

    switch(texCoordData.type)
    {
        case GL_SHORT:
            if (texCoordData.size == 1) glTexCoord1sv((GLshort*)texCoordData.pointer + texCoordIndex*texCoordData.stride);
            if (texCoordData.size == 2) glTexCoord2sv((GLshort*)texCoordData.pointer + texCoordIndex*texCoordData.stride);
            if (texCoordData.size == 3) glTexCoord3sv((GLshort*)texCoordData.pointer + texCoordIndex*texCoordData.stride);
            if (texCoordData.size == 4) glTexCoord4sv((GLshort*)texCoordData.pointer + texCoordIndex*texCoordData.stride);
            break;

        case GL_INT:
            if (texCoordData.size == 1) glTexCoord1iv((GLint*)texCoordData.pointer + texCoordIndex*texCoordData.stride);
            if (texCoordData.size == 2) glTexCoord2iv((GLint*)texCoordData.pointer + texCoordIndex*texCoordData.stride);
            if (texCoordData.size == 3) glTexCoord3iv((GLint*)texCoordData.pointer + texCoordIndex*texCoordData.stride);
            if (texCoordData.size == 4) glTexCoord4iv((GLint*)texCoordData.pointer + texCoordIndex*texCoordData.stride);
            break;

        case GL_FLOAT:
            if (texCoordData.size == 1) glTexCoord1fv((GLfloat*)texCoordData.pointer + texCoordIndex*texCoordData.stride);
            if (texCoordData.size == 2) glTexCoord2fv((GLfloat*)texCoordData.pointer + texCoordIndex*texCoordData.stride);
            if (texCoordData.size == 3) glTexCoord3fv((GLfloat*)texCoordData.pointer + texCoordIndex*texCoordData.stride);
            if (texCoordData.size == 4) glTexCoord4fv((GLfloat*)texCoordData.pointer + texCoordIndex*texCoordData.stride);
            break;

        case GL_DOUBLE:
            if (texCoordData.size == 1) glTexCoord1dv((GLdouble*)texCoordData.pointer + texCoordIndex*texCoordData.stride);
            if (texCoordData.size == 2) glTexCoord2dv((GLdouble*)texCoordData.pointer + texCoordIndex*texCoordData.stride);
            if (texCoordData.size == 3) glTexCoord3dv((GLdouble*)texCoordData.pointer + texCoordIndex*texCoordData.stride);
            if (texCoordData.size == 4) glTexCoord4dv((GLdouble*)texCoordData.pointer + texCoordIndex*texCoordData.stride);
            break;
    }
}

void GeomRenderer::sendNormal(GLuint normalIndex)
{
    assert(normalData.size == 3);

    switch(normalData.type)
    {
        case GL_BYTE:
            glNormal3bv((GLbyte*)normalData.pointer + normalIndex*normalData.stride);
            break;

        case GL_SHORT:
            glNormal3sv((GLshort*)normalData.pointer + normalIndex*normalData.stride);
            break;

        case GL_INT:
            glNormal3iv((GLint*)normalData.pointer + normalIndex*normalData.stride);
            break;

        case GL_FLOAT:
            glNormal3fv((GLfloat*)normalData.pointer + normalIndex*normalData.stride);
            break;

        case GL_DOUBLE:
            glNormal3dv((GLdouble*)normalData.pointer + normalIndex*normalData.stride);
            break;
    }
}

} // namespace GLEAN
