#include <openvdb/openvdb.h>
#include <openvdb/Types.h>
#include <openvdb/tools/PointIndexGrid.h>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>

#pragma once

class Particle {
	public:
	glm::vec3 p, n;
	openvdb::Vec3R o_p, o_n;

	Particle(glm::vec3 position, glm::vec3 normal){
		p = position;
		n = normal;
		for(int i = 0; i < 3; i++){
			o_p[i] = p[i];
			o_n[i] = n[i];
		}
	}
};

class ParticleList {
	public:
		std::vector<Particle> particles;
		
		typedef openvdb::Vec3R value_type;

		size_t size() const {
			return particles.size();
		}

		void getPos(size_t n, openvdb::Vec3R& xyz) const {
			xyz = particles[n].o_p;
		}

};

class ParticleSet {
	protected:
		typedef openvdb::tools::PointIndexGrid PointIndexGrid;

		PointIndexGrid::Ptr pointGridPtr;
		openvdb::CoordBBox bbox;

		openvdb::math::Transform::Ptr transform;
	
		typedef PointIndexGrid::ConstAccessor ConstAccessor;
		typedef openvdb::tools::PointIndexIterator<> PointIndexIterator;
	public:
		ParticleList pl;
		ParticleSet(){}

		glm::vec3 bmin, bmax;
		void add(glm::vec3 pos, glm::vec3 normal);
		void init();
		int getClosestParticle(glm::vec3 point, double radius);
};

inline void ParticleSet::add(glm::vec3 pos, glm::vec3 normal){
	if(!pl.particles.size()){
		for(int i = 0; i < 3; i++)
			bmax[i] = bmin[i] = pos[i];
	}
	pl.particles.emplace_back(pos,normal);
	for(int i = 0; i < 3; i++){
		bmin[i] = std::min(bmin[i], pos[i]);
		bmax[i] = std::max(bmax[i], pos[i]);
	}
}

void ParticleSet::init() {
	openvdb::initialize();

	const float voxelSize = 1.0f;
	transform = openvdb::math::Transform::createLinearTransform(voxelSize);

	pointGridPtr = openvdb::tools::createPointIndexGrid<PointIndexGrid>(pl, *transform);

	pointGridPtr->tree().evalActiveVoxelBoundingBox(bbox);
}

int ParticleSet::getClosestParticle(glm::vec3 point, double radius){
	ConstAccessor acc = pointGridPtr->getConstAccessor();
	PointIndexIterator it(bbox, acc);

	// radial search
    	openvdb::BBoxd region(bbox.min().asVec3d(), bbox.max().asVec3d());
	openvdb::Vec3d center = ParticleList::value_type(point.x,point.y,point.z);
	//region.getCenter();
	
	it.searchAndUpdate(center, radius, acc, pl, *transform);

	int closest = -1;
	bool first = true;
	double minDist;
	while(it.test()){
		if(first){
			first = false;
			closest = *it;
			glm::vec3 r = point - pl.particles[*it].p;
			minDist = glm::dot(r, r);
		}
		glm::vec3 r = point - pl.particles[*it].p;
		double dist = glm::dot(r, r);
		if(dist < minDist){
			closest = *it;
			minDist = dist;
		}
		it.next();
	}
	return closest;
}

