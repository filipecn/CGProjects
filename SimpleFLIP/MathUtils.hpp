#pragma once

#include <glm/glm.hpp>

typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned long ulong;

double trilinearHatFunction(double r){
	if(0.0 <= r && r <= 1.0)
		return 1.0 - r;
	if(-1.0 <= r && r <= 0.0)
		return 1.0 + r;
	return 0.0;
}

double quadraticBSpline(double r){
	if(-1.5 <= r && r < -0.5)
		return 0.5*(r + 1.5)*(r + 1.5);
	if(-0.5 <= r && r < 0.5)
		return 0.75 - r*r;
	if(0.5 <= r && r < 1.5)
		return 0.5*(1.5 - r)*(1.5 - r);
	return 0.0;
}

double bilinearInterpolation(double f00, double f10, double f11, double f01, double x, double y){
	return f00*(1.0 - x)*(1.0 - y) + f10*x*(1.0 - y) + f01*(1.0 - x)*y + f11*x*y;
}

template<typename T>
T cubicInterpolate (T p[4], T x) {
	return p[1] + 0.5 * x*(p[2] - p[0] + x*(2.0*p[0] - 5.0*p[1] + 4.0*p[2] - p[3] + x*(3.0*(p[1] - p[2]) + p[3] - p[0])));
}

template<typename T>
T bicubicInterpolate (T p[4][4], T x, T y) {
	T arr[4];
	arr[0] = cubicInterpolate(p[0], y);
	arr[1] = cubicInterpolate(p[1], y);
	arr[2] = cubicInterpolate(p[2], y);
	arr[3] = cubicInterpolate(p[3], y);
	return cubicInterpolate(arr, x);
}