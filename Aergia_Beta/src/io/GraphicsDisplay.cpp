/* 
 * GraphicsDisplay.cpp
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

#include "GraphicsDisplay.h"
using namespace aergia;

#include <iostream>

GraphicsDisplay GraphicsDisplay::instance;

GraphicsDisplay::GraphicsDisplay()
	: window(NULL),
	title(),
	width(400),
	height(400){
	}

GraphicsDisplay& GraphicsDisplay::create(int w, int h, const std::string& windowTitle, unsigned int flags){
	instance.width = w;
	instance.height = h;
	instance.title = windowTitle;

	instance.renderCallback = nullptr;
	instance.buttonCallback = nullptr;
	instance.keyCallback = nullptr;
	instance.mouseCallback = nullptr;
	instance.scrollCallback = nullptr;
	instance.keyCallback = nullptr;
	
	instance.init();
	
	return instance;
}

GraphicsDisplay::~GraphicsDisplay(){
	glfwDestroyWindow(window);
	glfwTerminate();
}

bool GraphicsDisplay::init(){
	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		return false;

	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	glfwWindowHint(GLFW_DEPTH_BITS, 32);

	window = glfwCreateWindow(this->width, this->height, this->title.c_str(), NULL, NULL);

	if (!window){
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback (window, button_callback);
	glfwSetCursorPosCallback (window, pos_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetWindowSizeCallback(window, resize_callback);

	return true;
}

void GraphicsDisplay::start(){
    while(!glfwWindowShouldClose(this->window)){
		
        glfwGetFramebufferSize(window, &this->width, &this->height);
		glViewport(0, 0, this->width, this->height);
		
		if(this->renderCallback) {
            this->renderCallback();
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void GraphicsDisplay::getWindowSize(int &w, int &h){
	w = this->width;
	h = this->height;
}

glm::vec2 GraphicsDisplay::getMousePos(){
	double x, y;
	glfwGetCursorPos(this->window, &x, &y);	
	return glm::vec2(x, this->height - y);
}

void GraphicsDisplay::stop(){
	glfwSetWindowShouldClose(window, GL_TRUE);
}

void GraphicsDisplay::error_callback(int error, const char* description){
	fputs(description, stderr);
}

void GraphicsDisplay::registerRenderFunc(void (*f)()){
	this->renderCallback = f;
}

/////////////////////////// KEY FUNCTIONS ////////////////////////////////////////////////////
void GraphicsDisplay::registerKeyFunc(void (*f)(int,int)){
	this->keyCallback = f;
}

void GraphicsDisplay::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	if(instance.keyCallback)
		instance.keyCallback(key,action);
	else instance.keyFunc(key,action);
}

void GraphicsDisplay::keyFunc(int key, int action){
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}
///////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////// BUTTON FUNCTIONS //////////////////////////////////////////////////
void GraphicsDisplay::registerButtonFunc(void (*f)(int,int)){
	this->buttonCallback = f;
}

void GraphicsDisplay::button_callback(GLFWwindow* window, int button, int action, int mods){
	if(instance.buttonCallback)
		instance.buttonCallback(button,action);
	else instance.buttonFunc(button,action);
}

void GraphicsDisplay::buttonFunc(int button, int action){
}
///////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////// MOUSE MOTION FUNCTIONS ////////////////////////////////////////////
void GraphicsDisplay::registerMouseFunc(void (*f)(double,double)){
	this->mouseCallback = f;
}

void GraphicsDisplay::pos_callback(GLFWwindow* window, double x, double y){
	if(instance.mouseCallback)
		instance.mouseCallback(x,y);
	else instance.mouseFunc(x,y);
}

void GraphicsDisplay::mouseFunc(double x, double y){
}
///////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////// MOUSE SCROLL FUNCTIONS ////////////////////////////////////////////
void GraphicsDisplay::registerScrollFunc(void (*f)(double,double)){
	this->scrollCallback = f;
}

void GraphicsDisplay::scroll_callback(GLFWwindow* window, double x, double y){
	if(instance.scrollCallback)
		instance.scrollCallback(x,y);
	else 
		instance.scrollFunc(x,y);
}

void GraphicsDisplay::scrollFunc(double x, double y){
}
///////////////////////////////////////////////////////////////////////////////////////////////
void GraphicsDisplay::registerResizeFunc(void (*f)(int,int)){
	this->resizeCallback = f;
}

void GraphicsDisplay::resize_callback(GLFWwindow* window, int w, int h){
	instance.resizeFunc(w,h);
	if(instance.resizeCallback){
		instance.getWindowSize(w,h);
		instance.resizeCallback(w,h);	
	}
}

void GraphicsDisplay::resizeFunc(int w, int h){
	glfwGetFramebufferSize(window, &this->width, &this->height);
}