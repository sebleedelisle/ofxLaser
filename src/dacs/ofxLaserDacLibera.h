//
//  ofxLaserDacLibera.h
//  ofxLaser
//
//  Generic ofxLaser DAC wrapper around libera::core::LaserController.
//
//  Design intent:
//  - Keep all transport/discovery logic in manager classes.
//  - Keep this wrapper focused on adapting ofxLaser's DacBase API to an
//    already-connected libera controller instance.
//  - Store only lightweight identity + point-conversion policy here.
//

#pragma once

#include "ofxLaserDacBase.h"
#include "ofxLaserPointConverter.h"

#include "libera/core/LaserController.hpp"

#include <memory>
#include <mutex>

namespace ofxLaser {

class DacLibera : public DacBase {
public:
    // Describes how this adapter should identify itself in the existing
    // ofxLaser UI/state system and how points should be normalized for libera.
    struct Descriptor {
        string type = "Libera";
        string rawId = "";
        PointConverter::Policy pointPolicy{};
    };

    // Constructor-injection pattern:
    // The controller is supplied by a manager that already handled discovery
    // and connection. This avoids hidden side effects in sendFrame()/update.
    explicit DacLibera(Descriptor descriptor,
                       const std::shared_ptr<libera::core::LaserController>& controller);
    ~DacLibera() override;

    bool sendFrame(const vector<Point>& points) override;
    bool setPointsPerSecond(uint32_t newpps) override;
    uint32_t getPointsPerSecond() override;
    bool setColourShift(float shiftSeconds) override;
    void setArmed(bool armed) override;

    string getType() override;
    string getRawId() override;
    int getStatus() override;

    void reset() override;
    void close() override;
    bool isReadyForFrame(int maxLatencyMS) override;

    // Allows a manager to swap controller instances after reconnects while
    // keeping the same wrapper object attached to a Laser.
    void setController(const std::shared_ptr<libera::core::LaserController>& controller);
    void setDescriptor(Descriptor newDescriptor);
    std::shared_ptr<libera::core::LaserController> getController() const;

private:
    std::shared_ptr<libera::core::LaserController> lockController() const;
    void applyRuntimeStateToController(libera::core::LaserController& controller) const;

    mutable std::mutex controllerMutex;
    Descriptor descriptor;
    std::weak_ptr<libera::core::LaserController> controllerWeak;
    std::atomic<uint32_t> configuredPointRate{30000};
};

} // namespace ofxLaser
