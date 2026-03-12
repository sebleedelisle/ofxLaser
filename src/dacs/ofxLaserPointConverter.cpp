//
//  ofxLaserPointConverter.cpp
//  ofxLaser
//

#include "ofxLaserPointConverter.h"

#include <algorithm>

namespace ofxLaser::PointConverter {
namespace {

constexpr float kFallbackCanvasSize = 800.0f;
constexpr float kInv255 = 1.0f / 255.0f;

// Guards against invalid config values so we never divide by zero in hot paths.
inline float getCanvasExtent(float requested) noexcept {
    return requested > 0.0f ? requested : kFallbackCanvasSize;
}

} // namespace

libera::core::LaserPoint convertPoint(const ofxLaser::Point& source,
                                      const Policy& policy) noexcept {
    const float width = getCanvasExtent(policy.canvasWidth);
    const float height = getCanvasExtent(policy.canvasHeight);

    // Flips are applied in source-space before normalization so the
    // normalization math stays consistent for all code paths.
    const float xInput = policy.flipX ? (width - source.x) : source.x;
    const float yInput = policy.flipY ? (height - source.y) : source.y;

    // Map 0..width / 0..height to -1..1.
    const float xScale = 2.0f / width;
    const float yScale = 2.0f / height;

    // Colour is stored as 0..255 in ofxLaser and 0..1 in libera.
    const float r = source.r * kInv255;
    const float g = source.g * kInv255;
    const float b = source.b * kInv255;

    libera::core::LaserPoint point;
    point.x = (xInput * xScale) - 1.0f;
    point.y = (yInput * yScale) - 1.0f;
    point.r = r;
    point.g = g;
    point.b = b;
    point.i = policy.deriveIntensityFromRgb
        ? std::max({r, g, b})
        : std::clamp(policy.defaultIntensity, 0.0f, 1.0f);
    point.u1 = 0.0f;
    point.u2 = 0.0f;
    return point;
}

void convertPoints(const std::vector<ofxLaser::Point>& source,
                   std::vector<libera::core::LaserPoint>& destination,
                   const Policy& policy) noexcept {
    const std::size_t count = source.size();
    destination.resize(count);
    if (count == 0) {
        return;
    }

    const float width = getCanvasExtent(policy.canvasWidth);
    const float height = getCanvasExtent(policy.canvasHeight);
    const float xScale = 2.0f / width;
    const float yScale = 2.0f / height;
    const float defaultIntensity = std::clamp(policy.defaultIntensity, 0.0f, 1.0f);

    // Frame conversion uses simple indexed writes to keep memory access
    // predictable and avoid repeated reallocation checks.
    for (std::size_t i = 0; i < count; i++) {
        const ofxLaser::Point& sourcePoint = source[i];
        libera::core::LaserPoint& destinationPoint = destination[i];

        const float xInput = policy.flipX ? (width - sourcePoint.x) : sourcePoint.x;
        const float yInput = policy.flipY ? (height - sourcePoint.y) : sourcePoint.y;

        const float r = sourcePoint.r * kInv255;
        const float g = sourcePoint.g * kInv255;
        const float b = sourcePoint.b * kInv255;

        destinationPoint.x = (xInput * xScale) - 1.0f;
        destinationPoint.y = (yInput * yScale) - 1.0f;
        destinationPoint.r = r;
        destinationPoint.g = g;
        destinationPoint.b = b;
        destinationPoint.i = policy.deriveIntensityFromRgb ? std::max({r, g, b}) : defaultIntensity;
        destinationPoint.u1 = 0.0f;
        destinationPoint.u2 = 0.0f;
    }
}

} // namespace ofxLaser::PointConverter
