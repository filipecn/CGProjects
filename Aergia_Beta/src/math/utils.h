/* 
 * MathUtils.h
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

#include <iostream>
using namespace std;
#include <Line.h>
#include <Plane.h>
#include <Sphere.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace aergia {

		bool checkPointInsideAABB(vec4 box, vec2 p);
		
		bool lineSphereIntersection(sphere s, line l, vec3 &h0, vec3 &h1);
		bool planeLineIntersection(plane pl, line ln, vec3 &hp);

		double distance(vec3 p, plane pl);

		ostream& operator<<(ostream &out, vec2 &v); 
		ostream& operator<<(ostream &out, vec3 &v); 
		ostream& operator<<(ostream &out, vec4 &v); 
		ostream& operator<<(ostream &out, mat4 &v); 
		ostream& operator<<(ostream &out, quat &q); 

		vec3 operator*(const mat4& m, const vec3& p);
}