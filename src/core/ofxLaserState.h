//
//  ofxLaserState.h
//
//  Read-only snapshot of laser system state, modelled on Liberation's AppState.
//
//  ManagerBase owns one LaserState instance and keeps it current before
//  firing any on*() notification. Views hold a const LaserState* set by
//  ManagerBase::addLaserView() and read from it freely.
//
//  Reference members are always live — no copying needed.
//  Scalar members are written by ManagerBase::syncLaserState() immediately
//  before the corresponding on*() signal fires.
//
//  LaserState has reference members so it cannot be default-constructed or
//  copied — it must be initialised once in ManagerBase's member-initialiser
//  list and never moved.
//

#pragma once

#include <vector>
#include <memory>

namespace ofxLaser {

class Laser;
class ShapeTargetCanvas;
class DacAssigner;
class BitmapMaskManager;

struct LaserState {

    // -------------------------------------------------------------------
    // Live references — always reflect the current state of ManagerBase
    // -------------------------------------------------------------------
    const std::vector<std::shared_ptr<Laser>>&  lasers;
    const std::shared_ptr<ShapeTargetCanvas>&    canvasTarget;
    const DacAssigner&                           dacAssigner;
    const BitmapMaskManager&                     laserMask;

    // -------------------------------------------------------------------
    // Scalar state — written by syncLaserState() before signals fire
    // -------------------------------------------------------------------
    float globalBrightness          = 0.2f;
    bool  testPatternGlobalActive   = false;
    int   testPatternGlobal         = 1;
    bool  allLasersArmed            = false;
    int   numLasers                 = 0;
};

} // namespace ofxLaser
