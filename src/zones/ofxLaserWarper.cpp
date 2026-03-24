//
//  ofxLaserWarper.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/02/2018.
//
//

#include "ofxLaserWarper.h"
#include <cmath>

using namespace ofxLaser; 

namespace {
constexpr int kHomographyEquationCount = 8;
constexpr double kHomographySolveEpsilon = 1e-10;
constexpr float kHomographyApplyEpsilon = 1e-8f;

bool solveHomography(
    const std::array<glm::vec2, 4>& srcPoints,
    const std::array<glm::vec2, 4>& dstPoints,
    std::array<double, 8>& solution
) {
    std::array<std::array<double, 9>, kHomographyEquationCount> augmented{};

    for(size_t i = 0; i < srcPoints.size(); i++) {
        const double x = srcPoints[i].x;
        const double y = srcPoints[i].y;
        const double u = dstPoints[i].x;
        const double v = dstPoints[i].y;
        const size_t row = i * 2;

        augmented[row] = {x, y, 1.0, 0.0, 0.0, 0.0, -u * x, -u * y, u};
        augmented[row + 1] = {0.0, 0.0, 0.0, x, y, 1.0, -v * x, -v * y, v};
    }

    for(int col = 0; col < kHomographyEquationCount; col++) {
        int pivotRow = col;
        double pivotAbs = fabs(augmented[col][col]);

        for(int row = col + 1; row < kHomographyEquationCount; row++) {
            const double candidateAbs = fabs(augmented[row][col]);
            if(candidateAbs > pivotAbs) {
                pivotAbs = candidateAbs;
                pivotRow = row;
            }
        }

        if(pivotAbs <= kHomographySolveEpsilon) {
            return false;
        }

        if(pivotRow != col) {
            std::swap(augmented[pivotRow], augmented[col]);
        }

        const double pivot = augmented[col][col];
        for(int valueIndex = col; valueIndex <= kHomographyEquationCount; valueIndex++) {
            augmented[col][valueIndex] /= pivot;
        }

        for(int row = 0; row < kHomographyEquationCount; row++) {
            if(row == col) {
                continue;
            }

            const double factor = augmented[row][col];
            if(fabs(factor) <= kHomographySolveEpsilon) {
                continue;
            }

            for(int valueIndex = col; valueIndex <= kHomographyEquationCount; valueIndex++) {
                augmented[row][valueIndex] -= factor * augmented[col][valueIndex];
            }
        }
    }

    for(int i = 0; i < kHomographyEquationCount; i++) {
        solution[i] = augmented[i][kHomographyEquationCount];
    }

    return true;
}

glm::mat3 buildHomographyMatrix(const std::array<double, 8>& coefficients) {
    glm::mat3 matrix(1.0f);
    matrix[0][0] = static_cast<float>(coefficients[0]);
    matrix[1][0] = static_cast<float>(coefficients[1]);
    matrix[2][0] = static_cast<float>(coefficients[2]);
    matrix[0][1] = static_cast<float>(coefficients[3]);
    matrix[1][1] = static_cast<float>(coefficients[4]);
    matrix[2][1] = static_cast<float>(coefficients[5]);
    matrix[0][2] = static_cast<float>(coefficients[6]);
    matrix[1][2] = static_cast<float>(coefficients[7]);
    matrix[2][2] = 1.0f;
    return matrix;
}
}



void Warper::updateHomography(glm::vec2 src1, glm::vec2 src2, glm::vec2 src3, glm::vec2 src4, glm::vec2 dst1, glm::vec2 dst2, glm::vec2 dst3, glm::vec2 dst4) {
	
    //ofLogNotice("Warper::updateHomography") << ofGetFrameNum() << " Src : (" << src1 << "), (" << src2 <<  "), ("  << src3 <<  "), ("  << src4 <<  ") Dst :  ("  << dst1 <<  "), ("  << dst2 <<  "), ("  << dst3 <<  "), ("  << dst4 <<")";
    
	// the source points are the zone points in screen space
	// the dest points are points in the output space
	
	srcPoints = {src1, src2, src3, src4};
	dstPoints = {dst1, dst2, dst3, dst4};

    std::array<double, 8> coefficients{};
    homographyValid = solveHomography(srcPoints, dstPoints, coefficients);

    if(homographyValid) {
        homography = buildHomographyMatrix(coefficients);
        const float determinant = glm::determinant(homography);
        homographyValid = std::isfinite(determinant) && (fabs(determinant) > kHomographyApplyEpsilon);
    }

    if(homographyValid) {
        inverseHomography = glm::inverse(homography);
    } else {
        homography = glm::mat3(1.0f);
        inverseHomography = glm::mat3(1.0f);
    }
}


glm::vec3 Warper::getWarpedPoint(const glm::vec3& p, bool useHomography){

	glm::vec2 warpedPoint = getWarpedPoint(p.x, p.y, useHomography);
	return glm::vec3(warpedPoint.x, warpedPoint.y, 0);



}



ofxLaser::Point Warper::getWarpedPoint(const ofxLaser::Point& p, bool useHomography){

	glm::vec2 cvp = getWarpedPoint(p.x, p.y, useHomography);
	ofxLaser::Point lp = p;
	lp.x = cvp.x;
	lp.y = cvp.y;
    
	return lp;

}
glm::vec2 Warper :: getWarpedPoint(float x, float y, bool useHomography) {


	if(useHomography && homographyValid) {
		return applyHomography(homography, {x, y});
	} else {
        return getWarpedBilinearPoint(x, y);
	}
}

glm::vec2 Warper::applyHomography(const glm::mat3& matrix, const glm::vec2& point) const {
    glm::vec3 projectedPoint = matrix * glm::vec3(point, 1.0f);
    if((!std::isfinite(projectedPoint.x))
       || (!std::isfinite(projectedPoint.y))
       || (!std::isfinite(projectedPoint.z))
       || (fabs(projectedPoint.z) <= kHomographyApplyEpsilon)) {
        return point;
    }

    return {
        projectedPoint.x / projectedPoint.z,
        projectedPoint.y / projectedPoint.z
    };
}

glm::vec2 Warper::getWarpedBilinearPoint(float x, float y) const {
//		P is the linear interpolation of A and B in u: P = A + (B-A)·u
//		Q is the linear interpolation of D and C in u: Q = D + (C-D)·u
//		X is the linear interpolation of P and Q in v: X = P + (Q-P)·v
//
//				therefore
//
//		X(u,v) = A + (B-A)·u + (D-A)·v + (A-B+C-D)·u·v

    glm::vec2 d = srcPoints[3] - srcPoints[0];
    if((fabs(d.x) <= kHomographyApplyEpsilon) || (fabs(d.y) <= kHomographyApplyEpsilon)) {
        return {x, y};
    }

    float u = (x - srcPoints[0].x) / d.x;
    float v = (y - srcPoints[0].y) / d.y;
    const glm::vec2& A = dstPoints[0];
    const glm::vec2& B = dstPoints[1];
    const glm::vec2& C = dstPoints[3];
    const glm::vec2& D = dstPoints[2];

    return A + (B - A) * u + (D - A) * v + (A - B + C - D) * u * v;
}


glm::vec3 Warper::getUnWarpedPoint(const glm::vec3& p, bool useHomography){

    if(!useHomography || !homographyValid) {
        return p;
    }

    glm::vec2 unwarpedPoint = applyHomography(inverseHomography, {p.x, p.y});
    return glm::vec3(unwarpedPoint.x, unwarpedPoint.y, 0.0f);

	
}


ofxLaser::Point Warper::getUnWarpedPoint(const ofxLaser::Point& p, bool useHomography){
    if(!useHomography || !homographyValid) {
        return p;
    }

    glm::vec2 unwarpedPoint = applyHomography(inverseHomography, {p.x, p.y});
    ofxLaser::Point point = p;
    point.x = unwarpedPoint.x;
    point.y = unwarpedPoint.y;
    return point;
	
	
	
}
