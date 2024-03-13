/****************************************************
 * 2016-2022 Eric Bachmann and Mike Zmuda
 * All Rights Reserved.
 * NOTICE:
 * Dissemination of this information or reproduction
 * of this material is prohibited unless prior written
 * permission is granted.
 ****************************************************/

#include "eshape.h"

 /**
  * @fn	EShapeData EShape::createEDisk(const Material &mat, int slices)
  * @brief	Creates a disk with radius 1, centered on origin and lying at z = 0
  * @param	mat   	Material.
  * @param	slices	Number of slices.
  * @return	The new disk.
  */

EShapeData EShape::createEDisk(const Material& mat, int slices) {
	EShapeData result;

	double angleInc = TWO_PI / slices;

	for (int i = 0; i < slices; i++) {
		double A1 = i * angleInc;
		double A2 = A1 + angleInc;
		dvec4 A(0.0, 0.0, 0.0, 1.0);
		dvec4 B(std::cos(A1), std::sin(A1), 0.0, 1.0);
		dvec4 C(std::cos(A2), std::sin(A2), 0.0, 1.0);
		VertexData::addTriVertsAndComputeNormal(result, A, B, C, mat);
	}

	return result;
}

/**
 * @fn	EShapeData EShape::createECylinder(const Material &mat, int slices)
 * @brief	Creates cylinder, which is centered on (0,0,0) and aligned with y axis and with
 *			height = 1 and radius = 1
 * @param	mat   	Material.
 * @param	slices	Slices.
 * @return	The new cylinder.
 */
 
EShapeData EShape::createECylinder(const Material& mat, int slices) {
	EShapeData result;

	double angle = TWO_PI / slices;

	for (int i = 0; i < slices; i++) {
		double A1 = i * angle;
		double A2 = A1 + angle;
		dvec4 A(std::sin(A1), 0.5, std::cos(A1), 1);
		dvec4 B(std::sin(A2), 0.5, std::cos(A2), 1);
		dvec4 C(std::sin(A1), -0.5, std::cos(A1), 1);
		dvec4 D(std::sin(A2), -0.5, std::cos(A2), 1);
		VertexData::addTriVertsAndComputeNormal(result, A, B, C, mat);
		VertexData::addTriVertsAndComputeNormal(result, B, D, C, mat);
	}
	return result;
}

/**
 * @fn	EShapeData EShape::createECone(const Material &mat, int slices)
 * @brief	Creates cone, which is aligned with y axis. Height and radius = 1
 * @param	mat   	Material.
 * @param	slices	Slices.
 * @return	The new cone.
 */

EShapeData EShape::createECone(const Material& mat, int slices) {
	/* CSE 386 - todo  */
	EShapeData result;
	double angle = TWO_PI / slices;
	for (int i = 0; i < slices; i++) {
		double A1 = i * angle;
		double A2 = A1 + angle;
		dvec4 A(0.0, 1.0, 0.0, 1.0);
		dvec4 B(std::cos(A1), 0.0, std::sin(A1), 1);
		dvec4 C(std::cos(A2), 0.0, std::sin(A2), 1);
		VertexData::addTriVertsAndComputeNormal(result, A, C, B, mat);
	}
	return result;
}

/**
 * @fn	EShapeData EShape::createETriangle(const Material &mat,
 *											const dvec4& A, const dvec4& B, const dvec4& C)
 * @brief	Creates one triangles from 3 vertices
 * @param	mat	Material.
 * @param	A  	First vertex.
 * @param	B  	Second vertex.
 * @param	C  	Third vertex.
 * @return	The new triangles.
 */

EShapeData EShape::createETriangle(const Material& mat,
	const dvec4& A, const dvec4& B, const dvec4& C) {
	EShapeData result;
	VertexData::addTriVertsAndComputeNormal(result, A, B, C, mat);
	return result;
}

/**
 * @fn	EShapeData EShape::createECheckerBoard(const Material &mat1, const Material &mat2, double WIDTH, double HEIGHT, int DIV)
 * @brief	Creates checker board pattern.
 * @param	mat1  	Material #1.
 * @param	mat2  	Material #2.
 * @param	WIDTH 	Width of overall plane.
 * @param	HEIGHT	Height of overall plane.
 * @param	DIV   	Number of divisions.
 * @return	The vertices in the checker board.
 */

EShapeData EShape::createECheckerBoard(const Material& mat1, const Material& mat2,
	double WIDTH, double HEIGHT, int DIV) {
	EShapeData result;

	const double INC = WIDTH / DIV;
	for (int X = 0; X < DIV; X++) {
		bool isMat1 = X % 2 == 0;
		for (double Z = 0; Z < DIV; Z++) {
			dvec4 V0(-WIDTH / 2.0 + X * INC, 0.0, -WIDTH / 2 + Z * INC, 1.0);
			dvec4 V1 = V0 + dvec4(0.0, 0.0, INC, 0.0);
			dvec4 V2 = V0 + dvec4(INC, 0.0, INC, 0.0);
			dvec4 V3 = V0 + dvec4(INC, 0.0, 0.0, 0.0);
			const Material& mat = isMat1 ? mat1 : mat2;

			result.push_back(VertexData(V0, Y_AXIS, mat));
			result.push_back(VertexData(V1, Y_AXIS, mat));
			result.push_back(VertexData(V2, Y_AXIS, mat));

			result.push_back(VertexData(V2, Y_AXIS, mat));
			result.push_back(VertexData(V3, Y_AXIS, mat));
			result.push_back(VertexData(V0, Y_AXIS, mat));
			isMat1 = !isMat1;
		}
	}
	return result;
}
