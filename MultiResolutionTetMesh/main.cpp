#define GLM_FORCE_RADIANS
#include <glm/gtx/quaternion.hpp>
#include "AdaptiveTetTree.h"
#include "MarchingTet.h"
#include <Aergia.h>
#include <GL/glut.h>

AdaptiveTetTree* tree;
MarchingTet mt;

float f(glm::vec3 p){
	return p.x*p.x + p.y*p.y - p.z*p.z - 0.8*0.8;
}

using aergia::io::GraphicsDisplay;
using aergia::graphics::helpers::Grid;
using aergia::math::Transform;
using aergia::graphics::scene::Camera3D;

#define WIDTH 	800
#define HEIGHT 	800

GraphicsDisplay* gd;
Grid grid(1.0);

Transform t;
Camera3D camera;
glm::vec2 start;
bool dragging = false;

void render(){
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT);

	camera.look();	
	//grid.draw();
	
	tree->iterateTetrahedra([](std::shared_ptr<AdaptiveTetTree::Tetrahedron> tet){
			switch(tet->level % 3){
				case 0: glColor3f(1,1,1); break;
				case 1: glColor3f(1,1,0); break;
				case 2: glColor3f(0,1,1); break;
			}
			int edges[] = { 0,1, 0,2, 0,3, 1,2, 1,3, 2,3 };
			glColor3f(1.0,1.0,1.0);
			glBegin(GL_LINES);
			for(int e = 0; e < 6*2; e++)
				glVertex3fv(&tree->vertices[tet->vertices[edges[e]]][0]);
			glEnd();

			glPointSize(3.0);
			glBegin(GL_POINTS);
			glColor3f(1,0,0); glVertex3fv(&tree->vertices[tet->vertices[0]][0]);
			glColor3f(0,1,0); glVertex3fv(&tree->vertices[tet->vertices[1]][0]);
			glColor3f(1,0,1); glVertex3fv(&tree->vertices[tet->vertices[2]][0]);
			glColor3f(0,0,1); glVertex3fv(&tree->vertices[tet->vertices[3]][0]);
			glEnd();
			});
	
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.7,0.4,0.9,0.4);
	glBegin(GL_TRIANGLES);
	for(int i = 0; i < mt.triangles.size(); i++)
		for(int j = 0; j < 3; j++)
			glVertex3fv(&mt.triangles[i].v[j][0]);
	glEnd();
}

glm::vec3 screenToSphere(double x, double y){
	glm::vec2 p((x / WIDTH)*2.0 - 1.0, (y/HEIGHT)*2.0 - 1.0);
	float z = std::max(0.0f, 1.0f - sqrt(p.x*p.x + p.y*p.y));
	return glm::normalize(vec3(p.x,p.y,z));
}

void mouse(double x, double y){
	if(!dragging)
		return;
	vec3 beg = screenToSphere(start.x, start.y);
	vec3 end = screenToSphere(x,HEIGHT-y);
	glm::vec3 axis = glm::normalize(glm::cross(beg,end));
	float phi = glm::distance(beg, end)/4;
	glm::vec3 newAxis = glm::normalize(glm::mat3(glm::inverse(glm::toMat4(t.r))*glm::inverse(camera.getView()))*axis);
	t.r = glm::rotate(t.r, glm::degrees(phi)*0.3f, newAxis);
	start = gd->getMousePos();
	camera.apply(t);
}

void mouseButton(int button, int action){
	if(dragging){
		dragging = false;
	}
	else {
		dragging = true;
		start = gd->getMousePos();
	}
}

void keyboard(int key, int action){
	if(key == GLFW_KEY_Q && action == GLFW_PRESS)
		gd->stop();
	if(key == GLFW_KEY_S && action == GLFW_PRESS)
		tree->step();
	if(key == GLFW_KEY_A && action == GLFW_PRESS){
		mt.triangles.clear();
		tree->iterateLeaves([](std::shared_ptr<AdaptiveTetTree::Tetrahedron> tet){
				glm::vec3 tetVertices[4];
				for(int i = 0; i < 4; i++)
				tetVertices[i] = tree->vertices[tet->vertices[i]];
				mt.marchTet(tetVertices,f);
				});
	}
}

int main(){
	glm::vec3 bmin(-1,-1,0);
	glm::vec3 bmax(1,1,2);

	tree = new AdaptiveTetTree(bmin,bmax); 
	tree->oracle = std::function<bool(std::shared_ptr<AdaptiveTetTree::Tetrahedron>)>([](std::shared_ptr<AdaptiveTetTree::Tetrahedron> tet){
			int signs[4]; int trueCount = 0;
			for(int i = 0; i < 4; i++){
				signs[i] = (f(tree->vertices[tet->vertices[i]]) < 0)? 0 : 1;
				trueCount += signs[i];
			}
			if(trueCount == 0 || trueCount == 4)
				return false;
			return true;
	});

	gd = GraphicsDisplay::create(WIDTH, HEIGHT, std::string("Simple3D"));
	gd->registerRenderFunc(render);
	gd->registerButtonFunc(mouseButton);
	gd->registerKeyFunc(keyboard);
	gd->registerMouseFunc(mouse);

	gd->start();

	return 0;
}
