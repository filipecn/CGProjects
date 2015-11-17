/* 
 * OpenGL.h
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 FilipeCN
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
*/

#pragma once

#include <GL/gl.h>
#include <GL/glu.h>
//#ifndef _STDAFX_H_
//#define _STDAFX_H_

//#include <GL/glew.h>
//#include <GL/freeglut.h>

//#endif

//#include <cstdio>
#include <glm/glm.hpp>

namespace aergia {
    
#define GL_DEBUG

#ifdef GL_DEBUG
#define CHECK_GL_ERRORS aergia::printOglError(__FILE__, __LINE__)
#define CHECK_FRAMEBUFFER aergia::checkFramebuffer()
#else
#define CHECK_GL_ERRORS
#define CHECK_FRAMEBUFFER
#endif

    int printOglError(const char *file, int line);
    bool checkFramebuffer();
    
    void glVertex(const glm::vec2& v);
}
