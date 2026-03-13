//
//  ofxLaserDacLibera.cpp
//  ofxLaser
//
//  Adapter strategy:
//  - This class never discovers controllers.
//  - It only forwards ofxLaser frame/rate/arm/shift calls into a controller
//    provided by a manager.
//  - Blanking after arm is delegated to libera so we can remove duplicated
//    blanking behavior from legacy ofxLaser DAC paths over time.

#include "ofxLaserDacLibera.h"

#include <algorithm>
#include <chrono>
#include <utility>

using namespace ofxLaser;

DacLibera::DacLibera(Descriptor descriptor,
                     const std::shared_ptr<libera::core::LaserController>& controller)
: descriptor(std::move(descriptor))
, controllerWeak(controller) {
    // ofxLaser should not do legacy per-frame colour shifting for this adapter;
    // libera handles scanner-sync colour delay internally.
    colourShiftImplemented = true;
    if (controller) {
        applyRuntimeStateToController(*controller);
    }
}

DacLibera::~DacLibera() {
    close();
}

std::shared_ptr<libera::core::LaserController> DacLibera::lockController() const {
    return controllerWeak.lock();
}

void DacLibera::applyRuntimeStateToController(libera::core::LaserController& controller) const {
    // When a wrapper is rebound to a new controller, we immediately mirror
    // the user-facing DAC settings so output behavior stays consistent.
    controller.setPointRate(configuredPointRate.load(std::memory_order_relaxed));
    controller.setScannerSync(std::max(0.0, static_cast<double>(colourShift.load(std::memory_order_relaxed))));
    controller.setArmed(armed);
}

bool DacLibera::sendFrame(const vector<Point>& points) {
    std::scoped_lock<std::mutex> lock(controllerMutex);
    std::shared_ptr<libera::core::LaserController> controller = lockController();
    if (!controller) {
        return false;
    }

    if (points.empty()) {
        return true;
    }

    if (!controller->isReadyForNewFrame()) {
        return false;
    }

    libera::core::Frame frame;
    PointConverter::convertPoints(points, frame.points, descriptor.pointPolicy);
    frame.time = std::chrono::steady_clock::now();

    return controller->sendFrame(std::move(frame));
}

bool DacLibera::setPointsPerSecond(uint32_t newpps) {
    configuredPointRate.store(newpps, std::memory_order_relaxed);

    std::scoped_lock<std::mutex> lock(controllerMutex);
    std::shared_ptr<libera::core::LaserController> controller = lockController();
    if (controller) {
        controller->setPointRate(newpps);
    }
    return true;
}

uint32_t DacLibera::getPointsPerSecond() {
    return configuredPointRate.load(std::memory_order_relaxed);
}

bool DacLibera::setColourShift(float shiftSeconds) {
    colourShift.store(shiftSeconds, std::memory_order_relaxed);
    const double tenThousandths = std::max(0.0, static_cast<double>(shiftSeconds));

    std::scoped_lock<std::mutex> lock(controllerMutex);
    std::shared_ptr<libera::core::LaserController> controller = lockController();
    if (controller) {
        controller->setScannerSync(tenThousandths);
    }
    return true;
}

void DacLibera::setArmed(bool state) {
    // We intentionally bypass DacBase::setArmed blank-point behavior here and
    // rely on libera's own startup/arm blanking semantics.
    armed = state;
    blankPointsAfterReArmRemaining = 0;

    std::scoped_lock<std::mutex> lock(controllerMutex);
    std::shared_ptr<libera::core::LaserController> controller = lockController();
    if (controller) {
        controller->setArmed(state);
        if (state) {
            controller->resetStartupBlank();
        }
    }
}

string DacLibera::getType() {
    return descriptor.type;
}

string DacLibera::getRawId() {
    return descriptor.rawId;
}

int DacLibera::getStatus() {
    std::shared_ptr<libera::core::LaserController> controller;
    {
        std::scoped_lock<std::mutex> lock(controllerMutex);
        controller = lockController();
    }
    if (!controller) {
        return OFXLASER_DACSTATUS_ERROR;
    }

    switch (controller->getStatus()) {
        case libera::core::ControllerStatus::Good:
            return OFXLASER_DACSTATUS_GOOD;
        case libera::core::ControllerStatus::Issues:
            return OFXLASER_DACSTATUS_WARNING;
        case libera::core::ControllerStatus::Error:
            return OFXLASER_DACSTATUS_ERROR;
    }

    return OFXLASER_DACSTATUS_ERROR;
}

void DacLibera::reset() {
    // Reset strategy for generic libera wrappers:
    // ask the current controller to stop and let the manager reconnect/create
    // a replacement controller if needed.
    close();
}

void DacLibera::close() {
    std::scoped_lock<std::mutex> lock(controllerMutex);
    std::shared_ptr<libera::core::LaserController> controller = lockController();
    if (controller) {
        controller->stop();
    }
    controllerWeak.reset();
}

bool DacLibera::isReadyForFrame(int maxLatencyMS) {
    this->maxLatencyMS = maxLatencyMS;

    std::scoped_lock<std::mutex> lock(controllerMutex);
    std::shared_ptr<libera::core::LaserController> controller = lockController();
    if (!controller) {
        return false;
    }

    // Legacy API asks "are we ready for a frame at this latency?". Libera
    // exposes a global target latency for frame mode, so we update that first.
    libera::core::LaserController::setTargetRenderLatency(std::chrono::milliseconds(maxLatencyMS));
    return controller->isReadyForNewFrame();
}

void DacLibera::setController(const std::shared_ptr<libera::core::LaserController>& controller) {
    std::scoped_lock<std::mutex> lock(controllerMutex);
    controllerWeak = controller;
    if (controller) {
        applyRuntimeStateToController(*controller);
    }
}

void DacLibera::setDescriptor(Descriptor newDescriptor) {
    std::scoped_lock<std::mutex> lock(controllerMutex);
    descriptor = std::move(newDescriptor);
}

std::shared_ptr<libera::core::LaserController> DacLibera::getController() const {
    std::scoped_lock<std::mutex> lock(controllerMutex);
    return lockController();
}
