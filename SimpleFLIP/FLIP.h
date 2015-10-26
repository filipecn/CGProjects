#pragma once

#include "MACGrid.h"
#include "ParticleSet.h"
#include "Solver.h"

#include <ctime>
#include <cstdlib>
#include <functional>


enum CellType {FLUID = 1, AIR, SOLID, CELLTYPES};

class FLIP {
public:
	FLIP(){}
	~FLIP(){}
	
	glm::ivec2 size;
	float dx;
	float dt;
	float gravity;
	float rho;

	ParticleSet particleSet;
	MACGrid<float> grid;
	Grid<float> uCopy, vCopy;
	Grid<char> cell;
	Grid<bool> isSolid;
	Grid<glm::vec3> solidVelocity;
	// pressure solver
	Solver ps; 

	void init();
	void fillCell(int i, int j);

	void step();

	// substeps
	void advect();
	void gather(uint gi);
	void scather(uint gi);
	void classifyCells();
	void solvePressure();
	void addForces();
	void enforceBoundary();
private:
	int ij(int i, int j);
};

void FLIP::init(){
	// MAC Grid
	grid.set(size[0], size[1]);
	// Solid Walls
	isSolid.set(size[0], size[1]);
	isSolid.setAll(false);
	solidVelocity.set(size[0], size[1]);
	solidVelocity.setAll(glm::vec3(0,0,0));
	// cells
	cell.set(size[0], size[1]);
	cell.setAll(0);
	
	srand (time(NULL));
}

void FLIP::fillCell(int i, int j){
	float subsize = dx / 2.0;
	for(int x = 0; x < 2; x++)
		for(int y = 0; y < 2; y++){
			particleSet.add(glm::vec3(float(i)*dx + (x-1)*subsize + 
						((double) rand() / (RAND_MAX))*subsize,
						float(j)*dx + (y-1)*subsize +
						((double) rand() / (RAND_MAX))*subsize,
						0.0),
					glm::vec3(0,0,0),
					glm::vec3(40,-40,0));
		}
}

void FLIP::step(){
	gather(GridType::U);
	gather(GridType::V);
	addForces();
	classifyCells();
	solvePressure();
	advect();
	particleSet.init();
}

void FLIP::gather(uint gi){
	GridPtr<float> g = grid.get(gi);
	for(int i = 0; i < g->size.x; i++)
		for(int j = 0; j < g->size.y; j++){
			glm::vec3 gp = dx*glm::vec3(float(i),float(j),0.0) + 
				       dx*glm::vec3(g->offset.x, g->offset.y, 0.0);
			glm::vec3 boxMin = gp - glm::vec3(dx,dx,dx);
			glm::vec3 boxMax = gp + glm::vec3(dx,dx,dx);
			float sum = 0.0;
			float wsum = 0.0;
			int total = 0;
			//std::cerr << "for grid " << gi << ", cell " << i << "," << j << ": bbox "
			//	<< boxMin.x << "," << boxMin.y << "," << boxMax.x << "," << boxMax.y 
			//	<< " ->\n";
			particleSet.iterateNeighbours(boxMin, boxMax, [&total, &wsum, &sum, gi, gp, this](const Particle& p){
				double k = 1.0;
				for(int d = 0; d < 3; d++)
					k *= quadraticBSpline((p.p[d] - gp[d])/dx);
				wsum += k;
				sum += p.v[gi]*k;
				total++;
			//	std::cerr << "particle  " << p.v.x << " " << p.v.y << std::endl;
			//	std::cerr << "wsum inside " << wsum << std::endl;
			});
			//std::cerr << sum << std::endl;
			//std::cerr << wsum << std::endl;
			if(total)
				(*g)(i,j) = sum / wsum;
			else 
				(*g)(i,j) = 0.0;
			
	}
}

void FLIP::scather(uint gi){
	GridPtr<float> g = grid.get(gi);
	for(Particle& pa : particleSet.pl.particles){
		pa.v.x = g->sample(pa.p.x,pa.p.y);
	}
}

void FLIP::addForces(){
	GridPtr<float> v = grid.get(GridType::V);
	for(int i = 0; i < v->size[0]; i++)
		for(int j = 0; j < v->size[1]; j++)
			(*v)(i,j) += gravity*dt;
	enforceBoundary();
}

void FLIP::classifyCells(){
	cell.setAll(0);
	float half = dx/2.0;
	// each cell containing at least one particle is a FLUID cell
	for(auto pa : particleSet.pl.particles){
		glm::ivec3 pCell = glm::ivec3(int((pa.p[0]+half)/dx),int((pa.p[1]+half)/dx),int((pa.p[2]+half)/dx));
		if(pCell[0] >= 0 && pCell[0] < cell.size[0] &&
		   pCell[1] >= 0 && pCell[1] < cell.size[1])
			cell(pCell[0], pCell[1]) = FLUID;
	}
	// find SOLID and AIR cells
	for(int i = 0; i < cell.size[0]; i++)
		for(int j = 0; j < cell.size[1]; j++){
			assert((cell(i,j) == FLUID && cell(i,j) != SOLID) ||
					(cell(i,j) == SOLID && cell(i,j) != FLUID));
			if(cell(i,j) == FLUID)
				continue;
			if(isSolid(i,j))
				cell(i,j) = SOLID;
			else cell(i,j) = AIR;
		}
}

void FLIP::solvePressure(){
	assert(cell.size[0] == size[0] && cell.size[1] == size[1]);

	ps.set(size[0]*size[1]);

	GridPtr<float> u = grid.get(GridType::U);
	GridPtr<float> v = grid.get(GridType::V);
	// reset b from linear system
	// construct RHS
	double scale = 1.0 / dx;
	for(int i = 0; i < size[0]; i++)
		for(int j = 0; j < size[1]; j++){
			if(cell(i,j) == FLUID){
				ps.B(ij(i,j)) = -scale * ((*u)(i+1,j) - (*u)(i,j) +
						(*v)(i,j+1) - (*v)(i,j));
				if(cell(i-1,j) == SOLID)
					ps.B(ij(i,j)) -= scale * ((*u)(i,j) - solidVelocity(i,j)[0]);
				if(cell(i+1,j) == SOLID)
					ps.B(ij(i,j)) += scale * ((*u)(i+1,j) - solidVelocity(i+1,j)[0]);
				if(cell(i,j-1) == SOLID)
					ps.B(ij(i,j)) -= scale * ((*v)(i,j) - solidVelocity(i,j)[1]);
				if(cell(i,j+1) == SOLID)
					ps.B(ij(i,j)) += scale * ((*v)(i,j+1) - solidVelocity(i,j+1)[1]);
			}
			else ps.B(ij(i,j)) = 0.0;
		}

	// set up the matrix
	scale = dt / (rho*dx*dx);
	for(int i = 0; i < size[0]; i++)
		for(int j = 0; j < size[1]; j++){
			if(cell(i,j) == FLUID){
				int fluidAndEmptyCount = 0;
				// i+1,j
				if(cell(i+1,j) == FLUID){
					ps.A.coeffRef(ij(i,j),ij(i+1,j)) = -scale;
					fluidAndEmptyCount++;
				}
				if(cell(i+1,j) == AIR)
					fluidAndEmptyCount++;
				// i-1,j
				if(cell(i-1,j) == FLUID){
					ps.A.coeffRef(ij(i,j),ij(i-1,j)) = -scale;
					fluidAndEmptyCount++;
				}
				if(cell(i-1,j) == AIR)
					fluidAndEmptyCount++;
				// i,j+1
				if(cell(i,j+1) == FLUID){
					ps.A.coeffRef(ij(i,j),ij(i,j+1)) = -scale;
					fluidAndEmptyCount++;
				}
				if(cell(i,j+1) == AIR)
					fluidAndEmptyCount++;
				// i,j-1
				if(cell(i,j-1) == FLUID){
					ps.A.coeffRef(ij(i,j),ij(i,j-1)) = -scale;
					fluidAndEmptyCount++;
				}
				if(cell(i,j-1) == AIR)
					fluidAndEmptyCount++;

				ps.A.coeffRef(ij(i,j),ij(i,j)) = fluidAndEmptyCount*scale;
			}
		}
	ps.solve();

	// update velocities
	scale = dt / (rho*dx);
	for(int i = 0; i < size[0]; i++)
		for(int j = 0; j < size[1]; j++){
			if(cell(i,j) == FLUID){
				(*u)(i,j) -= scale*ps.X(ij(i,j));	
				(*u)(i+1,j) += scale*ps.X(ij(i,j));	
				(*v)(i,j) -= scale*ps.X(ij(i,j));	
				(*v)(i,j+1) += scale*ps.X(ij(i,j));	
			}
		}
	enforceBoundary();
}

void FLIP::enforceBoundary(){
	GridPtr<float> u = grid.get(GridType::U);
	GridPtr<float> v = grid.get(GridType::V);
	for(int i = 0; i < size[0]; i++)
		for(int j = 0; j < size[1]; j++){
			if(cell(i,j) == SOLID){
				(*u)(i,j) = solidVelocity(i,j)[0];
				(*u)(i+1,j) = solidVelocity(i,j)[0];
				(*v)(i,j) = solidVelocity(i,j)[1];
				(*v)(i,j+1) = solidVelocity(i,j)[1];
			}
		}
}

void FLIP::advect(){
	for(Particle& pa : particleSet.pl.particles){
		pa.p = glm::vec3(
				pa.p.x + dt*pa.v.x,
				pa.p.y + dt*pa.v.y,
				pa.p.z + dt*pa.v.z);
	}
}

int FLIP::ij(int i, int j){
	return i*size[0] + j;
}
