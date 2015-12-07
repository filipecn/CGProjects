#include <aergia/Aergia.h>
#include <aergia/scene/Camera.h>
#include "UGrid.h"
#include <iostream>
#include <list>
#include <vector>
#include <Eigen/Dense>
#include <boost/numeric/interval.hpp>
using namespace aergia;

class Particle {
	public:
		glm::vec2 p, n;
};

std::vector<Particle> particles;

using namespace boost::numeric;
using namespace interval_lib;
void computeGradient(interval<double> X, interval<double> Y, Eigen::VectorXd v, interval<double>& GX, interval<double>& GY){
	// x' = 2ax + b
	GX = 2.0*v[0]*X;
	GY = interval<double>(-1,1);
}

class GridData : public IGridData<std::list<size_t> > {
	public:
		GridData(){}
		~GridData(){}
			//       3 _____>6______> 2
			//        ^      ^      ^
			//        |   3  |   2  |
			//       7|----->8----->|5
			//        ^   0  ^   1  ^
			//        |_____>|_____>|
			//       0       4       1
		GridData* split(int vIndices[], const std::vector<glm::vec2>& vertices) override {
			GridData* splittedData = new GridData[4];
			// move particles
			int bIndices[][2] = {{0,8},{4,5},{8,2},{7,6}};
			auto it = this->data.begin();
			while(it != this->data.end()){
				bool found = false;
				for(int c = 0; c < 4 && !found; c++)
					if(particles[*it].p.x >= vertices[vIndices[bIndices[c][0]]].x && particles[*it].p.y >= vertices[vIndices[bIndices[c][0]]].y
					&& particles[*it].p.x <= vertices[vIndices[bIndices[c][1]]].x && particles[*it].p.y <= vertices[vIndices[bIndices[c][1]]].y){
						auto _it = it; _it++;
						splittedData[c].data.emplace_back(*it);
						this->data.erase(it);
						it = _it;
						found = true;
						break;
					}
				if(!found){ std::cerr << "bad distribution!" << std::endl; exit(1); }
			}
			return splittedData;
		}
};

Camera2D camera;
UGrid<GridData> grid(-1,1,-1,1);
UGridIterator<GridData> cur(&grid);
std::function<bool(typename UGrid<GridData>::Cell&)> particlesFilter = [](typename UGrid<GridData>::Cell& c){
				return c.children[0] < 0 && c.children[1] < 0 && c.children[2] < 0 && c.children[3] < 0 && c.data->data.size(); 	
			};
			
double estimateCurvature(const typename UGrid<GridData>::Cell& c){
	if(c.data->data.size() == 0)
		return 0.0;
	const GridData* d = c.data;
	// COMPUTE PARTICLES CENTER AND MEAN NORMAL
	glm::vec2 center, normal;
	{
		bool first = true;
		for(auto pi : d->data){
			glVertex(particles[pi].p);
			if(first){
				first = false;
				center = particles[pi].p;
				normal = particles[pi].n;
			}
			else{
				center += particles[pi].p;
				normal += particles[pi].n;	
			}
		}
		if(d->data.size()){
			center = center / float(d->data.size());
			normal = glm::normalize(normal);
		}
	}
	
	// ROTATED PARTICLES
	glm::vec2 u1 = glm::normalize(normal);
	glm::vec2 u2 = glm::vec2(normal.y, -normal.x);
	Eigen::VectorXd x;
	{
		Eigen::MatrixXd A(d->data.size(),3);
		Eigen::VectorXd b(d->data.size());
		int pIndex = 0;
		for(auto pi : d->data){
			glm::vec2 point = particles[pi].p - center;
			glm::vec2 rotatedPoint = glm::vec2(glm::dot(point,u2),glm::dot(point,u1));
			
			//ax2 + bx + c
			A(pIndex,0) = rotatedPoint.x*rotatedPoint.x;
			A(pIndex,1) = rotatedPoint.x;
			A(pIndex,2) = 1.0;
			
			b(pIndex) = rotatedPoint.y;
			
			pIndex++;
		}
			
		x = A.colPivHouseholderQr().solve(b);
	}
	
	// GRADIENT BOX
	interval<double> GX,GY; 
	{
		float xmin, xmax, ymin, ymax;
		bool first = true;
		for(auto pi : d->data){
			glm::vec2 point = particles[pi].p - center;
			glm::vec2 rotatedPoint = glm::vec2(glm::dot(point,u2),glm::dot(point,u1));
			//ax2 + by2 + cxy + dx + ey + f
			if(first){
				first = false;
				xmin = xmax = rotatedPoint.x;
				ymin = ymax = rotatedPoint.y;
			}
			xmin = std::min(rotatedPoint.x,xmin);
			xmax = std::max(rotatedPoint.x,xmax);
			ymin = std::min(rotatedPoint.y,ymin);
			ymax = std::max(rotatedPoint.y,ymax);		
		}
		interval<double> X(xmin,xmax), Y(ymin,ymax);
		computeGradient(X,Y,x,GX,GY);
	}
			std::cout << "WIDTHS " << width(GX) << "  " << width(GY) << std::endl;
	return width(GX);
}

void render(){
	glClearColor(1,1,1,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	camera.look();
	
	// CELLS
	UGridIterator<GridData> it(&grid);
	it.leafs();
	while(it.test()){
		glColor4f(0,0,0,0.1);
		glBegin(GL_LINE_LOOP);
		for(int i = 0; i < 4; i++)
			glVertex(it.vertex(i));
		glEnd();
		it.next();
	}
	
	// PARTICLES AND P NORMALS
	glPointSize(3.0);
	glBegin(GL_POINTS);
	for(auto p : particles){
		glVertex(p.p);
	}
	glEnd();
	
	glBegin(GL_LINES);
	for(auto p : particles){
		glVertex(p.p);
			glVertex(p.p + p.n);
	}
	glEnd();
	
	// DRAW PARTICLES INSIDE CELL
	glBegin(GL_POINTS);
	glm::vec2 center, normal;
	glColor3f(1,0,1);
	if(cur.test()){
		const GridData* d = cur.data();
		bool first = true;
		for(auto pi : d->data){
			glVertex(particles[pi].p);
			if(first){
				first = false;
				center = particles[pi].p;
				normal = particles[pi].n;
			}
			else{
				center += particles[pi].p;
				normal += particles[pi].n;	
			}
		}
		if(d->data.size()){
			center = center / float(d->data.size());
			normal = glm::normalize(normal);
		}
	}
	glEnd();
	
    // CENTER AND NORMAL
	glPointSize(5.0);
	glColor4f(0,0.2,1,0.95);
	glBegin(GL_POINT);
		glVertex(center);
	glEnd();
	
	// CUR CELL
	glBegin(GL_LINE_LOOP);
	if(cur.test()){
		for(int i = 0; i < 4; i++)
			glVertex(cur.vertex(i));
	}
	glEnd();
	
	// CUR CENTER + NORMAL
	glBegin(GL_LINES);
		glVertex(center);
		glVertex(center + normal);
	glEnd();
	
	// VISUALIZATION OF ROTATED BASE
	glm::vec2 centerVis = glm::vec2(-10,0);
	glBegin(GL_LINES);
		glVertex(centerVis); glVertex(centerVis + glm::vec2(0,1));
		glVertex(centerVis); glVertex(centerVis + glm::vec2(1,0));
	glEnd();
	
	
	// ROTATED PARTICLES
	glm::vec2 u1 = glm::normalize(normal);
	glm::vec2 u2 = glm::vec2(normal.y, -normal.x);
	Eigen::VectorXd x;
	glColor4f(1,0,0,0.8);
	glBegin(GL_POINTS);
	if(cur.test()){
		const GridData* d = cur.data();
		Eigen::MatrixXd A(d->data.size(),3);
		Eigen::VectorXd b(d->data.size());
		int pIndex = 0;
		for(auto pi : d->data){
			glm::vec2 point = particles[pi].p - center;
			glm::vec2 rotatedPoint = glm::vec2(glm::dot(point,u2),glm::dot(point,u1));
			glVertex(rotatedPoint + centerVis);
			
			//ax2 + bx + c
			A(pIndex,0) = rotatedPoint.x*rotatedPoint.x;
			A(pIndex,1) = rotatedPoint.x;
			A(pIndex,2) = 1.0;
			
			b(pIndex) = rotatedPoint.y;
			
			pIndex++;
		}
		std::cout << "MATRIX A\n";
		std::cout << A << std::endl;
		std::cout << b << std::endl;
			
		x = A.colPivHouseholderQr().solve(b);
	}
	glEnd();
	
	
	// DRAW GRADIENT BOX
	glColor4f(0,1,0,0.7);
	glm::vec2 boxPos = centerVis + glm::vec2(0,3);
	glBegin(GL_LINE_LOOP);
	if(cur.test()){
		const GridData* d = cur.data();
		float xmin, xmax, ymin, ymax;
		bool first = true;
		for(auto pi : d->data){
			glm::vec2 point = particles[pi].p - center;
			glm::vec2 rotatedPoint = glm::vec2(glm::dot(point,u2),glm::dot(point,u1));
			//ax2 + by2 + cxy + dx + ey + f
			if(first){
				first = false;
				xmin = xmax = rotatedPoint.x;
				ymin = ymax = rotatedPoint.y;
			}
			xmin = std::min(rotatedPoint.x,xmin);
			xmax = std::max(rotatedPoint.x,xmax);
			ymin = std::min(rotatedPoint.y,ymin);
			ymax = std::max(rotatedPoint.y,ymax);		
		}
		interval<double> X(xmin,xmax), Y(ymin,ymax);
		interval<double> GX,GY; 
		computeGradient(X,Y,x,GX,GY);
		std::cout << "printign \n";
		std::cout << x << std::endl;
		std::cout << xmin << " -> " << xmax << std::endl;
		std::cout << lower(X) << " " << upper(X) << std::endl;
		std::cout << lower(GX) << " " << upper(GX) << " " << width(GX) << std::endl;
		std::cout << "WIDTHS " << width(GX) << "  " << width(GY) << std::endl;
		glVertex(boxPos);
		glVertex(boxPos + glm::vec2(float(width(GX)),0));
		glVertex(boxPos + glm::vec2(float(width(GX)),float(width(GY))));
		glVertex(boxPos + glm::vec2(0,float(width(GY))));
		estimateCurvature(cur.cell());
	}
	glEnd();		
}

void resize(int w, int h){
	camera.resize(w,h);
}

void button(int button, int action){
	if(action == GLFW_RELEASE){
		cur.next();
		if(!cur.test())
			cur.filterCells(particlesFilter);
	}
}

void readParticles(GridData& firstData){
	int n;
	std::cin >> n;
	float xmin,xmax,ymin,ymax;
	bool first = true;
	while(n--){
		float t;
		Particle p;
		std::cin >> p.p.x >> p.p.y >> t >> p.n.x >> p.n.y >> t;
		p.n *= -1.0f;
		particles.emplace_back(p);
		if(first){
			xmin = xmax = p.p.x;
			ymin = ymax = p.p.y;
			first = false;
		}
		xmin = std::min(xmin,p.p.x);
		ymin = std::min(ymin,p.p.y);
		xmax = std::max(xmax,p.p.x);
		ymax = std::max(ymax,p.p.y);
	}
	for(uint i = 0; i < particles.size(); i++)
		firstData.data.emplace_back(i);
}

int main(){
	GridData firstData;
	readParticles(firstData);		
	grid.set(-5,10,-10,10,&firstData);
	camera.setZoom(15);
	camera.resize(800,800);
	//grid.generateFullGrid(0);
	//grid.generateFullGrid(3, [](typename UGrid<GridData>::Cell& c){
	//			return c.data->data.size() > 2 && estimateCurvature(c) > 1.5; 	
	//		});
	cur.filterCells(particlesFilter);
	GraphicsDisplay& gd = GraphicsDisplay::create(800,800,std::string("qCurv"));
	gd.registerRenderFunc(render);
	gd.registerResizeFunc(resize);
	gd.registerButtonFunc(button);
	gd.start();
	return 0;
}