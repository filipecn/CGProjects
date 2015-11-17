/*
 * OpenGL.cpp
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

#include "OpenGL.h"

#include <string>
#include <iostream>

namespace aergia {
    /*
        int printOglError(const char *file, int line) {
            GLenum glErr;
            int retCode = 0;

            glErr = glGetError();
            while (glErr != GL_NO_ERROR) {
                printf("glError in file %s @ line %d: %s\n", file, line, gluErrorString(glErr));
                retCode = 1;
                glErr = glGetError();
            }
            return retCode;
        }

        bool checkFramebuffer(){
            std::string error;
            std::string name;
            switch(glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
                case GL_FRAMEBUFFER_COMPLETE:
                    return true;
                case GL_FRAMEBUFFER_UNDEFINED:
                    error += "target is the default framebuffer, but the default framebuffer does not exist.";
                    name += "GL_FRAMEBUFFER_UNDEFINED";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                    error += "any of the framebuffer attachment points are framebuffer incomplete.";
                    name += "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                    error += "the framebuffer does not have at least one image attached to it.";
                    name += "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                    error += "the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any color attachment point(s) named by GL_DRAW_BUFFERi.";
                    name += "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                    error += "GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named by GL_READ_BUFFER.";
                    name += "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
                    break;
                case GL_FRAMEBUFFER_UNSUPPORTED:
                    error += "the combination of internal formats of the attached images violates an implementation-dependent set of restrictions.";
                    name += "GL_FRAMEBUFFER_UNSUPPORTED";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                    error += "the value of GL_RENDERBUFFER_SAMPLES is not the same for all attached renderbuffers; if the value of GL_TEXTURE_SAMPLES is the not same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_RENDERBUFFER_SAMPLES does not match the value of GL_TEXTURE_SAMPLES.";
                    error += " OR the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not the same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not GL_TRUE for all attached textures.";
                    name += "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
                    break;
                case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                    error += "any framebuffer attachment is layered, and any populated attachment is not layered, or if all populated color attachments are not from textures of the same target.";
                    name += "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
                    break;
                default:break;
            }

            error += "\n";
            std::cout << "[CHECKFRAMBUFFER - " << name << "]\n" << error << std::endl;
            exit(1);

            return 0;
        }
*/
    void glVertex(const glm::vec2& v){
        glVertex2f(v.x,v.y);
    }

}