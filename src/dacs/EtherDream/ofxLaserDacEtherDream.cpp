//
//  ofxLaserDacEtherDream.cpp
//  ofxLaser
//
//  Modernised to use libera-core's EtherDreamDevice directly.
//

#include "ofxLaserDacEtherDream.h"
#include "libera/core/LaserPoint.hpp"
#include "libera/log/Log.hpp"
#include <algorithm>
#include <chrono>

using namespace ofxLaser;

DacEtherDream::DacEtherDream() {
    dacTotalPointBufferCapacity = 1799;
    colourShiftImplemented = true;
    verbose = false;
    networkConnected = false;
}

DacEtherDream::~DacEtherDream() {
    close();
}

libera::core::Frame DacEtherDream::makeLiberaFrame(const vector<Point>& points) {
    libera::core::Frame frame;
    frame.points.reserve(points.size());
    const float inv255 = 1.0f / 255.0f;
    for (const auto& p : points) {
        libera::core::LaserPoint lp;
        lp.x = ofMap(p.x, 0.0f, 800.0f, -1.0f, 1.0f, true);
        lp.y = ofMap(p.y, 800.0f, 0.0f, -1.0f, 1.0f, true); // flip Y to match old EtherDream mapping
        lp.r = p.r * inv255;
        lp.g = p.g * inv255;
        lp.b = p.b * inv255;
        lp.i = std::max({lp.r, lp.g, lp.b});
        frame.points.push_back(lp);
    }
    frame.time = std::chrono::steady_clock::now();
    return frame;
}

bool DacEtherDream::sendFrame(const vector<Point>& points) {
    if (liberaDevice) {
        if (!liberaDevice->frameModeEnabled()) {
            liberaDevice->startFrameMode();
        }
        return liberaDevice->sendFrame(makeLiberaFrame(points));
    }
    return false;
}

void DacEtherDream::setup(string _id, string _ip, EtherDreamData& ed) {
    pps = 30000; // default rate
    ipAddress = _ip;
    id = _id;
    etherDreamData = ed;
    networkConnected = false;

    versionNumber = 0;
    if(ed.hardwareRevision == 2) versionNumber = 1;
    else if(ed.hardwareRevision == 10) versionNumber = 2;
    else if(ed.hardwareRevision == 30) versionNumber = 3;
    else if((ed.hardwareRevision == 40) || (ed.hardwareRevision == 41)) versionNumber = 4;

    isMercury = (ed.hardwareRevision == 2) && (ed.softwareRevision == 2) && (ed.bufferCapacity != 1799);
    versionString = "v"+ofToString(versionNumber);
    if(versionNumber == 0) versionString = "(virtual)";

    unsigned short port = 7765;
    if(ed.hardwareRevision == 0) {
        port = static_cast<unsigned short>(port + ed.softwareRevision);
    }

    dacTotalPointBufferCapacity = ed.bufferCapacity;

    liberaInfo = libera::etherdream::EtherDreamDeviceInfo(id,
                                                          getHardwareName(),
                                                          ipAddress,
                                                          port,
                                                          ed.bufferCapacity,
                                                          versionString,
                                                          ed.maxPointRate);
    liberaDevice = std::make_shared<libera::etherdream::EtherDreamDevice>(*liberaInfo);
    liberaDevice->setPointRate(pps);

    if (auto result = liberaDevice->connect(); !result) {
        ofLogError() << "DacEtherDream (libera) connect failed for " << ipAddress;
        liberaDevice.reset();
        networkConnected = false;
        return;
    }

    liberaDevice->start();
    networkConnected = true;
}

void DacEtherDream::closeWhileRunning() {
    close();
}

void DacEtherDream::close() {
    if (liberaDevice) {
        liberaDevice->stop();
        liberaDevice->close();
        liberaDevice.reset();
    }
    networkConnected = false;
}

void DacEtherDream::reset() {
    close();
    if (liberaInfo) {
        setup(id, ipAddress, etherDreamData);
    }
}

string DacEtherDream::getRawId() {
    return id;
}

string DacEtherDream::getEdId() {
    return id;
}

int DacEtherDream::getStatus() {
    return networkConnected ? OFXLASER_DACSTATUS_GOOD : OFXLASER_DACSTATUS_ERROR;
}

const vector<ofAbstractParameter*>& DacEtherDream::getDisplayData() {
    return displayData;
}

EtherDreamData DacEtherDream::getEtherDreamData() {
    return etherDreamData;
}

string DacEtherDream::getEtherDreamStateString() {
    return networkConnected ? "Connected (libera)" : "Disconnected";
}

bool DacEtherDream::setPointsPerSecond(uint32_t newpps){
    if (liberaDevice) {
        liberaDevice->setPointRate(newpps);
    }
    pps = newpps;
    return true;
}

bool DacEtherDream::setColourShift(float shiftSeconds) {
    // colourShift in ofxLaser is expressed in 1/10,000s (same units as libera scanner sync)
    const double tenThousandths = std::max(0.0, static_cast<double>(shiftSeconds));
    if (liberaDevice) {
        liberaDevice->setScannerSync(tenThousandths);
    }
    colourShift = shiftSeconds;
    colourShiftImplemented = true;
    return true;
}
