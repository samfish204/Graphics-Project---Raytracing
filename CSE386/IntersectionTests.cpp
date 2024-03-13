#include "ishape.h"
#include "io.h"

void checkEm(const char *name, const IShape& shape) {
	Ray ray1(dvec3(0, 0, 0), glm::normalize(dvec3(0, 0.5, -1)));	// Viewing rays are normalized
	Ray ray2(dvec3(0, 0, 0), glm::normalize(dvec3(0, 0, -1)));
	Ray ray3(dvec3(0, 0, 0), dvec3(0, -0.5, -1));

	HitRecord hit1;
	HitRecord hit2;
	HitRecord hit3;

	shape.findClosestIntersection(ray1, hit1);
	shape.findClosestIntersection(ray2, hit2);
	shape.findClosestIntersection(ray3, hit3);

	cout << name << endl;
	cout << "==============" << endl;
	cout << hit1.t << ' ' << hit1.interceptPt << ' '  << hit1.normal << endl;
	cout << hit2.t << ' ' << hit2.interceptPt << ' ' << hit2.normal << endl;
	cout << hit3.t << ' ' << hit3.interceptPt << ' ' << hit3.normal << endl;
	cout << endl;
}

int main(int argc, char* argv[]) {
	double s3 = -1.0/glm::sqrt(3.0);
	checkEm("Plane", IPlane(dvec3(0, -1, 0), dvec3(0, 1, 0)));	// normal vectors will be unit length
	checkEm("Sphere", ISphere(dvec3(0.0, 0, -1.0), 0.75));
	checkEm("Disk1", IDisk(dvec3(0, 0, -1), dvec3(0, 0, 1), 1.0));
	checkEm("Disk2", IDisk(dvec3(0, 0, -10), dvec3(0, 0, 1), 1.0));

	return 0;
}
/*
Plane
==============
3.40282e+38[0 0 0][0 0 0]
3.40282e+38[0 0 0][0 0 0]
2.23607[0 - 1 - 2][0 1 0]

Sphere
==============
0.292347[0 0.130742 - 0.261484][0 0.174322 0.984689]
0.25[0 0 - 0.25][0 0 1]
0.292347[0 - 0.130742 - 0.261484][0 - 0.174322 0.984689]

Disk1
==============
1.11803[0 0.5 - 1][0 0 1]
1[0 0 - 1][0 0 1]
1.11803[0 - 0.5 - 1][0 0 1]

Disk2
==============
3.40282e+38[0 5 - 10][0 0 1]
10[0 0 - 10][0 0 1]
3.40282e+38[0 - 5 - 10][0 0 1]
*/