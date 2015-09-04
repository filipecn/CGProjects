#include <vector>
#include <map>
#include <queue>
#include <memory>
#include <list>
#include <functional>
#include <iostream>
#include <glm/glm.hpp>
#include "MathUtils.h"
#include "Tetrahedron.h"

#pragma once

class AdaptiveTetTree {
	public:
		AdaptiveTetTree();
		~AdaptiveTetTree(){};

		struct Particle {
			glm::vec3 p, n;
		};

		struct MeshVertex {
			bool processed;
			double value;
			MeshVertex(){processed = false;};
		};

		// Edges
		using Edge = std::pair<int,int>;
		Edge makeEdge(int a, int b);
		std::map<Edge, int> cutVertices;
		std::map<Edge, std::vector<TetPtr> > edge_to_tet;
		// Data
		std::vector<glm::vec3> vertices; 			// mesh vertices
		std::vector<MeshVertex> meshVertices; 			// mesh vertices data
		TetPtr root[6]; 					// initial tetrahedra
		std::queue<TetPtr > activeTetrahedra; 			// next to be processed
		std::vector<TetPtr > leaves; 				// approximate surface
		// Topology
		// Given a face with sorted vertices, here is its neighbours
		//std::map<int, std::map<int, std::map<int, std::vector<TetPtr> faceToTetMap;
		// Aux Functions
		void iterateLeaves(std::function<void(TetPtr)> f);
		void iterateTetrahedra(std::function<void(TetPtr)> f);
		void dfsTetrahedra(TetPtr tet, std::function<void(TetPtr)> f);
		// Statistics
		int maxLevel; 						// maxDepth Level
		int numberOfTetrahedra;
		// Call Functions
		void addTetrahedron(TetPtr tet);
		void addParticle(glm::vec3 p, glm::vec3 n);
		void init();
		void step();
		void explore();
		// Adaptivity
		glm::vec3 bmin, bmax;
		std::vector<Particle> particles;
		std::function<bool(TetPtr)> oracle;
		std::function<bool(TetPtr)> stopCondition;
		void updateMeshVertex(int v);
	//protected:
		// Bisection
		inline int cutEdge(int a, int b);
		void bisectTet(TetPtr tet);
		void splitEdge(int a, int b);
};

AdaptiveTetTree::AdaptiveTetTree(){
	oracle = nullptr;
	numberOfTetrahedra = 0;
}

inline AdaptiveTetTree::Edge AdaptiveTetTree::makeEdge(int a, int b){
	return Edge(std::min(a,b),std::max(a,b));
}

inline void AdaptiveTetTree::addTetrahedron(TetPtr tet){
	int edges[] = { 0,1, 0,2, 0,3, 1,2, 1,3, 2,3 };
	for(int e = 0; e < 6; e++){
		Edge edge = makeEdge(tet->vertices[edges[e*2+0]], tet->vertices[edges[e*2+1]]);
		auto it = edge_to_tet.find(edge);
		if(it == edge_to_tet.end()){
			edge_to_tet.insert(std::pair<Edge, std::vector<TetPtr> >(edge, 
						std::vector<TetPtr>()));
			it = edge_to_tet.find(edge);
			if(it == edge_to_tet.end()) exit(1);
		}
		it->second.emplace_back(tet);
	}
}

inline void AdaptiveTetTree::addParticle(glm::vec3 p, glm::vec3 n){
	Particle pa;
	pa.p = p; pa.n = n;
	if(!particles.size()){
		bmin = pa.p; bmax = pa.p;
	}
	particles.emplace_back(pa);
	for(int i = 0; i < 3; i++){
		bmin[i] = std::min(bmin[i], p[i]);
		bmax[i] = std::max(bmax[i], p[i]);
	}
}

void AdaptiveTetTree::init(){
	maxLevel = 0;
	numberOfTetrahedra = 0;
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
	root[0] = TetPtr(new Tetrahedron(glm::ivec4(0,4,5,7), glm::ivec4(0,5,4,7)));
	root[1] = TetPtr(new Tetrahedron(glm::ivec4(0,4,6,7), glm::ivec4(0,7,4,6)));
	root[2] = TetPtr(new Tetrahedron(glm::ivec4(0,2,6,7), glm::ivec4(0,7,6,2)));
	root[3] = TetPtr(new Tetrahedron(glm::ivec4(0,1,5,7), glm::ivec4(0,1,5,7)));
	root[4] = TetPtr(new Tetrahedron(glm::ivec4(0,1,3,7), glm::ivec4(0,1,7,3)));
	root[5] = TetPtr(new Tetrahedron(glm::ivec4(0,2,3,7), glm::ivec4(0,3,7,2)));
	
	for(int i = 0; i < 6; i++){
		root[i]->id = numberOfTetrahedra++;
		root[i]->setNormals(vertices);
		activeTetrahedra.emplace(root[i]);
		addTetrahedron(root[i]);
	}
	
	// distribute points
	for(size_t i = 0; i < particles.size(); i++){
		for(int t = 0; t < 6; t++){
			if(pointInsideTet(root[t]->planes, particles[i].p)){
				root[t]->particles.emplace_back(i);
				break;
			}	
		}
	}
}

inline int AdaptiveTetTree::cutEdge(int a, int b){
	auto it = cutVertices.find(makeEdge(a,b));
	if(it == cutVertices.end()){
		vertices.emplace_back((vertices[a] + vertices[b])/2.0f);
		return vertices.size()-1;
	}
	return it->second;
}

void AdaptiveTetTree::bisectTet(TetPtr tet){
	if(tet->children[0] || tet->children[1])
		exit(1);
	int v0 = tet->vertices[0];
	int v1 = tet->vertices[1];
	int v2 = tet->vertices[2];
	int v3 = tet->vertices[3];
	int q;
	Edge badEdge(v0,v0);
	switch(tet->level % 3){
		case 0: {// Type A
			q = cutEdge(v0,v3);
			tet->children[0] = TetPtr(new Tetrahedron(glm::ivec4(v0,v1,v2,q),
						                  tet->sortedChild(0,3,0,q)));
			tet->propagateNormals(vertices,0,q,0);
			tet->children[1] = TetPtr(new Tetrahedron(glm::ivec4(v3,v2,v1,q),
						                  tet->sortedChild(0,3,1,q)));
			tet->propagateNormals(vertices,1,q,3);
			
			badEdge.second = v3;
			}break;
		case 1: {// Type B
			q = cutEdge(v0,v2);
			tet->children[0] = TetPtr(new Tetrahedron(glm::ivec4(v0,v1,q,v3),
						                  tet->sortedChild(0,2,0,q)));
			tet->propagateNormals(vertices,0,q,0);
			tet->children[1] = TetPtr(new Tetrahedron(glm::ivec4(v2,v1,q,v3),
						                  tet->sortedChild(0,2,1,q)));
			tet->propagateNormals(vertices,1,q,2);

			badEdge.second = v2;
			}break;
		case 2: {// Type C
			q = cutEdge(v0,v1);
			tet->children[0] = TetPtr(new Tetrahedron(glm::ivec4(v0,q,v2,v3),
						                  tet->sortedChild(0,1,0,q)));
			tet->propagateNormals(vertices,0,q,0);
			tet->children[1] = TetPtr(new Tetrahedron(glm::ivec4(v1,q,v2,v3),
						                  tet->sortedChild(0,1,1,q)));
			tet->propagateNormals(vertices,1,q,1);

			badEdge.second = v1;
			}break;
		default: return;
	}
	for(int i = 0; i < 2; i++) {
		tet->children[i]->id = numberOfTetrahedra++;
		tet->children[i]->root = tet->root; 
		tet->children[i]->level = tet->level + 1;
		addTetrahedron(tet->children[i]);
	}

	maxLevel = std::max(tet->level + 1, maxLevel);
	// move particles
	auto it = tet->particles.begin();
	while(it != tet->particles.end()){
		bool found = false;
		for(int c = 0; c < 2 && !found; c++)
			if(pointInsideTet(tet->children[c]->planes, particles[*it].p)){
				auto _it = it; _it++;
				tet->children[c]->particles.emplace_back(*it);
				tet->particles.erase(it);
				it = _it;
				found = true;
				break;
			}
		if(!found){
			std::cerr << "bad distribution! tet type " << (tet->level % 3) << std::endl;
			exit(1);
		}
	}

	splitEdge(badEdge.first, badEdge.second);
}

void AdaptiveTetTree::splitEdge(int a, int b){
	auto it = edge_to_tet.find(makeEdge(a,b));
	if(it == edge_to_tet.end())
		return;
	for(auto t : it->second){
		if(!t)
			continue;
		if(t->children[0] || t->children[1])
			continue;
		if(!t->hasEdge(a, b))
			continue;
		bisectTet(t);
	}
}

void AdaptiveTetTree::iterateLeaves(std::function<void(TetPtr)> f){
	for(auto t : leaves){
		f(t);
	}	
}

void AdaptiveTetTree::iterateTetrahedra(std::function<void(TetPtr)> f){
	for(int i = 0; i < 6; i++)
		dfsTetrahedra(root[i], f);
}

void AdaptiveTetTree::dfsTetrahedra(TetPtr tet, std::function<void(TetPtr)> f){
	if(tet == nullptr)
		return;
	f(tet);
	dfsTetrahedra(tet->children[0], f);
	dfsTetrahedra(tet->children[1], f);
}

void AdaptiveTetTree::step(){
	if(!oracle) return;
	std::queue<TetPtr > newActive;
	while(!activeTetrahedra.empty()){
		auto t = activeTetrahedra.front();
		activeTetrahedra.pop();
		if(!t->active)
			continue;
		t->active = false;
		if(oracle(t)){
			if(stopCondition(t)){
				leaves.emplace_back(t);
				continue;
			}
			bisectTet(t);
			newActive.emplace(t->children[0]);
			//newActive.emplace(t->children[1]);
		}
	}
	activeTetrahedra = newActive;
}

void AdaptiveTetTree::explore(){
	if(!oracle || !stopCondition)
		return;
	leaves.clear();
	while(!activeTetrahedra.empty()){
		auto t = activeTetrahedra.front();
		activeTetrahedra.pop();
		if(!t->active)
			continue;
		t->active = false;
		if(oracle(t)){
			if(stopCondition(t)){
				leaves.emplace_back(t);
				continue;
			}
			bisectTet(t);
			activeTetrahedra.emplace(t->children[0]);
			activeTetrahedra.emplace(t->children[1]);
		}
	}
	meshVertices.resize(vertices.size(), MeshVertex());
}

void AdaptiveTetTree::updateMeshVertex(int v){
	if(meshVertices[v].processed)
		return;
	bool first = true;
	double minDist = 0.0, absMin = 0.0;
	for(auto p : particles){
		double dist = glm::dot(vertices[v] - p.p, p.n);
		double absDist = fabs(dist);
		if(first){
			minDist = dist;
			absMin = absDist;
			first = false;
		}
		if(absDist < absMin){
			minDist = dist;
			absMin = absDist;
		}
	}
	meshVertices[v].value = minDist;
	meshVertices[v].processed = true;
}
