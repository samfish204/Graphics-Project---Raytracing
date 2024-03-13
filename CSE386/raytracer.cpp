/****************************************************
 * 2016-2022 Eric Bachmann and Mike Zmuda
 * All Rights Reserved.
 * NOTICE:
 * Dissemination of this information or reproduction
 * of this material is prohibited unless prior written
 * permission is granted.
 ****************************************************/
#include "raytracer.h"
#include "ishape.h"
#include "io.h"

// Samuel Fisher (fishe108)
// CSE 386
// Dr. Zmuda
// Due: April 19, 2022

 /**
  * @fn	RayTracer::RayTracer(const color &defa)
  * @brief	Constructs a raytracers.
  * @param	defa	The clear color.
  */

RayTracer::RayTracer(const color& defa)
	: defaultColor(defa) {
}

/**
 * @fn	void RayTracer::raytraceScene(FrameBuffer &frameBuffer, int depth, const IScene &theScene) const
 * @brief	Raytrace scene
 * @param [in,out]	frameBuffer	Framebuffer.
 * @param 		  	depth	   	The current depth of recursion.
 * @param 		  	theScene   	The scene.
 */

void RayTracer::raytraceScene(FrameBuffer& frameBuffer, int depth,
	const IScene& theScene, int n) const {
	const RaytracingCamera& camera = *theScene.camera;
	const vector<VisibleIShapePtr>& objs = theScene.opaqueObjs;
	const vector<PositionalLightPtr>& lights = theScene.lights;

	for (int y = 0; y < frameBuffer.getWindowHeight(); ++y) {
		for (int x = 0; x < frameBuffer.getWindowWidth(); ++x) {
			DEBUG_PIXEL = (x == xDebug && y == yDebug);
			if (DEBUG_PIXEL) {
				cout << "";
			}
			Ray ray = camera.getRay(x, y);
			if (n == 1) {
				color C = black;
				C = traceIndividualRay(ray, theScene, depth);
				frameBuffer.setColor(x, y, C);
			}
			if (n == 3) {
				vector<Ray> rays;
				color C = black;
				rays.push_back(Ray(camera.getRay(x + .167, y + .167)));
				rays.push_back(Ray(camera.getRay(x + .167 + .333, y + .167)));
				rays.push_back(Ray(camera.getRay(x + .167 + .666, y + .167)));

				rays.push_back(Ray(camera.getRay(x + .167, y + .167 + .333)));
				rays.push_back(Ray(camera.getRay(x + .167 + .333, y + .167 + .333)));
				rays.push_back(Ray(camera.getRay(x + .167 + .667, y + .167 + .333)));

				rays.push_back(Ray(camera.getRay(x + .167, y + .167 + .666)));
				rays.push_back(Ray(camera.getRay(x + .167 + .333, y + .167 + .666)));
				rays.push_back(Ray(camera.getRay(x + .167 + .666, y + .167 + .666)));

				for (int i = 0; i < 9; i++) {
					C += traceIndividualRay(rays[i], theScene, depth);
				}
				C /= 9;
				frameBuffer.setColor(x, y, C);
			}
		frameBuffer.showAxes(x, y, ray, 0.25);
		}
	}
	frameBuffer.showColorBuffer();
}

/**
 * @fn	color RayTracer::traceIndividualRay(const Ray &ray,
 *											const IScene &theScene,
 *											int recursionLevel) const
 * @brief	Trace an individual ray.
 * @param	ray			  	The ray.
 * @param	theScene	  	The scene.
 * @param	recursionLevel	The recursion level.
 * @return	The color to be displayed as a result of this ray.
 */

color RayTracer::traceIndividualRay(const Ray& ray, const IScene& theScene, int recursionLevel) const {
	OpaqueHitRecord hit;
	TransparentHitRecord transHit;
	VisibleIShape::findIntersection(ray, theScene.opaqueObjs, hit);
	TransparentIShape::findIntersection(ray, theScene.transparentObjs, transHit);
	color temp, C = black;
	for (int i = 0; i < theScene.lights.size(); i++) {
		if (hit.t != FLT_MAX && transHit.t == FLT_MAX) {
			bool shadow = theScene.lights[i]->pointIsInAShadow(hit.interceptPt, hit.normal, theScene.opaqueObjs, theScene.camera->getFrame());
			C = theScene.lights[i]->illuminate(hit.interceptPt, hit.normal, hit.material, theScene.camera->getFrame(), shadow);
			if (hit.texture != nullptr) {
				C = hit.texture->getPixelUV(hit.u, hit.v);
			}
			temp += C;
		}
		else if (hit.t == FLT_MAX && transHit.t != FLT_MAX) {
			C = (black) * (1 - transHit.alpha) + (transHit.alpha) * (transHit.transColor);
			temp += C;
		}
		else if (hit.t != FLT_MAX && transHit.t != FLT_MAX) {
			if (transHit.t < hit.t) { // transparent hit is closer
				bool shadow = theScene.lights[i]->pointIsInAShadow(hit.interceptPt, hit.normal, theScene.opaqueObjs, theScene.camera->getFrame());
				C = theScene.lights[i]->illuminate(hit.interceptPt, hit.normal, hit.material, theScene.camera->getFrame(), shadow);
				C = C * (1 - transHit.alpha) + (transHit.alpha) * (transHit.transColor);
				if (hit.texture != nullptr) {
					C = hit.texture->getPixelUV(hit.u, hit.v);
					C = C * (1 - transHit.alpha) + (transHit.alpha) * (transHit.transColor);
				}
				temp += C;
			}
			else {
				bool shadow = theScene.lights[i]->pointIsInAShadow(hit.interceptPt, hit.normal, theScene.opaqueObjs, theScene.camera->getFrame());
				C = theScene.lights[i]->illuminate(hit.interceptPt, hit.normal, hit.material, theScene.camera->getFrame(), shadow);
				if (hit.texture != nullptr) {
					C = hit.texture->getPixelUV(hit.u, hit.v);
					C = C * (1 - transHit.alpha) + (transHit.alpha) * (transHit.transColor);
				}
				temp += C;
			}
		}
	}
	if (recursionLevel > 0) {
		glm::dvec3 i = glm::normalize(ray.dir);
		glm::dvec3 n = glm::normalize(hit.normal);

		glm::dvec3 rDirection = i - 2.0 * glm::dot(i, n) * n;
		glm::dvec3 rOrigin = hit.interceptPt + EPSILON * n;

		Ray rRay = Ray(rOrigin, rDirection);

		color rColor = traceIndividualRay(rRay, theScene, recursionLevel - 1);
		return temp + rColor * 0.3;
	}
	return temp;
}
