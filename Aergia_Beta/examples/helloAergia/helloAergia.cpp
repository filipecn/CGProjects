#include <aergia/Aergia.h>
#include <aergia/math/Transform.h>
#include <aergia/scene/Camera.h>
#include <aergia/scene/CameraController.h>
using namespace aergia;

Camera2D camera;
CameraController2D cameraController(&camera);
		
void render(){
	glClearColor(0,1,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	camera.look();
	
	glColor3f(1,1,1);
	glBegin(GL_QUADS);
		glVertex2f(-1,-1);
		glVertex2f(1,-1);
		glVertex2f(1,1);
		glVertex2f(-1,1);
	glEnd();
}

void resize(int w, int h){
	camera.resize(w,h);
}

void scroll(double x, double y){
	
}

void mouse(double x, double y){
	cameraController.processMouse(x,y);
}

void button(int button, int action){
	cameraController.processButton(button,action);
}

int main(){
	Transform transform;
	GraphicsDisplay& gd = GraphicsDisplay::create(800,800,std::string("helloAergia"));
	gd.registerRenderFunc(render);
	gd.registerResizeFunc(resize);
	gd.registerMouseFunc(mouse);
	gd.registerButtonFunc(button);
	gd.start();
	return 0;
}