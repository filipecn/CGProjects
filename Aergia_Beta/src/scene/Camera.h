/* 
 * Camera.h
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 FilipeCN
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

#include <math/Transform.h>
#include <graphics/OpenGL.h>

namespace aergia {
	
	template<typename T>
	class Camera {
		public:
			Camera();
			
			void resize(double w, double h);
			void look() const;
			
			void rotate();
					
			// Matrices
			glm::mat4 getProjection() const;
			glm::mat4 getView() const;

			// Control Functions
			void zoomIn();
			void zoomOut();
			
			// Setters
			void setPos(T _pos);
			void setZoom(double z);
			
			void apply(Transform transform);

		private:
			double ratio;
			double zoom;
					
			T target;
			T pos;
			glm::vec2 clipSize;
			glm::vec2 display;
			glm::vec3 up;
					
			glm::mat4 projection;
			glm::mat4 view;
			glm::mat4 model;
			
			void update();
	};

	typedef Camera<glm::vec3> Camera3D;
	typedef Camera<glm::vec2> Camera2D;
}