#include <iostream>
#include <vector>
#include <stdio.h>
using namespace std;
struct xyz {float x, y, z;};
int main(){
	vector<xyz> p, n;
	char line[1000];
	while(fgets(line, 1000, stdin) != NULL){
		xyz d;
		sscanf(line, "%*s %f %f %f", &d.x, &d.y, &d.z);
		if(line[0] == 'v' && line[1] == 'n')
			n.push_back(d);
		else if(line[0] == 'v')
			p.push_back(d);
	}
	cout << p.size() << endl;
	for(int i = 0; i < p.size(); i++)
		cout << 
			p[i].x << " " 
			<< p[i].y << " "
			<< p[i].z << " "
			<< n[i].x << " "
			<< n[i].y << " "
			<< n[i].z << "\n";
	return 0;
}
