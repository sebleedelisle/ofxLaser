//
//  ofxLaserBaseController.h
//
//  Base class for all laser controllers, modelled on Liberation's BaseController.
//
//  Controllers send typed messages through the message bus.
//  ManagerBase receives them and dispatches via std::visit.
//

#pragma once

#include "ofxLaserMsg.h"
#include "ofMain.h"

namespace ofxLaser {

class LaserBaseController {

public:
    virtual ~LaserBaseController() = default;

    // ============================================================
    // Message dispatch
    // ============================================================
    std::function<void(LaserMsgEnvelope&)> laserMessageCallback;

    template <class T>
    void setLaserMessageCallback(T* listener, void (T::*listenerMethod)(LaserMsgEnvelope&)) {
        laserMessageCallback = std::bind(listenerMethod, listener, std::placeholders::_1);
    }

    void sendLaserMessage(LaserAppMsg msg) {
        LaserMsgEnvelope env{ std::move(msg), ofGetElapsedTimef() };
        if(laserMessageCallback) {
            laserMessageCallback(env);
        }
    }

    // ============================================================
    // Convenience methods — wrap typed messages for common actions
    // ============================================================
    void requestAddLaser() {
        sendLaserMessage(LaserMsg::AddLaser{});
    }
    void requestDeleteLaser(int index) {
        sendLaserMessage(LaserMsg::DeleteLaser{index});
    }
    void requestSelectLaser(int index) {
        sendLaserMessage(LaserMsg::SelectLaser{index});
    }
    void requestCreateBeamZone() {
        sendLaserMessage(LaserMsg::CreateBeamZone{});
    }
    void requestArmAll() {
        sendLaserMessage(LaserMsg::ArmAllLasers{});
    }
    void requestDisarmAll() {
        sendLaserMessage(LaserMsg::DisarmAllLasers{});
    }
    void requestSetGlobalBrightness(float value) {
        sendLaserMessage(LaserMsg::SetGlobalBrightness{value});
    }
    void requestToggleAltZones() {
        sendLaserMessage(LaserMsg::ToggleAltZones{});
    }
    void requestSetTestPattern(int pattern, bool active) {
        sendLaserMessage(LaserMsg::SetTestPattern{pattern, active});
    }
    void requestSaveSettings() {
        sendLaserMessage(LaserMsg::SaveSettings{});
    }
    void requestResetAllLasers() {
        sendLaserMessage(LaserMsg::ResetAllLasers{});
    }
    void requestSetCanvasSize(int w, int h) {
        sendLaserMessage(LaserMsg::SetCanvasSize{w, h});
    }
    void requestAddCanvasZone(float x, float y, float w, float h) {
        sendLaserMessage(LaserMsg::AddCanvasZone{x, y, w, h});
    }
    void requestToggleGui() {
        sendLaserMessage(LaserMsg::ToggleGui{});
    }
    void requestSetGuiVisible(bool visible) {
        sendLaserMessage(LaserMsg::SetGuiVisible{visible});
    }
    void requestZoneMuteChanged(const std::string& zoneUid, bool isAlt, bool muted) {
        sendLaserMessage(LaserMsg::ZoneMuteChanged{zoneUid, isAlt, muted});
    }
    void requestZoneLockChanged(const std::string& zoneUid, bool isAlt, bool locked) {
        sendLaserMessage(LaserMsg::ZoneLockChanged{zoneUid, isAlt, locked});
    }

};

} // namespace ofxLaser
