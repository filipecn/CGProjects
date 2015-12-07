#pragma once

#include <memory>
#include <list>
#include <queue>
#include <vector>
#include <functional>
#include <iostream>
#include <aergia/math/numericalTypes.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

template <typename T>
class IGridData {
	public:
		IGridData(){}
		virtual ~IGridData(){}
		virtual IGridData<T>* split(int vIndices[], const std::vector<glm::vec2>& vertices){
			return nullptr;
		}
	//protected:
		T data;
};

template <class T>
class UGrid {
	public:
		UGrid(double xmin, double xmax, double ymin, double ymax, T* d = nullptr);
		~UGrid();

		void set(double xmin, double xmax, double ymin, double ymax, T* d);
		void generateFullGrid(uint level, std::function<const bool(typename UGrid<T>::Cell&)> oracle = nullptr);
		void splitCell(uint c);

		struct Edge;
		struct Cell;
		using EdgePtr = std::shared_ptr<Edge>;
		using CellPtr = std::shared_ptr<Cell>;

		struct Edge {
			glm::ivec2 v;
			//T data;
			EdgePtr l, r;
			Edge(glm::ivec2 _v) {
				l = r = nullptr;
				v = _v;
			}
			void split(std::vector<glm::vec2>& vertices) {
				assert((l && r) || (!l && !r));
				if (!l) {
					l = EdgePtr(new Edge(glm::ivec2(v[0],vertices.size())));
					r = EdgePtr(new Edge(glm::ivec2(vertices.size(),v[1])));
					vertices.emplace_back((vertices[v[0]] + vertices[v[1]]) / 2.0f);
				}
			}
		};
	
		struct Cell {
			uint level;
			bool splitted;
			T* data;
			EdgePtr edges[4];
			int children[4];
			Cell() {
				splitted = false;
				level = 0;
				for(int i = 0; i < 4; i++) children[i] = -1;
			}
		};
	
	//private:
		std::vector<glm::vec2> vertices;
		std::vector<EdgePtr> edges;
		std::vector<Cell> cells;
};

template <class T>
UGrid<T>::UGrid(double xmin, double xmax, double ymin, double ymax, T* d) {
	set(xmin,xmax,ymin,ymax,d);
}
	
template <class T>
UGrid<T>::~UGrid() {

}

template <class T>
void UGrid<T>::set(double xmin, double xmax, double ymin, double ymax, T* d) {
	vertices.clear();
	cells.clear();
	edges.clear();
	
	vertices.emplace_back(xmin, ymin);
	vertices.emplace_back(xmax, ymin);
	vertices.emplace_back(xmax, ymax);
	vertices.emplace_back(xmin, ymax);
	
	cells.emplace_back();
	edges.emplace_back(EdgePtr(new Edge(glm::ivec2(0, 1))));
	edges.emplace_back(EdgePtr(new Edge(glm::ivec2(1, 2))));
	edges.emplace_back(EdgePtr(new Edge(glm::ivec2(3, 2))));
	edges.emplace_back(EdgePtr(new Edge(glm::ivec2(0, 3))));
	
	for (int i = 0; i < 4; i++)
		cells[0].edges[i] = edges[i];
	
	cells[0].data = d;
}

template <class T>
void UGrid<T>::splitCell(uint c) {
	assert(c < cells.size() && !cells[c].splitted);
	//       3 _____>6______> 2
	//        ^      ^      ^
	//        |   3  |   2  |
	//       7|----->8----->|5
	//        ^   0  ^   1  ^
	//        |_____>|_____>|
	//       0       4       1
	
	// split edges
	for (int i = 0; i < 4; i++)
		cells[c].edges[i]->split(vertices);
	// create middle vertex
	int vIndices[] = {
		cells[c].edges[0]->v[0],
		cells[c].edges[0]->v[1],
		cells[c].edges[2]->v[1],
		cells[c].edges[2]->v[0],
		cells[c].edges[0]->l->v[1],
		cells[c].edges[1]->l->v[1],
		cells[c].edges[2]->l->v[1],
		cells[c].edges[3]->l->v[1],
		int(vertices.size())
	};
	vertices.emplace_back((vertices[vIndices[5]] + vertices[vIndices[7]]) / 2.0f);
	// create new edges
	edges.emplace_back(new Edge(glm::ivec2(vIndices[4], vIndices[8])));
	edges.emplace_back(new Edge(glm::ivec2(vIndices[8], vIndices[5])));
	edges.emplace_back(new Edge(glm::ivec2(vIndices[8], vIndices[6])));
	edges.emplace_back(new Edge(glm::ivec2(vIndices[7], vIndices[8])));
	// create new cells
	cells.resize(cells.size() + 4);
	cells[cells.size() - 4].edges[0] = cells[c].edges[0]->l;
	cells[cells.size() - 4].edges[1] = edges[edges.size() - 4];
	cells[cells.size() - 4].edges[2] = edges[edges.size() - 1];
	cells[cells.size() - 4].edges[3] = cells[c].edges[3]->l;
	cells[c].children[0] = cells.size() - 4;

	cells[cells.size() - 3].edges[0] = cells[c].edges[0]->r;
	cells[cells.size() - 3].edges[1] = cells[c].edges[1]->l;
	cells[cells.size() - 3].edges[2] = edges[edges.size() - 3];
	cells[cells.size() - 3].edges[3] = edges[edges.size() - 4];
	cells[c].children[1] = cells.size() - 3;

	cells[cells.size() - 2].edges[0] = edges[edges.size() - 3];
	cells[cells.size() - 2].edges[1] = cells[c].edges[1]->r;
	cells[cells.size() - 2].edges[2] = cells[c].edges[2]->r;
	cells[cells.size() - 2].edges[3] = edges[edges.size() - 2];
	cells[c].children[2] = cells.size() - 2;

	cells[cells.size() - 1].edges[0] = edges[edges.size() - 1];
	cells[cells.size() - 1].edges[1] = edges[edges.size() - 2];
	cells[cells.size() - 1].edges[2] = cells[c].edges[2]->l;
	cells[cells.size() - 1].edges[3] = cells[c].edges[3]->r;
	cells[c].children[3] = cells.size() - 1;

	for (int i = 1; i <= 4; i++)
		cells[cells.size() - i].level = cells[c].level + 1;
	
	if(cells[c].data != nullptr){
		T* splittedData = cells[c].data->split(vIndices, vertices);
		for(int i = 0; i < 4; i++)
			cells[cells.size() - i - 1].data = std::move(&splittedData[4 - i - 1]);
		
	}	
	cells[c].splitted = true;
}

template <class T>
void UGrid<T>::generateFullGrid(uint level, std::function<const bool(typename UGrid<T>::Cell&)> oracle) {
	assert(cells.size() == 1 && !cells[0].splitted);
	std::queue<uint> q;
	q.emplace(0);
	while (!q.empty()) {
		uint c = q.front();
		q.pop();
		if (cells[c].level == level)
			continue;
		if(oracle && !oracle(cells[c]))
			continue;
		splitCell(c);
		for (int i = 1; i <= 4; i++)
			q.emplace(cells.size() - i);
	}
}
		
template <class T>
class UGridIterator {
	friend class UGrid<T>;
	public:
		UGridIterator(UGrid<T>* g){
			grid = g;
			curIndex = 0;
			
			emptyFilter = [](typename UGrid<T>::Cell& c){
				
				return true;
			};
			
			leafFilter = [](typename UGrid<T>::Cell& c){
				return c.children[0] < 0 && c.children[1] < 0 && c.children[2] < 0 && c.children[3] < 0; 	
			};
			
			filter = emptyFilter;
		}
		
		void filterCells(std::function<bool(typename UGrid<T>::Cell&)> f){
			curIndex = 0;
			filter = f;
			while(test() && !filter(grid->cells[curIndex])) curIndex++;
		}
		
		void leafs(){
			curIndex = 0;
			filter = leafFilter;
			while(test() && !filter(grid->cells[curIndex])) curIndex++;
		}
		
		void cells(){
			curIndex = 0;
			filter = emptyFilter;
			while(test() && !filter(grid->cells[curIndex])) curIndex++;
		}
		
		bool test(){
			return grid != nullptr && curIndex >= 0 && curIndex < grid->cells.size();
		} 
		
		void next(){
			curIndex++;
			if(filter != nullptr)
				while(test() && !filter(grid->cells[curIndex])) curIndex++;
		}
		
		const typename UGrid<T>::Cell operator*(){
			return grid->cells[curIndex];
		}
		
		const typename UGrid<T>::Cell& cell(){
			return grid->cells[curIndex];
		}
		
		const T* data(){
			return grid->cells[curIndex].data;
		}
		
		const glm::vec2& vertex(uint i){
			i = i % 4;
			const typename UGrid<T>::Cell& cell = grid->cells[curIndex];
			return grid->vertices[cell.edges[i]->v[i/2]];
		}
		
	private:
		UGrid<T>* grid;
		typename UGrid<T>::CellPtr curCell;
		int curIndex;
		std::function<bool(typename UGrid<T>::Cell&)> emptyFilter, leafFilter;
		std::function<bool(typename UGrid<T>::Cell&)> filter;
};