/****************************************************
 * 2016-2022 Eric Bachmann and Mike Zmuda
 * All Rights Reserved.
 * NOTICE:
 * Dissemination of this information or reproduction
 * of this material is prohibited unless prior written
 * permission is granted.
 ****************************************************/

#include <iostream>
#include <vector>
#include "defs.h"
#include "light.h"

int main(int argc, char* argv[]) {
	cout << SpotLight::isInSpotlightCone(dvec3(0.0, 1.0, 0.0), -Y_AXIS, PI_2, dvec3(0.00, 0, 0)) << endl;  // 1
	cout << SpotLight::isInSpotlightCone(dvec3(0.0, 1.0, 0.0), -Y_AXIS, PI_2, dvec3(0.99, 0, 0)) << endl;  // 1
	cout << SpotLight::isInSpotlightCone(dvec3(0.0, 1.0, 0.0), -Y_AXIS, PI_2, dvec3(1.01, 0, 0)) << endl;  // 0

	cout << SpotLight::isInSpotlightCone(dvec3(3.0, 1.0, 0.0), dvec3(-1, -1, -1), PI_2, dvec3(2, 0, 0)) << endl;  // 1
	cout << SpotLight::isInSpotlightCone(dvec3(3.0, 1.0, 0.0), dvec3(1, -1, -1), PI_2, dvec3(2, 0, 0)) << endl;  // 0
	return 0;
}