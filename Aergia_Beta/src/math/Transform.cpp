/* 
 * Transform.cpp
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

#include "Transform.h"
using namespace aergia;

#include <iostream>
using namespace std;

Transform::Transform(){
	s = glm::vec3(1.0);
	t = glm::vec3(0);
	r = glm::quat(1,0,0,0);
}

glm::mat4 Transform::matrix() const{
	return glm::toMat4(r)*glm::scale(s)*glm::translate(t);
}

glm::mat4 Transform::inverseMatrix() const {
	return glm::inverse(matrix());		
}

void Transform::rotate(const float angle, const glm::vec3 axis){
	r = glm::rotate(r, glm::degrees(angle), axis);
}

glm::vec3 Transform::getRotationAxis(){
	return glm::axis(r);
}