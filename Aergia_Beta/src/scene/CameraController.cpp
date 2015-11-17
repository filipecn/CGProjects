/* 
 * CameraController.cpp
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

#include "CameraController.h"
#include <io/GraphicsDisplay.h>

#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <assert.h>

using namespace aergia;
namespace aergia {
	template<typename T>
	CameraControllerMode<T>::CameraControllerMode(CameraController<T>* _cc){ this->cc = _cc; }
	
// PAN MODE ///////////////////////////////////////////////////////////////////
	template<typename T>
	PanMode<T>::PanMode(CameraController<T>* _cc){ this->cc = _cc; }
	
	template<typename T>
	void PanMode<T>::begin(){
		this->start = GraphicsDisplay::getInstance().getMousePos();
	}
	
	template<typename T>
	void PanMode<T>::end(){
		
	}
	
	template<typename T>
	void PanMode<T>::processScroll(double x, double y){}
	
	template<>
	void PanMode<Camera2D>::processMouse(double x, double y){
		glm::vec2 pos = GraphicsDisplay::getInstance().getMousePos();
		glm::vec2 delta = pos - start;
		glm::ivec2 ws;
		//GraphicsDisplay::getInstance().getWindowSize(ws[0],ws[1]);
		//this->cc->transform.t = this->cc->transform.t + 
		//			2.0f*glm::vec3(delta.x/ws[0],delta.y/ws[1],0);
		//std::cout << glm::to_string(
		//	glm::vec3(delta.x/ws[0],delta.y/ws[1],0)) << std::endl;
		//this->cc->camera->apply(this->cc->transform);
	}

// CAMERA CONTROLLER //////////////////////////////////////////////////////////
	template<typename T>
	CameraController<T>::CameraController(){
		this->camera = nullptr;
		curMode = 0;
	}
	
	template<>
	CameraController<Camera2D>::CameraController(Camera2D* _camera){
		assert(_camera != nullptr);
		modes.emplace_back(new CameraControllerMode<Camera2D>(this));
		modes.emplace_back(new PanMode<Camera2D>(this));
		curMode = 0;
		this->camera = _camera;
	}

	template<>
	void CameraController<Camera2D>::processButton(int button, int action){
		if(this->camera == nullptr) return;
		if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
			modes[1]->begin();
			curMode = 1;
		}
		if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){
			modes[1]->end();
			curMode = 0;
		}
	}
	
	template<typename T>
	void CameraController<T>::processMouse(double x, double y){
		if(this->camera == nullptr) return;
		modes[curMode]->processMouse(x,y);
	}

	template<typename T>
	void CameraController<T>::processKey(int key, int action){
		if(this->camera == nullptr) return;
	}
	
	template class CameraController<Camera2D>;
	template class CameraController<Camera3D>;
}