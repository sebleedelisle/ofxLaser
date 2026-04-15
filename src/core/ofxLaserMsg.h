//
//  ofxLaserMsg.h
//
//  Typed message structs for the Controller → ManagerBase message bus.
//  Modelled on Liberation's AppMsg pattern.
//
//  Each struct documents exactly what data a message carries.
//  Use LaserMsgEnvelope to send messages; ManagerBase receives and std::visits.
//

#pragma once

#include <variant>
#include <memory>
#include <string>
#include <vector>
#include "ofMain.h"
#include "ofxLaserZoneId.h"

namespace ofxLaser {

class Laser;
class OutputZone;
class InputZone;

// ---------------------------------------------------------------------------
// overloaded visitor helper (same pattern as Liberation's AppMsg.h)
// ---------------------------------------------------------------------------
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

// ---------------------------------------------------------------------------
// All message types, grouped by domain.
// ---------------------------------------------------------------------------
namespace LaserMsg {

    // --- Laser management ---
    struct AddLaser {};
    struct DeleteLaser          { int laserIndex; };
    struct SelectLaser          { int laserIndex; };

    // --- Zone management ---
    struct AddZone              { float x; float y; float w; float h; };
    struct DeleteZone           { std::string zoneUid; };
    struct AddZoneToLaser       { std::string zoneUid; int laserIndex; };
    struct ZoneMoved      { std::string zoneUid; ofRectangle rect; };
    struct SetCanvasSize        { int width; int height; };

    // --- Global settings ---
    struct SetGlobalBrightness  { float value; };
    struct SetTestPattern       { int pattern; bool active; };
    struct SetGlobalLatency     { int ms; };

    // --- Arm / disarm ---
    struct ArmAllLasers         {};
    struct DisarmAllLasers      {};

    // --- Zone transform changes ---
    struct ZoneTransformChanged {
        std::string laserUid;       // which laser owns this zone
        std::string zoneUid;        // which zone was edited
        ofJson transformData;       // serialised transform state
    };
    struct ZoneMuteChanged      { std::string zoneUid; bool muted; };
    struct ZoneLockChanged      { std::string zoneUid; bool locked; };
    struct ZoneTypeChanged      { std::string zoneUid; int transformType; };
    struct ZoneResetTransform   { std::string zoneUid; };
    struct DeleteOutputZone     { std::string zoneUid; };

    // --- Mask management ---
    struct MaskChanged          { int laserIndex; int maskIndex; ofJson maskData; };
    struct DeleteMask           { int laserIndex; int maskIndex; };

    // --- View mode ---
    struct SetViewMode          { int mode; };
    struct ToggleGui            {};
    struct SetGuiVisible        { bool visible; };

    // --- Settings persistence ---
    struct SaveSettings         {};
    struct ResetAllLasers       {};

    // --- DAC assignment ---
    struct AssignDac            { std::string dacLabel; int laserIndex; };
    struct DisconnectDac        { int laserIndex; };

    // --- Per-laser settings ---
    struct SetLaserArmed        { int laserIndex; bool armed; };
    struct SetLaserTestPattern  { int laserIndex; int pattern; bool active; };
    struct SetLaserFlipX        { int laserIndex; bool flip; };
    struct SetLaserFlipY        { int laserIndex; bool flip; };
    struct ResetLaserRotation   { int laserIndex; };
    struct ResetLaserOffset     { int laserIndex; };
    struct SetHideContentDuringTestPattern { bool hide; };

    // --- Zone assignment on a laser ---
    struct RemoveZoneFromLaser  { std::string zoneUid; int laserIndex; };
}

// ---------------------------------------------------------------------------
// The variant type holding all possible messages
// ---------------------------------------------------------------------------
using LaserAppMsg = std::variant<
    LaserMsg::AddLaser,
    LaserMsg::DeleteLaser,
    LaserMsg::SelectLaser,

    LaserMsg::AddZone,
    LaserMsg::DeleteZone,
    LaserMsg::AddZoneToLaser,
    LaserMsg::ZoneMoved,
    LaserMsg::SetCanvasSize,

    LaserMsg::SetGlobalBrightness,
    LaserMsg::SetTestPattern,
    LaserMsg::SetGlobalLatency,

    LaserMsg::ArmAllLasers,
    LaserMsg::DisarmAllLasers,

    LaserMsg::ZoneTransformChanged,
    LaserMsg::ZoneMuteChanged,
    LaserMsg::ZoneLockChanged,
    LaserMsg::ZoneTypeChanged,
    LaserMsg::ZoneResetTransform,
    LaserMsg::DeleteOutputZone,

    LaserMsg::MaskChanged,
    LaserMsg::DeleteMask,

    LaserMsg::SetViewMode,
    LaserMsg::ToggleGui,
    LaserMsg::SetGuiVisible,

    LaserMsg::SaveSettings,
    LaserMsg::ResetAllLasers,

    LaserMsg::AssignDac,
    LaserMsg::DisconnectDac,

    LaserMsg::SetLaserArmed,
    LaserMsg::SetLaserTestPattern,
    LaserMsg::SetLaserFlipX,
    LaserMsg::SetLaserFlipY,
    LaserMsg::ResetLaserRotation,
    LaserMsg::ResetLaserOffset,
    LaserMsg::SetHideContentDuringTestPattern,
    LaserMsg::RemoveZoneFromLaser
>;

// ---------------------------------------------------------------------------
// Envelope wrapping a message with a timestamp (for latency compensation)
// ---------------------------------------------------------------------------
struct LaserMsgEnvelope {
    LaserAppMsg payload;
    float time = 0.0f;
};

} // namespace ofxLaser
