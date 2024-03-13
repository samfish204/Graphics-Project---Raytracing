#include <istream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <sstream>
#include <tuple>
#include "defs.h"
#include "io.h"
#include "utilities.h"
#include "hitrecord.h"
#include "light.h"
#include "camera.h"
#include "ishape.h"

ostream& operator << (ostream& os, const IPlane& plane) {
	os << plane.a << ' ' << plane.n;
	return os;
}

istream& operator >> (std::istream& is, IPlane& plane) {
	is >> plane.a >> plane.n;
	return is;
}
ostream& operator << (ostream& os, const Ray& ray) {
	os << ray.origin << ' ' << ray.dir;
	return os;
}

istream& operator >> (std::istream& is, Ray& ray) {
	is >> ray.origin >> ray.dir;
	return is;
}
ostream& operator << (ostream& os, const IDisk& disk) {
	os << disk.center << ' ' << disk.n << ' ' << disk.radius;
	return os;
}
istream& operator >> (std::istream& is, IDisk& disk) {
	is >> disk.center >> disk.n >> disk.radius;
	return is;
}
ostream& operator << (ostream& os, const HitRecord& hit) {
	os << hit.t << ' ' << hit.interceptPt << ' ' << hit.normal;
	return os;
}

istream& operator >> (istream& is, HitRecord& hit) {
	is >> hit.t >> hit.interceptPt >> hit.normal;
	return is;
}


static bool operator == (const HitRecord& h1, const HitRecord& h2) {
	if (h1.t == FLT_MAX && h2.t == FLT_MAX)
		return true;
	else
		return approximatelyEqual(h1.t, h2.t) && ave(h1.normal, h2.normal) && ave(h1.interceptPt, h2.interceptPt);

}

double pts = 0.0;

const bool ALL_OR_NONE = false;

void displayPoints(ostream& os, int corr, int total, double maxPts) {
	double thisProblemPoints = 0.0;
	if (corr == total) {
		thisProblemPoints = maxPts;
	} else if (corr >= 0.75 * total && !ALL_OR_NONE) {
		thisProblemPoints = maxPts / 2;
	} else {
		thisProblemPoints = 0;
	}
	pts += thisProblemPoints;

	const int N = 400;
	char buf[N];
	sprintf(buf, "%.2f/%.2f points\n\n", thisProblemPoints, maxPts);
	os << buf;

	return;
}

namespace Testing {

const char* TestFile = "testCases.txt";
std::ofstream output;

std::istream *openFile(string fname) {
	string contents;
	std::ifstream ifile(TestFile);
	while (!ifile.eof()) {
		string line = getLine(ifile);
		//cout << "[" << line << "]" << endl;
		//if (line == "----------")
		//	break;
		contents += line + "\n";
	}
	ifile.close();
	return new std::istringstream(contents);
}

void createTestsQuadraticVector(const char* name, 
								const std::vector<std::tuple<double, double, double>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		double arg1 = std::get<0>(testCases[i]);
		double arg2 = std::get<1>(testCases[i]);
		double arg3 = std::get<2>(testCases[i]);
		vector<double> roots = quadratic(arg1, arg2, arg3);
		os << arg1 << ' ' << arg2 << ' ' << arg3 << ' ' << roots << endl;
	}
	os << std::endl;
}
void createTestsQuadratic(const char* name, int (*f)(double, double, double, double []), 
							const std::vector<std::tuple<double, double, double>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		double arg1 = std::get<0>(testCases[i]);
		double arg2 = std::get<1>(testCases[i]);
		double arg3 = std::get<2>(testCases[i]);
		double roots[2] = { 0, 0 };
		os << arg1 << ' ' << arg2 << ' ' << arg3 << ' ';
		int nRoots = (*f)(arg1, arg2, arg3, roots);
		os << nRoots << ' ' << roots[0] << ' ' << roots[1] << endl;
	}
	os << std::endl;
}
template <class T>
void createPlaneIntersectionTests(const char* name, const std::vector<std::tuple<T, Ray>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		T arg1 = std::get<0>(testCases[i]);
		Ray arg2 = std::get<1>(testCases[i]);
		HitRecord hit;
		arg1.findClosestIntersection(arg2, hit);
		os << arg1 << ' ' << arg2.origin << ' ' << arg2.dir << ' ' <<
			hit << endl;
	}
	os << std::endl;
}

template <class T>
void runPlaneIntersectionTests(const char* N, double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;
				T obj;
				Ray ray(dvec3(0,0,0), dvec3(0,0,0));
				HitRecord expectedOutput;
				input >> obj >> ray >> expectedOutput;
				getLine(input);
				os << "IShape: " << obj << endl;
				os << "findClosestIntersection(" << "Ray: " << ray << ", HitRecord) --> " << endl;
				os << " final hitrecord(t, pt, n) = " << expectedOutput << endl;
				try {
					HitRecord actualOutput;
					obj.findClosestIntersection(ray, actualOutput);
					if  (expectedOutput == actualOutput) {
						corr++;
						os << " Correct" << endl << endl;
					} else {
						os << " Incorrect (Hit record actually returned was: " << actualOutput << " )" << endl << endl;
					}
				}
				catch (...) {
					os << " Incorrect (exception thrown)" << endl;
					exceptions++;
				}
			}
		}
	}
}


template <class I1>
void createTests(const char* name, void (*f)(I1&, I1&), 
					const std::vector<std::tuple<I1, I1, I1, I1>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		I1 arg1 = std::get<0>(testCases[i]);
		I1 arg2 = std::get<1>(testCases[i]);
		os << arg1 << ' ' << arg2 << ' ';
		(*f)(arg1, arg2);
		os << arg1 << ' ' << arg2 << endl;
	}
	os << std::endl;
}
template <class I1>
void createTests(const char* name, void (*f)(I1, I1 &, I1 &), 
					const std::vector<std::tuple<I1, I1, I1>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		I1 arg1 = std::get<0>(testCases[i]);
		I1 arg2 = std::get<1>(testCases[i]);
		I1 arg3 = std::get<2>(testCases[i]);
		os << arg1 << ' ' << arg2 << ' ' << arg3 << ' ';
		(*f)(arg1, arg2, arg3);
		os << arg1 << ' ' << arg2 << ' ' << arg3 << endl;
	}
	os << std::endl;
}
template <class I1>
void createTests(const char* name, void (*f)(I1, I1, I1, I1, I1, I1&), 
					const std::vector<std::tuple<I1, I1, I1, I1, I1, I1>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		I1 arg1 = std::get<0>(testCases[i]);
		I1 arg2 = std::get<1>(testCases[i]);
		I1 arg3 = std::get<2>(testCases[i]);
		I1 arg4 = std::get<3>(testCases[i]);
		I1 arg5 = std::get<4>(testCases[i]);
		I1 arg6 = std::get<5>(testCases[i]);
		os << arg1 << ' ' << arg2 << ' ' << arg3 << ' ' << arg4 << ' ' << arg5 << ' ' << arg6 << endl;
		(*f)(arg1, arg2, arg3, arg4, arg5, arg6);
		os << arg1 << ' ' << arg2 << ' ' << arg3 << ' ' << arg4 << ' ' << arg5 << ' ' << arg6 << endl;
	}
	os << std::endl;
}
template <class O, class I1>
void createTests(const char* name, O(*f)(I1), const std::vector<I1>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		const I1 &i1 = testCases[i];
		os << i1 << ' ' << (*f)(i1) << endl;
	}
	os << std::endl;
}
template <class O, class I1>
void createTestsVec(const char* name, O(*f)(const vector<I1>&), 
					const std::vector<vector<I1>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		vector<I1> i1 = testCases[i];
		os << i1 << ' ' << (*f)(i1) << endl;
	}
	os << endl;
}
template <class O, class I1, class I2>
void createTestsVec(const char* name, O(*f)(const vector<I1>&, const I2 &), 
					const std::vector<std::tuple<vector<I1>, I2>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		vector<I1> i1 = std::get<0>(testCases[i]);
		I2 i2 = std::get<1>(testCases[i]);
		os << i1 << ' ' << i2 << ' ' << (*f)(i1, i2) << endl;
	}
	os << std::endl;
}
template <class O, class I1, class I2>
void createTestsVec(const char* name, vector<O>(*f)(const I1&, const vector<I2>&), 
					const std::vector<std::tuple<I1, vector<I2>>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		I1 i1 = std::get<0>(testCases[i]);
		vector<I2> i2 = std::get<1>(testCases[i]);
		os << i1 << ' ' << i2 << ' ' << (*f)(i1, i2) << endl;
	}
	os << std::endl;
}
template <class O, class I1, class I2>
void createTestsVec(const char* name, vector<O>(*f)(const vector<I1>&, 
						const vector<I2>&), const std::vector<std::tuple<vector<I1>, vector<I2>>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		vector<I1> i1 = std::get<0>(testCases[i]);
		vector<I2> i2 = std::get<1>(testCases[i]);
		os << i1 << ' ' << i2 << ' ' << (*f)(i1, i2) << endl;
	}
	os << std::endl;
}
template <class O, class I1>
void createTests(const char* name, O(*f)(I1, I1),
					const std::vector<std::tuple<I1, I1>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		I1 i1 = std::get<0>(testCases[i]);
		I1 i2 = std::get<1>(testCases[i]);
		os << i1 << ' ' << i2 << ' ' << (*f)(i1, i2) << endl;
	}
	os << std::endl;
}
template <class O, class I1>
void createTests(const char* name, O(*f)(I1, I1, I1, I1),
	const std::vector<std::tuple<I1, I1, I1, I1>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		I1 i1 = std::get<0>(testCases[i]);
		I1 i2 = std::get<1>(testCases[i]);
		I1 i3 = std::get<2>(testCases[i]);
		I1 i4 = std::get<3>(testCases[i]);
		os << i1 << ' ' << i2 << ' ' << i3 << ' ' << i4 << ' ' << (*f)(i1, i2, i3, i4) << endl;
	}
	os << std::endl;
}
template <class O, class I1>
void createTests(const char* name, O(*f)(const I1 &, const I1 &), 
					const std::vector<std::tuple<I1, I1>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		I1 i1 = std::get<0>(testCases[i]);
		I1 i2 = std::get<1>(testCases[i]);
		os << i1 << ' ' << i2 << ' ' << (*f)(i1, i2) << endl;
	}
	os << std::endl;
}
template <class O, class I1>
void createTests(const char* name, O(*f)(const I1&, const I1&, const I1 &), 
				const std::vector<std::tuple<I1, I1, I1>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		I1 i1 = std::get<0>(testCases[i]);
		I1 i2 = std::get<1>(testCases[i]);
		I1 i3 = std::get<2>(testCases[i]);
		os << i1 << ' ' << i2 << ' ' << i3 << ' ' << (*f)(i1, i2, i3) << endl;
	}
	os << std::endl;
}
template <class O, class I1, class I2>
void createTests(const char* name, O(*f)(const I1 &, I2), 
				const std::vector<std::tuple<I1, I2>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		I1 i1 = std::get<0>(testCases[i]);
		I2 i2 = std::get<1>(testCases[i]);
		os << i1 << ' ' << i2 << ' ' << (*f)(i1, i2) << endl;
	}
	os << std::endl;
}
template <class O, class I1, class I2, class I3>
void createTests(const char* name, O(*f)(I1, I2, I3), 
				const std::vector<std::tuple<I1, I2, I3>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		I1 i1 = std::get<0>(testCases[i]);
		I2 i2 = std::get<1>(testCases[i]);
		I3 i3 = std::get<2>(testCases[i]);
		os << i1 << ' ' << i2 << ' ' << i3 << ' ' << (*f)(i1, i2, i3) << endl;
	}
	os << std::endl;
}
template <class O, class I1, class I2, class I3, class I4, class I5, class I6>
void createTests(const char* name, O(*f)(I1, I2, I3, I4, I5, I6),
	const std::vector<std::tuple<I1, I2, I3, I4, I5, I6>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		I1 i1 = std::get<0>(testCases[i]);
		I2 i2 = std::get<1>(testCases[i]);
		I3 i3 = std::get<2>(testCases[i]);
		I4 i4 = std::get<3>(testCases[i]);
		I5 i5 = std::get<4>(testCases[i]);
		I6 i6 = std::get<5>(testCases[i]);
		os << i1 << ' ' << i2 << ' ' << i3 << ' ' << i4 << ' ' << i5 << ' ' << i6 << ' ' << (*f)(i1, i2, i3, i4, i5, i6) << endl;
	}
	os << std::endl;
}

template <class O, class I1, class I2, class I3>
void createTests(const char* name, O(*f)(const I1 &, const I2 &, const I3 &), 
					const std::vector<std::tuple<I1, I2, I3>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		I1 i1 = std::get<0>(testCases[i]);
		I2 i2 = std::get<1>(testCases[i]);
		I3 i3 = std::get<2>(testCases[i]);
		os << i1 << ' ' << i2 << ' ' << i3 << ' ' << (*f)(i1, i2, i3) << endl;
	}
	os << std::endl;
}
template <class O, class I1, class I2, class I3, class I4, class I5>
void createTests(const char* name, O(*f)(const I1 &, const I2 &, I3, const I4 &, const I5 &), 
					const std::vector<std::tuple<I1, I2, I3, I4, I5>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		const I1 &i1 = std::get<0>(testCases[i]);
		const I2 &i2 = std::get<1>(testCases[i]);
		const I3 &i3 = std::get<2>(testCases[i]);
		const I4 &i4 = std::get<3>(testCases[i]);
		const I5 &i5 = std::get<4>(testCases[i]);
		os << i1 << ' ' << i2 << ' ' << i3 << ' ' << i4 << ' ' << i5 << ' ';
		os << (*f)(i1, i2, i3, i4, i5) << endl;
	}
	os << std::endl;
}
template <class O, class I1, class I2, class I3, class I4, class I5>
void createTests(const char* name, O(*f)(I1, I2, I3, I4, I5), 
					const std::vector<std::tuple<I1,I2,I3,I4,I5>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		const I1 &i1 = std::get<0>(testCases[i]);
		const I2 &i2 = std::get<1>(testCases[i]);
		const I3 &i3 = std::get<2>(testCases[i]);
		const I4 &i4 = std::get<3>(testCases[i]);
		const I5 &i5 = std::get<4>(testCases[i]);
		os << i1 << ' ' << i2 << ' ' << i3 << ' ' << i4 << ' ' << i5 << ' ';
		os << (*f)(i1, i2, i3, i4, i5) << endl;
	}
	os << std::endl;
}
template <class O, class I1>
void createTests(const char* name, O (*f)(const I1 &arg1), 
					const std::vector<I1>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		I1 i1 = testCases[i];
		os << i1 << ' ' << (*f)(i1) << endl;
	}
	os << std::endl;
}
template <class O, class I1, class I2>
void createTests(const char* name, O(*f)(const I1& arg1, const I2 &arg2), 
				const std::vector<std::tuple<I1, I2>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		std::tuple<I1,I2> args = testCases[i];
		const I1& a1 = std::get<0>(args);
		const I2& a2 = std::get<1>(args);
		os << a1 << ' ' << a2 << ' ' << (*f)(a1, a2) << endl;
	}
	os << std::endl;
}
void createTotalColorTests(const char* name, 
				const std::vector<std::tuple<Material, LightColor, dvec3, dvec3, dvec3, dvec3, bool, LightATParams>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		std::tuple<Material, LightColor, dvec3, dvec3, dvec3, dvec3, bool, LightATParams> args = testCases[i];
		Material mat = std::get<0>(args);
		const LightColor &lightColor = std::get<1>(args);
		const dvec3& v = std::get<2>(args);
		const dvec3& n = std::get<3>(args);
		const dvec3& lightPos = std::get<4>(args);
		const dvec3& intersectionPt = std::get<5>(args);
		bool attenOn = std::get<6>(args);
		const LightATParams &atParams = std::get<7>(args);
		os << mat << ' ' << lightColor
					<< v << ' ' << n << ' ' << lightPos << ' ' << intersectionPt
					<< attenOn << ' ' << atParams << ' '
					<< totalColor(mat, lightColor, v, n, lightPos, intersectionPt, attenOn, atParams) << endl;
	}
	os << std::endl;
}
void createIlluminateTests(const char* name, 
							const std::vector<std::tuple<Material, LightColor, dvec3, dvec3, dvec3, dvec3, bool, LightATParams, bool, bool, dvec3>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		std::tuple<Material, LightColor, dvec3, dvec3, dvec3, dvec3, bool, LightATParams, bool, bool, dvec3> args = testCases[i];
		Material mat = std::get<0>(args);
		LightColor lightColor = std::get<1>(args);
		dvec3 v = std::get<2>(args);
		dvec3 n = std::get<3>(args);
		dvec3 lightPos = std::get<4>(args);
		dvec3 intersectionPt = std::get<5>(args);
		bool attenIsOn = std::get<6>(args);
		LightATParams atParams = std::get<7>(args);
		bool isOn = std::get<8>(args);
		bool inShadow = std::get<9>(args);
		dvec3 eyePos = std::get<10>(args);
		os << mat << ' ' << lightColor
			<< ' ' << v << ' ' << n << ' ' << lightPos << ' ' << intersectionPt
			<< ' ' << attenIsOn << ' ' << atParams << ' ' << isOn << ' ' << inShadow << ' ' << eyePos;

		PositionalLight light(lightPos, lightColor);
		light.isOn = isOn;
		light.attenuationIsTurnedOn = attenIsOn;
		light.atParams = atParams;
		Frame cameraFrame;
		cameraFrame.setFrame(eyePos, X_AXIS, Y_AXIS, Z_AXIS);
		os << light.illuminate(intersectionPt, n, mat, cameraFrame, inShadow) << endl;
	}
	os << std::endl;
}

void createTestsArea(const char* name, const std::vector<std::tuple<dvec3, dvec3, dvec3>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		std::tuple<dvec3, dvec3, dvec3> args = testCases[i];
		const dvec3& p0 = std::get<0>(args);
		const dvec3& p1 = std::get<1>(args);
		const dvec3& p2 = std::get<2>(args);
		os << p0 << ' ' << p1 << ' ' << p2 << ' ' << areaOfTriangle(p0, p1, p2) << endl;
	}
	os << std::endl;
}

void createPCameraTests(const char *name, 
					const std::vector<std::tuple<dvec3, dvec3, double, int, int, glm::ivec2, glm::ivec2>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		std::tuple<dvec3, dvec3, double, int, int, glm::ivec2, glm::ivec2> args = testCases[i];

		dvec3 cameraPos = std::get<0>(args);
		dvec3 focusPt = std::get<1>(args);
		dvec3 up = Y_AXIS;
		double FOV = std::get<2>(args);
		int NX = std::get<3>(args);
		int NY = std::get<4>(args);
		glm::ivec2 pixel1 = std::get<5>(args);
		glm::ivec2 pixel2 = std::get<6>(args);

		os << cameraPos << ' ' << focusPt;
		os << ' ' << FOV << ' ' <<
					NX << ' ' << NY << ' ' <<
					pixel1 << ' ' << pixel2 << ' ';

		PerspectiveCamera pCamera(cameraPos, focusPt, dvec3(0, 1, 0), FOV, NX, NY);

	//	pCamera.calculateViewingParameters(NX, NY);

		os << pCamera.getFrame().origin
				<< ' ' << pCamera.getFrame().u
				<< ' ' << pCamera.getFrame().v
				<< ' ' << pCamera.getFrame().w
				<< ' ' << pCamera.getDistToPlane()
				<< ' ' << pCamera.getNX() << ' ' << pCamera.getNY()
				<< ' ' << pCamera.getLeft() << ' ' << pCamera.getRight()
				<< ' ' << pCamera.getBottom() << ' ' << pCamera.getTop() << ' ';

		Ray viewingRay1 = pCamera.getRay(pixel1.x, pixel1.y);
		Ray viewingRay2 = pCamera.getRay(pixel2.x, pixel2.y);

		os << viewingRay1.origin << ' ' << viewingRay1.dir << endl;
		os << viewingRay2.origin << ' ' << viewingRay2.dir << endl;
	}
	os << std::endl;
}
void createOCameraTests(const char* name, 
					const std::vector<std::tuple<dvec3, dvec3, double, int, int, glm::ivec2, glm::ivec2>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		std::tuple<dvec3, dvec3, double, int, int, glm::ivec2, glm::ivec2> args = testCases[i];

		dvec3 cameraPos = std::get<0>(args);
		dvec3 focusPt = std::get<1>(args);
		dvec3 up = Y_AXIS;
		double FOV = std::get<2>(args);
		int NX = std::get<3>(args);
		int NY = std::get<4>(args);
		glm::ivec2 pixel1 = std::get<5>(args);
		glm::ivec2 pixel2 = std::get<6>(args);

		os << cameraPos << ' ' << focusPt
				<< ' ' << FOV << ' ' << NX << ' ' << NY << ' '
				<< pixel1 << ' ' << pixel2 << ' ';

		OrthographicCamera oCamera(cameraPos, focusPt, dvec3(0, 1, 0), 1, NX, NY);

	///	oCamera.calculateViewingParameters(NX, NY);


		os << oCamera.getFrame().origin
			<< ' ' << oCamera.getFrame().u
			<< ' ' << oCamera.getFrame().v
			<< ' ' << oCamera.getFrame().w
			<< ' ' << oCamera.getNX() << ' ' << oCamera.getNY()
			<< ' ' << oCamera.getLeft() << ' ' << oCamera.getRight()
			<< ' ' << oCamera.getBottom() << ' ' << oCamera.getTop() << ' ';

		Ray viewingRay1 = oCamera.getRay(pixel1.x, pixel1.y);
		Ray viewingRay2 = oCamera.getRay(pixel2.x, pixel2.y);

		os << viewingRay1.origin << endl;
		os << viewingRay1.dir << endl;
		os << viewingRay2.origin << endl;
		os << viewingRay2.dir << endl;
	}
	os << std::endl;
}
template <class O, class I1, class I2, class I3, class I4>
void createTests(const char* name, 
				O(*f)(const I1& arg1, const I2& arg2, const I3& arg3, const I4& arg4), 
				const std::vector<std::tuple<I1, I2, I3, I4>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		std::tuple<I1, I2, I3, I4> args = testCases[i];
		const I1& a1 = std::get<0>(args);
		const I2& a2 = std::get<1>(args);
		const I3& a3 = std::get<2>(args);
		const I4& a4 = std::get<3>(args);
		os << a1 << ' ' << a2 << ' ' << a3 << ' ' << a4
			<< ' ' << (*f)(a1, a2, a3, a4) << endl;;
	}
	os << std::endl;
}
template <class O, class I1, class I2, class I3, class I4>
void createTests(const char* name, O(*f)(I1 arg1, I2 arg2, I3 arg3, I4 arg4), 
					const std::vector<std::tuple<I1, I2, I3, I4>>& testCases) {
	std::ostream& os = output;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	for (int i = 0; i < testCases.size(); i++) {
		std::tuple<I1, I2, I3, I4> args = testCases[i];
		const I1& a1 = std::get<0>(args);
		const I2& a2 = std::get<1>(args);
		const I3& a3 = std::get<2>(args);
		const I4& a4 = std::get<3>(args);
		os << a1 << ' ' << a2 << ' ' << a3 << ' ' << a4
			<< ' ' << (*f)(a1, a2, a3, a4) << endl;;
	}
	os << std::endl;
}

template <class I1>
void runTests(const char* N, void (*f)(I1 &, I1 &), double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;
				I1 arg1;
				I1 arg2;
				I1 arg1After;
				I1 arg2After;
				input >> arg1 >> arg2;
				os << "(" << arg1 << " " << arg2 << ")";
				input >> arg1After >> arg2After;
				getLine(input);
				try {
					(*f)(arg1, arg2);
					if (equal(arg1, arg1After) && equal(arg2, arg2After)) {
						os << " Correct" << endl;
						corr++;
					} else {
						os << " Inorrect" << endl;
					}
				}
				catch (...) {
					exceptions++;
					os << " Correct (exception thrown)" << endl;
				}
			}
		}
	}
}
template <class O, class I1>
void runTests(const char* N, O (*f)(I1 arg1), double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;
				I1 i1;
				O expectedOutput;
				input >> i1 >> expectedOutput;
				getLine(input);

				os << "(" << i1 << ") --> " << expectedOutput << " ";
				try {
					O actualOutput = (*f)(i1);
					if (equal(expectedOutput, actualOutput)) {
						corr++;
						os << " Correct" << endl;
					} else {
						os << " Incorrect ( returned: " << actualOutput << " )"<< endl;
					}
				}
				catch (...) {
					os << " Incorrect (exception thrown)" << endl;
					exceptions++;
				}
			}
		}
	}
}
template <class O, class I1>
void runTests2(const char* N, O(*f)(const I1 &arg1), double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;
				I1 i1;
				O expectedOutput;
				input >> i1 >> expectedOutput;
				getLine(input);

				os << "(" << i1 << ") --> " << expectedOutput << " ";
				try {
					O actualOutput = (*f)(i1);
					if (equal(expectedOutput, actualOutput)) {
						corr++;
						os << " Correct" << endl;
					} else {
						os << " Incorrect ( returned: " << actualOutput << " )" << endl;
					}
				}
				catch (...) {
					os << " Incorrect (exception thrown)" << endl;
					exceptions++;
				}
			}
		}
	}
}

template <class O, class I1, class I2>
void runTests(const char* N, O(*f)(I1 arg1, I2 arg2), double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;
				I1 i1;
				I2 i2;
				O expectedOutput;
				input >> i1 >> i2 >> expectedOutput;
				getLine(input);

				os << "(" << i1 << "," << i2 << ") --> " << expectedOutput << " ";
				try {
					O actualOutput = (*f)(i1, i2);
					if (equal(expectedOutput, actualOutput)) {
						corr++;
						os << " Correct" << endl;
					} else {
						os << " Incorrect ( returned: " << actualOutput << " )" << endl;
					}
				}
				catch (...) {
					os << " Incorrect (exception thrown)" << endl;
					exceptions++;
				}
			}
		}
	}
}
template <class O, class I1>
void runTests(const char* N, O (*f)(const I1 &arg1), double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					//input.close();
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;
				I1 i1;
				O expectedOutput;
				input >> i1 >> expectedOutput;
				getLine(input);
				os << "(" << i1 << ") -->" << expectedOutput;
				try {
					O actualOutput = (*f)(i1);
					if (equal(expectedOutput, actualOutput)) {
						corr++;
						os << " Correct" << endl;
					} else {
						os << " Incorrect ( returned: " << actualOutput << ")" << endl;
					}
				}
				catch (...) {
					os << " Incorrect (exception thrown)" << endl;
					exceptions++;
				}
			}
		}
	}
}
template <class O, class I1>
void runTestsVec(const char* N, O(*f)(const vector<I1>& arg1), double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
			//	line = getLine(input);
				if (input.eof() || input.peek() == '\n') {
					//input.close();
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;

				vector<I1> i1;
				O expectedOutput;
				input >> i1 >> expectedOutput;
				getLine(input);
				os << "(" << i1 << ") -->" << expectedOutput;
				//str >> i1 >> expectedOutput;
				try {
					O actualOutput = (*f)(i1);
					if (equal(expectedOutput, actualOutput)) {
						corr++;
						os << " Correct" << endl;
					} else {
						os << " Incorrect ( returned: " << actualOutput << ")" << endl;
					}
				}
				catch (...) {
					os << " Incorrect (exception thrown)" << endl;
					exceptions++;
				}
			}
		}
	}
}
template <class O, class I1, class I2>
void runTests(const char* N, O (*f)(const I1& arg1, const I2& arg2), double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					//input.close();
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;
				I1 i1;
				I2 i2;
				O expectedOutput;
				input >> i1 >> i2 >> expectedOutput;
				getLine(input);
				os << "(" << i1 << " " << i2 << ") --> " << expectedOutput;
				try {
					O actualOutput = (*f)(i1, i2);
					if (equal(expectedOutput, actualOutput)) {
						corr++;
						os << " Correct" << endl;
					} else {
						os << " Incorrect ( returned: " << actualOutput << ")" << endl;
					}
				}
				catch (...) {
					os << " Incorrect ( exception thrown )" << endl;
					exceptions++;
				}
			}
		}
	}
}
template <class O, class I1, class I2>
void runTests(const char* N, O(*f)(const I1& arg1, I2 arg2), double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					//input.close();
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;
				I1 i1;
				I2 i2;
				O expectedOutput;
				input >> i1 >> i2 >> expectedOutput;
				getLine(input);
				os << "(" << i1 << " " << i2 << ") --> " << expectedOutput;
				try {
					O actualOutput = (*f)(i1, i2);
					if (equal(expectedOutput, actualOutput)) {
						corr++;
						os << " Correct" << endl;
					} else {
						os << " Incorrect ( returned: " << actualOutput << ")" << endl;
					}
				}
				catch (...) {
					os << " Incorrect ( exception thrown )" << endl;
					exceptions++;
				}
			}
		}
	}
}
template <class O, class I1, class I2, class I3>
void runTests(const char* N, O (*f)(I1 arg1, I2 arg2, I3 arg3), double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;
				I1 i1;
				I2 i2;
				I3 i3;
				O expectedOutput;
				input >> i1 >> i2 >> i3 >> expectedOutput;
				getLine(input);

				os << "(" << i1 << " " << i2 << " " << i3 << ") --> " << expectedOutput;
				try {
					O actualOutput = (*f)(i1, i2, i3);
					if (equal(expectedOutput, actualOutput)) {
						corr++;
						os << " Correct" << endl;
					} else {
						os << " Incorrect ( returned: " << actualOutput << ")" << endl;
					}
				}
				catch (...) {
					os << " Incorrect (exception thrown)" << endl;
					exceptions++;
				}
			}
		}
	}
}
void runAreaTests(const char* N, double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;
				double i1;
				double  i2;
				double  i3;
				double  expectedOutput;
				input >> i1 >> i2 >> i3 >> expectedOutput;
				getLine(input);

				os << "(" << i1 << " " << i2 << " " << i3 << ") --> " << expectedOutput;
				try {
					double  actualOutput = areaOfTriangle(i1, i2, i3);
					if (equal(expectedOutput, actualOutput)) {
						corr++;
						os << " Correct" << endl;
					} else {
						os << " Incorrect ( returned: " << actualOutput << ")" << endl;
					}
				}
				catch (...) {
					os << " Incorrect (exception thrown)" << endl;
					exceptions++;
				}
			}
		}
	}
}
template <class O, class I1, class I2, class I3, class I4, class I5, class I6>
void runTests(const char* N, O(*f)(I1 arg1, I2 arg2, I3 arg3, I4 arg4, I5 arg5, I6 arg6), double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;
				I1 i1;
				I2 i2;
				I3 i3;
				I4 i4;
				I5 i5;
				I6 i6;
				O expectedOutput;
				input >> i1 >> i2 >> i3 >> i4 >> i5 >> i6 >> expectedOutput;
				getLine(input);

				os << "(" << i1 << " " << i2 << " " << i3 << " " << i4 << " " << i5 << " " << i6 << ") --> " << expectedOutput;
				try {
					O actualOutput = (*f)(i1, i2, i3, i4, i5, i6);
					if (equal(expectedOutput, actualOutput)) {
						corr++;
						os << " Correct" << endl;
					} else {
						os << " Incorrect ( returned: " << actualOutput << ")" << endl;
					}
				}
				catch (...) {
					os << " Incorrect (exception thrown)" << endl;
					exceptions++;
				}
			}
		}
	}
}
template <class I1>
void runTests(const char* N, void (*f)(I1 arg1, I1 arg2, I1 arg3), double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;
				I1 arg1;
				I1 arg2;
				I1 arg3;
				input >> arg1 >> arg2 >> arg3;
				I1 arg1After;
				I1 arg2After;
				I1 arg3After;
				input >> arg1After >> arg2After >> arg3After;
				getLine(input);

				os << "(" << arg1 << " " << arg2 << " " << arg3 << ") --> void ";
				try {
					(*f)(arg1, arg2, arg3);
					if (equal(arg2, arg2After) && equal(arg3, arg3After)) {
						os << " Correct" << endl;
					} else {
						os << " Incorrect" << endl;
					}
				}
				catch (...) {
					os << " Incorrect (exception thrown)" << endl;
					exceptions++;
				}
			}
		}
	}
}
template <class I1>
void runTests(const char* N, void (*f)(I1, I1&, I1&), double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	int total = 0;
	int corr = 0;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;
				I1 arg1;
				I1 arg2;
				I1 arg3;
				input >> arg1 >> arg2 >> arg3;
				I1 arg1After;
				I1 arg2After;
				I1 arg3After;
				input >> arg1After >> arg2After >> arg3After;
				getLine(input);

				os << "(" << arg1 << " ...) --> void";
				try {
					(*f)(arg1, arg2, arg3);
					if (equal(arg2, arg2After) && equal(arg3, arg3After)) {
						corr++;
						os << " Correct" << endl;
					}
					else {
						os << " Incorrect" << endl;
					}
				}
				catch (...) {
					os << " Incorrect (exception thrown)" << endl;
					exceptions++;
				}
			}
		}
	}
}
template <class O, class I1, class I2, class I3>
void runTests(const char* N, O (*f)(const I1& arg1, const I2& arg2, const I3& arg3), double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;
				I1 i1;
				I2 i2;
				I3 i3;
				O expectedOutput;
				input >> i1 >> i2 >> i3 >> expectedOutput;
				getLine(input);

				os << "(" << i1 << " " << i2 << " " << i3 << ") --> " << expectedOutput;
				try {
					O actualOutput = (*f)(i1, i2, i3);
					if (equal(expectedOutput, actualOutput)) {
						corr++;
						os << " Correct" << endl;
					} else {
						os << " Incorrect ( returned: " << actualOutput << ")" << endl;
					}
				}
				catch (...) {
					os << " Incorrect (exception thrown)" << endl;
					exceptions++;
				}
			}
		}
	}
}
template <class O, class I1, class I2, class I3>
void runTests(const char* N, O(*f)(const I1& arg1, I2 arg2, I3 arg3), double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;

				I1 i1;
				I2 i2;
				I3 i3;
				O expectedOutput;
				input >> i1 >> i2 >> i3 >> expectedOutput;
				getLine(input);

				os << "(" << i1 << " " << i2 << " " << i3 << ") --> " << expectedOutput;
				try {
					O actualOutput = (*f)(i1, i2, i3);
					if (equal(expectedOutput, actualOutput)) {
						corr++;
						os << " Correct" << endl;
					}
					else {
						os << " Incorrect ( returned: " << actualOutput << ")" << endl;
					}
				}
				catch (...) {
					os << " Incorrect (exception thrown)" << endl;
					exceptions++;
				}
			}
		}
	}
}
template <class O, class I1, class I2, class I3, class I4>
void runTests(const char* N, O(*f)(const I1& arg1, const I2& arg2, const I3& arg3, const I4 &arg4), 
				double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;
				I1 i1;
				I2 i2;
				I3 i3;
				I4 i4;
				O expectedOutput;
				input >> i1 >> i2 >> i3 >> i4 >> expectedOutput;
				getLine(input);

				os << "(" << i1 << " " << i2 << " " << i3 << " " << i4 << ") --> " << expectedOutput;
				try {
					O actualOutput = (*f)(i1, i2, i3, i4);
					if (equal(expectedOutput, actualOutput)) {
						corr++;
						os << " Correct" << endl;
					} else {
						os << " Incorrect ( returned: " << actualOutput << ")" << endl;
					}
				}
				catch (...) {
					os << " Incorrect (exception thrown)" << endl;
					exceptions++;
				}
			}
		}
	}
}
template <class O, class I1, class I2, class I3, class I4>
void runTests(const char* N, O(*f)(I1 arg1, I2 arg2, I3 arg3, I4 arg4), double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;
				I1 i1;
				I2 i2;
				I3 i3;
				I4 i4;
				O expectedOutput;
				input >> i1 >> i2 >> i3 >> i4 >> expectedOutput;
				getLine(input);

				os << "(" << i1 << " " << i2 << " " << i3 << " " << i4 << ") --> " << expectedOutput;
				try {
					O actualOutput = (*f)(i1, i2, i3, i4);
					if (equal(expectedOutput, actualOutput)) {
						corr++;
						os << " Correct" << endl;
					}
					else {
						os << " Incorrect ( returned: " << actualOutput << ")" << endl;
					}
				}
				catch (...) {
					os << " Incorrect (exception thrown)" << endl;
					exceptions++;
				}
			}
		}
	}
}
template <class O, class I1, class I2, class I3, class I4, class I5>
void runTests(const char* N, 
				O(*f)(const I1 &arg1, const I2 &arg2, I3 arg3, const I4 &arg4, const I5 &arg5), 
				double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;
				I1 i1;
				I2 i2;
				I3 i3;
				I4 i4;
				I5 i5;
				O expectedOutput;
				input >> i1 >> i2 >> i3 >> i4 >> i5 >> expectedOutput;
				getLine(input);

				os << "(" << i1 << " " << i2 << " " << i3 << " " << i4 << " " << i5 << ") --> " << expectedOutput;
				try {
					O actualOutput = (*f)(i1, i2, i3, i4, i5);
					if (equal(expectedOutput, actualOutput)) {
						corr++;
						os << " Correct" << endl;
					} else {
						os << " Incorrect ( returned: " << actualOutput << ")" << endl;
					}
				}
				catch (...) {
					os << " Incorrect (exception thrown)" << endl;
					exceptions++;
				}
			}
		}
	}
}
template <class O, class I1, class I2, class I3, class I4, class I5>
void runTests(const char* N, O(*f)(I1 arg1, I2 arg2, I3 arg3, I4 arg4, I5 arg5), double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;

				I1 i1;
				I2 i2;
				I3 i3;
				I4 i4;
				I5 i5;
				O expectedOutput;
				input >> i1 >> i2 >> i3 >> i4 >> i5 >> expectedOutput;
				getLine(input);

				os << "(" << i1 << " " << i2 << " " << i3 << " " << i4 << " " << i5 << ") --> " << expectedOutput;
				try {
					O actualOutput = (*f)(i1, i2, i3, i4, i5);
					if (equal(expectedOutput, actualOutput)) {
						corr++;
						os << " Correct" << endl;
					} else {
						os << " Incorrect ( returned: " << actualOutput << ")" << endl;
					}
				}
				catch (...) {
					os << " Incorrect (exception thrown)" << endl;
					exceptions++;
				}
			}
		}
	}
}
//template <class I1>
//void runTests(const char* N, void (*f)(I1 arg1, I1 arg2, I1 arg3, I1 arg4, I1 arg5, I1 &arg6), double maxPts) {
//	std::string name(N);
//	std::ostream& os = std::cout;
//
//	istream* inputSS = openFile(TestFile);
//	istream& input = *inputSS;
//
//	os << std::fixed << std::setprecision(20);
//	os << name << std::endl;
//
//	while (!input.eof()) {
//		std::string line = getLine(input);
//		if (line == name) {
//			int total = 0;
//			int corr = 0;
//			int exceptions = 0;
//			while (true) {
//				if (input.eof() || input.peek() == '\n') {
//					displayPoints(os, corr, total, maxPts);
//					return;
//				}
//				total++;
//				I1 i1;
//				I1 i2;
//				I1 i3;
//				I1 i4;
//				I1 i5;
//				I1 i6;
//				I1 arg6After;
//				input >> i1 >> i2 >> i3 >> i4 >> i5 >> i6;
//				getLine(input);
//
//				os << "(" << i1 << " " << i2 << " " << i3 << " " << i4 << " " << i5 << " " << i6 << ") --> void";
//				try {
//					(*f)(i1, i2, i3, i4, i5, i6);
//					if (equal(i6, arg6After)) {
//						corr++;
//						os << " Correct" << endl;
//					} else {
//						os << " Incorrect" << endl;
//					}
//				}
//				catch (...) {
//					os << " Incorrect (exception thrown)" << endl;
//					exceptions++;
//				}
//			}
//		}
//	}
//}
template <class I1>
void runTestsExplicit2(const char* N, I1 (*f)(const I1& arg1), double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;

				I1 i1;
				I1 expectedOutput;
				input >> i1;
				input >> expectedOutput;
				getLine(input);

				os << "(" << i1 << ") -->" << expectedOutput;
				try {
					I1 actualOutput = (*f)(i1);
					if (equal(expectedOutput, actualOutput)) {
						corr++;
						os << " Correct" << endl;
					}
					else {
						os << " Incorrect ( returned: " << actualOutput << ")" << endl;
					}
				}
				catch (...) {
					os << " Incorrect (exception thrown)" << endl;
					exceptions++;
				}
			}
		}
	}
}
template <class O, class I1>
void runTestsExplicit2(const char* N, O(*f)(const I1& arg1), double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;

				I1 i1;
				O expectedOutput;
				input >> i1;
				input >> expectedOutput;
				getLine(input);

				os << "(" << i1 << ") -->" << expectedOutput;
				try {
					O actualOutput = (*f)(i1);
					if (equal(expectedOutput, actualOutput)) {
						corr++;
						os << " Correct" << endl;
					}
					else {
						os << " Incorrect ( returned: " << actualOutput << ")" << endl;
					}
				}
				catch (...) {
					os << " Incorrect (exception thrown)" << endl;
					exceptions++;
				}
			}
		}
	}
}
template <class O, class I1>
void runTestsExplicit(const char* N, O(*f)(I1, I1, I1, I1, I1), double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;
				I1 i1;
				I1 i2;
				I1 i3;
				I1 i4;
				I1 i5;
				O expectedOutput;
				input >> i1;
				input >> i2;
				input >> i3;
				input >> i4;
				input >> i5;
				input >> expectedOutput;
				getLine(input);

				os << "(" << i1 << " " << i2 << " " << i3 << " " << i4 << " " << i5 << ") --> " << expectedOutput;
				try {
					O actualOutput = (*f)(i1, i2, i3, i4, i5);
					if (equal(expectedOutput, actualOutput)) {
						corr++;
						os << " Correct" << endl;
					}
					else {
						os << " Incorrect ( returned: " << actualOutput << ")" << endl;
					}
				}
				catch (...) {
					os << " Incorrect (exception thrown)" << endl;
					exceptions++;
				}
			}
		}
	}
}
template <class I1>
void runTestsExplicit5(const char* N, void (*f)(I1, I1, I1, I1, I1, I1 &), double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream *inputSS = openFile(TestFile);
	istream& input = *inputSS;
	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;
				I1 i1;
				I1 i2;
				I1 i3;
				I1 i4;
				I1 i5;
				I1 i6;
				input >> i1 >> i2 >> i3 >> i4 >> i5 >> i6;
				I1 i1After;
				I1 i2After;
				I1 i3After;
				I1 i4After;
				I1 i5After;
				I1 i6After;
				input >> i1After >> i2After >> i3After >> i4After >> i5After >> i6After;
				getLine(input);

				os << "(" << i1 << " " << i2 << " " << i3 << " " << i4 << " " << i5 << " " << i6 << ") --> void";
				try {
					(*f)(i1, i2, i3, i4, i5, i6);
					if (equal(i6, i6After)) {
						corr++;
						os << " Correct" << endl;
					}
					else {
						os << " Incorrect" << endl;
					}
				}
				catch (...) {
					os << " Incorrect" << endl;
					exceptions++;
				}
			}
		}
	}
}
void runTestsQuadraticArray(const char* N, double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;

				double i1;
				double i2;
				double i3;
				int expectedOutput;
				double expectedRoots[2] = { 0,0 };
				input >> i1 >> i2 >> i3;
				input >> expectedOutput >> expectedRoots[0] >> expectedRoots[1];
				getLine(input);

				double actualRoots[2];
				os << "(" << i1 << " " << i2 << " " << i3 << " rootsArray " << ") --> " << expectedOutput;
				try {
					int n = quadratic(i1, i2, i3, actualRoots);
					bool correct = (n == expectedOutput);
					if (correct) {
						if (n == 0) {
							;
						} else if (n == 1) {
							correct = equal(actualRoots[0], expectedRoots[0]);
						} else {
							correct = equal(actualRoots[0], expectedRoots[0]) &&
										equal(actualRoots[1], expectedRoots[1]);
						}
					}
					if (correct) {
						corr++;
						os << " Correct" << endl;
					} else {
						os << " Incorrect (details below)" << endl;
						os << "   Expected number of roots: " << expectedOutput << " :";
						for (int i = 0; i < expectedOutput; i++)
							os << expectedRoots[i] << ' ';
						os << endl;
						os << "   Your code produced: " << n << " ";
						for (int i = 0; i < n; i++)
							os << actualRoots[i] << ' ';
						os << endl;
					}
				}
				catch (...) {
					os << " Incorrect" << endl;
					exceptions++;
				}
			}
		}
	}
}
void runTestsQuadraticVector(const char* N, double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;
				double i1;
				double i2;
				double i3;
				vector<double> expectedRoots;
				input >> i1 >> i2 >> i3 >> expectedRoots;
				getLine(input);

				vector<double> actualRoots;
				os << "(" << i1 << " " << i2 << " " << i3 << ") --> " << expectedRoots;
				try {
					vector<double> actualRoots = quadratic(i1, i2, i3);
					if (ave(actualRoots, expectedRoots)) {
						corr++;
						os << " Correct" << endl << endl;
					}
					else {
						os << " Incorrect " << endl;
						os << "   Your code produced: " << actualRoots << endl << endl;
					}
				}
				catch (...) {
					os << " Incorrect" << endl;
					exceptions++;
				}
			}
		}
	}
}
void runOCameraTests(const char* N, double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	os << "====================" << endl;
	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corrFrame = 0;
			int corrParams = 0;
			int corrViewingRays = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					double each = maxPts / 3.0;
					os << "THIS TEST'S POINT TOTALS: " << endl;
					os << "Frame points: " << endl;
					displayPoints(os, corrFrame, total, each);
					os << "Camera parameter points: " << endl;
					displayPoints(os, corrParams, total, each);
					os << "Viewing rays points: " << endl;
					displayPoints(os, corrViewingRays, total, each);
					return;
				}
				total++;
				dvec3 cameraPos;
				dvec3 focusPt;
				dvec3 up = Y_AXIS;
				double FOV;
				int NX, NY;
				glm::ivec2 pixel1;
				glm::ivec2 pixel2;

				input >> cameraPos >> focusPt >> FOV >> NX >> NY >> pixel1 >> pixel2;
				os << endl;
				os << "Inputs: " << endl;
				os << "Camera pos: " << cameraPos << endl;
				os << "Camera focus pt: " << focusPt << endl;
				os << "FOV: " << FOV << endl;
				os << "NX/NY: " << NX << '/' << NY << endl;
				os << "Pixel #1: " << pixel1 << endl;
				os << "Pixel #2: " << pixel2 << endl;
				os << endl;

				dvec3 cameraOrigin;
				dvec3 u, v, w;
				double nx, ny;
				double left, right, bottom, top;
				dvec2 pp1, pp2;
				dvec3 vr1Origin, vr1Dir;
				dvec3 vr2Origin, vr2Dir;
				input >> cameraOrigin >> u >> v >> w >> nx >> ny
						>> left >> right >> bottom >> top 
						>> vr1Origin >> vr1Dir
						>> vr2Origin >> vr2Dir;

				Ray viewingRay1(vr1Origin, vr1Dir);
				Ray viewingRay2(vr2Origin, vr2Dir);

				OrthographicCamera oCamera(cameraPos, focusPt, dvec3(0, 1, 0), 1, NX, NY);

			///	oCamera.calculateViewingParameters((int)NX, (int)NY);
				bool originOK = ave(oCamera.getFrame().origin, cameraOrigin);
				bool uOK = ave(oCamera.getFrame().u, u);
				bool vOK = ave(oCamera.getFrame().v, v);
				bool wOK = ave(oCamera.getFrame().w, w);
				bool nxOK = ae(oCamera.getNX(), nx);
				bool nyOK = ae(oCamera.getNY(), ny);
				bool leftOK = ae(oCamera.getLeft(), left);
				bool rightOK = ae(oCamera.getRight(), right);
				bool bottomOK = ae(oCamera.getBottom(), bottom);
				bool topOK = ae(oCamera.getTop(), top);

				Ray R1 = oCamera.getRay(pixel1.x, pixel1.y);
				Ray R2 = oCamera.getRay(pixel2.x, pixel2.y);
				bool vr1OriginOK = ave(R1.origin, vr1Origin);
				bool vr1DirOK = ave(R1.dir, vr1Dir);
				bool vr2OriginOK = ave(R2.origin, vr2Origin);
				bool vr2DirOK = ave(R2.dir, vr2Dir);
				if (originOK && uOK && vOK && wOK) {
					corrFrame++;
					os << " Camera Frame CORRECT" << endl;
					os << " Origin: " << cameraOrigin << endl;
					os << " U: " << u << endl;
					os << " V: " << v << endl;
					os << " W: " << w << endl;
					os << endl;
				} else {
					os << " Camera Frame INCORRECT (details below)" << endl;
					os << " Should be: " << endl;
					os << " Origin: " << cameraOrigin << endl;
					os << " U: " << u << endl;
					os << " V: " << v << endl;
					os << " W: " << w << endl;
					os << " Output produced: " << endl;
					os << " Origin: " << oCamera.getFrame().origin << endl;
					os << " U: " << oCamera.getFrame().u << endl;
					os << " V: " << oCamera.getFrame().v << endl;
					os << " W: " << oCamera.getFrame().w << endl;
					os << endl;
				}
				if (/*distOK &&*/leftOK && rightOK && bottomOK && topOK && nxOK && nyOK) {
					corrParams++;
					os << " l/r/b/t/nx/ny CORRECT" << endl;
					os << "  " << left << ' ' << right << ' ' << bottom << ' ' << top << ' ' << nx << ' ' << ny << endl; // << ' ' << distToPlane << endl;
					os << endl;
				} else {
					os << " l/r/b/t/nx/ny INCORRECT (details below)" << endl;
					os << "       Should be: " << left << ' ' << right << ' ' << bottom << ' ' << top << ' ' << nx << ' ' << ny << ' ' << oCamera << endl;
					os << " Output produced: " << oCamera.getLeft() 
												<< ' ' << oCamera.getRight() 
												<< ' ' << oCamera.getBottom()
												<< ' ' << oCamera.getTop() 
												<< ' ' << oCamera.getNX() 
												<< ' ' << oCamera.getNY() << endl; // ' ' << pCamera.distToPlane << endl;
					os << endl;
				}

				if (vr1OriginOK && vr1DirOK && vr2OriginOK && vr2DirOK) {
					corrViewingRays++;
					os << " Viewing Rays CORRECT" << endl;
					os << " Ray 1" << R1.origin << ' ' << R1.dir << endl;
					os << " Ray 2" << R2.origin << ' ' << R2.dir << endl;
					os << endl;
				}
				else {
					os << " Viewing Rays INCORRECT (details below)" << endl;
					os << " Should be: " << endl;
					os << " Ray 1" << vr1Origin << ' ' << vr1Dir << endl;
					os << " Ray 2" << vr2Origin << ' ' << vr2Dir << endl;
					os << " Output produced: " << endl;
					os << " Ray 1" << R1.origin << ' ' << R1.dir << endl;
					os << " Ray 2" << R2.origin << ' ' << R2.dir << endl;
					os << endl;
				}
			}
		}
	}
}
void runPCameraTests(const char* N, double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	os << "====================" << endl;
	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corrFrame = 0;
			int corrParams = 0;
			int corrViewingRays = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					double each = maxPts / 3.0;
					os << "THIS TEST'S POINT TOTALS: " << endl;
					os << "Frame points: " << endl;
					displayPoints(os, corrFrame, total, each);
					os << "Camera parameter points: " << endl;
					displayPoints(os, corrParams, total, each);
					os << "Viewing rays points: " << endl;
					displayPoints(os, corrViewingRays, total, each);
					return;
				}
				total++;

				dvec3 cameraPos;
				dvec3 focusPt;
				dvec3 up = Y_AXIS;
				double FOV;
				int NX, NY;

				glm::ivec2 pixel1, pixel2;

				input >> cameraPos >> focusPt >> FOV >> NX >> NY >> pixel1 >> pixel2;
				os << endl;
				os << "Inputs: " << endl;
				os << "Camera pos: " << cameraPos << endl;
				os << "Camera focus pt: " << focusPt << endl;
				os << "FOV: " << FOV << endl;
				os << "NX/NY: " << NX << '/' << NY << endl;
				os << "Pixel #1: " << pixel1 << endl;
				os << "Pixel #2: " << pixel2 << endl;
				os << endl;

				dvec3 cameraOrigin;
				dvec3 u, v, w;
				double distToPlane;
				double nx, ny;
				double left, right, bottom, top;
				dvec2 pp1, pp2;
				dvec3 vr1Origin, vr1Dir;
				dvec3 vr2Origin, vr2Dir;
				input >> cameraOrigin >> u >> v >> w
					>> distToPlane >> nx >> ny
					>> left >> right >> bottom >> top
	//				>> pp1 >> pp2
					>> vr1Origin >> vr1Dir
					>> vr2Origin >> vr2Dir;

				Ray viewingRay1(vr1Origin, vr1Dir);
				Ray viewingRay2(vr2Origin, vr2Dir);

				PerspectiveCamera pCamera(cameraPos, focusPt, dvec3(0, 1, 0), FOV, NX, NY);

			///	pCamera.calculateViewingParameters((int)NX, (int)NY);
				bool originOK = ave(pCamera.getFrame().origin, cameraOrigin);
				bool uOK = ave(pCamera.getFrame().u, u);
				bool vOK = ave(pCamera.getFrame().v, v);
				bool wOK = ave(pCamera.getFrame().w, w);
				bool nxOK = ae(pCamera.getNX(), nx);
				bool nyOK = ae(pCamera.getNY(), ny);
				bool leftOK = ae(pCamera.getLeft(), left);
				bool rightOK = ae(pCamera.getRight(), right);
				bool bottomOK = ae(pCamera.getBottom(), bottom);
				bool topOK = ae(pCamera.getTop(), top);
				bool distOK = ae(pCamera.getDistToPlane(), distToPlane);

				Ray R1 = pCamera.getRay(pixel1.x, pixel1.y);
				Ray R2 = pCamera.getRay(pixel2.x, pixel2.y);
				bool vr1OriginOK = ave(R1.origin, vr1Origin);
				bool vr1DirOK = ave(R1.dir, vr1Dir);
				bool vr2OriginOK = ave(R2.origin, vr2Origin);
				bool vr2DirOK = ave(R2.dir, vr2Dir);
				if (originOK && uOK && vOK && wOK) {
					corrFrame++;
					os << " Camera Frame CORRECT" << endl;
					os << " Origin: " << cameraOrigin << endl;
					os << " U: " << u << endl;
					os << " V: " << v << endl;
					os << " W: " << w << endl;
					os << endl;
				} else {
					os << " Camera Frame INCORRECT (details below)" << endl;
					os << " Should be: " << endl;
					os << " Origin: " << cameraOrigin << endl;
					os << " U: " << u << endl;
					os << " V: " << v << endl;
					os << " W: " << w << endl;
					os << " Output produced: " << endl;
					os << " Origin: " << pCamera.getFrame().origin << endl;
					os << " U: " << pCamera.getFrame().u << endl;
					os << " V: " << pCamera.getFrame().v << endl;
					os << " W: " << pCamera.getFrame().w << endl;
					os << endl;
				}
				if (distOK && leftOK && rightOK && bottomOK && topOK && nxOK && nyOK) {
					corrParams++;
					os << " l/r/b/t/nx/ny/distanceToPlane CORRECT" << endl;
					os << "  " << left << ' ' << right << ' ' << bottom << ' ' << top << ' ' << nx << ' ' << ny << ' ' << distToPlane << endl;
					os << endl;
				} else {
					os << " l/r/b/t/nx/ny/distanceToPlane INCORRECT (details below)" << endl;
					os << "       Should be: " << left << ' ' << right << ' ' << bottom << ' ' << top << ' ' << nx << ' ' << ny << ' ' << distToPlane << endl;
					os << " Output produced: " << pCamera.getLeft()
											<< ' ' << pCamera.getRight() << ' '
											<< pCamera.getBottom() << ' '
											<< pCamera.getTop() << ' '
											<< pCamera.getNX() << ' '
											<< pCamera.getNY() << ' '
											<< pCamera.getDistToPlane() << endl;
					os << endl;
				}
				if (vr1OriginOK && vr1DirOK && vr2OriginOK && vr2DirOK) {
					corrViewingRays++;
					os << " Viewing Rays CORRECT" << endl;
					os << " Ray 1" << R1.origin << ' ' << R1.dir << endl;
					os << " Ray 2" << R2.origin << ' ' << R2.dir << endl;
					os << endl;
				} else {
					os << " Viewing Rays INCORRECT (details below)" << endl;
					os << " Should be: " << endl;
					os << " Ray 1" << vr1Origin << ' ' << vr1Dir << endl;
					os << " Ray 2" << vr2Origin << ' ' << vr2Dir << endl;
					os << " Output produced: " << endl;
					os << " Ray 1" << R1.origin << ' ' << R1.dir << endl;
					os << " Ray 2" << R2.origin << ' ' << R2.dir << endl;
					os << endl;
				}
			}
		}
	}
}
void runTotalColorTests(const char* N, double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	os << "====================" << endl;
	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;

				Material mat;
				LightColor lightColor = pureWhiteLight;
				dvec3 v, n;
				dvec3 lightPos, intersectPt;
				bool atIsOn;
				LightATParams at(1, 0, 0);
				color targetColor;
				input >> mat;
				input >> lightColor;
				input >> v >> n >> lightPos >> intersectPt;
				input >> atIsOn;
				input >> at;
				input >> targetColor;
				getLine(input);

				os << endl;
				os << "Inputs: " << endl;
				os << "Material: " << mat << endl;
				os << "LightColor: " << lightColor << endl;
				os << "v: " << v << endl;
				os << "n: " << n << endl;
				os << "Light Pos: " << lightPos << endl;
				os << "Intersection Point: " << intersectPt << endl;
				os << "Attenuation is on: " << atIsOn << endl;
				os << "Attenuation parameters: " << at << endl;
				os << endl;

 				color C = totalColor(mat, lightColor, v, n, lightPos, intersectPt, atIsOn, at);
				if (ave(targetColor, C)) {
					corr++;
					os << "CORRECT" << targetColor << endl << endl;
				} else {
					os << "INCORRECT (details below)" << endl;
					os << " Should be: " << targetColor << endl;
					os << " Output produced: " << C << endl;
					os << endl;
				}
			}
		}
	}
}
void runIlluminateColorTests(const char* N, double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;
	os << "====================" << endl;
	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
			//	line = getLine(input);
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;
				//std::stringstream str(line);
				//putback(input, line);

				Material mat;
				LightColor lightColor = pureWhiteLight;
				dvec3 v, n;
				dvec3 lightPos, intersectPt, eyePos;
				bool atIsOn, isOn, inShadow;
				LightATParams at(1, 0, 0);
				color targetColor;
				input >> mat >> lightColor
					>> v >> n >> lightPos >> intersectPt
					>> atIsOn >> at >> isOn >> inShadow >> eyePos
					>> targetColor;
				getLine(input);

				os << endl;
				os << "Inputs: " << endl;
				os << "Material: " << mat << endl;
				os << "LightColor: " << lightColor << endl;
				os << "v: " << v << endl;
				os << "n: " << n << endl;
				os << "Light Pos: " << lightPos << endl;
				os << "Intersection Point: " << intersectPt << endl;
				os << "Attenuation is on: " << atIsOn << endl;
				os << "Attenuation parameters: " << at << endl;
				os << "Light is on: " << isOn << endl;
				os << "In Shadow: " << inShadow << endl;
				os << "Eye position: " << eyePos << endl;
				os << endl;

				PositionalLight light(lightPos, lightColor);
				light.isOn = isOn;
				light.attenuationIsTurnedOn = atIsOn;
				light.atParams = at;
				Frame cameraFrame;
				cameraFrame.setFrame(eyePos, X_AXIS, Y_AXIS, Z_AXIS);
				color outputColor = light.illuminate(intersectPt, n, mat, cameraFrame, inShadow);
				if (ave(targetColor, outputColor)) {
					corr++;
					os << "CORRECT" << targetColor << endl << endl;
				} else {
					os << "INCORRECT (details below)" << endl;
					os << " Should be: " << targetColor << endl;
					os << " Output produced: " << outputColor << endl;
					os << endl;
				}
			}

		}
	}
}
template <class O, class I1>
void runTestsExplicit(const char* N, O(*f)(const I1& arg1), double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;

				I1 i1;
				O expectedOutput;
				input >> i1;
				input >> expectedOutput;
				getLine(input);
				os << "(" << i1 << ") --> " << expectedOutput;
				try {
					O actualOutput = (*f)(i1);
					if (equal(expectedOutput, actualOutput)) {
						corr++;
						os << " Correct" << endl;
					} else {
						os << " Incorrect ( returned: " << actualOutput << ")" << endl;
					}
				}
				catch (...) {
					os << " Incorrect (exception thrown)" << endl;
					exceptions++;
				}
			}
		}
	}
}
template <class O, class I1, class I2>
void runTestsExplicit(const char* N, O(*f)(const I1& arg1, const I2& arg2), double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;

				I1 i1;
				I2 i2;
				O expectedOutput;
				input >> i1 >> i2 >> expectedOutput;
				getLine(input);
				os << "(" << i1 << " " << i2 << ") --> " << expectedOutput;
				try {
					O actualOutput = (*f)(i1, i2);
					if (equal(expectedOutput, actualOutput)) {
						corr++;
						os << " Correct" << endl;
					}
					else {
						os << " Incorrect ( returned: " << actualOutput << ")" << endl;
					}
				}
				catch (...) {
					os << " Incorrect (exception thrown)" << endl;
					exceptions++;
				}
			}
		}
	}
}
template <class O, class I1>
void runTestsExplicit3(const char* N, O(*f)(const I1& arg1, const I1& arg2), double maxPts) {
	std::string name(N);
	std::ostream& os = std::cout;

	istream* inputSS = openFile(TestFile);
	istream& input = *inputSS;

	os << std::fixed << std::setprecision(20);
	os << name << std::endl;

	while (!input.eof()) {
		std::string line = getLine(input);
		if (line == name) {
			int total = 0;
			int corr = 0;
			int exceptions = 0;
			while (true) {
				if (input.eof() || input.peek() == '\n') {
					displayPoints(os, corr, total, maxPts);
					return;
				}
				total++;
				I1 i1;
				I1 i2;
				O expectedOutput;
				input >> i1 >> i2 >> expectedOutput;
				getLine(input);
				os << "(" << i1 << " " << i2 << ") --> " << expectedOutput;
				try {
					O actualOutput = (*f)(i1, i2);
					if (equal(expectedOutput, actualOutput)) {
						corr++;
						os << " Correct" << endl;
					}
					else {
						os << " Incorrect ( returned: " << actualOutput << ")" << endl;
					}
				}
				catch (...) {
					os << " Incorrect (exception thrown)" << endl;
					exceptions++;
				}
			}
		}
	}
}
void initCreateTests() {
	output.open(TestFile);
}
void finishCreateTests() {
	output << endl << "----------" << endl;
	output.close();
}
void initTesting() {
}
void finishTesting() {
}
void createTests() {
	initCreateTests();
	// ==================== C++ ==================== 
	createTests("swap", swap, { std::make_tuple(4, 4, 0, 0), 
								std::make_tuple(2, 3, 3, 2), 
								std::make_tuple(-2, -3, -3, -2),
								std::make_tuple(-2.5, -3.2, -3.2, -2.5),
		} );
	createTests("approximatelyEqual", approximatelyEqual, { std::make_tuple(4.0, 5.0), 
															std::make_tuple(-4.0, -5.0),
															std::make_tuple(4.0, -5.20),
															std::make_tuple(4.0, 4.000001),
															std::make_tuple(4.0, 3.9999999),
															std::make_tuple(-4.0, -3.9999999),
															std::make_tuple(-3.9999999, -4.0),
		});
	createTests("approximatelyZero", approximatelyZero, { 0, 0.0000001, -0.0000001, 0.5, -0.5, 100 });
	createTests("normalizeDegrees", normalizeDegrees, {0.0,0.1,-0.1,360.0,-360.0, 721, -721, 360.75, 2000000000.0, -2000000000.0 });
	createTests("normalizeRadians", normalizeRadians, { 0.0,0.1,-0.1, PI,-PI, 2*PI,-2*PI, 100 * PI, -100, PI, 0.5, 100.5, 1000*PI + 0.2, -1000 * PI + 0.2 });
	createTests("rad2deg", rad2deg, { 0, 1, -1, 3, -3, 20, -20, 0.5, 10001 });
	createTests("deg2rad", deg2rad, { 0, 1, -1, 300, -300, 200, -200, 1000, -1000, 0.5 });
	createTests("min", min, { std::make_tuple(-10,-10,-10),
								std::make_tuple(-10,10,-10),
								std::make_tuple(-10,-10,10),
								std::make_tuple(-10,10,10),
								std::make_tuple(10,-10,10),
								std::make_tuple(10,10,-10),
								std::make_tuple(1,2,3),
								std::make_tuple(2,1,3),
								std::make_tuple(3,2,1),
								std::make_tuple(3.5, 4.5, 2.0),
		});
	createTests("max", max, { std::make_tuple(-10,-10,-10),
								std::make_tuple(-10,10,-10),
								std::make_tuple(-10,-10,10),
								std::make_tuple(-10,10,10),
								std::make_tuple(10,-10,10),
								std::make_tuple(10,10,-10),
								std::make_tuple(1,2,3),
								std::make_tuple(2,1,3),
								std::make_tuple(3,2,1),
								std::make_tuple(3.5, 4.5, 2.7),
		});
	createTests("distanceFromOrigin", distanceFromOrigin, { std::make_tuple(1,0),
								std::make_tuple(0,1),
								std::make_tuple(1,1),
								std::make_tuple(-10,30),
								std::make_tuple(0,-1),
								std::make_tuple(-1,-1),
								std::make_tuple(-10,-30),
								std::make_tuple(2010,3333),
		});
	createTests("distanceBetween", distanceBetween, { std::make_tuple(0,0,1,1),
								std::make_tuple(1.0,1.0,0.0,0.0),
								std::make_tuple(0.0,0.0, 1.0,1.0),
								std::make_tuple(10,10,11,11),
								std::make_tuple(100,100,99,99),
								std::make_tuple(54, 1, -34, -99),
								std::make_tuple(-54, -1, -34, -99),
								std::make_tuple(2000, -9, 3333, 88),
		});
	createTests<double, double, double, double>("areaOfTriangle3", areaOfTriangle, {
													std::make_tuple(3.0, 4.0, 5.0),
													std::make_tuple(3.0, 4.0, 5.0),
													std::make_tuple(3.0, 4.0, 50.0),
													std::make_tuple(1.0, 2.0, 2.0),
													std::make_tuple(1.0, -2.0, 3.0),
													std::make_tuple(-1.0, -2.0, -3.0),
													std::make_tuple(1.0, 1.0, 1.0), 
		});
	createTests<double, double, double, double, double, double>("areaOfTriangle6", areaOfTriangle, { 
													std::make_tuple(0, 0, 1, 0, 1, 1),
													std::make_tuple(0, 0, 3, 0, 0, 4),
													std::make_tuple(10, 10, 13, 10, 10, 14),
													std::make_tuple(-10, -10, -13, -10, -10, -14),
		});

	// ==================== GLM, TRIG, AND DIRECTION ==================== 
	createTests("pointOnUnitCircle", pointOnUnitCircle, { std::make_tuple(0, 0, 0),
															std::make_tuple(PI, 0, 0),
															std::make_tuple(PI_2, 0, 0),
															std::make_tuple(-PI_4, 0, 0),
															std::make_tuple(0.75*PI, 0, 0),
		});
	createTests<double, dvec3, dvec3, dvec3>("areaOfTriangleDVec", areaOfTriangle, {
												std::make_tuple(dvec3(0,0,0), dvec3(1,1,1), dvec3(0, 1, 2)),
												std::make_tuple(dvec3(10,0,0), dvec3(1,1,1), dvec3(0, 1, 2)),
		});
	createTests<double,dvec2,dvec2>("directionInRadians_2Parameters_OutputAngleIn[0,PI]", directionInRadians, { 
															std::make_tuple(dvec2(0,0), dvec2(2,2)),
															std::make_tuple(dvec2(2,10), dvec2(3,11)),
															std::make_tuple(dvec2(-1,-1), dvec2(-2,0)),
															//std::make_tuple(dvec2(3,11), dvec2(2,10)),
															std::make_tuple(dvec2(0,0), dvec2(10,10)),
															//std::make_tuple(dvec2(2,2), dvec2(2,0)),
															std::make_tuple(dvec2(1,2), dvec2(2,3)),
															//std::make_tuple(dvec2(-1,4), dvec2(3,-5)),
		});
	createTests<double, dvec2, dvec2>("directionInRadians_2Parameters_OutputAngleIn(PI,2PI]", directionInRadians, {
															//std::make_tuple(dvec2(0,0), dvec2(2,2)),
															//std::make_tuple(dvec2(2,10), dvec2(3,11)),
															//std::make_tuple(dvec2(-1,-1), dvec2(-2,0)),
															std::make_tuple(dvec2(3,11), dvec2(2,10)),
															//std::make_tuple(dvec2(0,0), dvec2(10,10)),
															std::make_tuple(dvec2(2,2), dvec2(2,0)),
															//std::make_tuple(dvec2(1,2), dvec2(2,3)),
															std::make_tuple(dvec2(-1,4), dvec2(3,-5)),
		});
	createTests<double, double, double, double, double>("directionInRadians4Parameters_OutputAngleIn[0,PI]", directionInRadians, {
															std::make_tuple(0,0,2,2),
															std::make_tuple(2,10,3,11),
															std::make_tuple(-1,-1,-2,0),
															//std::make_tuple(3,11,2,10),
															std::make_tuple(0,0,10,10),
															//std::make_tuple(2,2,2,0),
															std::make_tuple(1,2,2,3),
															//std::make_tuple(-1,4, 3,-5),
		});
	createTests<double, double, double, double, double>("directionInRadians4Parameters_OutputAngleIn(PI,2PI]", directionInRadians, {
															//std::make_tuple(0,0,2,2),
															//std::make_tuple(2,10,3,11),
															//std::make_tuple(-1,-1,-2,0),
															std::make_tuple(3,11,2,10),
															//std::make_tuple(0,0,10,10),
															std::make_tuple(2,2,2,0),
															//std::make_tuple(1,2,2,3),
															std::make_tuple(-1,4, 3,-5),
		});
	createTests<double, dvec2>("directionInRadians1Parameter_OutputAngleIn[0,PI]", directionInRadians, {
																			dvec2(2,2),
																			dvec2(-2,2),
																			//dvec2(2,-2),
																			//dvec2(-2,-2),
																			dvec2(3,2),							
																			//dvec2(3,-2),
																			dvec2(-3,2),
																			//dvec2(-3,-2),
		});
	createTests<double, dvec2>("directionInRadians1Parameter_OutputAngleIn(PI,2PI]", directionInRadians, {
																			//dvec2(2,2),
																			//dvec2(-2,2),
																			dvec2(2,-2),
																			dvec2(-2,-2),
																			//dvec2(3,2),
																			dvec2(3,-2),
																			//dvec2(-3,2),
																			dvec2(-3,-2),
		});
	createTests<dvec2,const dvec2&,double,double>("pointOnCircle", pointOnCircle, {
						std::make_tuple(dvec2(0,0),1.0,0.0 * PI),
						std::make_tuple(dvec2(0,0),1.0,0.2 * PI),
						std::make_tuple(dvec2(0,0),1.0,0.4 * PI),
						std::make_tuple(dvec2(0,0),1.0,0.6 * PI),
						std::make_tuple(dvec2(0,0),1.0,0.8 * PI),
						std::make_tuple(dvec2(0,0),1.0,1.0 * PI),
						std::make_tuple(dvec2(0,0),1.0,1.2 * PI),
						std::make_tuple(dvec2(0,0),1.0,1.4 * PI),
						std::make_tuple(dvec2(0,0),1.0,1.6 * PI),
						std::make_tuple(dvec2(0,0),1.0,1.8 * PI),

						std::make_tuple(dvec2(2.1,-3),1.0,0.0 * PI),
						std::make_tuple(dvec2(2.1,-3),1.0,0.2 * PI),
						std::make_tuple(dvec2(2.1,-3),1.0,0.4 * PI),
						std::make_tuple(dvec2(2.1,-3),1.0,0.6 * PI),
						std::make_tuple(dvec2(2.1,-3),1.0,0.8 * PI),
						std::make_tuple(dvec2(2.1,-3),1.0,1.0 * PI),
						std::make_tuple(dvec2(2.1,-3),1.0,1.2 * PI),
						std::make_tuple(dvec2(2.1,-3),1.0,1.4 * PI),
						std::make_tuple(dvec2(2.1,-3),1.0,1.6 * PI),
						std::make_tuple(dvec2(2.1,-3),1.0,1.8 * PI),
		});
	createTests<dvec2,dvec2>("doubleIt", doubleIt, { dvec2(1,1), dvec2(2,2), dvec2(-1, -1), dvec2(-1, 0), dvec2(0,0) });
	createTests<dvec3, dvec3>("myNormalize", myNormalize, { dvec3(2, 0, 0), dvec3(-3, 4, 0), dvec3(-1, -1, 1), dvec3(-1, 0, 1) });
	createTests<bool, dvec3, dvec3>("isOrthogonal", isOrthogonal, {
																	std::make_tuple(dvec3(0,0,1), dvec3(1,2,2)),
																	std::make_tuple(dvec3(1,0,1), dvec3(0,1,0)),
																	std::make_tuple(dvec3(2, 2, 2), dvec3(4, -5, 1)),
																	std::make_tuple(dvec3(0.5, 0.5, -1.0), dvec3(2.0, 1.0, 1.5)),
																	std::make_tuple(dvec3(-1,-1,1), dvec3(1,2,2)),
																	std::make_tuple(dvec3(1,1,1), dvec3(0,1,0)),
																	std::make_tuple(dvec3(2, 2, 3), dvec3(4, -5, 1)),
																	std::make_tuple(dvec3(0.5, 1.5, -1.0), dvec3(2.0, 1.0, 1.5)),
		});
	createTests<bool, dvec3, dvec3>("formAcuteAngle", formAcuteAngle, {
																	std::make_tuple(dvec3(-1,-1,-1), dvec3(-2,-2,-2)),
																	std::make_tuple(dvec3(0,0,1), dvec3(1,2,2)),
																	std::make_tuple(dvec3(1,0,1), dvec3(0,1,0)),
																	std::make_tuple(dvec3(2, 2, 2), dvec3(4, -5, 1)),
																	std::make_tuple(dvec3(0.5, 0.5, -1.0), dvec3(2.0, 1.0, 1.5)),
																	std::make_tuple(dvec3(-1,-1,1), dvec3(1,2,2)),
																	std::make_tuple(dvec3(1,1,1), dvec3(0,1,0)),
																	std::make_tuple(dvec3(2, 2, 3), dvec3(4, -5, 1)),
																	std::make_tuple(dvec3(-1, -2, -1), dvec3(-2, -1, -1)),
																	std::make_tuple(dvec3(0.5, 1.5, -1.0), dvec3(2.0, 1.0, 1.5)),
		});
	createTests<double, dvec4, dvec4>("cosBetweenVec4", cosBetween, { 
																	std::make_tuple(dvec4(0,0,0,1), dvec4(1,0,0,0)),
																	std::make_tuple(dvec4(0,2,2,1), dvec4(2,2,2,2)),
		});
	createTests<double,dvec3,dvec3>("cosBetweenVec3", cosBetween, { std::make_tuple(dvec3(0,0,1), dvec3(1,2,2)),
																	std::make_tuple(dvec3(1,0,1), dvec3(0,1,0)),
																	std::make_tuple(dvec3(1,-4,1), dvec3(1,1,6)),
																	std::make_tuple(dvec3(1,0,1), dvec3(0,1,0)),
		});
	createTests<double, dvec2, dvec2>("cosBetweenVec2", cosBetween, { std::make_tuple(dvec2(0,1), dvec2(2,2)),
																	std::make_tuple(dvec2(1,0), dvec2(1,1)),
																	std::make_tuple(dvec2(1,5), dvec2(5,1)),
																	std::make_tuple(dvec2(1, 1), dvec2(3, 3)),
		});
	createTests<double, dvec3, dvec3>("areaOfParallelogram", areaOfParallelogram, { std::make_tuple(dvec3(3,4,5), dvec3(1,0,0)),
															std::make_tuple(dvec3(1,1,1), dvec3(0,1,0)),
															std::make_tuple(dvec3(1,0,0), dvec3(1,1,1)),
															std::make_tuple(dvec3(-1,-2,-3), dvec3(-2,-3,-4)),
		});
	createTestsArea("areaOfTriangleDvec", {
											std::make_tuple(dvec3(0,0,0), dvec3(1,0,0), dvec3(1,1,0)),
											std::make_tuple(dvec3(1,1,1), dvec3(0,1,0), dvec3(0,0,0)),
											std::make_tuple(dvec3(1,1,1), dvec3(2,2,2), dvec3(1,2,2)),
											std::make_tuple(dvec3(-1,-2,-3), dvec3(-2,-3,-4), dvec3(0,0,0)),
		});
	createTests<dvec3, dvec3, dvec3>("pointingVector", pointingVector, { std::make_tuple(dvec3(3,4,5), dvec3(1,0,0)),
															std::make_tuple(dvec3(1,1,1), dvec3(0,1,0)),
															std::make_tuple(dvec3(1,0,0), dvec3(1,1,1)),
															std::make_tuple(dvec3(-1,-2,-3), dvec3(-2,-3,-4)),
		});
	createTests<dvec3, dvec3>("normalFrom3Points3Args", normalFrom3Points, { std::make_tuple(dvec3(0,0,0), dvec3(1,0,0), dvec3(1,1,0)),
															std::make_tuple(dvec3(0,0,0), dvec3(1,1,0), dvec3(1,0,0)),
															std::make_tuple(dvec3(1,1,1), dvec3(0,1,0), dvec3(0,0,0)),
															std::make_tuple(dvec3(1,1,1), dvec3(2,2,2), dvec3(1,2,2)),
															std::make_tuple(dvec3(-1,-2,-3), dvec3(-2,-3,-4), dvec3(0,0,0)),
		});
	createTests<dvec3, vector<dvec3>>("normalFrom3Points1Arg", normalFrom3Points, { {dvec3(0,0,0), dvec3(1,0,0), dvec3(1,1,0)},
															{dvec3(0,0,0), dvec3(1,1,0), dvec3(1,0,0)},
															{dvec3(1,1,1), dvec3(0,1,0), dvec3(0,0,0)},
															{dvec3(1,1,1), dvec3(2,2,2), dvec3(1,2,2)},
															{dvec3(-1,-2,-3), dvec3(-2,-3,-4), dvec3(0,0,0)},
		});
	createTests<double, double, double, double, double, double>("mapReturnDouble", map,
		{ std::make_tuple(5,0,10,4,5),
			std::make_tuple(-5,-10,0,40,50),
			std::make_tuple(0.5, 0.0, 1.0, 10.0, 20.0),
			std::make_tuple(15.0, 10.0, 20.0, 0.0, 1.0),
			std::make_tuple(-10.0, -20.0, 0.0, 100.0, 200.0),
			std::make_tuple(15.0, 10.0, 20.0, -200.0, -100.0),
		}
	);
	createTestsQuadratic("quadraticArray", quadratic,
		{   std::make_tuple(1, 4, 3),   // 2 roots
		    std::make_tuple(3,4,1),		// 2 roots
			//std::make_tuple(-1.5,0,0),		// 2 roots
			//std::make_tuple(-3,4,-1),		// 2 roots
			//std::make_tuple(-.1,.2,.5),		// 2 roots
			std::make_tuple(1, 0, 0),	// 1 root = 0
			std::make_tuple(1, 2, 1),	// 1 root = -1
			std::make_tuple(1, -2, 1),	// 1 root = +1
			std::make_tuple(-4, -2, -1),  // 0 roots
			std::make_tuple(-1, 4.5, -20),  // 0 roots
		});
	createTestsQuadratic("quadraticArrayRequiresSorting", quadratic,
		{ //std::make_tuple(1, 4, 3),   // 2 roots
			//std::make_tuple(3,4,1),		// 2 roots
			std::make_tuple(-1.5,0,0),		// 2 roots
			std::make_tuple(-3,4,-1),		// 2 roots
			std::make_tuple(-.1,.2,.5),		// 2 roots
			//std::make_tuple(1, 0, 0),	// 1 root
			//std::make_tuple(1, 2, 1),	// 1 root
			//std::make_tuple(-4, -2, -1),  // 0 roots
			//std::make_tuple(-1, 4.5, -20),  // 0 roots
		});
	createTestsQuadraticVector("quadraticVector",
		{ std::make_tuple(1, 4, 3),   // 2 roots
			std::make_tuple(3,4,1),		// 2 roots
			//std::make_tuple(-1.5,0,0),		// 2 roots
			//std::make_tuple(-3,4,-1),		// 2 roots
			//std::make_tuple(-.1,.2,.5),		// 2 roots
			std::make_tuple(1, 0, 0),	// 1 root
			std::make_tuple(1, 2, 1),	// 1 root
			std::make_tuple(1, -2, 1),	// 1 root
			std::make_tuple(-4, -2, -1),  // 0 roots
			std::make_tuple(-1, 4.5, -20),  // 0 roots
		});
	createTestsQuadraticVector("quadraticVectorRequiresSorting",
		{ //std::make_tuple(1, 4, 3),   // 2 roots
			//std::make_tuple(3,4,1),		// 2 roots
			std::make_tuple(-1.5,0,0),		// 2 roots
			std::make_tuple(-3,4,-1),		// 2 roots
			std::make_tuple(-.1,.2,.5),		// 2 roots
			//std::make_tuple(1, 0, 0),	// 1 root
			//std::make_tuple(1, 2, 1),	// 1 root
			//std::make_tuple(-4, -2, -1),  // 0 roots
			//std::make_tuple(-1, 4.5, -20),  // 0 roots
		});

	IPlane p1(dvec3(0, 0, 0), dvec3(0, 1, 0));
	IPlane p2(dvec3(1, 0, 1), dvec3(0, 2, 0));
	IPlane p3(dvec3(0, 1, 0), dvec3(0, 1, 0));
	IPlane p4(dvec3(0, 0, 0), dvec3(1, 1, 0));
	IPlane p5(dvec3(-1, 1, 0), dvec3(1, 1, 0));

	IPlane p6(dvec3(5, 5, 5), dvec3(0, 1, 1));
	IPlane p7(dvec3(5, 6, 4), dvec3(0, 1, 1));
	IPlane p8(dvec3(5, 6, 6), dvec3(0, 1, 1));
	createTests<bool, IPlane, IPlane>("equalPlanes", equalPlanes, {
				std::make_tuple(p1, p1),
				std::make_tuple(p1, p2),
				std::make_tuple(p1, p3),
				std::make_tuple(p1, p4),
				std::make_tuple(p1, p5),
				std::make_tuple(p2, p1),
				std::make_tuple(p2, p2),
				std::make_tuple(p2, p3),
				std::make_tuple(p2, p4),
				std::make_tuple(p2, p5),
				std::make_tuple(p3, p1),
				std::make_tuple(p3, p2),
				std::make_tuple(p3, p3),
				std::make_tuple(p3, p4),
				std::make_tuple(p3, p5),
				std::make_tuple(p4, p1),
				std::make_tuple(p4, p2),
				std::make_tuple(p4, p3),
				std::make_tuple(p4, p4),
				std::make_tuple(p4, p5),
				std::make_tuple(p5, p1),
				std::make_tuple(p5, p2),
				std::make_tuple(p5, p3),
				std::make_tuple(p5, p4),
				std::make_tuple(p5, p5),

				std::make_tuple(p6, p7),
				std::make_tuple(p6, p8),
				std::make_tuple(p7, p8),
		}
	);

	IPlane P1(dvec3(0, -1, 0), dvec3(0, 1, 0));
	IPlane P2(dvec3(1, 1, 1), dvec3(3.5, 3, 1.5));

	Ray ray1(dvec3(0, 0, 0), glm::normalize(dvec3(0, 0.5, -1)));
	Ray ray2(dvec3(0, 0, 0), glm::normalize(dvec3(0, 0, -1)));
	Ray ray3(dvec3(0, 0, 0), dvec3(0, -0.5, -1));
	Ray ray4(dvec3(6, 6, 6), dvec3(-1, -1, -1));

	createPlaneIntersectionTests<IPlane>("planeIntersectionFromClass", {
				std::make_tuple(P1, ray1),
				std::make_tuple(P1, ray2),
				std::make_tuple(P1, ray3),
		});

	createPlaneIntersectionTests<IPlane>("planeIntersectionNewTestCases", {
				std::make_tuple(P2, ray1),
				std::make_tuple(P2, ray2),
				std::make_tuple(P2, ray3),
				std::make_tuple(P2, ray4),
		});

	IDisk D1(dvec3(0, 0, -1), dvec3(0, 0, 1), 1.0);
	IDisk D2(dvec3(0, 0, -10), dvec3(0, 0, 1), 1.0);
	IDisk D3(dvec3(1.5, 1.5, -1.5), dvec3(1.0, 1.0, 1.0), 2.5);

	createPlaneIntersectionTests<IDisk>("diskIntersectionFromClass", {
				std::make_tuple(D1, ray1),
				std::make_tuple(D1, ray2),
				std::make_tuple(D1, ray3),
				std::make_tuple(D2, ray1),
				std::make_tuple(D2, ray2),
				std::make_tuple(D2, ray3),
		});

	createPlaneIntersectionTests<IDisk>("diskIntersectionNewTestCases", {
				std::make_tuple(D3, ray1),
				std::make_tuple(D3, ray2),
				std::make_tuple(D3, ray3),
				std::make_tuple(D3, ray4),
		});


	createTests<dmat4, double>("S", S, { 2, 1, -1 });
	createTests<dmat4, double, double, double>("S3args", S, { std::make_tuple(1,2,3), std::make_tuple(-2,2,1) });
	createTests<dmat4, double>("Rx", Rx, { 0, 1, -1, 2, TWO_PI,-TWO_PI });
	createTests<dmat4, double>("Ry", Ry, { 0, 1, -1, 2, TWO_PI,-TWO_PI });
	createTests<dmat4, double>("Rz", Rz, { 0, 1, -1, 2, TWO_PI,-TWO_PI });

	dvec3 eyePos(3.0, 0.0, 1.0);

	dvec3 interceptPt(5.0, 2.0, -3.0);
	dvec3 pointToRight(4.0, 4.0, 0.0);
	dvec3 pointToLeft(0.0, 0.0, 0.0);

	color L1amb(0.3, 0.2, 0.1);
	color L1diff(1.0, 1.0, 1.0);
	color L1spec(0.5, 0.6, 0.7);
	dvec3 L1pos(6.0, 1.0, 0.0);

	color mat1amb(0.4, 0.5, 0.6);
	color mat1diff(0.9, 1.0, 0.9);
	color mat1spec(0.9, 0.8, 0.7);

	dvec3 V1 = pointToRight - interceptPt;
	dvec3 V2 = pointToLeft - interceptPt;
	dvec3 n = glm::normalize(glm::cross(V1, V2));
	dvec3 v = glm::normalize(eyePos - interceptPt);
	dvec3 l = glm::normalize(L1pos - interceptPt);
	dvec3 r = glm::normalize(2.0 * glm::dot(l, n) * n - l);
	LightColor L1color(L1amb, L1diff, L1spec);
	PositionalLight L1(L1pos, L1color);
	Material mat1(mat1amb, mat1diff, mat1spec, 1.0);
	LightATParams atParams(1.0, 2.0, 0.0);
	bool useAttenuation = false;

	createTests<color, color, color>("ambientColorProvidedTests", ambientColor,
		{ 
			std::make_tuple(color(0.4, 0.5, 0.6),color(0.3, 0.2, 0.1)),
		});
	createTests<color, color, color>("ambientColorNewTests", ambientColor,
		{
			std::make_tuple(color(1,1,1),color(0.5,0.5,1)),
			std::make_tuple(color(0.5,0.2,0.1),color(0.5,0.5,1)),
			std::make_tuple(color(0.1,0.2,0.3),color(1.0, 1.0, 1.0))
		});
	createTests<color, color, color, dvec3, dvec3>("diffuseColorPositiveLdotNProvidedTests", diffuseColor,
		{
			std::make_tuple(color(0.9, 1.0, 0.9),color(1.0, 1.0, 1.0), l, n),
		});
	createTests<color, color, color, dvec3, dvec3>("diffuseColorNegativeLdotNProvidedTests", diffuseColor,
		{
			std::make_tuple(color(0.9, 1.0, 0.9),color(1.0, 1.0, 1.0), glm::normalize(dvec3(-1.0, -1.0, -1.0)), n),
		});
	createTests<color, color, color, dvec3, dvec3>("diffuseColorNewTests", diffuseColor,
		{
			std::make_tuple(color(0.9, 1.0, 0.9),color(1.0, 1.0, 1.0),glm::normalize(dvec3(1, -1, 3)),glm::normalize(dvec3(12,-12,12))),
			std::make_tuple(color(0.4,0.5,0.6),color(1,1,1),glm::normalize(dvec3(1,1,1)),glm::normalize(dvec3(1,1,1))),
			std::make_tuple(color(1,1,1),color(1,1,1),glm::normalize(dvec3(1,1,1)),glm::normalize(dvec3(0.9,1,1))),
			std::make_tuple(color(1,1,1),color(1,1,1),glm::normalize(dvec3(1,1,1)),glm::normalize(dvec3(-1,-1,-1))),
		});


	createTests<color, color, color, double, dvec3, dvec3>("specularPositiveRdotVProvidedTests", specularColor,
		{
			std::make_tuple(color(0.9, 0.8, 0.7),color(0.5,0.6,0.7), 1.0, r ,v),
		});
	createTests<color, color, color, double, dvec3, dvec3>("specularNegativeRdotVProvidedTests", specularColor,
		{
			std::make_tuple(color(0.9, 0.8, 0.7),color(0.5,0.6,0.7), 1.0, r , glm::normalize(dvec3(-1.0, -1.0, -1.0))),
		});
	createTests<color, color, color, double, dvec3, dvec3>("specularNewTests", specularColor,
		{
			std::make_tuple(color(0.9, 0.8, 0.7),color(0.5,0.6,0.7), 2.0, 2 * glm::dot(l, n) * n - l , v),
			std::make_tuple(color(0.9, 0.8, 0.7),color(0.5,0.6,0.7), 4.0, 2 * glm::dot(l, n) * n - l , v),
			std::make_tuple(color(0.9, 0.8, 0.7),color(0.5,0.6,0.7), 1.0, 2 * glm::dot(l, n) * n - l , -v),
			std::make_tuple(color(0.9, 0.8, 0.7),color(0.5,0.6,0.7), 2.6, 2 * glm::dot(l, n) * n - l , -v),
		});

	createTotalColorTests("totalColorAttenuationOffGivenTests",
		{
			std::make_tuple(mat1, L1color, v, n, L1pos, interceptPt, false, atParams),
		});

	createIlluminateTests("illuminateLightIsOffGivenTests",
		{
			std::make_tuple(mat1, L1color, v, n, L1pos, interceptPt, true, atParams, false, false, eyePos),
			std::make_tuple(mat1, L1color, v, n, L1pos, interceptPt, false, atParams, false, false, eyePos),
			std::make_tuple(mat1, L1color, v, n, L1pos, interceptPt, true, atParams, false, true, eyePos),
			std::make_tuple(mat1, L1color, v, n, L1pos, interceptPt, false, atParams, false, true, eyePos),
		});

	createIlluminateTests("illuminateLightIsOnShadowOnGivenTests",
		{
			std::make_tuple(mat1, L1color, v, n, L1pos, interceptPt, false, atParams, true, true, eyePos),
			std::make_tuple(mat1, L1color, v, n, L1pos, interceptPt, true, atParams, true, true, eyePos),
		});
	createIlluminateTests("illuminateLightIsOnShadowOffAttenuationOnGivenTests",
		{
			std::make_tuple(mat1, L1color, v, n, L1pos, interceptPt, true, atParams, true, false, eyePos),
		});
	createIlluminateTests("illuminateLightIsOnShadowOffAttenuationOffGivenTests",
		{
			std::make_tuple(mat1, L1color, v, n, L1pos, interceptPt, false, atParams, true, false, eyePos),
		});

	createPCameraTests("PerspectiveCameraTestsOriginal",
		{
			std::make_tuple(dvec3(0, 0, 0), dvec3(0, 0, -2), PI_4, 200, 200, glm::ivec2(0, 0), glm::ivec2(100, 100)),
			std::make_tuple(dvec3(5, 5, 0), dvec3(4, 5, 0), PI_4, 200, 400, glm::ivec2(0, 0), glm::ivec2(100, 200)),
		});
	createPCameraTests("PerspectiveCameraTestsNew",
		{
			std::make_tuple(dvec3(10, -5, 4), dvec3(1, 4, -2), PI_2, 100, 200, glm::ivec2(0, 0), glm::ivec2(50, 75)),
			std::make_tuple(dvec3(-5, -5, 0), dvec3(0, 5, 5), PI_6, 500, 600, glm::ivec2(0, 0), glm::ivec2(100, 200)),
		});
	createOCameraTests("OrthographicCameraTestsOriginal",
		{
			std::make_tuple(dvec3(0, 0, 0), dvec3(0, 0, -2), PI_4, 200, 200, glm::ivec2(0, 0), glm::ivec2(100, 100)),
			std::make_tuple(dvec3(5, 5, 0), dvec3(4, 5, 0), PI_4, 200, 400, glm::ivec2(0, 0), glm::ivec2(100, 200)),
		});
	createOCameraTests("OrthographicCameraTestsNew",
		{
			std::make_tuple(dvec3(10, -5, 4), dvec3(1, 4, -2), PI_2, 100, 200, glm::ivec2(0, 0), glm::ivec2(50, 75)),
			std::make_tuple(dvec3(-5, -5, 0), dvec3(0, 5, 5), PI_6, 500, 600, glm::ivec2(0, 0), glm::ivec2(100, 200)),
		});
	dvec3 VV1(1, 2, 3);
	dvec3 VV2(-1, -2, -3);
	dvec3 VV3(3,2,1);

	dmat3 M3x3A(1, 0, 3, 2, 1, 1, 3, 2, 0);
	dmat3 M3x3B(1, 2, 3, 2, 4, 0, 0, 0, 1);
	dmat3 M3x3C(10, 0, 0, 0, 10, 0, 0, 0, 10);
	dmat3 M3x3D(10,20,30,40,50,60,70,80,90);

	vector<dmat3> vecMats = { M3x3A, M3x3B, M3x3C, M3x3D };
	vector<dvec3> vecVecs = { VV1, VV2, VV3 };

	createTests<dvec3, dmat3, int>("getRow", getRow, { std::make_tuple(M3x3A, 0),
														std::make_tuple(M3x3A, 1),
														std::make_tuple(M3x3A, 2) });
	createTests<dvec3, dmat3, int>("getCol", getCol, { std::make_tuple(M3x3A, 0),
														std::make_tuple(M3x3A, 1),
														std::make_tuple(M3x3A, 2) });
	createTests<bool, dmat3>("isInvertible", isInvertible, { M3x3A, M3x3B, M3x3C });
	createTests<dvec3, dmat3, dvec3>("solveLinearSystem", solveLinearSystem, { std::make_tuple(M3x3A, V1),
														std::make_tuple(M3x3B, VV1),
														std::make_tuple(M3x3C, VV1) });
	createTestsVec<dmat3, dmat3>("addMatrices", addMatrices, { vecMats });
	createTestsVec<dmat3, dmat3>("multiplyMatrices", multiplyMatrices, { vecMats });
	createTests<dvec3, dmat3, dvec3>("multiplyMatrixAndVertex", multiplyMatrixAndVertex, 
														{ std::make_tuple(M3x3A, VV1),
														std::make_tuple(M3x3B, VV2),
														std::make_tuple(M3x3C, VV3) }
												);
	createTestsVec<dvec3, dmat3, dvec3>("multiplyMatricesAndVertex", multiplyMatricesAndVertex, { std::make_tuple(vecMats, V1), });
	createTestsVec<dvec3, dmat3, dvec3>("multiplyMatrixAndVertices", multiplyMatrixAndVertices, { std::make_tuple(M3x3A, vecVecs),
																									std::make_tuple(M3x3B, vecVecs),
																									std::make_tuple(M3x3C, vecVecs),
		});
	createTestsVec("multiplyMatricesAndVertices", multiplyMatricesAndVertices, { std::make_tuple(vecMats, vecVecs) }
		);

	finishCreateTests();
}
void runTests() {
	// ==================== GLM, TRIG, AND DIRECTION ==================== 

	//runTests("swap", swap, 2.0);
	//runTests<bool, double, double>("approximatelyEqual", approximatelyEqual, 2.0);
	//runTests("approximatelyZero", approximatelyZero, 2.0);
	//runTests("normalizeDegrees", normalizeDegrees, 2.0);
	//runTests("normalizeRadians", normalizeRadians, 2.0);
	//runTests("rad2deg", rad2deg, 2.0);
	//runTests("deg2rad", deg2rad, 2.0);
	//runTests("min", min, 2.0);
	//runTests("max", max, 2.0);
	//runTests("distanceFromOrigin", distanceFromOrigin, 2.0);
	//runTests("distanceBetween", distanceBetween, 2.0);
	//runAreaTests("areaOfTriangle3", 2.0);
	//runTests<double, double, double, double, double, double, double>("areaOfTriangle6", areaOfTriangle, 2.0);

	//runTests<double, dvec3, dvec3, dvec3>("areaOfTriangleDVec", areaOfTriangle, 2.0);
	// ==================== GLM, TRIG, AND DIRECTION ==================== 

	//runTests("pointOnUnitCircle", pointOnUnitCircle, 2.0);
	//runTests("pointOnCircle", pointOnCircle, 2.0);
	//runTests<double, dvec2, dvec2>("directionInRadians_2Parameters_OutputAngleIn[0,PI]", directionInRadians, 1.0);
	//runTests<double, dvec2, dvec2>("directionInRadians_2Parameters_OutputAngleIn(PI,2PI]", directionInRadians, 1.0);
	//runTests<double, double, double, double, double>("directionInRadians4Parameters_OutputAngleIn[0,PI]", directionInRadians, 1.0);
	//runTests<double, double, double, double, double>("directionInRadians4Parameters_OutputAngleIn(PI,2PI]", directionInRadians, 1.0);
	//runTests2<double, dvec2>("directionInRadians1Parameter_OutputAngleIn[0,PI]", directionInRadians, 1.0);
	//runTests2<double, dvec2>("directionInRadians1Parameter_OutputAngleIn(PI,2PI]", directionInRadians, 1.0);

	//runTestsExplicit<double, double>("mapReturnDouble", map, 2.0);
	//runTestsQuadraticArray("quadraticArray", 1.0);
	//runTestsQuadraticArray("quadraticArrayRequiresSorting", 1.0);
	//runTestsQuadraticVector("quadraticVector", 1.5);
	//runTestsQuadraticVector("quadraticVectorRequiresSorting", 0.5);

	//runTestsExplicit2<dvec2>("doubleIt", doubleIt, 2.0);
	//runTestsExplicit2<dvec3>("myNormalize", myNormalize, 2.0);
	//runTestsExplicit3<bool, dvec3>("isOrthogonal", isOrthogonal, 2.0);
	//runTestsExplicit3<bool, dvec3>("formAcuteAngle", formAcuteAngle, 2.0);
	//runTestsExplicit<double, dvec4>("cosBetweenVec4", cosBetween, 2.0);
	//runTestsExplicit<double, dvec3>("cosBetweenVec3", cosBetween, 2.0);
	//runTestsExplicit<double, dvec2>("cosBetweenVec2", cosBetween, 2.0);
	//runTestsExplicit<double, dvec3>("areaOfParallelogram", areaOfParallelogram, 2.0);
	//runTests<double, dvec3, dvec3, dvec3>("areaOfTriangleDvec", areaOfTriangle, 2.0);
	//runTests<dvec3, dvec3>("pointingVector", pointingVector, 2.0);
	//runTests<dvec3, dvec3>("normalFrom3Points3Args", normalFrom3Points, 2.0);
	//runTests<dvec3, vector<dvec3>>("normalFrom3Points1Arg", normalFrom3Points, 2.0);
	//runTests<bool, IPlane, IPlane>("equalPlanes", equalPlanes, 3.0);
//	runPlaneIntersectionTests<IPlane>("planeIntersectionFromClass", 3.0);
//	runPlaneIntersectionTests<IPlane>("planeIntersectionNewTestCases", 2.0);
//	runPlaneIntersectionTests<IDisk>("diskIntersectionFromClass", 3.0);
//	runPlaneIntersectionTests<IDisk>("diskIntersectionNewTestCases", 2.0);
	//runTests <dmat4, double, double, double>("S3args", static_cast<dmat4(*)(double,double,double)>(&S), 2.0);
	//runTests<dmat4, double>("S", static_cast<dmat4 (*)(double)>(&S), 2.0);
	//runTests("Rx", Rx, 2.0);
	//runTests("Ry", Ry, 2.0);
	//runTests("Rz", Rz, 2.0);
	runTests<color, color>("ambientColorProvidedTests", ambientColor, 1.0);
	runTests<color, color>("ambientColorNewTests", ambientColor, 1.0);

	runTests<color, color, color, dvec3, dvec3>("diffuseColorPositiveLdotNProvidedTests", diffuseColor, 0.5);
	runTests<color, color, color, dvec3, dvec3>("diffuseColorNegativeLdotNProvidedTests", diffuseColor, 0.5);
	runTests<color, color, color, dvec3, dvec3>("diffuseColorNewTests", diffuseColor, 1.0);

	runTests<color, color, color, double, dvec3, dvec3>("specularPositiveRdotVProvidedTests", specularColor, 0.5);
	runTests<color, color, color, double, dvec3, dvec3>("specularNegativeRdotVProvidedTests", specularColor, 0.5);
	runTests<color, color, color, double, dvec3, dvec3>("specularNewTests", specularColor, 1.0);

	runTotalColorTests("totalColorAttenuationOffGivenTests", 4.0);

	runIlluminateColorTests("illuminateLightIsOffGivenTests", 1.0);
	runIlluminateColorTests("illuminateLightIsOnShadowOnGivenTests", 1.0);
	runIlluminateColorTests("illuminateLightIsOnShadowOffAttenuationOffGivenTests", 0.5);
	runIlluminateColorTests("illuminateLightIsOnShadowOffAttenuationOnGivenTests", 0.5);


	//runPCameraTests("PerspectiveCameraTestsOriginal", 6);
	//runPCameraTests("PerspectiveCameraTestsNew", 3);
	//runOCameraTests("OrthographicCameraTestsOriginal", 6);
	//runOCameraTests("OrthographicCameraTestsNew", 3);
	//runTests<dvec3, dmat3, int>("getRow", getRow, 2);
	//runTests<dvec3, dmat3, int>("getCol", getCol, 2);
	//runTests<bool, dmat3>("isInvertible", isInvertible, 2);
	//runTests<dvec3, dmat3, dvec3>("solveLinearSystem", solveLinearSystem, 2);
	//runTestsVec<dmat3, dmat3>("addMatrices", addMatrices, 2);
	//runTests("multiplyMatrices", multiplyMatrices, 2);
	//runTests("multiplyMatrixAndVertex", multiplyMatrixAndVertex, 2);
	//runTests("multiplyMatricesAndVertex", multiplyMatricesAndVertex, 2);
	//runTests("multiplyMatrixAndVertices", multiplyMatrixAndVertices, 2);
	//runTests("multiplyMatricesAndVertices", multiplyMatricesAndVertices, 2);
	cout << endl << "Total Points = " << pts << endl;
}
int main(int argc, char* argv[]) {
	createTests();
	runTests();
	return 0;
}

}
