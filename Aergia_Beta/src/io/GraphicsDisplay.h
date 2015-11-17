/* 
 * GraphicsDisplay.h
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

#include <GLFW/glfw3.h>

#include <memory>
#include <functional>
#include <string>

#include <glm/glm.hpp>

namespace aergia {
	
	class GraphicsDisplay {
		public:
			~GraphicsDisplay();
			
			inline static GraphicsDisplay& getInstance(){
				return instance;
			}
			
			static GraphicsDisplay& create(int w, int h, const std::string& windowTitle, unsigned int flags = 0);

			void getWindowSize(int &w, int &h);
			glm::vec2 getMousePos();
			
			// Run
			void start();
			void stop();

			// IO
			void registerRenderFunc(void (*f)());	
			void registerButtonFunc(void (*f)(int,int));
			void registerKeyFunc(void (*f)(int,int));
			void registerMouseFunc(void (*f)(double,double));
			void registerScrollFunc(void (*f)(double,double));
			void registerResizeFunc(void (*f)(int,int));
			
		private:
			static GraphicsDisplay instance;
			GraphicsDisplay();
			GraphicsDisplay(GraphicsDisplay const&) = delete;
    		void operator=(GraphicsDisplay const&) = delete;
	
			// WINDOW
			GLFWwindow* window;
			std::string title;
			int width;
			int height;
				
			bool init();

			// USER CALLBACKS
			std::function<void()> renderCallback;
			std::function<void(int,int)> buttonCallback;
			std::function<void(int,int)> keyCallback;
			std::function<void(double,double)> mouseCallback;
			std::function<void(double,double)> scrollCallback;
			std::function<void(int,int)> resizeCallback;

			// DEFAULT CALLBACKS
			void buttonFunc(int button, int action);
			void keyFunc(int key, int action);
			void mouseFunc(double x, double y);
			void scrollFunc(double x, double y);
			void resizeFunc(int w, int h);

			// CALLBACKS
			static void error_callback(int error, const char* description);
			static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
			static void button_callback(GLFWwindow* window, int button, int action, int mods);
			static void pos_callback(GLFWwindow* window, double x, double y);
			static void scroll_callback(GLFWwindow* window, double x, double y);
			static void resize_callback(GLFWwindow* window, int w, int h);
	};
}