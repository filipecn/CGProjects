#pragma once

#include "Grid.h"

template<typename T>
class GridPtrAccessor {
	private:
	GridPtr<T> grid;
	
	int curI, curJ;
	
	public:
	GridPtrAccessor();
	
	void setGrid(GridPtr<T> g);
	
	void move(glm::ivec2 delta);
	
	void iterateAll(std::function<void(int i, int j) >);
	
	//OpenGL
	void fill(int i, int j);
	void drawVelocities(glm::vec2 direction, float scale);
	void drawGridNodes();
	void processCurrentNode(double radius, std::function<void(glm::vec3 boxMin, glm::vec3 boxMax) > f);
};

template<typename T>
GridPtrAccessor<T>::GridPtrAccessor(){
	setGrid(nullptr);
}

template<typename T>
void GridPtrAccessor<T>::setGrid(GridPtr<T> g){
	grid = g;
	curI = curJ = 0;
}

template<typename T>
void GridPtrAccessor<T>::move(glm::ivec2 delta){
	if(grid != nullptr){
		curJ = max(0,min(grid->size[1],curJ+delta.y));
		curI = max(0,min(grid->size[0],curI+delta.x));
	}
}

template<typename T>
void GridPtrAccessor<T>::fill(int i, int j){
	if(grid == nullptr || curI < 0 || curJ < 0 || curI >= grid->size[0] || curJ >= grid->size[1])
		return;
	float dx = grid->cellsize;
	float half = dx*0.5;
	glm::vec2 wp = grid->gridToWorld(glm::vec2(float(curI),float(curJ)));
	glBegin(GL_QUADS);
		glVertex3f(wp.x - half, wp.y - half, -0.001);
		glVertex3f(wp.x + half, wp.y - half, -0.001);
		glVertex3f(wp.x + half, wp.y + half, -0.001);
		glVertex3f(wp.x - half, wp.y + half, -0.001);
	glEnd();
}

template<typename T>
void GridPtrAccessor<T>::drawVelocities(glm::vec2 direction, float scale){
	if(grid == nullptr)
		return;
	glBegin(GL_LINES);
	for(int i = 0; i < grid->size[0]; i++)
		for(int j = 0; j < grid->size[1]; j++){
			glm::vec2 v = direction*scale*(*grid)(i,j);
			glm::vec2 wp = grid->gridToWorld(glm::vec2(float(i),float(j)));
			glVertex(wp);	
			glVertex(wp + v);
		} 
	glEnd();
}

template<typename T>
void GridPtrAccessor<T>::drawGridNodes(){
	glBegin(GL_POINTS);
		for(int i = 0; i < grid->size.x; i++)
			for(int j = 0; j < grid->size.y; j++){
				glm::vec2 wp = grid->gridToWorld(glm::vec2(float(i),float(j)));
				glVertex(wp);
			}
	glEnd();
}

template<typename T>
void GridPtrAccessor<T>::processCurrentNode(double radius, 
	std::function<void(glm::vec3 boxMin, glm::vec3 boxMax) > f){
	if(grid == nullptr || curI < 0 || curJ < 0 || curI >= grid->size[0] || curJ >= grid->size[1])
		return;
	glm::vec2 wp = grid->gridToWorld(glm::vec2(float(curI),float(curJ)));
	glm::vec3 boxMin = glm::vec3(wp.x - radius, wp.y - radius, -1.0f);
	glm::vec3 boxMax = glm::vec3(wp.x + radius, wp.y + radius, 1.0f);
	glColor3f(1,0,1);
	glPointSize(4.0);
	glBegin(GL_POINTS);
			glVertex3f(wp.x,wp.y,0.001);
	glEnd();
	glBegin(GL_LINE_LOOP);
		glVertex3f(boxMin.x,boxMin.y,0.001);
		glVertex3f(boxMin.x,boxMax.y,0.001);
		glVertex3f(boxMax.x,boxMax.y,0.001);
		glVertex3f(boxMax.x,boxMin.y,0.001);
	glEnd();
	glPointSize(5.0);
	glColor3f(1,0,1);
	glBegin(GL_POINTS);		
		f(boxMin,boxMax);
	glEnd();
}