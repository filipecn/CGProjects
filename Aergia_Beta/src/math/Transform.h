/* 
 * Transform.h
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

//#include <MathUtils.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace aergia {
		
		class Transform {
			public:
				Transform();

				Transform operator*(const Transform &t) const;
				Transform &operator*=(const Transform &t);

				glm::vec3 operator*(const glm::vec3 &p) const;

				Transform &setIdentity();
				Transform &setScale(const double s);
				Transform &setTranslate(const glm::vec3 &v);

				void rotate(float angle, const glm::vec3 axis); 
				glm::vec3 getRotationAxis();
				Transform &setRotate(const glm::quat &q);

				glm::mat4 matrix() const;
				glm::mat4 inverseMatrix() const;
				void fromMatrix(const glm::mat4 &m);

				glm::quat r;
				glm::vec3 t;
				glm::vec3 s; 
		};
}