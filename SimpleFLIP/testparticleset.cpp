#define GLM_FORCE_RADIANS
#include <glm/gtx/quaternion.hpp>
#include "StatUtils.h"
#include <Aergia.h>
#include <GL/glut.h>
#include "ParticleSet.h"
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

int W = 10, H = 10;
int I, J;
ParticleSet ps;
double OFFSET = 0.1;
double SPACING = 0.2;
double RADIUS = 0.2;

void init(){
	srand (time(NULL));

	// first add points for the grid
	for(int i = 0; i < 6; i++)
		for(int j = 0; j < 5; j++)
			ps.add(glm::vec3(SPACING*float(i)-OFFSET,SPACING*j,0),glm::vec3(0,0,0),glm::vec3(0,0,0));
	for(int i = 0; i < 5; i++)
		for(int j = 0; j < 6; j++)
			ps.add(glm::vec3(SPACING*i,SPACING*float(j)-OFFSET,0),glm::vec3(0,0,0),glm::vec3(0,0,0));
	for(int i = 0; i < 5; i++)
		for(int j = 0; j < 5; j++)
			ps.add(glm::vec3(SPACING*i,SPACING*j,0),glm::vec3(0,0,0),glm::vec3(0,0,0));
	for(int i = 0; i < W*W; i++){
		float x = float(rand())/RAND_MAX; x*=5*SPACING;
		float y = float(rand())/RAND_MAX; y*=5*SPACING;
		ps.add(glm::vec3(x,y,0),glm::vec3(0,0,0),glm::vec3(0,0,0));
	}

	W = H = 5*6; 
}

void render(){

	ParticleSetAccessor psu, psp, psv;
	psu.set(ps, glm::vec3(OFFSET,0,0),SPACING);
	psv.set(ps, glm::vec3(0,OFFSET,0),SPACING);
	psp.set(ps, glm::vec3(0,0,0),SPACING);
	
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	camera.look();	
	
	int II = I + 5*6;
	int JJ = J + 6*5;

	glPointSize(3.0);
	glBegin(GL_POINTS);
	glColor3f(1,0,0);
		for(int i = 0; i < 5*6; i++)
			glVertex2f(ps.particles[i].p.x,ps.particles[i].p.y);
	glColor3f(0,1,0);
		for(int i = 5*6; i < 5*6*2; i++)
			glVertex2f(ps.particles[i].p.x,ps.particles[i].p.y);
	glColor3f(1,1,1);
		for(int i = 2*5*6; i < 2*5*6 + 5*5; i++)
			glVertex2f(ps.particles[i].p.x,ps.particles[i].p.y);
	glEnd();
	glPointSize(3.0);
	glBegin(GL_POINTS);
	glColor3f(0,1,1);
	for(int i = 0; i < ps.particles.size(); i++)
		glVertex2f(ps.particles[i].p.x,ps.particles[i].p.y);
	glEnd();
	
	glPointSize(10.0);
	glBegin(GL_POINTS);
	glColor3f(1,0,1);
		glVertex2f(ps.particles[II].p.x,ps.particles[II].p.y);
	glEnd();
	

	glm::vec3 bmin = glm::vec3(ps.particles[II].p.x-RADIUS*1.0,ps.particles[II].p.y-RADIUS*1.0,-1.0);
	glm::vec3 bmax = glm::vec3(ps.particles[II].p.x+RADIUS*1.0,ps.particles[II].p.y+RADIUS*1.0,1.0);
	glBegin(GL_LINE_LOOP);
		glVertex2f(bmin.x,bmin.y);
		glVertex2f(bmin.x,bmax.y);
		glVertex2f(bmax.x,bmax.y);
		glVertex2f(bmax.x,bmin.y);
	glEnd();
	
	glColor3f(1,0,1);
	glBegin(GL_POINTS);
	psu.iterateNeighbours(ps, bmin, bmax, [](const Particle& pa){
				glVertex2f(pa.p.x,pa.p.y);
			});
	glEnd();
	
	glBegin(GL_POINTS);
	glColor3f(0,0,1);
		glVertex2f(ps.particles[JJ].p.x,ps.particles[JJ].p.y);
	glEnd();

	glColor3f(1,1,0);
	glBegin(GL_POINTS);
	psu.iterateNeighbours(ps, ps.particles[JJ].p, RADIUS, [](const Particle& pa){
				glVertex2f(pa.p.x,pa.p.y);
			});
	glEnd();
	glBegin(GL_LINE_LOOP);
		float step = acos(-1.0)/20;
		float angle = 0.0;
		while(angle < 2*acos(-1.0)){
			glVertex2f(	RADIUS*cos(angle) + ps.particles[JJ].p.x,
					RADIUS*sin(angle) + ps.particles[JJ].p.y);
			angle += step;
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
	if(key == GLFW_KEY_M && action == GLFW_PRESS){
	for(int i = 5*6*2; i < ps.particles.size(); i++)
		ps.particles[i].setPos(glm::vec3(
			ps.particles[i].p.x +  (float(rand())/RAND_MAX)*SPACING,
			ps.particles[i].p.y +  (float(rand())/RAND_MAX)*SPACING,
			ps.particles[i].p.z +  (float(rand())/RAND_MAX)*SPACING
		));
	}
	if(key == GLFW_KEY_C && action == GLFW_PRESS){
	float x = float(rand())/RAND_MAX; x*=5*SPACING;
	float y = float(rand())/RAND_MAX; y*=5*SPACING;
	ps.add(glm::vec3(x,y,0),glm::vec3(0,0,0),glm::vec3(0,0,0));
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
