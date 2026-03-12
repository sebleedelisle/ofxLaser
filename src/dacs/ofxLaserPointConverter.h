//
//  ofxLaserPointConverter.h
//  ofxLaser
//
//  Shared point conversion from ofxLaser's point domain into libera's point domain.
//
//  This module gives us one canonical conversion path so each DAC backend
//  doesn't quietly re-implement slightly different math.
//

#pragma once

#include "ofxLaserPoint.h"
#include "libera/core/LaserPoint.hpp"

#include <vector>

namespace ofxLaser::PointConverter {

struct Policy {
    // ofxLaser output-space size after processPoints(). Current default is 800x800.
    float canvasWidth = 800.0f;
    float canvasHeight = 800.0f;

    // Optional compatibility flips for backends that historically expected a
    // different axis orientation.
    bool flipX = false;
    bool flipY = true;

    // Intensity mapping:
    // - true: derive intensity from colour channels using max(r,g,b)
    // - false: force a fixed intensity value
    bool deriveIntensityFromRgb = true;
    float defaultIntensity = 1.0f;
};

// Converts one point. Useful for unit tests and one-off debug paths.
libera::core::LaserPoint convertPoint(const ofxLaser::Point& source,
                                      const Policy& policy = Policy{}) noexcept;

// Converts a full frame in-place into an already-owned destination buffer.
// The function resizes destination and writes by index to avoid push_back churn.
void convertPoints(const std::vector<ofxLaser::Point>& source,
                   std::vector<libera::core::LaserPoint>& destination,
                   const Policy& policy = Policy{}) noexcept;

} // namespace ofxLaser::PointConverter
