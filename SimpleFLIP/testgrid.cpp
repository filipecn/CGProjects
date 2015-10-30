#define GLM_FORCE_RADIANS
#include <glm/gtx/quaternion.hpp>
#include "StatUtils.h"
#include <Aergia.h>
#include <GL/glut.h>
#include "Grid.h"
#include <ctime>
#include <cstdlib>

using aergia::io::GraphicsDisplay;
using aergia::math::Transform;
using aergia::graphics::scene::Camera3D;

#define WIDTH 	1200
#define HEIGHT 	1200

GraphicsDisplay* gd;

Transform trans;
Camera3D camera;
glm::vec2 start;
bool rotating = false;
bool translating = false;
glm::vec2 panStart;

int W = 10, H = 5;
int I, J;
double SPACING = 1.0;
Grid<float> g;

void init(){
	srand (time(NULL));
	
	g.set(W,H);
	g.cellSize = SPACING;
	g.offset = glm::vec2(0,0);
}

void render(){
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	camera.look();	
	
	glColor4f(1,1,1,0.5);
	glBegin(GL_LINES);
		for(int i = 0; i <= W; i++){
			glm::vec2 p = g.gridToWorld(glm::vec2(i,0)); p -= g.cellSize*0.5f;
			glVertex2f(p.x, p.y); 
			p = g.gridToWorld(glm::vec2(i,H)); p -= g.cellSize*0.5f;
			glVertex2f(p.x, p.y);
		}
		for(int i = 0; i <= H; i++){
			glm::vec2 p = g.gridToWorld(glm::vec2(0,i)); p -= g.cellSize*0.5f;
			glVertex2f(p.x, p.y); 
			p = g.gridToWorld(glm::vec2(W,i)); p -= g.cellSize*0.5f;
			glVertex2f(p.x, p.y);
		}
	glEnd();

	glPointSize(5.0);
	glBegin(GL_POINTS);
		glColor3f(1,0,1);
		for(int i = 0; i < W; i++)
			for(int j = 0; j < H; j++){
				glm::vec2 p = g.gridToWorld(glm::vec2(i,j));
				glVertex2f(p.x,p.y);
			}
			
		for(int i = 0; i < W; i++)
			for(int j = 0; j < H; j++){
				float c = g(i,j);
				glColor3f(c,c,c);
				glm::vec2 p = g.gridToWorld(glm::vec2(i,j));
				if(c > 0.0)
					glVertex3f(p.x,p.y,0.1);
				else glVertex2f(p.x,p.y);
			}
			
		//std::cerr << g.sample(1*g.cellSize,1*g.cellSize) << std::endl;
		//glColor3f(1,1,1);
		//glVertex3f(1*g.cellSize,1*g.cellSize,0.1);
		float step = 1.0/5.0;
		float s1 = -g.cellSize;
		while(s1 < ((float)W)*g.cellSize){
			float s2 = -g.cellSize;
			while(s2 < ((float)H)*g.cellSize){
				float c = g.sample(s1,s2);
				glColor3f(c,c,c);
				glVertex2f(s1,s2);
				s2 += step;
			}
			s1 += step;
		}
	glEnd();
}

glm::vec3 screenToSphere(double x, double y){
	glm::vec2 p((x / WIDTH)*2.0 - 1.0, (y/HEIGHT)*2.0 - 1.0);
	float z = std::max(0.0f, 1.0f - sqrt(p.x*p.x + p.y*p.y));
	return glm::normalize(vec3(p.x,p.y,z));
}

void mouse(double x, double y){
	if(rotating){
		vec3 beg = screenToSphere(start.x, start.y);
		vec3 end = screenToSphere(x,HEIGHT-y);
		glm::vec3 axis = glm::normalize(glm::cross(beg,end));
		float phi = glm::distance(beg, end)/4;
		glm::vec3 newAxis = glm::normalize(glm::mat3(glm::inverse(glm::toMat4(trans.r))*glm::inverse(camera.getView()))*axis);
		trans.r = glm::rotate(trans.r, glm::degrees(phi)*0.3f, newAxis);
		start = gd->getMousePos();
	}
	if(translating){
		glm::vec2 delta = gd->getMousePos() - panStart;
		delta.x /= WIDTH; delta.y /= HEIGHT;
		glm::vec3 newDelta = glm::mat3(glm::inverse(glm::toMat4(trans.r))*glm::inverse(camera.getView()))*glm::vec3(delta.x,delta.y,0.0);
		trans.t = trans.t + newDelta*3.0f;
		panStart = gd->getMousePos();
	}
	camera.apply(trans);
}

void mouseButton(int button, int action){
	if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){
		rotating = false;
	}
	else if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
		rotating = true;
		start = gd->getMousePos();
	}
	else if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE){
		translating = false;
	}
	else if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
		translating = true;
		panStart = gd->getMousePos();
	}
}

void mouseScroll(double x, double y){
	if(y < 0.0)
		trans.s *= 0.95;
	else 	trans.s *= 1.05;
	camera.apply(trans);
}

void keyboard(int key, int action){
	if(key == GLFW_KEY_Q && action == GLFW_PRESS)
		gd->stop();
	if(key == GLFW_KEY_W && action == GLFW_PRESS)
		J = min(J+1,H-1);
	if(key == GLFW_KEY_S && action == GLFW_PRESS)
		J = max(0, J - 1);
	if(key == GLFW_KEY_A && action == GLFW_PRESS)
		I = max(0, I - 1);
	if(key == GLFW_KEY_D && action == GLFW_PRESS)
		I = min(I+1,H-1);
	if(key == GLFW_KEY_Z && action == GLFW_PRESS)
		g.setAll(0);
	if(key == GLFW_KEY_F && action == GLFW_PRESS){
		g.setAll(0);
		//g(1,1) = 1;
		//g(2,2) = 1;
		//g(2,1) = 1;
		//g(1,2) = 1;
		//return;
		int n = rand() % W;
		for(int i = 0; i < n; i++)
			g(rand() % W, rand() % H) = 1.0;
	}

}

int main(int argc, char **argv){
	init();

	gd = GraphicsDisplay::create(WIDTH, HEIGHT, std::string("SimpleFLIP3D"));
	gd->registerRenderFunc(render);
	gd->registerButtonFunc(mouseButton);
	gd->registerKeyFunc(keyboard);
	gd->registerMouseFunc(mouse);
	gd->registerScrollFunc(mouseScroll);

	gd->start();

	return 0;
}
