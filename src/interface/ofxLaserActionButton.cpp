#include "ofxLaserActionButton.h"
#include "ofGraphics.h"

ofxLaserActionButton::ofxLaserActionButton() {
}

ofxLaserActionButton::~ofxLaserActionButton() {
}

ofxLaserActionButton* ofxLaserActionButton::setup(const std::string& buttonName,
                                                  float width,
                                                  float height) {
    parameter.setName(buttonName);
    b.set(0, 0, width, height);
    bGuiActive = false;
    checkboxRect.set(1, 1, b.height - 2, b.height - 2);
    registerMouseEvents();
    setNeedsRedraw();
    return this;
}

ofAbstractParameter& ofxLaserActionButton::getParameter() {
    return parameter;
}

bool ofxLaserActionButton::mouseMoved(ofMouseEventArgs& args) {
    return isGuiDrawing() && b.inside(args);
}

bool ofxLaserActionButton::mousePressed(ofMouseEventArgs& args) {
    bool result = setValue(args.x, args.y, true);
    return result;
}

bool ofxLaserActionButton::mouseDragged(ofMouseEventArgs& args) {
    if (bGuiActive && b.inside(args)) {
        return true;
    }
    return false;
}

bool ofxLaserActionButton::mouseReleased(ofMouseEventArgs& args) {
    bool wasActive = bGuiActive;
    bGuiActive = false;
    pressed = false;
    setNeedsRedraw();
    if (wasActive) {
        parameter.trigger();
        return true;
    }
    return false;
}

bool ofxLaserActionButton::mouseScrolled(ofMouseEventArgs& args) {
    return b.inside(args);
}

bool ofxLaserActionButton::setValue(float mx, float my, bool bCheckBounds) {
    if (!isGuiDrawing()) {
        bGuiActive = false;
        return false;
    }

    if (bCheckBounds && b.inside(mx, my)) {
        bGuiActive = true;
        pressed = true;
        setNeedsRedraw();
        return true;
    }

    return false;
}

void ofxLaserActionButton::generateDraw() {
    background.clear();
    background.setFilled(true);
    background.setFillColor(thisBackgroundColor);
    background.rectangle(b);

    checkboxRect.set(1, 1, b.height - 2, b.height - 2);

    border.clear();
    if (pressed) {
        border.setFilled(true);
        border.setFillColor(thisFillColor);
    } else {
        border.setFilled(false);
        border.setStrokeWidth(1);
        border.setStrokeColor(thisFillColor);
    }
    border.rectangle(b.getPosition() + checkboxRect.getTopLeft(), checkboxRect.width, checkboxRect.height);

    auto textX = b.x + textPadding + checkboxRect.width;
    textMesh = getTextMesh(getName(), textX, getTextVCenteredInRect(b));
}

void ofxLaserActionButton::render() {
    background.draw();
    border.draw();

    ofColor previousColor = ofGetStyle().color;
    ofBlendMode previousBlendMode = ofGetStyle().blendingMode;
    if (previousBlendMode != OF_BLENDMODE_ALPHA) {
        ofEnableAlphaBlending();
    }

    ofSetColor(thisTextColor);
    bindFontTexture();
    textMesh.draw();
    unbindFontTexture();
    ofSetColor(previousColor);

    if (previousBlendMode != OF_BLENDMODE_ALPHA) {
        ofEnableBlendMode(previousBlendMode);
    }
}
