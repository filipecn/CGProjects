#pragma once

#include "MathUtils.hpp"

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

	glm::vec2 offset;
	glm::ivec2 size;
	T border;
	bool useBorder;

	T& operator() (int i, int j);
	T  operator() (int i, int j) const;

	T sample(T x, T y);
	
	void destroy();
	void set(int w, int h);
	void setAll(T v);

private:
	T **data;
};

template<typename T>
Grid<T>::Grid(){
	size = glm::ivec2(0,0);
	data = nullptr;
	useBorder = false;
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
void Grid<T>::destroy(){
	if(data){
		for (int i = 0; i < size[0]; i++)
			delete[] data[i];
		delete[] data;

	}
}

template<typename T>
void Grid<T>::set(int w, int h){
	destroy();
	size = glm::ivec2(w, h);
	
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
T Grid<T>::sample(T x, T y) {
	int x0 = int(x);
	int y0 = int(y);
	int x1 = x0 + 1;
	int y1 = y0 + 1;
	if(useBorder){
		return 1.0;
	}
	else{
		x0 = max(0, min(size.x-1,x0));
		y0 = max(0, min(size.y-1,y0));
		x1 = max(0, min(size.x-1,x1));
		y1 = max(0, min(size.y-1,y1));
		return bilinearInterpolation(data[x0][y0],data[x1][y0],data[x1][y1],data[x0][y1],
				x - double(x0),y - double(y0));
	}
}

template<typename T>
void Grid<T>::setAll(T v){
	assert(data != nullptr);
	for (int i = 0; i < size[0]; i++)
		for (int j = 0; j < size[1]; j++)
			data[i][j] = v;
}
