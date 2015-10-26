#include <openvdb/openvdb.h>
#include <openvdb/Types.h>
#include <openvdb/tools/PointIndexGrid.h>
#include <vector>
#include <iostream>

class ParticleList {
	protected:
		struct PData{
			openvdb::Vec3R p;
			openvdb::Vec3R n;
		};
		
		std::vector<PData> pList;
	public:
		typedef openvdb::Vec3R value_type;

		void add(value_type pos, value_type normal){
			PData pd;
			pd.p = pos;
			pd.n = normal;
			pList.push_back(pd);
		}
		
		size_t size() const {
			return pList.size();
		}

		void getPos(size_t n, openvdb::Vec3R& xyz) const {
			xyz = pList[n].p;
		}
};

int main() {
	openvdb::initialize();

	ParticleList pl;

	int n;
	std::cin >> n;
	while(n--){
		ParticleList::value_type pos, normal;
		for(int i = 0; i < 3; i++) std::cin >> pos[i];
		for(int i = 0; i < 3; i++) std::cin >> normal[i];
		pl.add(pos,normal);
	}

	const float voxelSize = 0.01f;
	const openvdb::math::Transform::Ptr transform =
		openvdb::math::Transform::createLinearTransform(voxelSize);

	typedef openvdb::tools::PointIndexGrid PointIndexGrid;

	PointIndexGrid::Ptr pointGridPtr =
		openvdb::tools::createPointIndexGrid<PointIndexGrid>(pl, *transform);

	openvdb::CoordBBox bbox;
	pointGridPtr->tree().evalActiveVoxelBoundingBox(bbox);

	typedef PointIndexGrid::ConstAccessor ConstAccessor;
	typedef openvdb::tools::PointIndexIterator<> PointIndexIterator;

	ConstAccessor acc = pointGridPtr->getConstAccessor();
	PointIndexIterator it(bbox, acc);

	// radial search
    	openvdb::BBoxd region(bbox.min().asVec3d(), bbox.max().asVec3d());
	openvdb::Vec3d center = ParticleList::value_type(0,0,1.0);//region.getCenter();
	
	double radius = 0.2;
	it.searchAndUpdate(center, radius, acc, pl, *transform);

	std::cout << it.size() << std::endl;
	while(it.next()){
		std::cout << (*it) << std::endl;
	}
	return 0;
}
