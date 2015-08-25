#include <vector>
#include <map>
#include <queue>
#include <memory>
#include <functional>
#include <iostream>
#include <glm/glm.hpp>

class AdaptiveTetTree {
	public:
		AdaptiveTetTree(glm::vec3 bmin, glm::vec3 bmax);
		~AdaptiveTetTree(){};

		struct Tetrahedron {
			int level;
			bool active;
			glm::ivec4 vertices;
			std::vector<size_t> points;
			std::shared_ptr<Tetrahedron> children[2];
			Tetrahedron(glm::ivec4 v){
				level = 0;
				active = true;
				vertices = v;
				for(int i = 0; i < 2; i++)
					children[i] = nullptr;
			}
		};
		// Edges
		using Edge = std::pair<int,int>;
		std::map<Edge, int> cutVertices;
		// Data
		std::vector<glm::vec3> vertices;
		std::shared_ptr<Tetrahedron> root[6];
		std::queue<std::shared_ptr<Tetrahedron> > activeTetrahedra;
		std::vector<std::shared_ptr<Tetrahedron> > leaves;
		// Aux Functions
		void iterateLeaves(std::function<void(std::shared_ptr<Tetrahedron>)> f);
		void iterateTetrahedra(std::function<void(std::shared_ptr<Tetrahedron>)> f);
		void dfsTetrahedra(std::shared_ptr<Tetrahedron> tet, std::function<void(std::shared_ptr<Tetrahedron>)> f);
		// Call Functions
		void step();
		void explore();
		// Adaptivity
		std::vector<glm::vec3> pointSet;
		std::function<bool(std::shared_ptr<Tetrahedron>)> oracle;
	protected:
		inline int cutEdge(int a, int b);
		void bisectTet(std::shared_ptr<Tetrahedron> tet);
};

AdaptiveTetTree::AdaptiveTetTree(glm::vec3 bmin, glm::vec3 bmax){
	//  p6 -------------p7     Y
	//    /|          /|       | 
	//  p4------------ |       |
	//   | |      p5 | |       /---- X
	//   | |         | |     Z/ 
	//   | |p2_______|_|p3
	//   |/          |/ 
	//  p0-----------p1
	vertices.emplace_back(bmin.x, bmin.y, bmax.z); //p0
	vertices.emplace_back(bmax.x, bmin.y, bmax.z); //p1
	vertices.emplace_back(bmin.x, bmin.y, bmin.z); //p2
	vertices.emplace_back(bmax.x, bmin.y, bmin.z); //p3
	vertices.emplace_back(bmin.x, bmax.y, bmax.z); //p4
	vertices.emplace_back(bmax.x, bmax.y, bmax.z); //p5
	vertices.emplace_back(bmin.x, bmax.y, bmin.z); //p6
	vertices.emplace_back(bmax.x, bmax.y, bmin.z); //p7
	// initial mesh
	root[0] = std::shared_ptr<Tetrahedron>(new Tetrahedron(glm::ivec4(0,4,5,7)));
	root[1] = std::shared_ptr<Tetrahedron>(new Tetrahedron(glm::ivec4(0,4,6,7)));
	root[2] = std::shared_ptr<Tetrahedron>(new Tetrahedron(glm::ivec4(0,2,6,7)));
	root[3] = std::shared_ptr<Tetrahedron>(new Tetrahedron(glm::ivec4(0,1,5,7)));
	root[4] = std::shared_ptr<Tetrahedron>(new Tetrahedron(glm::ivec4(0,1,3,7)));
	root[5] = std::shared_ptr<Tetrahedron>(new Tetrahedron(glm::ivec4(0,2,3,7)));
	
	for(int i = 0; i < 6; i++)
		activeTetrahedra.emplace(root[i]);

	oracle = nullptr;
}

inline int AdaptiveTetTree::cutEdge(int a, int b){
	auto it = cutVertices.find(Edge(std::min(a,b),std::max(a,b)));
	if(it == cutVertices.end()){
		vertices.emplace_back((vertices[a] + vertices[b])/2.0f);
		return vertices.size()-1;
	}
	return it->second;
}

void AdaptiveTetTree::bisectTet(std::shared_ptr<Tetrahedron> tet){
	int v0 = tet->vertices[0];
	int v1 = tet->vertices[1];
	int v2 = tet->vertices[2];
	int v3 = tet->vertices[3];

	switch(tet->level % 3){
		case 0: {// Type A
			int q0 = cutEdge(v0,v3);
			tet->children[0] = std::shared_ptr<Tetrahedron>(new Tetrahedron(glm::ivec4(v0,v1,v2,q0)));
			tet->children[1] = std::shared_ptr<Tetrahedron>(new Tetrahedron(glm::ivec4(v3,v2,v1,q0)));
			}break;
		case 1: {// Type B
			int q1 = cutEdge(v0,v2);
			tet->children[0] = std::shared_ptr<Tetrahedron>(new Tetrahedron(glm::ivec4(v0,v1,q1,v3)));
			tet->children[1] = std::shared_ptr<Tetrahedron>(new Tetrahedron(glm::ivec4(v2,v1,q1,v3)));
			}break;
		case 2: {// Type C
			int q2 = cutEdge(v0,v1);
			tet->children[0] = std::shared_ptr<Tetrahedron>(new Tetrahedron(glm::ivec4(v0,q2,v2,v3)));
			tet->children[1] = std::shared_ptr<Tetrahedron>(new Tetrahedron(glm::ivec4(v1,q2,v2,v3)));
			}break;
		default: return;
	}
	tet->children[0]->level = tet->children[1]->level = tet->level + 1;
}

void AdaptiveTetTree::iterateLeaves(std::function<void(std::shared_ptr<Tetrahedron>)> f){
	for(auto t : leaves){
		f(t);
	}	
}

void AdaptiveTetTree::iterateTetrahedra(std::function<void(std::shared_ptr<Tetrahedron>)> f){
	for(int i = 0; i < 6; i++)
		dfsTetrahedra(root[i], f);
}

void AdaptiveTetTree::dfsTetrahedra(std::shared_ptr<Tetrahedron> tet, std::function<void(std::shared_ptr<Tetrahedron>)> f){
	if(tet == nullptr)
		return;
	f(tet);
	dfsTetrahedra(tet->children[0], f);
	dfsTetrahedra(tet->children[1], f);
}

void AdaptiveTetTree::step(){
	if(!oracle) return;
	std::queue<std::shared_ptr<Tetrahedron> > newActive;
	while(!activeTetrahedra.empty()){
		auto t = activeTetrahedra.front();
		activeTetrahedra.pop();
		if(!t->active)
			continue;
		t->active = false;
		if(oracle(t)){
			if(t->level > 5){
				leaves.emplace_back(t);
				continue;
			}
			bisectTet(t);
			newActive.emplace(t->children[0]);
			newActive.emplace(t->children[1]);
		}
	}
	activeTetrahedra = newActive;
}

void AdaptiveTetTree::explore(){
	
}
