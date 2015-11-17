/* 
 * CameraController.h
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

#include <math/Transform.h>
#include <scene/Camera.h>

#include <vector>
#include <memory>

namespace aergia {
	
	template<typename T>
	class CameraController;
	
	template<typename T>
	class CameraControllerMode {
		friend class CameraController<T>;
		public:
			CameraControllerMode(){}
			virtual ~CameraControllerMode(){}
			CameraControllerMode(CameraController<T>*);
			virtual void begin(){}
			virtual void end(){}
			virtual void processMouse(double x, double y){};
			virtual void processScroll(double x, double y){};
		protected:
			CameraController<T>* cc;
			glm::vec2 start;
	};
	
	template<typename T>
	class PanMode : public CameraControllerMode<T> {
		friend class CameraController<T>;
		public:
			PanMode(CameraController<T>*);
			~PanMode(){}
			void begin();
			void end();
			void processMouse(double x, double y) override;
			void processScroll(double x, double y) override;
	};
	
	template<typename T>
	class CameraController {
		friend class Camera<glm::vec2>;
		friend class Camera<glm::vec3>;
		public:
			CameraController();
			CameraController(T*);
			void processMouse(double x, double y);
			void processScroll(double x, double y);
			void processButton(int button, int action);
			void processKey(int key, int action);
			T* camera;
			Transform transform;
		private:
			int curMode;
			std::vector<CameraControllerMode<T>*> modes;
	};
	
	typedef CameraController<Camera2D> CameraController2D;
	typedef CameraController<Camera3D> CameraController3D;
}