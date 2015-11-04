#pragma once

#include "Grid.h"

template<typename T>
class GridAccessor {
	private:
	int curI, curJ;
	
	public:
	GridAccessor();
	void move(Grid<T>& grid, glm::ivec2 delta);
	
	//OpenGL
	void fill(Grid<T>& grid, int i, int j);
	void paintGrid(Grid<T>& grid);
	void drawGridNodes(Grid<T>& grid);
	void drawGrid(Grid<T>& grid);
	void processCurrentNode(Grid<T>& grid, double radius, std::function<void(glm::vec3 boxMin, glm::vec3 boxMax) > f);
};

template<typename T>
GridAccessor<T>::GridAccessor(){
	curI = curJ = 0;
}

template<typename T>
void GridAccessor<T>::move(Grid<T>& grid, glm::ivec2 delta){
	curJ = max(0,min(grid.size[1],curJ+delta.y));
	curI = max(0,min(grid.size[0],curI+delta.x));
}

template<typename T>
void GridAccessor<T>::fill(Grid<T>& grid, int i, int j){
	if(i < 0 || j < 0 || i >= grid.size[0] || j >= grid.size[1])
		return;
	float dx = grid.cellSize;
	float half = dx*0.5;
	glm::vec2 wp = grid.gridToWorld(glm::vec2(float(i),float(j)));
	glBegin(GL_QUADS);
		glVertex3f(wp.x - half, wp.y - half, -0.001);
		glVertex3f(wp.x + half, wp.y - half, -0.001);
		glVertex3f(wp.x + half, wp.y + half, -0.001);
		glVertex3f(wp.x - half, wp.y + half, -0.001);
	glEnd();
}

template<typename T>
void GridAccessor<T>::drawGridNodes(Grid<T>& grid){
	glBegin(GL_POINTS);
	for(int i = 0; i < grid.size.x; i++)
		for(int j = 0; j < grid.size.y; j++){
			glm::vec2 wp = grid.gridToWorld(glm::vec2(float(i),float(j)));
			glVertex(wp);
		}
	glEnd();
}

template<typename T>
void GridAccessor<T>::drawGrid(Grid<T>& grid){
	glColor4f(1,1,1,0.5);
	glBegin(GL_LINES);
	float half = grid.cellSize*0.5;
	float dx = grid.cellSize;
	for(int i = 0; i <= grid.size.x; i++){
			glVertex2f(float(i)*dx - half,0.0 - half);
			glVertex2f(float(i)*dx - half,float(grid.size.y)*dx - half);
		}
	for(int i = 0; i <= grid.size.y; i++){
			glVertex2f(0.0 - half,float(i)*dx - half);
			glVertex2f(float(grid.size.x)*dx - half,float(i)*dx - half);
		}
	glEnd();
}

template<>
void GridAccessor<char>::paintGrid(Grid<char>& grid){
	for(int i = 0; i < grid.size[0]; i++)
		for(int j = 0; j < grid.size[1]; j++){
			switch(grid(i,j)){
				case CellType::FLUID: glColor4f(0,0,1,0.3); break;
				case CellType::SOLID: glColor4f(0.5,0.5,0.5,0.3); break;
				case CellType::AIR: glColor4f(1,1,1,0.3); break;
				default: continue;
			}
			fill(grid,i,j);
		}
}

template<typename T>
void GridAccessor<T>::processCurrentNode(Grid<T>& grid, double radius, 
std::function<void(glm::vec3 boxMin, glm::vec3 boxMax) > f){
	glm::vec2 wp = grid.gridToWorld(glm::vec2(float(curI),float(curJ)));
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