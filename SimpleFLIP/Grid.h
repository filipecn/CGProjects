#pragma once

#include "utils.h"

#include <iostream>
#include <glm/glm.hpp>

enum class WrapMode { BORDER, EDGE, REPEAT };

template<typename T>
class Grid;

template<typename T>
using GridPtr = std::shared_ptr<Grid<T> >;

template<typename T>
class Grid {
public:
	Grid();
	Grid(int w, int h);
	
	~Grid();
	
	double cellSize;
	glm::vec2 offset;
	glm::ivec2 size;
	glm::vec2 gridToWorld(glm::vec2 p) const;
	
	T border;
	bool useBorder;

	T& operator() (int i, int j);
	T  operator() (int i, int j) const;
    T sample(double x, double y) const;
	T dSample(double x, double y, T r);
	glm::ivec2 cellID(double x, double y) const;
	T safeData(int i, int j) const;
	
	void destroy();
	void set(int w, int h, double cellSize = 1.0);
	void setAll(T v);

private:
	T **data;
};

template<typename T>
Grid<T>::Grid(){
	size = glm::ivec2(0,0);
	data = nullptr;
	useBorder = false;
	cellSize = 1.0;
}

template<typename T>
Grid<T>::Grid(int w, int h) {
	set(w,h);
}

template<typename T>
Grid<T>::~Grid() {
	destroy();
}

template<typename T>
glm::vec2 Grid<T>::gridToWorld(glm::vec2 p) const{
	return float(cellSize)*p + offset;
}

template<typename T>
void Grid<T>::destroy(){
	if(data){
		for (int i = 0; i < size[0]; i++)
			delete[] data[i];
		delete[] data;

	}
}

template<typename T>
void Grid<T>::set(int w, int h, double cellSize){
	destroy();
	size = glm::ivec2(w, h);
	this->cellSize = cellSize;
	
	data = new T*[w];
	for (int i = 0; i < w; i++)
		data[i] = new T[h];
}

template<typename T>
T& Grid<T>::operator() (int i, int j) {
	assert(data != nullptr);
	assert(i >= 0 && i < size[0] && j >= 0 && j < size[1]);
	return data[i][j];
}

template<typename T>
T Grid<T>::operator() (int i, int j) const {
	assert(data != nullptr);
	assert(i >= 0 && i < size[0] && j >= 0 && j < size[1]);
	return data[i][j];
}

template<typename T>
T Grid<T>::sample(double x, double y) const{
	int x0 = (x - offset.x)/(cellSize);
	int y0 = (y - offset.y)/(cellSize);
	int x1 = (x - offset.x)/(cellSize) + 1;
	int y1 = (y - offset.y)/(cellSize) + 1;
	if(useBorder){
		return 1.0;
	}
	else{
		x0 = max(0, min(size.x-1,x0));
		y0 = max(0, min(size.y-1,y0));
		x1 = max(0, min(size.x-1,x1));
		y1 = max(0, min(size.y-1,y1));
		
		glm::vec2 wp = gridToWorld(glm::vec2(x0,y0));
		
		//std::cerr << "for xy = " << x << " " << y << std::endl;
		//std::cerr << x0 << " " << y0 << " - " << x1 << " " << y1 << std::endl; 
		//std::cerr << "bilinear " << 
		//return	bilinearInterpolation(data[x0][y0],data[x1][y0],data[x1][y1],data[x0][y1],
		//		 (x - wp.x)/cellSize, (y - wp.y)/cellSize);
		//		<< std::endl;
		T p[4][4];
		int delta[] = {-1,0,1,2};
		for(int i = 0; i < 4; i++)
			for(int j = 0; j < 4; j++)
				p[i][j] = safeData(x0+delta[i],y0+delta[j]);
		return bicubicInterpolate<T>(p, (x - wp.x)/cellSize, (y - wp.y)/cellSize);
	}
}

template<typename T>
T Grid<T>::dSample(double x, double y, T r){
	glm::ivec2 cell = cellID(x,y);
	if(cell[0] < 0 || cell[0] >= size[0] || cell[1] < 0 || cell[1] >= size[1])
		return r;
	return data[cell[0]][cell[1]];
}

template<typename T>
glm::ivec2 Grid<T>::cellID(double x, double y) const {
	double half = cellSize*0.5;
	return glm::ivec2(int((x + half + offset.x)/cellSize),
					  int((y + half + offset.y)/cellSize));
}

template<typename T>
void Grid<T>::setAll(T v){
	assert(data != nullptr);
	for (int i = 0; i < size[0]; i++)
		for (int j = 0; j < size[1]; j++)
			data[i][j] = v;
}

template<typename T>
T Grid<T>::safeData(int i, int j) const{
	assert(size[0] && size[1] && data != nullptr);
	return data[max(0, min(size.x-1,i))][max(0, min(size.y-1,j))];
}