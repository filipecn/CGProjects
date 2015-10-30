#pragma once

#include "Grid.h"

#include <iostream>
#include <functional>
#include <memory>

enum GridType {U = 0, V, P, GRIDTYPES};

template<typename T>
class MACGrid {
public:
	MACGrid();
	~MACGrid();

	void set(int w, int h, double dx);

	void iterateGrids(std::function<void(GridPtr<T>) > f);
	GridPtr<T> get(uint i);
private:
	std::vector<GridPtr<T> > grids;
};

template<typename T>
MACGrid<T>::MACGrid(){}

template<typename T>
MACGrid<T>::~MACGrid(){}

template<typename T>
void MACGrid<T>::set(int w, int h, double dx){
	grids.emplace_back(new Grid<T>(w+1,h)); // U
	grids[0]->offset = glm::vec2(-0.5*dx,0.0);
	grids.emplace_back(new Grid<T>(w,h+1)); // V
	grids[1]->offset = glm::vec2(0.0,-0.5*dx);
	grids.emplace_back(new Grid<T>(w,h));   // P
	grids[2]->offset = glm::vec2(0.0,0.0);

	for(int i = 0; i < 3; i++){
		grids[i]->cellSize = dx;
		grids[i]->setAll(0.0);
	}
}

template<typename T>
void MACGrid<T>::iterateGrids(std::function<void(GridPtr<T>) > f){
	for(auto g : grids)
		f(g);
}

template<typename T>
GridPtr<T> MACGrid<T>::get(uint g){
	return grids[g];
}

