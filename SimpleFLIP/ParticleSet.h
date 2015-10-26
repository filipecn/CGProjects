#include <openvdb/openvdb.h>
#include <openvdb/Types.h>
#include <openvdb/tools/PointIndexGrid.h>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <functional>

#pragma once

class Particle {
	public:
	glm::vec3 p, n, v;
	openvdb::Vec3R o_p, o_n;

	Particle(glm::vec3 position, glm::vec3 normal, glm::vec3 velocity){
		p = position;
		n = normal;
		v = velocity;
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
		void add(glm::vec3 pos, glm::vec3 normal, glm::vec3 velocity);
		void init();
		int getClosestParticle(glm::vec3 point, double radius);
		void iterateNeighbours(glm::vec3 boxMin, glm::vec3 boxMax, std::function<void(const Particle&)> f);
		void iterateNeighbours(glm::vec3 point, double radius, std::function<void(const Particle&)> f);
};

inline void ParticleSet::add(glm::vec3 pos, glm::vec3 normal, glm::vec3 velocity){
	if(!pl.particles.size()){
		for(int i = 0; i < 3; i++)
			bmax[i] = bmin[i] = pos[i];
	}
	pl.particles.emplace_back(pos,normal,velocity);
	for(int i = 0; i < 3; i++){
		bmin[i] = std::min(bmin[i], pos[i]);
		bmax[i] = std::max(bmax[i], pos[i]);
	}
}

void ParticleSet::init() {
	openvdb::initialize();


// The grid spacing
const double delta = 1.0;
// The offset to cell-center points
const openvdb::math::Vec3d offset(delta/2., 0,0);


	const float voxelSize = 0.5f;
	transform = openvdb::math::Transform::createLinearTransform(delta);
	transform->postTranslate(offset);

	pointGridPtr = openvdb::tools::createPointIndexGrid<PointIndexGrid>(pl, *transform);

	std::cout << bbox << std::endl;
	pointGridPtr->tree().evalActiveVoxelBoundingBox(bbox);
	std::cout << bbox << std::endl;
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

void ParticleSet::iterateNeighbours(glm::vec3 boxMin, glm::vec3 boxMax, std::function<void(const Particle&)> f){
	ConstAccessor acc = pointGridPtr->getConstAccessor();
	
	std::cout << boxMin.x << " , " << boxMin.y << " -> " <<
	boxMax.x << " , " << boxMax.y << std::endl;
	openvdb::CoordBBox cbbox = openvdb::CoordBBox(
	openvdb::Coord(boxMin.x,boxMin.y,boxMin.z),
	openvdb::Coord(boxMax.x,boxMax.y,boxMax.z));
	
	openvdb::Vec3d worldSpacePoint(boxMin.x,boxMin.y,boxMin.z);
	std::cout << "bmin -> " <<  worldSpacePoint << std::endl;
	openvdb::Vec3d indexSpacePoint = transform->worldToIndex(worldSpacePoint);
	std::cout << indexSpacePoint << std::endl;
	
	std::cout << bbox << std::endl;
	std::cout << cbbox << std::endl;
	
	
	PointIndexIterator it(bbox, acc);

	glm::vec3 center = boxMin + boxMax;
	center /= 2.0;
	glm::vec3 size = boxMax - center;
    
	openvdb::BBoxd region(
	transform->worldToIndex(openvdb::Vec3d(boxMin.x,boxMin.y,boxMin.z)),
	transform->worldToIndex(openvdb::Vec3d(boxMax.x,boxMax.y,boxMax.z)));
	
	//openvdb::BBoxd region(bbox.min().asVec3d(), bbox.max().asVec3d());
	
	//region.expand(1.0*0.5);
	
	//std::cout << "eh noise\n";
	//std::cout << region << std::endl;
	//std::cout << bbox << std::endl;
	it.searchAndUpdate(region, acc, pl, *transform);

	while(it.test()){
		f(pl.particles[*it]);
		it.next();
	}
}

void ParticleSet::iterateNeighbours(glm::vec3 point, double radius, std::function<void(const Particle&)> f){
	ConstAccessor acc = pointGridPtr->getConstAccessor();
	PointIndexIterator it(bbox, acc);
	
	// radial search
	//openvdb::Vec3d center = ParticleList::value_type(point.x,point.y,point.z);
	openvdb::Vec3d center = ParticleList::value_type(
		transform->worldToIndex(openvdb::Vec3d(point.x,point.y,point.z)));
	it.searchAndUpdate(center, radius, acc, pl, *transform);

	while(it.test()){
		f(pl.particles[*it]);
		it.next();
	}
}