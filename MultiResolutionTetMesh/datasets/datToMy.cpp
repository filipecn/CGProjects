#include <iostream>
#include <vector>

using namespace std;

struct Point {
	double d[6];
};

int main(){
	int n; cin >> n;
	vector<Point> points;
	while(n--){
		Point p;
		for(int i = 0; i < 6; i++)
			cin >> p.d[i];
		int border = 0;
		cin >> border;
		if(border)
			points.push_back(p);
	}
	cout << points.size() << endl;
	for(int i = 0; i < points.size(); i++){
		for(int j = 0; j < 6; j++)
			cout << points[i].d[j] << " ";
		cout << endl;
	}
	return 0;
}
