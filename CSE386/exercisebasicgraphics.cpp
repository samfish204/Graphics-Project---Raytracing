/****************************************************
 * 2016-2022 Eric Bachmann and Mike Zmuda
 * All Rights Reserved.
 * NOTICE:
 * Dissemination of this information or reproduction
 * of this material is prohibited unless prior written
 * permission is granted.
 ****************************************************/

#include <ctime>
#include <vector>
#include "defs.h"
#include "utilities.h"
#include "framebuffer.h"
#include "colorandmaterials.h"
#include "rasterization.h"
#include "io.h"

FrameBuffer frameBuffer(WINDOW_WIDTH, WINDOW_HEIGHT);

void render() {
	frameBuffer.clearColorBuffer();
	drawLine(frameBuffer, 0, 0, 100, 100, red);
	drawLine(frameBuffer, 100, 100, 200, 100, blue);
	frameBuffer.showColorBuffer();
}

void resize(int width, int height) {
	frameBuffer.setFrameBufferSize(width, height);
	glutPostRedisplay();
}
int main(int argc, char* argv[]) {
//	graphicsInit(argc, argv, __FILE__);

//	glutDisplayFunc(render);
//	glutReshapeFunc(resize);
//	glutKeyboardFunc(keyboardUtility);
//	glutMouseFunc(mouseUtility);

//	frameBuffer.setClearColor(paleGreen);

//	glutMainLoop();

//	double x, y;
//	pointOnUnitCircle(PI_2, x, y);
//	cout << x << ' ' << y << endl;

//	cout << approximatelyEqual(4.0, 5.0) << endl;

//	cout << normalizeDegrees(-0.1000000000000000055) << endl;

//	cout << distanceBetween(2000, -9, 3333, 88) << endl;

//	cout << areaOfTriangle(1.0, -2.0, 3.0) << endl;

//	dvec2 d2(0, -2);
//	dvec2 d2b(2, 0);

//  cout << directionInRadians(0, 0, 2, 2) << endl;

//	cout << map(2, 0, 5, 10, 11) << endl;

//	cout << quadratic(1, 0, 0) << endl;

//	double roots[2];
//	cout << quadratic(1, 4, 3, roots) << endl;


//	cout << doubleIt(dvec2(3, -5)) << endl;

//	cout << myNormalize(dvec3(1, 1, 1)) << endl;

//	cout << cosBetween(dvec2(-1.0, glm::sqrt(3.0)), dvec2(1.0, glm::sqrt(3.0))) << endl;

//	cout << areaOfTriangle(dvec3(-10.0, -10.0, -10.0), dvec3(-11.0, -10.0, -10.0), dvec3(-10.0, -11.0, -10.0)) << endl;

	double roots[2];
	cout << quadratic(-0.10000000000000000555, 0.20000000000000001110, 0.5, roots) << ": " << roots[0] << ", " << roots[1] << endl;

	return 0;
}