#include <glm/glm.hpp>
#include <list>
#include <vector>
#include "MathUtils.h"
#include "ParticleSet.h"

#pragma once 

class Tetrahedron;

//typedef std::shared_ptr<Tetrahedron> TetPtr;
typedef int TetPtr;

class Tetrahedron {
	public:
		int id; 			// id
		int level; 			// depth in tree
		double curvature; 		// tet particles curvature
		bool active; 			// to be processed
		glm::vec4 planes[4];    	// face plane equations
		glm::ivec4 vertices;    	// vertices indices
		glm::ivec4 sortedVertices;    	// vertex order for faces
		std::list<size_t> particles; 	// particle indices
		TetPtr children[2]; 		// children
		
		Tetrahedron(glm::ivec4 v, glm::ivec4 sv){
			level = 0;
			active = true;
			vertices = v;
			sortedVertices = sv;
			for(int i = 0; i < 2; i++)
				children[i] = -1;
		}
		
		bool hasChildren();
		bool hasEdge(int e0, int e1);
		// Normal propagation
		void setNormals(const std::vector<glm::vec3>& vs);
		int findIndex(const glm::ivec4 vs, int v);
		glm::ivec4 sortedChild(int a, int b, int child, int q);
		void propagateNormals(std::vector<Tetrahedron>& tetrahedra, 
				const std::vector<glm::vec3>& vs, int child, int q, int o);
		bool highCurvatureTest(const std::vector<Particle>& particleData, double T);
};

bool Tetrahedron::hasChildren(){
	return children[0] >= 0 && children[1] >= 0;
}

bool Tetrahedron::hasEdge(int e0, int e1){
	return (e0 == vertices[0] || e0 == vertices[1] || e0 == vertices[2] || e0 == vertices[3]) && 
	       (e1 == vertices[0] || e1 == vertices[1] || e1 == vertices[2] || e1 == vertices[3]);
}

void Tetrahedron::setNormals(const std::vector<glm::vec3>& vs){
	int faces[] = { 0,1,2, 0,3,1, 0,2,3, 1,3,2 };

	for(int p = 0; p < 4; p++){
		planes[p] = computePlane(vs[sortedVertices[faces[p*3+0]]],
					 vs[sortedVertices[faces[p*3+1]]],
					 vs[sortedVertices[faces[p*3+2]]]);
	}
}

inline int Tetrahedron::findIndex(const glm::ivec4 vs, int v){
	for(int i = 0; i < 4; i++)
		if(v == vs[i])
			return i;
	std::cerr << "vertex index not found!\n";
	exit(1);
}

inline glm::ivec4 Tetrahedron::sortedChild(int a, int b, int child, int q){
	int dv = (child)? a : b;
	glm::ivec4 sv = sortedVertices;
	int ind = findIndex(sortedVertices, vertices[dv]); // vertex dv will give place to q
	sv[ind] = q;
	return sv;
}

inline void Tetrahedron::propagateNormals(std::vector<Tetrahedron>& tetrahedra, 
		const std::vector<glm::vec3>& vs, int child, int q, int d){
	if(children[child] < 0) return;
	int deletedVertex = findIndex(tetrahedra[children[child]].sortedVertices, q);
	int direction = findIndex(tetrahedra[children[child]].sortedVertices, vertices[d]);

	for(int i = 0; i < 4; i++)
		tetrahedra[children[child]].planes[i] = planes[i];
	
	int faces[] = { 0,1,2, 0,3,1, 0,2,3, 1,3,2 };
	// find the face that needs to be updated
	// is the one that has deletedVertex and doesn't has direction
	int badFace = -1;
	for(int i = 0; i < 4; i++)
		if((deletedVertex == faces[i*3 + 0] ||
		    deletedVertex == faces[i*3 + 1] ||
		    deletedVertex == faces[i*3 + 2])&&
		   (direction != faces[i*3 + 0] &&
		    direction != faces[i*3 + 1] &&
	            direction != faces[i*3 + 2])){
			if(badFace != -1){std::cerr << "multiple bad faces!\n"; exit(1);}
			badFace = i;
		}
	if(badFace == -1){std::cerr << "no bad face!\n"; exit(1);}
	tetrahedra[children[child]].planes[badFace] = 
		computePlane(vs[tetrahedra[children[child]].sortedVertices[faces[badFace*3+0]]],
			     vs[tetrahedra[children[child]].sortedVertices[faces[badFace*3+1]]],
			     vs[tetrahedra[children[child]].sortedVertices[faces[badFace*3+2]]]);
}

bool Tetrahedron::highCurvatureTest(const std::vector<Particle>& particleData, double T){
	curvature = 0.0;
	bool first = true;
	for(auto it : particles){
		for(auto it2 : particles){
			double c = glm::dot(particleData[it].n,particleData[it2].n);
			if(first){
				curvature = c;
				first = false;
			}
			curvature = std::min(curvature,c);
			if(curvature < T)
				return false;
		}
	}
	return true;
}
