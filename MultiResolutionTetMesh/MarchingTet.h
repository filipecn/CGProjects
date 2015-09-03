#include <glm/glm.hpp>
#include <functional>

class MarchingTet {
	public:
	struct Triangle{
		glm::vec3 v[3];
	};

	std::vector<Triangle> triangles;

	glm::vec3 interp(glm::vec3 p1, glm::vec3 p2, float valp1, float valp2){
		double mu;
		glm::vec3 p;

		if (fabs(valp1) < 0.00001) return p1;
		if (fabs(valp2) < 0.00001) return p2;
		if (fabs(valp2) < 0.00001) return p1;
		mu = (0.0f - valp1) / (valp2 - valp1);
		p.x = p1.x + mu * (p2.x - p1.x);
		p.y = p1.y + mu * (p2.y - p1.y);
		p.z = p1.z + mu * (p2.z - p1.z);

		return p;
	}

	void marchTet(glm::vec3 tetVertices[], std::function<float(glm::vec3)> f){
		float v[4];
		for(int i = 0; i < 4; i++)
			v[i] = f(tetVertices[i]);
		runMarchTet(tetVertices, v);
	}
	
	void marchTet(glm::vec3 tetVertices[], float v[]){
		runMarchTet(tetVertices, v);
	}

	void runMarchTet(glm::vec3 tetVertices[], float v[]){
		int triindex = 0;
		if (v[0] < 0) triindex |= 1;
		if (v[1] < 0) triindex |= 2;
		if (v[2] < 0) triindex |= 4;
		if (v[3] < 0) triindex |= 8;
		switch (triindex) {
			case 0x00:
			case 0x0F:
				break;
			case 0x0E:
			case 0x01:{Triangle t;
					  t.v[0] = interp(tetVertices[0],tetVertices[1],v[0],v[1]);
					  t.v[1] = interp(tetVertices[0],tetVertices[2],v[0],v[2]);
					  t.v[2] = interp(tetVertices[0],tetVertices[3],v[0],v[3]);
					  triangles.emplace_back(t);
				  } break;
			case 0x0D:
			case 0x02:{Triangle t;
					  t.v[0] = interp(tetVertices[1],tetVertices[0],v[1],v[0]);
					  t.v[1] = interp(tetVertices[1],tetVertices[3],v[1],v[3]);
					  t.v[2] = interp(tetVertices[1],tetVertices[2],v[1],v[2]);
					  triangles.emplace_back(t);
				  } break;
			case 0x0C:
			case 0x03:{Triangle t0;
					  t0.v[0] = interp(tetVertices[0],tetVertices[3],v[0],v[3]);
					  t0.v[1] = interp(tetVertices[0],tetVertices[2],v[0],v[2]);
					  t0.v[2] = interp(tetVertices[1],tetVertices[3],v[1],v[3]);
					  triangles.emplace_back(t0);
					  Triangle t1;
					  t1.v[0] = t0.v[2];
					  t1.v[1] = interp(tetVertices[1],tetVertices[2],v[1],v[2]);
					  t1.v[2] = t0.v[1];
					  triangles.emplace_back(t1);
				  } break;
			case 0x0B:
			case 0x04:{Triangle t;
					  t.v[0] = interp(tetVertices[2],tetVertices[0],v[2],v[0]);
					  t.v[1] = interp(tetVertices[2],tetVertices[1],v[2],v[1]);
					  t.v[2] = interp(tetVertices[2],tetVertices[3],v[2],v[3]);
					  triangles.emplace_back(t);
				  } break;
			case 0x0A:
			case 0x05:{Triangle t0;
					  t0.v[0] = interp(tetVertices[0],tetVertices[1],v[0],v[1]);
					  t0.v[1] = interp(tetVertices[2],tetVertices[3],v[2],v[3]);
					  t0.v[2] = interp(tetVertices[0],tetVertices[3],v[0],v[3]);
					  triangles.emplace_back(t0);
					  Triangle t1;
					  t1.v[0] = t0.v[0];
					  t1.v[1] = interp(tetVertices[1],tetVertices[2],v[1],v[2]);
					  t1.v[2] = t0.v[1];
					  triangles.emplace_back(t1);
				  }break;
			case 0x09:
			case 0x06:{Triangle t0;
					  t0.v[0] = interp(tetVertices[0],tetVertices[1],v[0],v[1]);
					  t0.v[1] = interp(tetVertices[1],tetVertices[3],v[1],v[3]);
					  t0.v[2] = interp(tetVertices[2],tetVertices[3],v[2],v[3]);
					  triangles.emplace_back(t0);
					  Triangle t1;
					  t1.v[0] = t0.v[0];
					  t1.v[1] = interp(tetVertices[0],tetVertices[2],v[0],v[2]);
					  t1.v[2] = t0.v[2];
					  triangles.emplace_back(t1);
				  }break;
			case 0x07:
			case 0x08:{Triangle t;
					  t.v[0] = interp(tetVertices[3],tetVertices[0],v[3],v[0]);
					  t.v[1] = interp(tetVertices[3],tetVertices[2],v[3],v[2]);
					  t.v[2] = interp(tetVertices[3],tetVertices[1],v[3],v[1]);
					  triangles.emplace_back(t);
				  }break;
		}
	}
};




