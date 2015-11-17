/* 
 * Camera.cpp
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

#include "Camera.h"

using namespace aergia;

#include <iostream>
#include <math.h>

namespace aergia {
	
	template<typename T>
	Camera<T>::Camera()
	{
		pos = T(0);
		clipSize = glm::vec2(1,1);
		zoom = 1.0;
		display = glm::vec2(1,1);
		projection = glm::mat4(); 
		view = glm::mat4(); 
		model = glm::mat4(); 
	}
	
	template<typename T>
	inline glm::mat4 Camera<T>::getProjection() const {
		return projection;
	}
	
	template<typename T>
	inline glm::mat4 Camera<T>::getView() const {
		return view;
	}
	
	template<typename T>
	inline void Camera<T>::setPos(T _pos){
		pos = _pos;
		update();
	}

	template<typename T>
	inline void Camera<T>::setZoom(double z){
		zoom = z;
		update();
	}
	
	template<typename T>
	void Camera<T>::resize(double w, double h){
		display = glm::vec2(w,h);
		ratio = w/h;
		clipSize = glm::vec2(zoom);
		if(w < h)
			clipSize.y = clipSize.x/ratio;
		else clipSize.x = clipSize.y*ratio;
		update();
	}
	
	template<typename T>
	void Camera<T>::look() const {
		glMatrixMode(GL_PROJECTION);
		
		glLoadIdentity();
		
		glMultMatrixf(&projection[0][0]);
		
		glMatrixMode(GL_MODELVIEW);

		glLoadIdentity();

		glMultMatrixf(&view[0][0]);
	
		glMultMatrixf(&model[0][0]);
	}
	
	template<typename T>
	void Camera<T>::apply(Transform t){
		glm::mat4 m = glm::toMat4(t.r);
		model = m;
		model = glm::translate(m, t.t);
		model = glm::scale(m, t.s);
		model = glm::scale(glm::mat4(1.0f),t.s)*glm::toMat4(t.r)*glm::translate(t.t);
	}
	
	template<>
	void Camera<glm::vec2>::update(){
		projection = glm::ortho(pos.x - clipSize.x, pos.x + clipSize.x, 
							    pos.y - clipSize.y, pos.y + clipSize.y);
	}
	
	template<>
	void Camera<glm::vec3>::update(){}
}
/*
void Camera::rotate(){
	
}

void Camera::zoomIn(){
	this->zoom *= 1.03;
	resize(display.x,display.y);
}

void Camera::zoomOut(){
	this->zoom *= 0.97;
	resize(display.x,display.y);
}

Camera3D::Camera3D(){
	if(Config::getInstance().GENERAL_BEHAVIOUR == A_DEFAULT){
		float halfH = 10.0;
		pos = vec3(0,0,halfH/tan(22.5*3.1415/180.0));
		pos = vec3(0,0,10);
		target = vec3(0,0,0);
		up = vec3(0,1,0);

		projection = glm::perspective(45.0f, 1.0f, 0.1f, 100.f);
		view = glm::lookAt(pos,target,up);
		model = mat4(1.0f);
	}
}

Camera3D::Camera3D(vec3 _pos, vec3 _target, vec3 _up){
	set(_pos, _target, _up);
}

void Camera3D::set(vec3 _pos, vec3 _target, vec3 _up){
	pos = _pos;
	target = _target;
	up = _up;

	projection = glm::perspective(45.0f, 1.0f, 0.1f, 100.f);
	view = glm::lookAt(pos,target,up);
	model = mat4(1.0f);
}

void Camera3D::look(){
	glMatrixMode(GL_PROJECTION);
	
	glLoadIdentity();
	
	glMultMatrixf(&projection[0][0]);

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	glMultMatrixf(&view[0][0]);
	
	glMultMatrixf(&model[0][0]);

//	glMatrixMode(GL_MODELVIEW);
}

void Camera3D::apply(Transform t){
	//mat4 m = t.matrix();
	//vec3 axis = t.getRotationAxis();
	//cout << "apply!\n" << axis << " " << glm::angle(t.r) << endl;
	//vec3 camAxis = glm::normalize(glm::inverse(glm::mat3(view))*t.getRotationAxis());  
	//cout << "apply!\n" << camAxis << endl;
	//cout << pos << endl;
	//mat4 m = glm::toMat4(glm::angleAxis(glm::angle(t.r),camAxis));
	mat4 m = glm::toMat4(t.r);
	model = m;

	model = translate(m, t.t);
	model = scale(m, glm::vec3(t.s,t.s,t.s));
	//pos = m*pos;

	model = glm::scale(mat4(1.0f),glm::vec3(t.s,t.s,t.s))*glm::toMat4(t.r)*glm::translate(t.t);
		
	//view = glm::lookAt(pos,target,up);
}
*/

template class Camera<glm::vec2>;
template class Camera<glm::vec3>;