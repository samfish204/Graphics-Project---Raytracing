/****************************************************
 * 2016-2022 Eric Bachmann and Mike Zmuda
 * All Rights Reserved.
 * NOTICE:
 * Dissemination of this information or reproduction
 * of this material is prohibited unless prior written
 * permission is granted.
 ****************************************************/

// Samuel Fisher (fishe108)
// CSE 386
// Dr. Zmuda
// Raytracing Project
// Due: April 19, 2022

#include <ctime>
#include "defs.h"
#include "io.h"
#include "ishape.h"
#include "framebuffer.h"
#include "raytracer.h"
#include "iscene.h"
#include "light.h"
#include "image.h"
#include "camera.h"
#include "rasterization.h"

int currLight = 0;
double angle = 0.5;
const int MAX = 10;
double z = -MAX;
double inc = 0.4;
bool isAnimated = false;
int numReflections = 0;
int antiAliasing = 1;
bool multiViewOn = false;
double spotDirX = 0;
double spotDirY = -1;
double spotDirZ = 0;

dvec3 cameraPos1(5, 5, 5);
dvec3 cameraFocus1(0, 5, 0);
dvec3 cameraUp1 = Y_AXIS;

double cameraFOV = glm::radians(120.0);

vector<PositionalLightPtr> lights = {
						new PositionalLight(dvec3(0, 20, 0), white),
						new SpotLight(dvec3(0, 5, 0),
										dvec3(spotDirX,spotDirY,spotDirZ),
										glm::radians(90.0),
										white)
};

PositionalLightPtr posLight = lights[0];
SpotLightPtr spotLight = (SpotLightPtr)lights[1];

FrameBuffer frameBuffer(WINDOW_WIDTH, WINDOW_HEIGHT);
RayTracer rayTrace(paleGreen);
IScene scene;

Image im("usflag.ppm");

void render() {
	int frameStartTime = glutGet(GLUT_ELAPSED_TIME);
	int width = frameBuffer.getWindowWidth();
	int height = frameBuffer.getWindowHeight();
	scene.camera = new PerspectiveCamera(cameraPos1, cameraFocus1, cameraUp1, cameraFOV, width, height);
	rayTrace.raytraceScene(frameBuffer, numReflections, scene, antiAliasing);

	int frameEndTime = glutGet(GLUT_ELAPSED_TIME); // Get end time
	double totalTimeSec = (frameEndTime - frameStartTime) / 1000.0;
	cout << "Render time: " << totalTimeSec << " sec." << endl;
}

void resize(int width, int height) {
	frameBuffer.setFrameBufferSize(width, height);
	glutPostRedisplay();
}

IPlane* plane = new IPlane(dvec3(0.0, -2.0, 0.0), dvec3(0.0, 1.0, 0.0));
IPlane* clearPlane = new IPlane(dvec3(0.0, 0.0, 0.0), dvec3(0.0, 0.0, 1.0));
//ISphere* sphere1 = new ISphere(dvec3(-10.0, 0.0, 8.0), 4.0);
ISphere* sphere2 = new ISphere(dvec3(-5.0, 0.0, 8.0), 2.0);
IDisk* disk = new IDisk(dvec3(0.0, 0.0, -2), dvec3(0, 0, 1), 3.0);
ICylinderY* cylinder = new ICylinderY(dvec3(2, 0, 2), 1, 3);
ICylinderZ* cylinder2 = new ICylinderZ(dvec3(5, 0, -2), 1, 2);
IClosedCylinderY* closedCyl = new IClosedCylinderY(dvec3(-2, -1, 4), 1, 3);
ICone* cone = new IConeY(dvec3(3, 2, -12), 3, 5);

void buildScene() {
	scene.addOpaqueObject(new VisibleIShape(plane, tin));
	scene.addTransparentObject(new TransparentIShape(clearPlane, red, 0.25));

	//scene.addOpaqueObject(new VisibleIShape(sphere1, silver));
	scene.addOpaqueObject(new VisibleIShape(sphere2, silver));
	scene.addOpaqueObject(new VisibleIShape(disk, copper));

	scene.addOpaqueObject(new VisibleIShape(cylinder, gold, &im));
	scene.addOpaqueObject(new VisibleIShape(cylinder2, copper));
	scene.addOpaqueObject(new VisibleIShape(closedCyl, copper));

	scene.addOpaqueObject(new VisibleIShape(cone, silver));

	scene.addLight(lights[0]);
	scene.addLight(lights[1]);
}

void incrementClamp(double& v, double delta, double lo, double hi) {
	v = glm::clamp(v + delta, lo, hi);
}

void incrementClamp(int& v, int delta, int lo, int hi) {
	v = glm::clamp(v + delta, lo, hi);
}

void timer(int id) {
	if (isAnimated) {
		z += inc;
		if (z <= -MAX) {
			inc = -inc;
		} else if (z >= MAX) {
			inc = -inc;
		}
	}
	clearPlane->a = dvec3(0, 0, z);
	glutTimerFunc(TIME_INTERVAL, timer, 0);
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
	int W, H;
	const double INC = 0.5;
	switch (key) {
	case 'A':
	case 'a':	currLight = 0;
		cout << *lights[0] << endl;
		break;
	case 'B':
	case 'b':	currLight = 1;
		cout << *lights[1] << endl;
		break;
	case 'O':
	case 'o':	lights[currLight]->isOn = !lights[currLight]->isOn;
		cout << (lights[currLight]->isOn ? "ON" : "OFF") << endl;
		break;
	case 'V':
	case 'v':	lights[currLight]->isTiedToWorld = !lights[currLight]->isTiedToWorld;
		cout << (lights[currLight]->isTiedToWorld ? "World" : "Camera") << endl;
		break;
	case 'Q':
	case 'q':	lights[currLight]->attenuationIsTurnedOn = !lights[currLight]->attenuationIsTurnedOn;
		cout << (lights[currLight]->attenuationIsTurnedOn ? "Atten ON" : "Atten OFF") << endl;
		break;
	case 'W':
	case 'w':	incrementClamp(lights[currLight]->atParams.constant, isupper(key) ? INC : -INC, 0.0, 10.0);
		cout << lights[currLight]->atParams << endl;
		break;
	case 'E':
	case 'e':	incrementClamp(lights[currLight]->atParams.linear, isupper(key) ? INC : -INC, 0.0, 10.0);
		cout << lights[currLight]->atParams << endl;
		break;
	case 'R':
	case 'r':	incrementClamp(lights[currLight]->atParams.quadratic, isupper(key) ? INC : -INC, 0.0, 10.0);
		cout << lights[currLight]->atParams << endl;
		break;
	case 'X':
	case 'x': lights[currLight]->pos.x += (isupper(key) ? INC : -INC);
		cout << lights[currLight]->pos << endl;
		break;
	case 'Y':
	case 'y': lights[currLight]->pos.y += (isupper(key) ? INC : -INC);
		cout << lights[currLight]->pos << endl;
		break;
	case 'Z':
	case 'z': lights[currLight]->pos.z += (isupper(key) ? INC : -INC);
		cout << lights[currLight]->pos << endl;
		break;
	case 'J':
	case 'j':	spotDirX += (isupper(key) ? INC : -INC);
		spotLight->setDir(spotDirX, spotDirY, spotDirZ);
		cout << spotLight->spotDir << endl;
		break;
	case 'K':
	case 'k':	spotDirY += (isupper(key) ? INC : -INC);
		spotLight->setDir(spotDirX, spotDirY, spotDirZ);
		cout << spotLight->spotDir << endl;
		break;
	case 'L':
	case 'l':	spotDirZ += (isupper(key) ? INC : -INC);
		spotLight->setDir(spotDirX, spotDirY, spotDirZ);
		cout << spotLight->spotDir << endl;
		break;
	case 'F':
	case 'f':	incrementClamp(spotLight->fov, isupper(key) ? 0.2 : -0.2, 0.1, PI);
		cout << spotLight->fov << endl;
		break;
	case 'P':
	case 'p':	isAnimated = !isAnimated;
		break;
	case 'C':
	case 'c':
		break;
	case 'U':
	case 'u':	incrementClamp(cameraFOV, isupper(key) ? 0.2 : -0.2, glm::radians(10.0), glm::radians(160.0));
		W = frameBuffer.getWindowWidth();
		H = frameBuffer.getWindowWidth();
		cout << cameraFOV << endl;
		break;
	case 'M':
	case 'm':	break;
	case '+':	antiAliasing = 3;
		cout << "Anti aliasing: " << antiAliasing << endl;
		break;
	case '-':	antiAliasing = 1;
		cout << "Anti aliasing: " << antiAliasing << endl;
		break;

	case '?':	multiViewOn = !multiViewOn;
		break;
	case '0':
	case '1':
	case '2':	numReflections = key - '0';
		cout << "Num reflections: " << numReflections << endl;
		break;
	case 'd':	isAnimated = !isAnimated;
		break;
	case ESCAPE:
		glutLeaveMainLoop();
		break;
	default:
		cout << (int)key << "unmapped key pressed." << endl;
	}

	glutPostRedisplay();
}

int main(int argc, char* argv[]) {
	graphicsInit(argc, argv, __FILE__);

	glutDisplayFunc(render);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouseUtility);
	glutTimerFunc(TIME_INTERVAL, timer, 0);
	buildScene();

	glutMainLoop();

	return 0;
}
