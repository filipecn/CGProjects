#include <openvdb/openvdb.h>
#include <openvdb/Types.h>
#include <openvdb/tree/LeafNode.h>
#include <openvdb/tools/ParticlesToLevelSet.h>
#include <vector>

class ParticleList {
	protected:
		struct PData {
			openvdb::Vec3R p, v;
			openvdb::Real r;
		};
		openvdb::Real rScale;
		openvdb::Real vScale;

		std::vector<PData> pList;
	public:
		typedef openvdb::Vec3R value_type;

		ParticleList(openvdb::Real radScale=1, openvdb::Real velScale=1)
			: rScale(radScale), vScale(velScale) {}

		void add(const openvdb::Vec3R &p, const openvdb::Real &r, const openvdb::Vec3R &v=openvdb::Vec3R(0,0,0)) {
			PData pd;
			pd.p = p;
			pd.r = r;
			pd.v = v;
			pList.push_back(pd);
		}

		// Return the total number of particles in list.
		// Always required!
		size_t size() const { return pList.size(); }
		// Get the world space position of the nth particle.
		// Required by ParticledToLevelSet::rasterizeSphere(*this,radius).
		void getPos(size_t n, openvdb::Vec3R& xyz) const { xyz = pList[n].p; }
		// Get the world space position and radius of the nth particle.
		// Required by ParticledToLevelSet::rasterizeSphere(*this).
		void getPosRad(size_t n, openvdb::Vec3R& xyz, openvdb::Real& rad) const { 
			xyz = pList[n].p; 
			rad = rScale*pList[n].r;
		}
		// Get the world space position, radius and velocity of the nth particle.
		// Required by ParticledToLevelSet::rasterizeSphere(*this,radius).
		void getPosRadVel(size_t n, openvdb::Vec3R& xyz, openvdb::Real& rad, openvdb::Vec3R& vel) const {
			xyz = pList[n].p; 
			rad = rScale*pList[n].r;
			vel = vScale*pList[n].v;
		}
};

int main() {
	openvdb::initialize();

	ParticleList pl;
	for(int i = -50; i <= 50; i++)
		for(int j = -50; j <= 50; j++)
			pl.add(openvdb::Vec3R(double(i), double(j), 0.0), 2.0);

	const float voxelSize = 1.0f, halfWidth = 2.0f;
	openvdb::FloatGrid::Ptr ls = openvdb::createLevelSet<openvdb::FloatGrid>(voxelSize, halfWidth);
	openvdb::tools::ParticlesToLevelSet<openvdb::FloatGrid> raster(*ls);

	raster.setGrainSize(1);
	raster.rasterizeSpheres(pl);
	raster.finalize();

	// Create a VDB file object.
	openvdb::io::File file("mygrids.vdb");
	// Add the grid pointer to a container.
	openvdb::GridPtrVec grids;
	grids.push_back(ls);
	// Write out the contents of the container.
	file.write(grids);
	file.close();

	return 0;
}
