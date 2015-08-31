#include <glm/glm.hpp>
#include <iostream>

#pragma once

void print(const glm::vec3& v){
	for(int i = 0; i < 3; i++)
		std::cerr << v[i] << " ";
	std::cerr << std::endl;
}

void print(const glm::vec4& plane){
	for(int i = 0; i < 4; i++)
		std::cerr << plane[i] << " ";
	std::cerr << std::endl;
}

void print(const glm::ivec4& tetIndices){
	for(int i = 0; i < 4; i++)
		std::cerr << tetIndices[i] << " ";
	std::cerr << std::endl;
}

void print(const std::vector<glm::vec3>& tetVertices, const glm::ivec4& tetIndices){
	for(int v = 0; v < 4; v++)
		print(tetVertices[tetIndices[v]]);
}

glm::vec4 computePlane(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2){
	glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v1));
	return glm::vec4(normal.x, normal.y, normal.z, -glm::dot(normal,v0));
}

bool planeSide(const glm::vec3& v, const glm::vec4& p){
	return (p.x*v.x + p.y*v.y + p.z*v.z + p.w) > 0.0f;
}

bool pointInsideTet(const glm::vec4 faces[], const glm::vec3& p){
	bool sign = planeSide(p, faces[0]);
	for(int i = 1; i < 4; i++)
		if(sign != planeSide(p, faces[i]))
			return false;
	return true;
}
