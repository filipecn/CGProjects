#include <openvdb/openvdb.h>
#include <openvdb/Types.h>
#include <openvdb/tools/PointIndexGrid.h>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <functional>

#pragma once

class ParticleSet;

class Particle {
	friend class ParticleSet;
	private:
	glm::vec3 p, n, v;
	openvdb::Vec3R o_p, o_n;
	
	public:
	Particle(glm::vec3 position, glm::vec3 normal, glm::vec3 velocity){
		p = position;
		n = normal;
		v = velocity;
		for(int i = 0; i < 3; i++){
			o_p[i] = p[i];
			o_n[i] = n[i];
		}
	}
	
	void setPos(glm::vec3 position){
		p = position;
		for(int i = 0; i < 3; i++){
			o_p[i] = p[i];
		}
	}
	
	void setVelocity(glm::vec3 velocity){
		v = velocity;
	}
	
	glm::vec3 getPos() const{
		return p;
	}
	
	glm::vec3 getVelocity() const{
		return v;
	}
};

class ParticleSet {
	public:
		std::vector<Particle> particles;
		glm::vec3 bmin, bmax;
		
		typedef openvdb::Vec3R value_type;

		size_t size() const {
			return particles.size();
		}

		void getPos(size_t n, openvdb::Vec3R& xyz) const {
			xyz = particles[n].o_p;
		}
		
		void add(glm::vec3 pos, glm::vec3 normal, glm::vec3 velocity){
			if(!particles.size()){
				for(int i = 0; i < 3; i++)
					bmax[i] = bmin[i] = pos[i];
			}
			particles.emplace_back(pos,normal,velocity);
			for(int i = 0; i < 3; i++){
				bmin[i] = std::min(bmin[i], pos[i]);
				bmax[i] = std::max(bmax[i], pos[i]);
			}
		}
};

class ParticleSetAccessor {
	protected:
		typedef openvdb::tools::PointIndexGrid PointIndexGrid;
		typedef PointIndexGrid::ConstAccessor ConstAccessor;
		typedef openvdb::tools::PointIndexIterator<> PointIndexIterator;
		
		PointIndexGrid::Ptr pointGridPtr;
		openvdb::CoordBBox bbox;
		openvdb::math::Transform::Ptr transform;
		
		double scale;
		glm::vec3 offset;

	public:
		ParticleSetAccessor();
		ParticleSetAccessor(ParticleSet& ps, glm::vec3 _offset, double _scale);
		void set(ParticleSet& ps, glm::vec3 offset, double scale = 1.0);
		void update(ParticleSet& ps);
		
		int getClosestParticle(ParticleSet& ps, glm::vec3 point, double radius);
		void iterateNeighbours(ParticleSet& ps, glm::vec3 boxMin, glm::vec3 boxMax, std::function<void(const Particle&)> f);
		void iterateNeighbours(ParticleSet& ps, glm::vec3 point, double radius, std::function<void(const Particle&)> f);
};

ParticleSetAccessor::ParticleSetAccessor(){
	offset = glm::vec3(0,0,0);
	scale = 1.0;
	pointGridPtr = nullptr;
}

ParticleSetAccessor::ParticleSetAccessor(ParticleSet& ps, glm::vec3 _offset, double _scale) {
	set(ps, _offset, _scale);
}

void ParticleSetAccessor::set(ParticleSet& ps, glm::vec3 offset, double scale){
	openvdb::initialize();

	this->scale = scale;
	this->offset = offset;

	transform = openvdb::math::Transform::createLinearTransform(scale);
	transform->postTranslate(openvdb::math::Vec3d(offset.x,offset.y,offset.z));

	pointGridPtr = openvdb::tools::createPointIndexGrid<PointIndexGrid>(ps, *transform);
	pointGridPtr->tree().evalActiveVoxelBoundingBox(bbox);	
}

void ParticleSetAccessor::update(ParticleSet& ps){
	if(!pointGridPtr)
		return;
	pointGridPtr = openvdb::tools::getValidPointIndexGrid<PointIndexGrid>(ps, pointGridPtr);
}

int ParticleSetAccessor::getClosestParticle(ParticleSet& ps, glm::vec3 point, double radius){
	ConstAccessor acc = pointGridPtr->getConstAccessor();
	PointIndexIterator it(bbox, acc);

	// radial search
	openvdb::Vec3d center = ParticleSet::value_type(point.x,point.y,point.z);
	
	it.searchAndUpdate(center, radius, acc, ps, *transform);

	int closest = -1;
	bool first = true;
	double minDist;
	while(it.test()){
		if(first){
			first = false;
			closest = *it;
			glm::vec3 r = point - ps.particles[*it].getPos();
			minDist = glm::dot(r, r);
		}
		glm::vec3 r = point - ps.particles[*it].getPos();
		double dist = glm::dot(r, r);
		if(dist < minDist){
			closest = *it;
			minDist = dist;
		}
		it.next();
	}
	return closest;
}

void ParticleSetAccessor::iterateNeighbours(ParticleSet& ps, glm::vec3 boxMin, glm::vec3 boxMax, std::function<void(const Particle&)> f){
	ConstAccessor acc = pointGridPtr->getConstAccessor();
	
	//std::cout << boxMin.x << " , " << boxMin.y << " -> " <<
	//boxMax.x << " , " << boxMax.y << std::endl;
	//openvdb::CoordBBox cbbox = openvdb::CoordBBox(
	//openvdb::Coord(boxMin.x,boxMin.y,boxMin.z),
	//openvdb::Coord(boxMax.x,boxMax.y,boxMax.z));
	
	//openvdb::Vec3d worldSpacePoint(boxMin.x,boxMin.y,boxMin.z);
	//std::cout << "bmin -> " <<  worldSpacePoint << std::endl;
	//openvdb::Vec3d indexSpacePoint = transform->worldToIndex(worldSpacePoint);
	//std::cout << indexSpacePoint << std::endl;
	
	//std::cout << bbox << std::endl;
	//std::cout << cbbox << std::endl;
	
	
	PointIndexIterator it(bbox, acc);

	//glm::vec3 center = boxMin + boxMax;
	//center /= 2.0;
	//glm::vec3 size = boxMax - center;
    
	openvdb::BBoxd region(
	transform->worldToIndex(openvdb::Vec3d(boxMin.x,boxMin.y,boxMin.z)),
	transform->worldToIndex(openvdb::Vec3d(boxMax.x,boxMax.y,boxMax.z)));
	
	//openvdb::BBoxd region(openvdb::Vec3d(boxMin.x,boxMin.y,boxMin.z), 
	//					  openvdb::Vec3d(boxMax.x,boxMax.y,boxMax.z));
	
	region.expand(scale*1.0);
	
	//std::cout << "eh noise\n";
	//std::cout << region << std::endl;
	//std::cout << bbox << std::endl;
	it.searchAndUpdate(region, acc, ps, *transform);

	while(it.test()){
		f(ps.particles[*it]);
		it.next();
	}
}

void ParticleSetAccessor::iterateNeighbours(ParticleSet& ps, glm::vec3 point, double radius, std::function<void(const Particle&)> f){
	ConstAccessor acc = pointGridPtr->getConstAccessor();
	PointIndexIterator it(bbox, acc);
	
	// radial search
	openvdb::Vec3d center = ParticleSet::value_type(
		transform->worldToIndex(openvdb::Vec3d(point.x,point.y,point.z)));
		
	it.searchAndUpdate(center, radius/scale, acc, ps, *transform);
	

	while(it.test()){
		f(ps.particles[*it]);
		it.next();
	}
}