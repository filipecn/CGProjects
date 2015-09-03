#define GLM_FORCE_RADIANS
#include <glm/gtx/quaternion.hpp>
#include "AdaptiveTetTree.h"
#include "MarchingTet.h"
#include "StatUtils.h"
#include <Aergia.h>
#include <GL/glut.h>

AdaptiveTetTree tree;
MarchingTet mt;
int curTetrahedron;

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

Transform trans;
Camera3D camera;
glm::vec2 start;
bool rotating = false;
bool translating = false;
glm::vec2 panStart;

void render(){
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT);

	camera.look();	
	//grid.draw();
	tree.iterateTetrahedra([](TetPtr tet){
			glEnable (GL_BLEND);
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			// edges
			int edges[] = { 0,1, 0,2, 0,3, 1,2, 1,3, 2,3 };
			glColor4f(1.0,1.0,1.0,0.1);
			glBegin(GL_LINES);
			for(int e = 0; e < 6*2; e++)
				glVertex3fv(&tree.vertices[tet->vertices[edges[e]]][0]);
			glEnd();
			// draw particles
			glPointSize(3.0);
			glColor4f(1,1,0,0.2);
			glBegin(GL_POINTS);
			for(auto it : tet->particles)
				glVertex3fv(&tree.particles[it].p[0]);
			glEnd();

			return;
			//if(tet->id != curTetrahedron) return;
			// vertices
			glPointSize(5.0);
			glBegin(GL_POINTS);
			glColor3f(1,0,0); glVertex3fv(&tree.vertices[tet->sortedVertices[0]][0]);
			glColor3f(0,1,0); glVertex3fv(&tree.vertices[tet->sortedVertices[1]][0]);
			glColor3f(1,0,1); glVertex3fv(&tree.vertices[tet->sortedVertices[2]][0]);
			glColor3f(0,0,1); glVertex3fv(&tree.vertices[tet->sortedVertices[3]][0]);
			glEnd();
			//if(tet->level != tree.maxLevel) return;
						// face normals
			int faces[] = { 0,1,2, 0,3,1, 0,2,3, 1,3,2 };
			glBegin(GL_LINES);
			glColor3f(0,1,0.3);
			for(int f = 0; f < 4; f++){
				glm::vec3 midpoint = (
				 	tree.vertices[tet->sortedVertices[faces[f*3+0]]]+
					tree.vertices[tet->sortedVertices[faces[f*3+1]]]+
					tree.vertices[tet->sortedVertices[faces[f*3+2]]])/3.0f;
				glm::vec3 endpoint = midpoint + 0.15f*glm::vec3(
					tet->planes[f].x,tet->planes[f].y,tet->planes[f].z);
				glVertex3fv(&midpoint[0]);
				glVertex3fv(&endpoint[0]);
			}
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
	return;
	// draw particles
	glPointSize(1.0);
	glColor3f(1,0,0);
	glBegin(GL_POINTS);
		for(int i = 0; i < tree.particles.size(); i++){
			if(i == 4)
				glColor3f(0,0,1);
			glVertex3fv(&tree.particles[i].p[0]);
		}
	glEnd();
	glBegin(GL_LINES);
		glBegin(GL_POINTS);
		for(int i = 0; i < tree.particles.size(); i++){
				glColor3f(1,0,1);
			glVertex3fv(&tree.particles[i].p[0]);
		}
	glEnd();
/*
	// face normals
	int faces[][12] = {
		{ 0,2,1, 0,1,3, 0,3,2, 1,2,3 }, // even root
		{ 0,1,2, 0,3,1, 0,2,3, 1,3,2 }, // odd root
	};
	glBegin(GL_LINES);
	glColor3f(0,1,0.3);
	glm::vec4 planes[4];
	planes[0] = glm::vec4(-0, 1, 0, -0.991941);
	planes[1] = glm::vec4(0.58526, -0.810846, 0, -0.000420749); 
	planes[2] = glm::vec4(-0.533256, 0, -0.845954, -0.0120862); 
	planes[3] = glm::vec4(0.533256, 0, -0.845954, -0.0127687); 
	for(int f = 0; f < 4; f++){
		glm::vec3 midpoint = (
				tree.particles[faces[0][f*3+0]].p+
				tree.particles[faces[0][f*3+1]].p+
				tree.particles[faces[0][f*3+2]].p)/3.0f;
		glm::vec4 plane = computePlane(
				tree.particles[faces[0][f*3+0]].p,
				tree.particles[faces[0][f*3+1]].p,
				tree.particles[faces[0][f*3+2]].p);
		glm::vec3 endpoint = midpoint + 0.15f*glm::vec3(
				planes[f].x,planes[f].y,planes[f].z);
		glVertex3fv(&midpoint[0]);
		glVertex3fv(&endpoint[0]);
	}
	glEnd();
*/
	return;
	glColor3f(1,1,0);
	glBegin(GL_LINES);
	for(int i = 0; i < tree.particles.size(); i++){
		glm::vec3 endpoint = tree.particles[i].p + tree.particles[i].n;
		glVertex3fv(&tree.particles[i].p[0]);
		glVertex3fv(&endpoint[0]);
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
	if(key == GLFW_KEY_S && action == GLFW_PRESS)
		tree.step();
	if(key == GLFW_KEY_M && action == GLFW_PRESS){
		mt.triangles.clear();
		tree.iterateLeaves([](TetPtr tet){
				glm::vec3 tetVertices[4];
				float v[4];
				for(int i = 0; i < 4; i++){
					tree.updateMeshVertex(tet->vertices[i]);
					v[i] = tree.meshVertices[i].value;
					tetVertices[i] = tree.vertices[tet->vertices[i]];
				}
				mt.marchTet(tetVertices,v);
				});
	}
	if(key == GLFW_KEY_W && action == GLFW_PRESS)
		curTetrahedron = std::max(0, curTetrahedron - 1);
	if(key == GLFW_KEY_E && action == GLFW_PRESS)
		curTetrahedron = std::min(tree.numberOfTetrahedra-1, curTetrahedron + 1);
}

int main(){
	tree.oracle = std::function<bool(TetPtr)>([](TetPtr tet){
			return tet->particles.size() > 0;

			int signs[4]; int trueCount = 0;
			for(int i = 0; i < 4; i++){
				signs[i] = (f(tree.vertices[tet->vertices[i]]) < 0)? 0 : 1;
				trueCount += signs[i];
			}
			if(trueCount == 0 || trueCount == 4)
				return false;
			return true;
			});

	tree.stopCondition = std::function<bool(TetPtr)>([](TetPtr tet){
			return tet->particles.size() <= 50;
			});

	int n; std::cin >> n;
	for(int i = 0; i < n; i++){
		glm::vec3 p, n;
		std::cin >> p.x >> p.y >> p.z;
		std::cin >> n.x >> n.y >> n.z;
		tree.addParticle(p,n);
	}

	Timer timer;
	timer.start();
	tree.init();
	timer.report();
	//tree.explore();
	timer.report();
	std::cerr << "number of tetrahedra " << tree.numberOfTetrahedra << std::endl;

	gd = GraphicsDisplay::create(WIDTH, HEIGHT, std::string("Simple3D"));
	gd->registerRenderFunc(render);
	gd->registerButtonFunc(mouseButton);
	gd->registerKeyFunc(keyboard);
	gd->registerMouseFunc(mouse);
	gd->registerScrollFunc(mouseScroll);

	gd->start();

	return 0;
}
