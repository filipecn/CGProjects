#pragma once

#include <memory>
#include <list>
#include <queue>
#include <vector>
#include "common\NumericalTypes.hpp"
#include <glm\glm.hpp>

template <class T>
class UGrid {
public:
	UGrid(T xmin, T xmax, T ymin, T ymax);
	~UGrid();

	void generateFullGrid(uint level);
	void splitCell(uint c);

private:

	std::vector<glm::vec2> vertices;

	struct Edge;
	struct Cell;
	using EdgePtr = std::shared_ptr<Edge>;
	using CellPtr = std::shared_ptr<Cell>;

	struct Edge {
		glm::ivec2 v;
		T data;
		EdgePtr l, r;
		Edge(glm::ivec2 _v) {
			l = r = nullptr;
			v = _v;
		}
		void split() {
			assert((l && r) || (!l && !r));
			if (!l) {
				l = EdgePtr(new Edge(glm::ivec2(v[0],vertices.size())));
				r = EdgePtr(new Edge(glm::ivec2(vertices.size(),v[1])));
				vertices.emplace_back((vertices[v[0]] + vertices[v[1]]) / 2.0);
			}
		}
	};
	
	struct Cell {
		uint level;
		bool splitted;
		T data;
		EdgePtr edges[4];
		Cell() {
			splitted = false;
			level = 0;
		}
	};
	
	std::vector<EdgePtr> edges;
	std::vector<Cell> cells;
};

template <class T>
UGrid<T>::UGrid(T xmin, T xmax, T ymin, T ymax) {
	
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
}

template <class T>
UGrid<T>::~UGrid() {

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
		cells[c].edges[i]->split();
	// create middle vertex
	uint vIndices[] = {
		cells[c].edges[0]->v[0],
		cells[c].edges[0]->v[1],
		cells[c].edges[2]->v[1],
		cells[c].edges[2]->v[0],
		cells[c].edges[0]->l->v[1],
		cells[c].edges[1]->l->v[1],
		cells[c].edges[2]->l->v[1],
		cells[c].edges[3]->l->v[1],
		vertices.size();
	}
	vertices.emplace_back((vertices[vIndices[5]] + vertices[vIndices[7]]) / 2.0);
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

	cells[cells.size() - 3].edges[0] = cells[c].edges[0]->r;
	cells[cells.size() - 3].edges[1] = cells[c].edges[1]->l;
	cells[cells.size() - 3].edges[2] = edges[edges.size() - 3];
	cells[cells.size() - 3].edges[3] = edges[edges.size() - 4];

	cells[cells.size() - 2].edges[0] = edges[edges.size() - 3];
	cells[cells.size() - 2].edges[1] = cells[c].edges[1]->r;
	cells[cells.size() - 2].edges[2] = cells[c].edges[2]->r;
	cells[cells.size() - 2].edges[3] = edges[edges.size() - 2];

	cells[cells.size() - 1].edges[0] = edges[edges.size() - 1];
	cells[cells.size() - 1].edges[1] = edges[edges.size() - 2];
	cells[cells.size() - 1].edges[2] = cells[c].edges[2]->l;
	cells[cells.size() - 1].edges[3] = cells[c].edges[3]->r;

	for (int i = 1; i <= 4; i++)
		cells[cells.size() - i].level = cells[c].level + 1;

	cells[c].splitted = true;
}

template <class T>
void UGrid<T>::generateFullGrid(uint level) {
	assert(cells.size() == 1 && !cells[0].splitted);
	std::queue<uint> q;
	q.emplace(0);
	while (!q.empty()) {
		uint c = q.front();
		q.pop();
		if (cells[c].level == level)
			continue;
		splitCell(c);
		for (int i = 1; i <= 4; i++)
			q.emplace(cells.size() - i);
	}
}