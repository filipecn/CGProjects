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
#include "ParticleSet.h"

#pragma once

class AdaptiveTetTree {
	public:
		AdaptiveTetTree();
		~AdaptiveTetTree(){};

		struct MeshVertex {
			double value;
			double dist;
			int closestParticle;
			MeshVertex(){closestParticle = -1;};
		};

		// Edges
		using Edge = std::pair<int,int>;
		Edge makeEdge(int a, int b);
		std::map<Edge, int> cutVertices;
		std::map<Edge, std::vector<TetPtr> > edge_to_tet;
		std::queue<Edge> badEdges;
		// Data
		std::vector<Tetrahedron> tetrahedra; 			// mesh tetrahedra
		std::vector<glm::vec3> vertices; 			// mesh vertices
		std::vector<MeshVertex> meshVertices; 			// mesh vertices data
		std::queue<TetPtr> activeTetrahedra; 			// next to be processed
		std::vector<TetPtr> leaves; 				// approximate surface
		// Aux Functions
		void iterateLeaves(std::function<void(Tetrahedron&)> f);
		void iterateTetrahedra(std::function<void(Tetrahedron&)> f);
		void dfsTetrahedra(TetPtr tet, std::function<void(Tetrahedron&)> f);
		// Statistics
		int maxLevel; 						// maxDepth Level
		// Call Functions
		void addTetrahedron(TetPtr tet);
		void addParticle(glm::vec3 p, glm::vec3 n);
		void init();
		void step();
		void explore();
		// Adaptivity
		int maxLevelLimit, minLevelLimit;
		glm::vec3 bmin, bmax;
		std::vector<Particle> particles;
		std::function<bool(Tetrahedron&)> oracle;
		std::function<bool(Tetrahedron&)> stopCondition;
		void updateMeshVertices();
	//protected:
		// Bisection
		inline int cutEdge(int a, int b);
		void bisectTet(TetPtr tet, Edge e);
		void splitBadEdge(Edge e);
};

AdaptiveTetTree::AdaptiveTetTree(){
	oracle = nullptr;
	stopCondition = nullptr;
	maxLevelLimit = 10;
	minLevelLimit = 3;
}

inline AdaptiveTetTree::Edge AdaptiveTetTree::makeEdge(int a, int b){
	return Edge(std::min(a,b),std::max(a,b));
}

inline void AdaptiveTetTree::addTetrahedron(TetPtr tet){
	int edges[] = { 0,1, 0,2, 0,3, 1,2, 1,3, 2,3 };
	for(int e = 0; e < 6; e++){
		Edge edge = makeEdge(tetrahedra[tet].vertices[edges[e*2+0]], 
				     tetrahedra[tet].vertices[edges[e*2+1]]);
		auto it = edge_to_tet.find(edge);
		if(it == edge_to_tet.end()){
			edge_to_tet.insert(std::pair<Edge, std::vector<TetPtr> >(edge, 
						std::vector<TetPtr>()));
			it = edge_to_tet.find(edge);
			if(it == edge_to_tet.end()){
				std::cerr << "addTet fail.\n";
				exit(1);
			}
		}
		it->second.emplace_back(tet);
	}
}

inline void AdaptiveTetTree::addParticle(glm::vec3 p, glm::vec3 n){
	Particle pa(p,n);
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
	float delta = 0.05;
	bmin.x -= (bmax.x - bmin.x)*delta;
	bmax.x += (bmax.x - bmin.x)*delta;
	bmin.y -= (bmax.y - bmin.y)*delta;
	bmax.y += (bmax.y - bmin.y)*delta;
	bmin.z -= (bmax.z - bmin.z)*delta;
	bmax.z += (bmax.z - bmin.z)*delta;
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
	tetrahedra.emplace_back(glm::ivec4(0,4,5,7), glm::ivec4(0,5,4,7));
	tetrahedra.emplace_back(glm::ivec4(0,4,6,7), glm::ivec4(0,7,4,6));
	tetrahedra.emplace_back(glm::ivec4(0,2,6,7), glm::ivec4(0,7,6,2));
	tetrahedra.emplace_back(glm::ivec4(0,1,5,7), glm::ivec4(0,1,5,7));
	tetrahedra.emplace_back(glm::ivec4(0,1,3,7), glm::ivec4(0,1,7,3));
	tetrahedra.emplace_back(glm::ivec4(0,2,3,7), glm::ivec4(0,3,7,2));
	
	for(int i = 0; i < 6; i++){
		tetrahedra[i].id = i;
		tetrahedra[i].setNormals(vertices);
		activeTetrahedra.emplace(i);
		addTetrahedron(i);
	}
	
	// distribute points
	for(size_t i = 0; i < particles.size(); i++){
		for(int t = 0; t < 6; t++){
			if(pointInsideTet(tetrahedra[t].planes, particles[i].p)){
				tetrahedra[t].particles.emplace_back(i);
				break;
			}	
		}
	}
}

inline int AdaptiveTetTree::cutEdge(int a, int b){
	Edge edge = makeEdge(a,b);
	auto it = cutVertices.find(edge);
	if(it == cutVertices.end()){
		cutVertices.insert(std::pair<Edge, int >(edge, vertices.size()));
		vertices.emplace_back((vertices[a] + vertices[b])/2.0f);
		return vertices.size()-1;
	}
	return it->second;
}

void AdaptiveTetTree::bisectTet(TetPtr tet, Edge e = Edge(-1,-1)){
	if(tetrahedra[tet].hasChildren()){std::cerr << "bisect parent!\n";exit(1);}
	int v0 = tetrahedra[tet].vertices[0];
	int v1 = tetrahedra[tet].vertices[1];
	int v2 = tetrahedra[tet].vertices[2];
	int v3 = tetrahedra[tet].vertices[3];
	int q;
	Edge badEdge; badEdge.first = v0;
	tetrahedra[tet].children[0] = tetrahedra.size();
	tetrahedra[tet].children[1] = tetrahedra.size() + 1;
	switch(tetrahedra[tet].level % 3){
		case 0: {// Type A
			q = cutEdge(v0,v3);
			tetrahedra.emplace_back(glm::ivec4(v0,v1,v2,q), 
					tetrahedra[tet].sortedChild(0,3,0,q));
			tetrahedra[tet].propagateNormals(tetrahedra,vertices,0,q,0);
			tetrahedra.emplace_back(glm::ivec4(v3,v2,v1,q),
					tetrahedra[tet].sortedChild(0,3,1,q));
			tetrahedra[tet].propagateNormals(tetrahedra,vertices,1,q,3);
			badEdge.second = v3;
			}break;
		case 1: {// Type B
			q = cutEdge(v0,v2);
			tetrahedra.emplace_back(glm::ivec4(v0,v1,q,v3),
					tetrahedra[tet].sortedChild(0,2,0,q));
			tetrahedra[tet].propagateNormals(tetrahedra,vertices,0,q,0);
			tetrahedra.emplace_back(glm::ivec4(v2,v1,q,v3),
					tetrahedra[tet].sortedChild(0,2,1,q));
			tetrahedra[tet].propagateNormals(tetrahedra,vertices,1,q,2);
			badEdge.second = v2;
			}break;
		case 2: {// Type C
			q = cutEdge(v0,v1);
			tetrahedra.emplace_back(glm::ivec4(v0,q,v2,v3),
					tetrahedra[tet].sortedChild(0,1,0,q));
			tetrahedra[tet].propagateNormals(tetrahedra,vertices,0,q,0);
			tetrahedra.emplace_back(glm::ivec4(v1,q,v2,v3),
					tetrahedra[tet].sortedChild(0,1,1,q));
			tetrahedra[tet].propagateNormals(tetrahedra,vertices,1,q,1);
			badEdge.second = v1;
			}break;
		default: return;
	}
	for(int i = 0; i < 2; i++) {
		tetrahedra[tetrahedra[tet].children[i]].id = tetrahedra.size() - 2 + i;
		tetrahedra[tetrahedra[tet].children[i]].level = tetrahedra[tet].level + 1;
		addTetrahedron(tetrahedra[tet].children[i]);
	}

	maxLevel = std::max(tetrahedra[tet].level + 1, maxLevel);
	// move particles
	auto it = tetrahedra[tet].particles.begin();
	while(it != tetrahedra[tet].particles.end()){
		bool found = false;
		for(int c = 0; c < 2 && !found; c++)
			if(pointInsideTet(tetrahedra[tetrahedra[tet].children[c]].planes, particles[*it].p)){
				auto _it = it; _it++;
				tetrahedra[tetrahedra[tet].children[c]].particles.emplace_back(*it);
				tetrahedra[tet].particles.erase(it);
				it = _it;
				found = true;
				break;
			}
		if(!found){
			std::cerr << "bad distribution! tet type " << (tetrahedra[tet].level % 3) << std::endl;
			exit(1);
		}
	}

	activeTetrahedra.emplace(tetrahedra[tet].children[0]);
	activeTetrahedra.emplace(tetrahedra[tet].children[1]);
	splitBadEdge(makeEdge(badEdge.first,badEdge.second));
}

void AdaptiveTetTree::splitBadEdge(Edge e){
	auto it = edge_to_tet.find(e);
	if(it == edge_to_tet.end()){std::cerr << "no bad edge\n"; exit(1);}
	std::queue<TetPtr> q;
	for(int T = 0; T < it->second.size(); T++){
		TetPtr t = it->second[T];
		if(!tetrahedra[t].hasEdge(e.first, e.second)){
			std::cerr << t << std::endl;
			print(tetrahedra[t].vertices);
			std::cerr << e.first << " " << e.second << std::endl;
			std::cerr << "no edge on tet\n";
			exit(1);
		}
		q.emplace(t);
	}
	while(!q.empty()){
		TetPtr t = q.front();
		q.pop();
		if(!tetrahedra[t].hasEdge(e.first, e.second))
			continue;
		if(tetrahedra[t].hasChildren()){
			q.emplace(tetrahedra[t].children[0]);
			q.emplace(tetrahedra[t].children[1]);
			continue;
		}
		bisectTet(t);
		q.emplace(tetrahedra[t].children[0]);
		q.emplace(tetrahedra[t].children[1]);
	}
}

void AdaptiveTetTree::iterateLeaves(std::function<void(Tetrahedron&)> f){
	for(auto t : leaves){
		f(tetrahedra[t]);
	}	
//	for(auto t : tetrahedra){
//		if(!t.hasChildren() && t.particles.size())
//			f(t);
//	}
}

void AdaptiveTetTree::iterateTetrahedra(std::function<void(Tetrahedron&)> f){
	for(int i = 0; i < 6; i++)
		dfsTetrahedra(i, f);
}

void AdaptiveTetTree::dfsTetrahedra(TetPtr tet, std::function<void(Tetrahedron&)> f){
	if(tet < 0)
		return;
	f(tetrahedra[tet]);
	dfsTetrahedra(tetrahedra[tet].children[0], f);
	dfsTetrahedra(tetrahedra[tet].children[1], f);
}

void AdaptiveTetTree::step(){
	if(!oracle || !stopCondition) return;
	std::queue<TetPtr > newActive;
	while(!activeTetrahedra.empty()){
		auto t = activeTetrahedra.front();
		activeTetrahedra.pop();
		if(!tetrahedra[t].active)
			continue;
		tetrahedra[t].active = false;
		if(tetrahedra[t].hasChildren()){
			continue;
			std::cerr << "has children " << t << std::endl;
			activeTetrahedra.emplace(tetrahedra[t].children[0]);
			activeTetrahedra.emplace(tetrahedra[t].children[1]);
			continue;
		}
		
		if(oracle(tetrahedra[t])){
			//std::cerr << "refine " << t << std::endl;
		//	if(stopCondition(tetrahedra[t])){
		//		leaves.emplace_back(t);
		//		continue;
		//	}
			if(!tetrahedra[t].hasChildren())
				bisectTet(t);
			//std::cerr << "new active!\n";
			newActive.emplace(tetrahedra[t].children[0]);
			newActive.emplace(tetrahedra[t].children[1]);
		}
	}
	activeTetrahedra = newActive;
	std::cerr << activeTetrahedra.size() << std::endl;
	meshVertices.resize(vertices.size(), MeshVertex());
}

void AdaptiveTetTree::explore(){
	if(!oracle || !stopCondition)
		return;
	leaves.clear();
	while(!activeTetrahedra.empty()){
		auto t = activeTetrahedra.front();
		activeTetrahedra.pop();
		if(!tetrahedra[t].active)
			continue;
		tetrahedra[t].active = false;
		if(oracle(tetrahedra[t])){
			if(stopCondition(tetrahedra[t])){
				leaves.emplace_back(t);
				continue;
			}
			if(!tetrahedra[t].hasChildren())
				bisectTet(t);
			//activeTetrahedra.emplace(tetrahedra[t].children[0]);
			//activeTetrahedra.emplace(tetrahedra[t].children[1]);
		}
	}
	meshVertices.resize(vertices.size(), MeshVertex());
}

void AdaptiveTetTree::updateMeshVertices(){
	leaves.clear();
	std::queue<TetPtr> emptyLeaves;
	for(int t = 0; t < tetrahedra.size(); t++){
		Tetrahedron& tet = tetrahedra[t];
		if(tet.hasChildren()) continue;
		if(tet.particles.size()){
			leaves.emplace_back(t);
			for(int i = 0; i < 4; i++){
				int v = tet.vertices[i];
				// update vertex dist
				bool first = meshVertices[v].closestParticle < 0;
				double minDist = meshVertices[v].dist;
				int closest = meshVertices[v].closestParticle;;
				for(auto p : tet.particles){
					double dist = glm::dot(particles[p].p - vertices[v], 
							particles[p].p - vertices[v]);
					if(first){
						minDist = dist;
						first = false;
						closest = p;
					}
					if(dist < minDist){
						minDist = dist;
						closest = p;
					}

				}
				meshVertices[v].dist = minDist;
				meshVertices[v].closestParticle = closest;
				meshVertices[v].value = glm::dot(particles[closest].p - vertices[v], 
						particles[closest].n);
			}
		}
		else emptyLeaves.emplace(t);
	}
	while(!emptyLeaves.empty()){
		int tetIndex = emptyLeaves.front();
		Tetrahedron& tet = tetrahedra[emptyLeaves.front()];
		emptyLeaves.pop();
		bool intercept = false;
		for(int i = 0; i < 4; i++)
			if(meshVertices[tet.vertices[i]].closestParticle >= 0)
				intercept = true;
		if(!intercept)
			continue;
		leaves.emplace_back(tetIndex);
		for(int i = 0; i < 4; i++){
			if(meshVertices[tet.vertices[i]].closestParticle >= 0)
				continue;
			int v = tet.vertices[i];
			bool first = true;
			for(int p = 0; p < particles.size(); p++){
				double dist = glm::dot(particles[p].p - vertices[v], 
						particles[p].p - vertices[v]);
				if(first){
					meshVertices[v].dist = dist;
					first = false;
					meshVertices[v].closestParticle = p;
				}
				if(dist < meshVertices[v].dist){
					meshVertices[v].dist = dist;
					meshVertices[v].closestParticle = p;
				}

			}
			meshVertices[v].value = glm::dot(
					particles[meshVertices[v].closestParticle].p - vertices[v], 
					particles[meshVertices[v].closestParticle].n);

		}
		for(int i = 0; i < 4; i++)
			if(meshVertices[tet.vertices[i]].closestParticle < 0){
				std::cerr << "BAD!\n"; exit(1);
			}

	}
}
