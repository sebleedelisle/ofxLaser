#include "ofxSvgExtra.h"

#if defined(OFXLASER_DISABLE_SVG_SUPPORT)

#include "ofLog.h"

namespace {

bool& loggedSvgFallback() {
    static bool logged = false;
    return logged;
}

void logSvgFallbackOnce() {
    if (!loggedSvgFallback()) {
        ofLogWarning("ofxLaser") << "SVG support is disabled for this MSYS2 build";
        loggedSvgFallback() = true;
    }
}

} // namespace

ofxSVGExtra::~ofxSVGExtra() = default;

float ofxSVGExtra::getWidth() const {
    return width;
}

float ofxSVGExtra::getHeight() const {
    return height;
}

void ofxSVGExtra::load(std::string) {
    logSvgFallbackOnce();
    width = 0.0f;
    height = 0.0f;
    paths.clear();
}

void ofxSVGExtra::loadFromString(std::string, std::string) {
    logSvgFallbackOnce();
    width = 0.0f;
    height = 0.0f;
    paths.clear();
}

void ofxSVGExtra::setImprovedCompatibilityMode(bool enabled) {
    increaseCompatibility = enabled;
}

void ofxSVGExtra::draw(bool) {
}

int ofxSVGExtra::getNumPath() {
    return static_cast<int>(paths.size());
}

ofPath& ofxSVGExtra::getPathAt(int n) {
    return paths.at(static_cast<std::size_t>(n));
}

const std::vector<ofPath>& ofxSVGExtra::getPaths() const {
    return paths;
}

#endif
