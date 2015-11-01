#pragma once

#include "MathUtils.hpp"

void glVertex(glm::vec2 p){
	glVertex2fv(&p[0]);
}

void glVertex(glm::vec3 p){
	glVertex3fv(&p[0]);
}
