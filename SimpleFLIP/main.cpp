#define GLM_FORCE_RADIANS
#include <glm/gtx/quaternion.hpp>
#include "StatUtils.h"
#include <Aergia.h>
#include <GL/glut.h>
#include "FLIP.h"
#include "GridPtrAccessor.h"
#include "GridAccessor.h"

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

FLIP flip;
ParticleSetAccessor psa_u, psa_v, psa_c;
GridPtrAccessor<float> uAcc, vAcc;
GridAccessor<char> cAcc;

int W = 30, H = 30;
int mode;

void init(){
	flip.size = glm::ivec2(W,H);
	flip.dx = 0.1;
	flip.dt = 0.001;
	flip.rho = 1.0;
	flip.gravity = -9.8;
	
	for(int i = 2; i <= 27; i++)
		for(int j = 2; j <= 10; j++)
			flip.fillCell(i,j);
			
	for(int i = 12; i <= 12; i++)
		for(int j = 20; j <= 25; j++)
			flip.fillCell(i,j);
	
	flip.init();
	
	for(int i = 0; i < W; i++){
		flip.isSolid(i,0) = true;
		flip.isSolid(i,1) = true;
		flip.isSolid(i,H-1) = true;
		flip.isSolid(i,H-2) = true;
	}
	for(int j = 0; j < H; j++){
		flip.isSolid(0,j) = true;
		flip.isSolid(1,j) = true;
		flip.isSolid(W-1,j) = true;
		flip.isSolid(W-2,j) = true;
	}
	
	psa_v.set(flip.particleSet, glm::vec3(0.5*flip.dx,0,0),flip.dx);
	psa_u.set(flip.particleSet, glm::vec3(0,0.5*flip.dx,0),flip.dx);
	psa_c.set(flip.particleSet, glm::vec3(0,0,0),flip.dx);
	uAcc.setGrid(flip.grid.get(0));
	vAcc.setGrid(flip.grid.get(1));
}

void drawParticles(){
	glPointSize(3.0);
	glColor3f(0,0,1);
	glBegin(GL_POINTS);
	for(auto p : flip.particleSet.particles){
		glm::vec3 pos = p.getPos();
		glVertex2f(pos.x,pos.y);
	}
	glEnd();
	return;
	glColor4f(0,0,1,0.5);
	glBegin(GL_LINES);
	for(auto p : flip.particleSet.particles){
		glm::vec3 pos = p.getPos();
		glm::vec3 v = p.getVelocity();
		glVertex2f(pos.x,pos.y);
		glVertex2f(pos.x + /*flip.dt**/v.x, pos.y + /*flip.dt**/v.y);
	}
	glEnd();
}

void render(){
	flip.step();
	
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	camera.look();	
	
	cAcc.paintGrid(flip.cell);
	//uAcc.drawVelocities(glm::vec2(1.0,0.0),flip.dt);
	//vAcc.drawVelocities(glm::vec2(0.0,1.0),flip.dt);
	glPointSize(5.0);
	//glColor3f(1,1,1); cAcc.drawGridNodes(flip.cell);
	//glColor3f(0,1,1); uAcc.drawGridNodes();
	//glColor3f(1,1,0); vAcc.drawGridNodes();
	cAcc.drawGrid(flip.cell);	
	drawParticles();
	
	psa_u.update(flip.particleSet);
	psa_v.update(flip.particleSet);
	psa_c.update(flip.particleSet);

	switch(mode){
		case 0:
			uAcc.processCurrentNode(flip.dx, [](glm::vec3 boxMin, glm::vec3 boxMax){
			psa_u.iterateNeighbours(flip.particleSet, boxMin, boxMax, [](const Particle& p){
					glm::vec3 pos = p.getPos();
					glVertex3f(pos.x,pos.y,pos.z);
				});
			});
	break;
	case 1:
			vAcc.processCurrentNode(flip.dx, [](glm::vec3 boxMin, glm::vec3 boxMax){
			psa_v.iterateNeighbours(flip.particleSet, boxMin, boxMax, [](const Particle& p){
					glm::vec3 pos = p.getPos();
					glVertex3f(pos.x,pos.y,pos.z);
				});
			});
	break;
	case 2:
			cAcc.processCurrentNode(flip.cell, 0.5*flip.dx, [](glm::vec3 boxMin, glm::vec3 boxMax){
			psa_c.iterateNeighbours(flip.particleSet, boxMin, boxMax, [](const Particle& p){
					glm::vec3 pos = p.getPos();
					glVertex3f(pos.x,pos.y,pos.z);
				});
			});	
	}
	
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
	glm::vec2 delta(0,0);
	if(key == GLFW_KEY_W && action == GLFW_PRESS) delta = glm::ivec2(0,1);
	if(key == GLFW_KEY_S && action == GLFW_PRESS) delta = glm::ivec2(0,-1);
	if(key == GLFW_KEY_A && action == GLFW_PRESS) delta = glm::ivec2(-1,0);
	if(key == GLFW_KEY_D && action == GLFW_PRESS) delta = glm::ivec2(1,0);
	switch(mode){
		case 0: uAcc.move(delta); break;
		case 1: vAcc.move(delta); break;
		case 2: cAcc.move(flip.cell, delta); break;
	}
	if(key == GLFW_KEY_G && action == GLFW_PRESS){
		flip.gather(0);
		flip.gather(1);
	}
	if(key == GLFW_KEY_T && action == GLFW_PRESS)
		flip.step();
	if(key == GLFW_KEY_Y && action == GLFW_PRESS)
		flip.solvePressure();
	if(key == GLFW_KEY_M && action == GLFW_PRESS)
		mode = (mode+1)%3;
}

int main(int argc, char **argv){
	init();

	Timer timer;
	timer.start();
	timer.report();

	gd = GraphicsDisplay::create(WIDTH, HEIGHT, std::string("SimpleFLIP3D"));
	gd->registerRenderFunc(render);
	gd->registerButtonFunc(mouseButton);
	gd->registerKeyFunc(keyboard);
	gd->registerMouseFunc(mouse);
	gd->registerScrollFunc(mouseScroll);

	gd->start();

	return 0;
}
