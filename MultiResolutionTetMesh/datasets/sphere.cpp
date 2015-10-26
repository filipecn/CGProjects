#include <iostream>
#include <cmath>
using namespace std;

int main(){
	double step = 2.0*acos(-1.0)/100.0;
	double phi = -acos(-1.0)/2.0;
	while(phi < acos(-1.0)){
		double gama = 0.0;
		while(gama < 2.0*acos(-1.0)){
			double x = cos(gama)*sin(phi);
			double y = sin(gama)*sin(phi);
			double z = cos(phi);
			cout << x << " " << y << " " << z << " " <<
			x/2.0 << " " << y/2.0 << " " << z/2.0 << endl;
			gama += step;
		}
		phi += step;
	}
	return 0;
}
