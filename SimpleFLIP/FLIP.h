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
	ParticleSetAccessor psa[3];
	
	MACGrid<float> grid;
	Grid<float> vCopy[2];
	Grid<char> cell;
	Grid<bool> isSolid;
	Grid<glm::vec3> solidVelocity;
	// pressure solver
	Solver ps; 

	void init();
	void fillCell(int i, int j);
	void step();

	// substepss
	void advect();
	void gather(uint gi);
	void scatter(uint gi);
	void classifyCells();
	void solvePressure();
	void addForces();
	void enforceBoundary();
private:
	int ij(int i, int j);
};

void FLIP::init(){
	// MAC Grid
	grid.set(size[0], size[1], dx);
	// Solid Walls
	isSolid.set(size[0], size[1], dx);
	isSolid.setAll(false);
	solidVelocity.set(size[0], size[1], dx);
	solidVelocity.setAll(glm::vec3(0,0,0));
	// cells
	cell.set(size[0], size[1], dx);
	cell.setAll(0);
	// velocity copy
	vCopy[0].set(size[0]+1, size[1], dx);
	vCopy[1].set(size[0], size[1]+1, dx);
	// ParticleSet Accessors
	psa[GridType::U].set(particleSet, glm::vec3(0,0.5*dx,0),dx);
	psa[GridType::V].set(particleSet, glm::vec3(0.5*dx,0,0),dx);
	psa[GridType::P].set(particleSet, glm::vec3(0,0,0),dx);
	
	srand (time(NULL));
}

void FLIP::fillCell(int i, int j){
	float subsize = dx / 2.0;
	for(int x = 0; x < 2; x++){
		for(int y = 0; y < 2; y++){
			/*particleSet.add(glm::vec3(float(i)*dx + (x-1)*subsize + subsize/2.0,
									  float(j)*dx + (y-1)*subsize + subsize/2.0, 0.0 ),
									  glm::vec3(0,0,0),
									  glm::vec3(0,0,0));*/
			particleSet.add(glm::vec3(float(i)*dx + (x-1)*subsize + ((double) rand() / (RAND_MAX))*subsize,
									  float(j)*dx + (y-1)*subsize + ((double) rand() / (RAND_MAX))*subsize, 0.0),
					glm::vec3(0,0,0),
					glm::vec3(0,0,0));
					
			/*particleSet.add(glm::vec3(float(i)*dx,
									  float(j)*dx - dx*0.5, 0.0),
					glm::vec3(0,0,0),
					glm::vec3(0,10,0));*/
					return;
		}
	}
}

void FLIP::step(){
	advect();
	for(int i = 0; i < 3; i++)
		psa[i].update(particleSet);
	gather(GridType::U);
	gather(GridType::V);
	classifyCells();
	addForces();
	solvePressure();
	enforceBoundary();
	scatter(GridType::U);
	scatter(GridType::V);
}

void FLIP::gather(uint gi){
	GridPtr<float> g = grid.get(gi);
	for(int i = 0; i < g->size.x; i++)
		for(int j = 0; j < g->size.y; j++){
			glm::vec2 gp = g->gridToWorld(glm::vec2(float(i),float(j)));
			glm::vec3 boxMin = glm::vec3(gp.x,gp.y,0.0) - glm::vec3(dx,dx,dx);
			glm::vec3 boxMax = glm::vec3(gp.x,gp.y,0.0) + glm::vec3(dx,dx,dx);
			float sum = 0.0;
			float wsum = 0.0;
			int total = 0;
			psa[gi].iterateNeighbours(particleSet, boxMin, boxMax, [&total, &wsum, &sum, gi, gp, this](const Particle& p){
				double k = 1.0;
				glm::vec3 pos = p.getPos();
				glm::vec3 vel = p.getVelocity();
				for(int d = 0; d < 2; d++)
					k *= quadraticBSpline((pos[d] - gp[d])/dx);
				wsum += k;
				sum += vel[gi]*k;
				total++;
			});
			if(total && wsum != 0.0){
				(*g)(i,j) = sum / wsum;
			}
			else 
				(*g)(i,j) = 0.0;
			vCopy[gi](i,j) = (*g)(i,j);
	}
}

void FLIP::scatter(uint gi){
	GridPtr<float> g = grid.get(gi);
	for(Particle& pa : particleSet.particles){
		glm::vec3 p = pa.getPos();
		glm::vec3 v = pa.getVelocity();
		v[gi] = g->sample(p.x,p.y);
		pa.setVelocity(v);
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
	float half = 0.5*dx;
	// each cell containing at least one particle is a FLUID cell
	for(auto pa : particleSet.particles){
		glm::vec3 p = pa.getPos();
		glm::ivec2 pCell = cell.cellID(p.x,p.y);
		//std::cerr << "particle in cell " << pCell[0] << " " << pCell[1] << std::endl;
		//std::cerr << p << std::endl;
		assert(pCell[0] >= 0 && pCell[0] < cell.size[0] && pCell[1] >= 0 && pCell[1] < cell.size[1]);
		//std::cerr << pCell[0] << " " << pCell[1] << std::endl;
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
				ps.B(ij(i,j)) = -scale * ((*u)(i+1,j) - (*u)(i,j) + (*v)(i,j+1) - (*v)(i,j));
				if(cell(i-1,j) == SOLID)
					ps.B(ij(i,j)) -= scale * ((*u)(i,j) - solidVelocity(i-1,j)[0]);
				if(cell(i+1,j) == SOLID)
					ps.B(ij(i,j)) += scale * ((*u)(i+1,j) - solidVelocity(i+1,j)[0]);
				if(cell(i,j-1) == SOLID)
					ps.B(ij(i,j)) -= scale * ((*v)(i,j) - solidVelocity(i,j-1)[1]);
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
	for(Particle& pa : particleSet.particles){
		glm::vec3 p = pa.getPos();
		glm::vec3 v = pa.getVelocity();
		glm::vec3 e = p + dt*v;
		
		int curCell = cell.dSample(e.x,e.y,-1);
		if(curCell < 0 || curCell == CellType::SOLID){
			while(glm::distance(p,e) > 1e-5 && (curCell < 0 || curCell == CellType::SOLID)){
				glm::vec3 m = 0.5f*p + 0.5f*e;
				curCell = cell.dSample(m.x,m.y,-1);
				if(curCell < 0 || curCell == CellType::SOLID)
					e = m;
				else p = m;
				curCell = cell.dSample(e.x,e.y,-1);
			}
			e -= (dt/10.0f)*v;
		}
		pa.setPos(e);
	}
}

int FLIP::ij(int i, int j){
	return i*size[0] + j;
}
