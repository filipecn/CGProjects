#define GLM_FORCE_RADIANS
#include <glm/gtx/quaternion.hpp>
#include "AdaptiveTetTree.h"
#include "MarchingTet.h"
#include "StatUtils.h"
#include <Aergia.h>
#include <GL/glut.h>

AdaptiveTetTree tree;
MarchingTet mt;
double CURVATURE_ANGLE = 30.0;
int curTetrahedron;
int curEdge;
int curVertex;
int curLeaf;

float f(glm::vec3 p){
	return p.x*p.x + p.y*p.y - p.z*p.z - 0.8*0.8;
}

using aergia::io::GraphicsDisplay;
using aergia::graphics::helpers::Grid;
using aergia::math::Transform;
using aergia::graphics::scene::Camera3D;

#define WIDTH 	1200
#define HEIGHT 	1200

GraphicsDisplay* gd;
Grid grid(1.0);

Transform trans;
Camera3D camera;
glm::vec2 start;
bool rotating = false;
bool translating = false;
glm::vec2 panStart;
bool showGrid = true;
bool showSurface = true;

void render(){
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	camera.look();	
	if(showGrid){
		tree.iterateLeaves([](Tetrahedron& tet){
				glEnable (GL_BLEND);
				glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				// edges
				int edges[] = { 0,1, 0,2, 0,3, 1,2, 1,3, 2,3 };
				glColor4f(1.0,1.0,1.0,0.5);
				if(tree.leaves.size() && tet.id == tree.leaves[curLeaf]){
					glColor4f(1.0,0.5,1.0,0.7);
				}
				glBegin(GL_LINES);
				for(int e = 0; e < 6*2; e++)
				glVertex3fv(&tree.vertices[tet.vertices[edges[e]]][0]);
				glEnd();
				// draw particles
				if(tet.id == curTetrahedron){
					glPointSize(3.0);
					glColor4f(1,1,0,0.2);
				}
				else{
					glPointSize(2.0);
					glColor4f(0,1,0,0.4);
				}
				return;
				glPointSize(3.0);
				glBegin(GL_POINTS);
				for(auto it : tet.particles)
					glVertex3fv(&tree.particleSet.pl.particles[it].p[0]);
				if(tree.meshVertices.size()){
					for(int i = 0; i < 4; i++){
						if(tree.meshVertices[tet.vertices[i]].value < 0)
							glColor3f(0,0,1);
						else glColor3f(1,0,0);
						glVertex3fv(&tree.vertices[tet.vertices[i]][0]);
					}
				}
				glEnd();
				
				return;
				//if(tet.id != curTetrahedron) return;
				// vertices
				glPointSize(5.0);
				glBegin(GL_POINTS);
				glColor3f(1,0,0); glVertex3fv(&tree.vertices[tet.sortedVertices[0]][0]);
				glColor3f(0,1,0); glVertex3fv(&tree.vertices[tet.sortedVertices[1]][0]);
				glColor3f(1,0,1); glVertex3fv(&tree.vertices[tet.sortedVertices[2]][0]);
				glColor3f(0,0,1); glVertex3fv(&tree.vertices[tet.sortedVertices[3]][0]);
				glEnd();
				//if(tet.level != tree.maxLevel) return;
				// face normals
				int faces[] = { 0,1,2, 0,3,1, 0,2,3, 1,3,2 };
				glBegin(GL_LINES);
				glColor3f(0,1,0.3);
				for(int f = 0; f < 4; f++){
					glm::vec3 midpoint = (
							tree.vertices[tet.sortedVertices[faces[f*3+0]]]+
							tree.vertices[tet.sortedVertices[faces[f*3+1]]]+
							tree.vertices[tet.sortedVertices[faces[f*3+2]]])/3.0f;
					glm::vec3 endpoint = midpoint + 0.15f*glm::vec3(
							tet.planes[f].x,tet.planes[f].y,tet.planes[f].z);
					glVertex3fv(&midpoint[0]);
					glVertex3fv(&endpoint[0]);
				}
				glEnd();
		});
	}
	glPointSize(5.0);
	glBegin(GL_POINTS);
	glColor3f(1,0,0);
	glVertex3fv(&tree.vertices[curVertex][0]);
	glColor3f(1,1,0);
	int closest = tree.particleSet.getClosestParticle(tree.vertices[curVertex],1.2);
	glVertex3fv(&tree.particleSet.pl.particles[closest].p[0]);
	glEnd();
	// draw surface
	if(showSurface){
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.2,0.4,0.5,1.0);
		glEnable (GL_POLYGON_OFFSET_FILL);
		glPolygonOffset (1.,1.);
		glBegin(GL_TRIANGLES);
		for(int i = 0; i < mt.triangles.size(); i++)
			for(int j = 0; j < 3; j++)
				glVertex3fv(&mt.triangles[i].v[j][0]);
		glEnd();
		glColor4f(0.9,0.7,0.3,1.0);
		glBegin(GL_LINES);
		for(int i = 0; i < mt.triangles.size(); i++)
			for(int j = 0; j < 4; j++){
				glVertex3fv(&mt.triangles[i].v[((j+1)%3)][0]);
				glVertex3fv(&mt.triangles[i].v[(j%3)][0]);
			}
		glEnd();
	}
	return;
	// show particles
	glPointSize(1.0);
	glBegin(GL_POINTS);
	glColor4f(1,1,1,0.2);
		for(auto p : tree.particleSet.pl.particles)	
			glVertex3fv(&p.p[0]);
	glEnd();
	return;
	// draw currentEdge
	int edges[] = { 0,1, 0,2, 0,3, 1,2, 1,3, 2,3 };
	glColor3f(1.0,1.0,0.0);
	glBegin(GL_LINES);
		glVertex3fv(&tree.vertices[
				tree.tetrahedra[curTetrahedron].vertices[edges[curEdge*2+0]]][0]);
		glVertex3fv(&tree.vertices[
				tree.tetrahedra[curTetrahedron].vertices[edges[curEdge*2+1]]][0]);
	glEnd();
	return;
	// draw closest particle
	glPointSize(5.0);
	glBegin(GL_POINTS);
	if(tree.meshVertices.size() > curVertex && tree.meshVertices[curVertex].closestParticle >= 0){
		if(tree.meshVertices[curVertex].value < 0.0)
			glColor3f(1,0,0);
		else 
			glColor3f(0,1,0);
		glVertex3fv(&tree.vertices[curVertex][0]);
		int closest = tree.meshVertices[curVertex].closestParticle;
		glColor3f(0,0,1);
		glVertex3fv(&tree.particleSet.pl.particles[closest].p[0]);
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
	if(key == GLFW_KEY_S && action == GLFW_PRESS){
		Timer timer;
		timer.start();
		tree.step();
		timer.report();
		keyboard(GLFW_KEY_M, GLFW_PRESS);
		timer.report();
		std::cerr << "max level " << tree.maxLevel << std::endl;
		std::cerr << "number of tetrahedra " << tree.tetrahedra.size() << std::endl;
	}
	if(key == GLFW_KEY_M && action == GLFW_PRESS){
		tree.updateMeshVertices();
		mt.triangles.clear();
		tree.iterateLeaves([](const Tetrahedron& tet){
				glm::vec3 tetVertices[4];
				float v[4];
				for(int i = 0; i < 4; i++){
					int vi = tet.vertices[i];
					v[i] = tree.meshVertices[vi].value;
					if(tree.meshVertices[vi].closestParticle < 0){
					std::cerr << "no closest particle!\n";
						exit(1);
					}
					tetVertices[i] = tree.vertices[vi];
				}
				mt.marchTet(tetVertices,v);
				});
		std::cerr << mt.triangles.size() << std::endl;
	}
	if(key == GLFW_KEY_B && action == GLFW_PRESS)
		tree.bisectTet(curTetrahedron);
	if(key == GLFW_KEY_W && action == GLFW_PRESS)
		curTetrahedron = (curTetrahedron - 1 < 0)? tree.tetrahedra.size()-1:curTetrahedron-1;
	if(key == GLFW_KEY_E && action == GLFW_PRESS)
		curTetrahedron = std::min((int)(tree.tetrahedra.size()-1), curTetrahedron + 1);
	if(key == GLFW_KEY_R && action == GLFW_PRESS)
		curEdge = (curEdge+1)%6;
	if(key == GLFW_KEY_V && action == GLFW_PRESS)
		curVertex = (curVertex+1)%tree.vertices.size();
	if(key == GLFW_KEY_C && action == GLFW_PRESS)
		curVertex = (curVertex - 1 < 0)? tree.vertices.size()-1 : curVertex - 1;
	if(key == GLFW_KEY_G && action == GLFW_PRESS)
		showGrid = !showGrid;
	if(key == GLFW_KEY_T && action == GLFW_PRESS)
		showSurface = !showSurface;
	if(key == GLFW_KEY_L && action == GLFW_PRESS)
		curLeaf = (curLeaf+1)%tree.leaves.size();
	if(key == GLFW_KEY_K && action == GLFW_PRESS)
		curLeaf = (curLeaf - 1 < 0)? tree.leaves.size()-1 : curLeaf - 1;
}

int main(int argc, char **argv){
	if(argc > 1){
		FILE *fp = fopen(argv[1], "r+");
		if(!fp) return 1;
		fscanf(fp, " %*s %lf ", &CURVATURE_ANGLE);
		fscanf(fp, " %*s %d ", &tree.minLevelLimit);
		fscanf(fp, " %*s %d ", &tree.maxLevelLimit);
		fclose(fp);
	}

	tree.oracle = std::function<bool(Tetrahedron&)>([](Tetrahedron& tet){
			if(tet.particles.size() == 0)
				return false;
			if(tet.level >= tree.maxLevelLimit)
				return false;
			return !tet.highCurvatureTest(tree.particleSet.pl.particles, 
				cos(CURVATURE_ANGLE*(acos(-1.0)/180.0)));
			});

	tree.stopCondition = std::function<bool(Tetrahedron& tet)>([](const Tetrahedron& tet){
			if(tet.level < tree.minLevelLimit)
				return false;
			if(tet.level >= tree.maxLevelLimit)
				return true;
			return tet.particles.size() <= 50;
			});

	int n; std::cin >> n;
	for(int i = 0; i < n; i++){
		glm::vec3 p, n;
		std::cin >> p.x >> p.y >> p.z;
		std::cin >> n.x >> n.y >> n.z;
		tree.particleSet.add(p,n);
	}
	
	Timer timer;
	timer.start();
	tree.init();
	timer.report();

	gd = GraphicsDisplay::create(WIDTH, HEIGHT, std::string("Simple3D"));
	gd->registerRenderFunc(render);
	gd->registerButtonFunc(mouseButton);
	gd->registerKeyFunc(keyboard);
	gd->registerMouseFunc(mouse);
	gd->registerScrollFunc(mouseScroll);

	gd->start();

	return 0;
}
