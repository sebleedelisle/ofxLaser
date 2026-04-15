//
//  ofxLaserBaseView.h
//
//  Base class for all laser views, modelled on Liberation's BaseView.
//
//  ManagerBase calls the on*() signals after updating LaserState.
//  Subclasses override to react to state changes.
//  Views hold a const LaserState* set by ManagerBase::addLaserView().
//

#pragma once

#include "ofxLaserState.h"

namespace ofxLaser {

class LaserBaseView {

public:
    virtual ~LaserBaseView() = default;

    // ============================================================
    // Lifecycle hooks — override in subclasses
    // ============================================================
    virtual void laserViewUpdate() {}
    virtual void laserViewDraw() {}
    virtual void laserViewDrawGui() {}

    // ============================================================
    // Thin notification signals — ManagerBase calls these after
    // updating LaserState. Override in subclasses to respond.
    // ============================================================
    virtual void onLasersChanged() {}
    virtual void onZonesChanged() {}
    virtual void onCanvasChanged() {}
    virtual void onDacStatusChanged() {}
    virtual void onGlobalSettingsChanged() {}
    virtual void onTestPatternChanged() {}
    virtual void onLaserSelectionChanged() {}
    virtual void onMasksChanged() {}

    // ============================================================
    // The read-only window into ManagerBase — set by addLaserView()
    // ============================================================
    const LaserState* laserState = nullptr;

};

} // namespace ofxLaser
