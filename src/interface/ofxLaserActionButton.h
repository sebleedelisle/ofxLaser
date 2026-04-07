#pragma once

#include "ofParameter.h"
#include "ofxBaseGui.h"

class ofxLaserActionButton : public ofxBaseGui {
public:
    ofxLaserActionButton();
    ~ofxLaserActionButton() override;

    ofxLaserActionButton* setup(const std::string& buttonName,
                                float width = defaultWidth,
                                float height = defaultHeight);

    template<class ListenerClass, typename ListenerMethod>
    void addListener(ListenerClass* listener, ListenerMethod method) {
        parameter.addListener(listener, method);
    }

    template<class ListenerClass, typename ListenerMethod>
    void removeListener(ListenerClass* listener, ListenerMethod method) {
        parameter.removeListener(listener, method);
    }

    template<typename Listener>
    auto newListener(Listener&& listener) {
        return parameter.newListener(std::forward<Listener>(listener));
    }

    ofAbstractParameter& getParameter() override;
    bool mouseMoved(ofMouseEventArgs& args) override;
    bool mousePressed(ofMouseEventArgs& args) override;
    bool mouseDragged(ofMouseEventArgs& args) override;
    bool mouseReleased(ofMouseEventArgs& args) override;
    bool mouseScrolled(ofMouseEventArgs& args) override;

protected:
    void render() override;
    bool setValue(float mx, float my, bool bCheckBounds) override;
    void generateDraw() override;

private:
    ofParameter<void> parameter;
    bool bGuiActive = false;
    bool pressed = false;
    ofRectangle checkboxRect;
    ofPath background;
    ofPath border;
    ofVboMesh textMesh;
};
