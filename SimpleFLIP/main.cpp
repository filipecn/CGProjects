#define GLM_FORCE_RADIANS
#include <glm/gtx/quaternion.hpp>
#include "StatUtils.h"
#include <Aergia.h>
#include <GL/glut.h>
#include "FLIP.h"

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
int W = 10, H = 10;
int I, J;

void init(){
	flip.size = glm::ivec2(W,H);
	flip.dx = 0.1;
	flip.dt = 0.001;
	flip.rho = 1.0;
	flip.gravity = 9.8;
	flip.init();
	
	for(int i = 3; i <= 6; i++)
		for(int j = 3; j <= 6; j++)
			flip.fillCell(i,j);
	for(int i = 0; i < W; i++){
		flip.isSolid(i,0) = true;
		flip.isSolid(i,H-1) = true;
	}
	for(int j = 0; j < H; j++){
		flip.isSolid(0,j) = true;
		flip.isSolid(W-1,j) = true;
	}
	flip.particleSet.init();
}

void drawCells(){
	for(int i = 0; i < flip.size[0]; i++)
		for(int j = 0; j < flip.size[1]; j++){
			switch(flip.cell(i,j)){
				case CellType::FLUID: glColor4f(0,0,1,0.3); break;
				case CellType::SOLID: glColor4f(0.5,0.5,0.5,0.3); break;
				case CellType::AIR: glColor4f(1,1,1,0.3); break;
				default: continue;
			}
			float half = flip.dx/2.0;
			glBegin(GL_QUADS);
				glVertex3f(float(i)*flip.dx - half, float(j)*flip.dx - half, -0.001);
				glVertex3f(float(i)*flip.dx + flip.dx - half, float(j)*flip.dx - half, -0.001);
				glVertex3f(float(i)*flip.dx + flip.dx - half, float(j)*flip.dx + flip.dx - half, -0.001);
				glVertex3f(float(i)*flip.dx - half, float(j)*flip.dx + flip.dx - half, -0.001);
			glEnd();
		}
}

void drawGridVelocities(){
	glPointSize(5.0);
	glBegin(GL_LINES);
	flip.grid.iterateGrids([](std::shared_ptr<Grid<float> > g){
			static int gind = 0;
			switch(gind % 3){
			case 0: glColor3f(1,1,1); break;
			case 1: glColor3f(0,1,1); break;
			case 2: glColor3f(1,1,0); break;
			}
			for(int i = 0; i < g->size.x; i++)
			for(int j = 0; j < g->size.y; j++){
				glVertex2f(float(i)*flip.dx + g->offset.x*flip.dx, float(j)*flip.dx + g->offset.y*flip.dx);
				switch(gind % 3){
				case 0: glVertex2f(float(i)*flip.dx + g->offset.x*flip.dx + flip.dt*(*g)(i,j),  
					           float(j)*flip.dx + g->offset.y*flip.dx); break;
				case 1: glVertex2f(float(i)*flip.dx + g->offset.x*flip.dx,  
					           float(j)*flip.dx + g->offset.y*flip.dx + flip.dt*(*g)(i,j)); break;
				case 2: glVertex2f(float(i)*flip.dx + g->offset.x*flip.dx, 
						   float(j)*flip.dx + g->offset.y*flip.dx);
				}
			}
			gind++;
			});
	glEnd();
}

void drawGrid(){
	glPointSize(5.0);
	glBegin(GL_POINTS);
	flip.grid.iterateGrids([](std::shared_ptr<Grid<float> > g){
			static int gind = 0;
			switch(gind++ % 3){
			case 0: glColor3f(1,1,1); break;
			case 1: glColor3f(0,1,1); break;
			case 2: glColor3f(1,1,0); break;
			}
			for(int i = 0; i < g->size.x; i++)
			for(int j = 0; j < g->size.y; j++)
				glVertex2f(float(i)*flip.dx + g->offset.x*flip.dx, float(j)*flip.dx + g->offset.y*flip.dx);
			});
	glEnd();

	glColor4f(1,1,1,0.5);
	glBegin(GL_LINES);
	float half = flip.dx/2.0;
	for(int i = 0; i < flip.size.x; i++){
			glVertex2f(float(i)*flip.dx - half,0.0 - half);
			glVertex2f(float(i)*flip.dx - half,float(flip.size.y)*flip.dx - half);
		}
	for(int i = 0; i < flip.size.y; i++){
			glVertex2f(0.0 - half,float(i)*flip.dx - half);
			glVertex2f(float(flip.size.x)*flip.dx - half,float(i)*flip.dx - half);
		}
	glEnd();
}

void drawParticles(){
	glPointSize(3.0);
	glColor3f(0,0,1);
	glBegin(GL_POINTS);
	for(auto p : flip.particleSet.pl.particles){
		glVertex2f(p.p.x,p.p.y);
	}
	glEnd();
	glColor4f(0,0,1,0.5);
	glBegin(GL_LINES);
	for(auto p : flip.particleSet.pl.particles){
		glVertex2f(p.p.x,p.p.y);
		glVertex2f(p.p.x + flip.dt*p.v.x, p.p.y + flip.dt*p.v.y);
	}
	glEnd();
}

void render(){
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	camera.look();	

	drawCells();
	drawGrid();	
	drawParticles();
	drawGridVelocities();

	glPointSize(4.0);
	glColor3f(1,0,1);
	glm::vec3 boxMin = glm::vec3(float(I)*flip.dx - flip.dx, 
				     float(J)*flip.dx - 0.5*flip.dx - flip.dx, 0.f);
	glm::vec3 boxMax = glm::vec3(float(I)*flip.dx + flip.dx, 
				     float(J)*flip.dx - 0.5*flip.dx + flip.dx, 0.f);
	glBegin(GL_LINE_LOOP);
		glVertex3f(boxMin.x,boxMin.y,0.001);
		glVertex3f(boxMin.x,boxMax.y,0.001);
		glVertex3f(boxMax.x,boxMax.y,0.001);
		glVertex3f(boxMax.x,boxMin.y,0.001);
	glEnd();
	
	glPointSize(5.0);
	glBegin(GL_POINTS);
		glVertex3f(float(I)*flip.dx, float(J)*flip.dx - 0.5*flip.dx, 0.001);
		//std::cout << (float(I)*flip.dx) << " , " << (float(J)*flip.dx - 0.5*flip.dx) << std::endl;
		flip.particleSet.iterateNeighbours(boxMin,boxMax,[](const Particle& p){
				glVertex3f(p.p.x,p.p.y,p.p.z);
				});
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
	if(key == GLFW_KEY_G && action == GLFW_PRESS){
		flip.gather(0);
		flip.gather(1);
	}
	if(key == GLFW_KEY_T && action == GLFW_PRESS)
		flip.step();
	if(key == GLFW_KEY_Y && action == GLFW_PRESS)
		flip.solvePressure();
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
