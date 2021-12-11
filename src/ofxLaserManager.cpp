//
//  ofxLaserManager.cpp
//  example_HelloLaser 2
//
//  Created by Seb Lee-Delisle on 06/05/2021.
//

#include "ofxLaserManager.h"

using namespace ofxLaser;

Manager :: Manager() {
    selectedLaser = -1;
    guiIsVisible = true;
    guiLaserSettingsPanelWidth = 320;
    guiSpacing = 8;
    draggingPreview = false;
    previewScale = 1;
    
    setDefaultPreviewOffsetAndScale();
    
    params.add(previewNavigationEnabled.set("Enable preview navigation", false));
    params.add(showGuideImage.set("Show guide image", false));
    params.add(guideImageColour.set("Guide image colour", ofColor::white));
    // bit of a hack - ideally UI elements should be completely separate from
    // the ManagerBase but I'm not quite there yet.
    if(!loadJson.empty()) {
        if(loadJson.contains("Laser")) {
            //cout << loadJson["Laser"]["Show_guide_image"].dump(3)<< endl;
            //cout << loadJson["Laser"]["Guide_image_colour"].dump(3)<< endl;
            //cout << showGuideImage << " " <<loadJson["Laser"]["Show_guide_image"]<< endl;
            try {
                ofDeserialize(loadJson["Laser"], previewNavigationEnabled);
                ofDeserialize(loadJson["Laser"], showGuideImage);
                ofDeserialize(loadJson["Laser"], guideImageColour);
                
            } catch(...) {
                //cout << showGuideImage << " " <<loadJson["Laser"]["Show_guide_image"]<< endl;
            }
            
            
        }
    }
    
    ofAddListener(ofEvents().mousePressed, this, &Manager::mousePressed, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().mouseReleased, this, &Manager::mouseReleased, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().mouseDragged, this, &Manager::mouseDragged, OF_EVENT_ORDER_BEFORE_APP);
    
    iconGrabOpen.loadFromString("<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" version=\"1.1\" x=\"0px\" y=\"0px\" viewBox=\"0 0 24 30\" style=\"enable-background:new 0 0 24 24;\" xml:space=\"preserve\"><g><g><path fill=\"white\"  d=\"M16.313,23.134H3.384c-0.827,0-1.5-0.673-1.5-1.5c0-1.388,1.036-2.582,2.409-2.778l4.03-0.576l-7.543-7.543 c-0.428-0.428-0.665-0.998-0.665-1.604S0.352,7.958,0.781,7.53c0.325-0.325,0.731-0.539,1.173-0.624    C1.147,6.017,1.173,4.638,2.031,3.78c0.858-0.858,2.239-0.883,3.127-0.076C5.24,3.274,5.448,2.863,5.781,2.53    c0.884-0.885,2.323-0.885,3.207,0l0.171,0.171c0.083-0.429,0.29-0.839,0.622-1.171c0.884-0.885,2.323-0.885,3.207,0l8.982,8.982    c1.234,1.234,1.914,2.875,1.914,4.621s-0.68,3.387-1.914,4.621l-1.768,1.768C19.163,22.561,17.782,23.134,16.313,23.134z     M2.384,7.866c-0.325,0-0.649,0.124-0.896,0.371c-0.494,0.494-0.494,1.299,0,1.793l8.25,8.25c0.134,0.134,0.181,0.332,0.121,0.512    c-0.06,0.18-0.216,0.31-0.403,0.337l-5.02,0.717c-0.884,0.126-1.551,0.895-1.551,1.788c0,0.276,0.224,0.5,0.5,0.5h12.929    c1.202,0,2.332-0.468,3.182-1.318l1.768-1.768c1.045-1.045,1.621-2.436,1.621-3.914s-0.576-2.869-1.621-3.914l-8.982-8.982    c-0.494-0.494-1.299-0.494-1.793,0s-0.494,1.299,0,1.793l4.75,4.75c0.098,0.098,0.146,0.226,0.146,0.354s-0.049,0.256-0.146,0.354    c-0.195,0.195-0.512,0.195-0.707,0l-6.25-6.25c-0.479-0.479-1.313-0.479-1.793,0c-0.494,0.494-0.494,1.299,0,1.793l6.25,6.25    c0.098,0.098,0.146,0.226,0.146,0.354s-0.049,0.256-0.146,0.354c-0.195,0.195-0.512,0.195-0.707,0l-7.5-7.5    c-0.494-0.494-1.299-0.494-1.793,0s-0.494,1.299,0,1.793l7.5,7.5c0.098,0.098,0.146,0.226,0.146,0.354s-0.049,0.256-0.146,0.354    c-0.195,0.195-0.512,0.195-0.707,0l-6.25-6.25C3.033,7.99,2.709,7.866,2.384,7.866z\"/></g></g></svg>");
    
    iconGrabClosed.loadFromString("<svg xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' version='1.1' x='0px' y='0px' viewBox='0 0 24 30' style='enable-background:new 0 0 24 24;' xml:space='preserve'><g><g><path fill='white' d='M14.058,21.129H9.129c-2.481,0-4.5-2.019-4.5-4.5v-1.793l-1.595-1.595c-0.884-0.884-0.884-2.323,0-3.207    c0.336-0.337,0.76-0.555,1.221-0.632C3.663,8.521,3.756,7.313,4.534,6.534C5.238,5.83,6.293,5.688,7.139,6.103    C7.199,5.62,7.415,5.154,7.784,4.784c0.857-0.858,2.237-0.883,3.126-0.077c0.084-0.441,0.298-0.848,0.624-1.173    c0.884-0.885,2.323-0.885,3.207,0l4.974,4.974c1.234,1.234,1.914,2.875,1.914,4.621s-0.68,3.387-1.914,4.621l-1.768,1.768    C16.908,20.557,15.527,21.129,14.058,21.129z M5.629,15.836v0.793c0,1.93,1.57,3.5,3.5,3.5h4.929c1.202,0,2.332-0.468,3.182-1.318    l1.768-1.768c1.045-1.045,1.621-2.436,1.621-3.914s-0.576-2.869-1.621-3.914l-4.974-4.974c-0.479-0.479-1.313-0.479-1.793,0    c-0.494,0.494-0.494,1.299,0,1.793l0.241,0.241c0.098,0.098,0.146,0.226,0.146,0.354s-0.049,0.256-0.146,0.354    c-0.195,0.195-0.512,0.195-0.707,0l-1.491-1.491c-0.494-0.494-1.299-0.494-1.793,0C8.252,5.731,8.12,6.05,8.12,6.388    s0.132,0.657,0.371,0.896l1.491,1.491c0.098,0.098,0.146,0.226,0.146,0.354S10.08,9.385,9.982,9.483    c-0.195,0.195-0.512,0.195-0.707,0L7.034,7.241c-0.494-0.494-1.299-0.494-1.793,0s-0.494,1.299,0,1.793l2.241,2.241    c0.098,0.098,0.146,0.226,0.146,0.354s-0.049,0.256-0.146,0.354c-0.195,0.195-0.512,0.195-0.707,0l-1.241-1.241    c-0.479-0.479-1.313-0.479-1.793,0C3.502,10.981,3.37,11.3,3.37,11.638s0.132,0.657,0.371,0.896l3.741,3.741    c0.098,0.098,0.146,0.226,0.146,0.354s-0.049,0.256-0.146,0.354c-0.195,0.195-0.512,0.195-0.707,0L5.629,15.836z'/></g></g></svg>");
    
    iconMagPlus.loadFromString("<?xml version='1.0' encoding='UTF-8' standalone='no'?> <!-- Created with Vectornator for iOS (http://vectornator.io/) --><!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 1.1//EN' 'http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd'><svg height='100%' style='fill-rule:nonzero;clip-rule:evenodd;stroke-linecap:round;stroke-linejoin:round;' xmlns:xlink='http://www.w3.org/1999/xlink' xmlns='http://www.w3.org/2000/svg' xml:space='preserve' width='100%' xmlns:vectornator='http://vectornator.io' version='1.1' viewBox='0 0 25 25'><defs/><g id='Untitled' vectornator:layerName='Untitled'><g opacity='1'><path d='M23.359+20.1039L20.4339+17.1788C19.5811+18.4307+18.4979+19.5139+17.246+20.3666L20.1711+23.2918C21.0517+24.1724+22.4798+24.1724+23.359+23.2918C24.2396+22.4111+24.2396+20.9845+23.359+20.1039Z' opacity='1' fill='#ffffff'/><path d='M21.0134+10.4223C21.0134+4.61158+16.3035-0.098298+10.4928-0.098298C4.68212-0.098298-0.0277553+4.61158-0.0277553+10.4223C-0.0277553+16.233+4.68212+20.9428+10.4928+20.9428C16.3035+20.9428+21.0134+16.2329+21.0134+10.4223ZM10.4928+18.6884C5.93482+18.6884+2.22665+14.9795+2.22665+10.4223C2.22665+5.86428+5.93482+2.15611+10.4928+2.15611C15.05+2.15611+18.759+5.86428+18.759+10.4223C18.759+14.9795+15.05+18.6884+10.4928+18.6884Z' opacity='1' fill='#ffffff'/></g><path d='M14.0491+8.87293L6.96295+8.88761C6.0614+8.88952+5.33189+9.62109+5.33379+10.5226C5.3357+11.4242+6.06874+12.1537+6.97029+12.1518L14.0564+12.1371C14.958+12.1352+15.6875+11.4036+15.6856+10.5021C15.6837+9.60054+14.9506+8.87103+14.0491+8.87293ZM12.1612+14.0485L12.1466+6.96241C12.1447+6.06085+11.4131+5.33134+10.5115+5.33325C9.60997+5.33515+8.88046+6.06819+8.88237+6.96975L8.89704+14.0559C8.89895+14.9574+9.63052+15.6869+10.5321+15.685C11.4336+15.6831+12.1631+14.9501+12.1612+14.0485Z' opacity='1' fill='#ffffff'/></g></svg>");
    
    iconMagMinus.loadFromString("<?xml version='1.0' encoding='UTF-8' standalone='no'?> <!-- Created with Vectornator for iOS (http://vectornator.io/) --><!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 1.1//EN' 'http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd'><svg height='100%' style='fill-rule:nonzero;clip-rule:evenodd;stroke-linecap:round;stroke-linejoin:round;' xmlns:xlink='http://www.w3.org/1999/xlink' xmlns='http://www.w3.org/2000/svg' xml:space='preserve' width='100%' xmlns:vectornator='http://vectornator.io' version='1.1' viewBox='0 0 25 25'><defs/><g id='Untitled' vectornator:layerName='Untitled'><g opacity='1'><path d='M23.359+20.1039L20.4339+17.1788C19.5811+18.4307+18.4979+19.5139+17.246+20.3666L20.1711+23.2918C21.0517+24.1724+22.4798+24.1724+23.359+23.2918C24.2396+22.4111+24.2396+20.9845+23.359+20.1039Z' fill-rule='evenodd' fill='#ffffff' opacity='1'/><path d='M21.0134+10.4223C21.0134+4.61158+16.3035-0.098298+10.4928-0.098298C4.68212-0.098298-0.0277553+4.61158-0.0277553+10.4223C-0.0277553+16.233+4.68212+20.9428+10.4928+20.9428C16.3035+20.9428+21.0134+16.2329+21.0134+10.4223ZM10.4928+18.6884C5.93482+18.6884+2.22665+14.9795+2.22665+10.4223C2.22665+5.86428+5.93482+2.15611+10.4928+2.15611C15.05+2.15611+18.759+5.86428+18.759+10.4223C18.759+14.9795+15.05+18.6884+10.4928+18.6884Z' fill-rule='evenodd' fill='#ffffff' opacity='1'/></g><path d='M14.0491+8.87293L6.96295+8.88761C6.0614+8.88952+5.33189+9.62109+5.33379+10.5226C5.3357+11.4242+6.06874+12.1537+6.97029+12.1518L14.0564+12.1371C14.958+12.1352+15.6875+11.4036+15.6856+10.5021C15.6837+9.60054+14.9506+8.87103+14.0491+8.87293Z' opacity='1' fill='#ffffff'/></g></svg>");
    
}

Manager::~Manager() {
    ofRemoveListener(ofEvents().mousePressed, this, &Manager::mousePressed, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(ofEvents().mouseReleased, this, &Manager::mouseReleased, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(ofEvents().mouseDragged, this, &Manager::mouseDragged, OF_EVENT_ORDER_BEFORE_APP);
    
}



bool Manager :: mousePressed(ofMouseEventArgs &e){
    if(ofGetKeyPressed(' ')) {
        
        if (getSelectedLaser()<0) {
            if(ofGetKeyPressed(OF_KEY_COMMAND)) {
                // zoom in
                zoomPreviewAroundPoint(e,1.2);
                return true;
            } else if(ofGetKeyPressed(OF_KEY_ALT)) {
                // zoom out
                zoomPreviewAroundPoint(e,0.8);
                return true;
            } else if(ofGetKeyPressed(OF_KEY_CONTROL)) {
                setDefaultPreviewOffsetAndScale();
                
            } else {
                // start dragging
                draggingPreview = true;
                dragStartPoint = e - previewOffset;
                return true;
            }
        } else {
            // do the stuff but for the individual laser
            ofxLaser::Laser& currentLaser = *lasers[getSelectedLaser()];
            if(ofGetKeyPressed(OF_KEY_COMMAND)) {
                // zoom in
                currentLaser.zoomAroundPoint(e,1.2);
                return true;
            } else if(ofGetKeyPressed(OF_KEY_ALT)) {
                // zoom out
                currentLaser.zoomAroundPoint(e,0.8);
                return true;
            } else if(ofGetKeyPressed(OF_KEY_CONTROL)) {
                currentLaser.setOffsetAndScale(glm::vec2(guiSpacing, guiSpacing), 1);
                
            } else {
                // start dragging
                currentLaser.startDrag(e); //  - previewOffset;
                return true;
            }
        }
    }
    return false;
}

void Manager :: setDefaultPreviewOffsetAndScale(){
    previewOffset = glm::vec2(guiSpacing, guiSpacing);
    previewScale = 1;
    float thirdOfHeight = ofGetHeight()/3;
    
    if(height>(thirdOfHeight*2)) {
        previewScale = (float)(thirdOfHeight*2) / (float)height;
    }
    if(width * previewScale> ofGetWidth()-(guiSpacing*3)-guiLaserSettingsPanelWidth) {
        previewScale = (float)(ofGetWidth()-(guiSpacing*3)-guiLaserSettingsPanelWidth)/width;
    }
    
}

bool Manager :: mouseReleased(ofMouseEventArgs &e){
    draggingPreview = false;
    for(ofxLaser::Laser* laser : lasers) {
        laser->stopDrag();
        
    }
    return false;
}
bool Manager :: mouseDragged(ofMouseEventArgs &e){
    if(draggingPreview) {
        previewOffset = e-dragStartPoint;
        return true;
    } else {
        return false;
    }
}

//
//bool Manager :: keyPressed(ofKeyEventArgs &e){
//    if(showInputPreview) {
//        if(e.key==' ') {
//            ofHideCursor();
//        }
//        
//        
//    }
//    return false;
//}
//
//bool Manager :: keyReleased(ofKeyEventArgs &e){
//    if((e.key == ' ') || (ofGetKeyPressed(' '))) ofShowCursor();
//    return false;
//}


void Manager :: zoomPreviewAroundPoint(glm::vec2 anchor, float zoomMultiplier) {
    glm::vec2 offset = anchor-previewOffset;
    offset-=(offset*zoomMultiplier);
    previewOffset+=offset;
    previewScale*=zoomMultiplier;
    
}

bool Manager :: deleteLaser(Laser* laser) {
    bool success = ManagerBase::deleteLaser(laser);
    if(success) {
        selectedLaser = -1;
        if(getNumLasers()==0) showLaserSettings = false;
        return true;
    } else {
        return false;
    }
    
}

void Manager::selectNextLaser() {
    int next = selectedLaser+1;
    if(next>=(int)lasers.size()) next=-1;
    
    setSelectedLaser(next);
    
}

void Manager::selectPreviousLaser() {
    int prev = selectedLaser-1;
    if(prev<-1) prev=(int)lasers.size()-1;
    setSelectedLaser(prev);
    
}
int Manager::getSelectedLaser(){
    return selectedLaser;
}
void Manager::setSelectedLaser(int i){
    if((selectedLaser!=i) && (i<getNumLasers())) {
        selectedLaser = i;
    }
}
bool Manager::isAnyLaserSelected() {
    return selectedLaser>=0;
}

bool Manager::setGuideImage(string filename){
    return guideImage.load(filename);
    
    
}

void Manager:: drawUI(){
    
    
    
    drawPreviews();
    
    ofxLaser::UI::startGui();
    
    drawLaserGui();
    
    finishLaserUI();
    
}

void Manager :: startLaserUI() {
    ofxLaser::UI::updateGui();
    ofxLaser::UI::startGui();
    
}
void Manager :: finishLaserUI() {
    ofxLaser::UI::render();
    renderCustomCursors();
    
    
}

void Manager :: renderCustomCursors() {
    if(previewNavigationEnabled && ofGetKeyPressed(' ')) {
        ofHideCursor(); // TODO add so that it only happens over the preview
        ofPushMatrix();
        ofTranslate(ofGetMouseX(), ofGetMouseY());
        ofScale(0.6);
        ofTranslate(-12,-12);
        // ofEnableDepthTest();
        if(ofGetKeyPressed(OF_KEY_COMMAND)) {
            iconMagPlus.draw();
        } else if(ofGetKeyPressed(OF_KEY_ALT)) {
            iconMagMinus.draw();
        } else if(ofGetMousePressed()){
            iconGrabClosed.draw();
        } else {
            iconGrabOpen.draw();
        }
        // ofDisableDepthTest();
        ofPopMatrix();
    } else ofShowCursor();
    
}

void Manager :: drawPreviews() {
    
    // if none of the lasers are selected, then draw the
    // input preview
    // Note that the scale and offset is adjusted with
    // key commands.
    if(selectedLaser<0) {
        if(showInputPreview) {
        
        
            ofPushStyle();
            ofFill();
            ofSetColor(0);
            ofPushMatrix();
            ofTranslate(previewOffset);
            ofScale(previewScale);
            ofDrawRectangle(0,0,width, height);
            ofPopMatrix();
            ofPopStyle();
        

            if(showInputZones) {
                // this renders the input zones in the graphics source space
                for(size_t i= 0; i<zones.size(); i++) {
                    zones[i]->setOffsetAndScale(previewOffset,previewScale);
                    zones[i]->draw();
                }
            }
            
            renderPreview();
            
            if(showBitmapMask) {
                ofPushMatrix();
                laserMask.setOffsetAndScale(previewOffset,previewScale);
                laserMask.draw(showBitmapMask);
                ofTranslate(previewOffset);
                ofScale(previewScale, previewScale);
                ofPopMatrix();
            }
        }
        
        if(showOutputPreviews) {
           // int numrows = 1;
            float outputpreviewscale = 0.375;
            float outputpreviewsize = 800*outputpreviewscale;
            
            float spaceatbottom = (ofGetHeight() - getPreviewRect().getBottom() ) -(guiSpacing*2);
            if (spaceatbottom<50) spaceatbottom = 50;
            if(outputpreviewsize>spaceatbottom) outputpreviewsize = spaceatbottom;
            
            // so we have spaceatbottom which is the gap at the bottom
            // then we have the height and width of the previews
            // which is outputpreviewsize.
            // We know this will fit vertically but we don't know if it
            // fit horizontally
            float availablespace = ofGetWidth()-guiLaserSettingsPanelWidth- (guiSpacing*2);
            if(outputpreviewsize*lasers.size() > availablespace) {
                outputpreviewsize = (availablespace/lasers.size())-guiSpacing;
            }
//            if(spaceatbottom>(outputpreviewsize*2)+guiSpacing) {
//                numrows=2;
//                outputpreviewsize = (spaceatbottom/2)-guiSpacing;
//            }
            
            for(size_t i= 0; i<lasers.size(); i++) {
                
                ofRectangle laserOutputPreviewRect(guiSpacing+((outputpreviewsize+guiSpacing)*i),ofGetHeight()-guiSpacing-outputpreviewsize,outputpreviewsize,outputpreviewsize);
               
//                if(numrows>1) {
//                    int numinrow = lasers.size()/numrows;
//                    int rownum = floor(i/numinrow);
//                    laserOutputPreviewRect.y-=(rownum*(outputpreviewsize+guiSpacing));
//                    laserOutputPreviewRect.x =guiSpacing+ ((outputpreviewsize+guiSpacing)*(i%numinrow));
//
//                }
                
                ofFill();
                ofSetColor(0);
                ofDrawRectangle(laserOutputPreviewRect);
                
                lasers[i]->drawTransformAndPath(laserOutputPreviewRect);
                
                // disables the warp interfaces
                lasers[i]->disableTransformGui();
            }
            
        }
        
        
    } else {
        float thirdOfHeight = ofGetHeight()/3;
        ofxLaser::Laser* selectedlaser = nullptr;
        // draw laser adjustment screen
        for(size_t i= 0; i<lasers.size(); i++) {
            if((int)i==selectedLaser) {
                selectedlaser = lasers[i];
                //ofFill();
                //ofSetColor(0);
                float size = 800;
                if(size>thirdOfHeight*2) size = thirdOfHeight*2;
                
                //ofDrawRectangle(guiSpacing,guiSpacing,size,size);
                selectedlaser->enableTransformGui();
                selectedlaser->drawTransformUI();
                selectedlaser->drawLaserPath();
                
                
            } else {
                lasers[i]->disableTransformGui();
            }
            
        }
        
        if(showOutputPreviews) {
            
            
            for(size_t i= 0; i<lasers.size(); i++) {
                float outputpreviewscale = 0.375;
                
                float outputpreviewsize = 800*outputpreviewscale;
                float previewbotton = (selectedlaser->previewScale*800)+selectedlaser->previewOffset.y;
                float spaceatbottom = (ofGetHeight() - previewbotton) -(guiSpacing*2);
                if (spaceatbottom<50) spaceatbottom = 50;
                if(outputpreviewsize>spaceatbottom) outputpreviewsize = spaceatbottom;
                ofRectangle laserOutputPreviewRect(guiSpacing+((outputpreviewsize+guiSpacing)*i),ofGetHeight()-guiSpacing-outputpreviewsize,outputpreviewsize,outputpreviewsize);
                
                ofFill();
                ofSetColor(0);
                ofDrawRectangle(laserOutputPreviewRect);
                
                lasers[i]->drawTransformAndPath(laserOutputPreviewRect);
                if((int)i==selectedLaser) {
                    ofNoFill();
                    ofSetLineWidth(1);
                    ofSetColor(255);
                    ofDrawRectangle(laserOutputPreviewRect);
                    
                }
            }
            
        }
        
        //            if(showInputPreview) {
        //                previewOffset = glm::vec2(guiSpacing,thirdOfHeight*2);
        //                previewScale = (float)thirdOfHeight/(float)height;
        //                renderPreview();
        //            }
        
        
    }
    
    
}
void Manager :: renderPreview() {
    
    //
    //    ofPushStyle();
    //    ofFill();
    //    ofSetColor(0);
    //    ofPushMatrix();
    //    ofTranslate(previewOffset);
    //    ofScale(previewScale);
    //    ofDrawRectangle(0,0,width, height);
    //    ofPopMatrix();
    //    ofPopStyle();
    //
    
    ofRectangle previewRect = getPreviewRect();
    ofRectangle screenRect(0,0,ofGetWidth(), ofGetHeight());
    ofRectangle visibleRect = previewRect.getIntersection(screenRect);
    
    int fbowidth = visibleRect.getWidth();
    int fboheight = visibleRect.getHeight();
    
    if((canvasPreviewFbo.getWidth()!=fbowidth) || (canvasPreviewFbo.getHeight()!=fboheight)) {
        // previewFbo.clear();
        canvasPreviewFbo.allocate(fbowidth, fboheight, GL_RGB, 3);
    }
    
    canvasPreviewFbo.begin();
    
    ofClear(0,0,0,0);
    ofPushStyle();
    // ofEnableSmoothing();
    ofPushMatrix();
    ofTranslate(previewRect.getTopLeft()-visibleRect.getTopLeft());
    //ofTranslate(previewOffset);
    ofScale(previewScale, previewScale);
    
    
    // draw outline of laser output area
    ofSetColor(0);
    ofFill();
    ofDrawRectangle(0,0,width,height);
    ofSetColor(50);
    ofNoFill();
    ofDrawRectangle(0,0,width,height);
    
    // Draw laser graphics preview ----------------
    ofMesh mesh;
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    mesh.clear();
    mesh.setMode(OF_PRIMITIVE_LINE_STRIP);
    
    // Draw the preview laser graphics, with zones overlaid
    for(size_t i= 0; i<shapes.size(); i++) {
        shapes[i]->addPreviewToMesh(mesh);
    }
    
    ofRectangle laserRect(0,0,width, height);
    if(useBitmapMask) {
        const vector<glm::vec3>& points = mesh.getVertices();
        std::vector<ofFloatColor>& colours = mesh.getColors();
        
        for(size_t i= 0;i<points.size(); i++ ){
            
            ofFloatColor& c = colours.at(i);
            const glm::vec3& p = points[i];
            
            
            float brightness;
            
            if(laserRect.inside(p)) {
                brightness = laserMask.getBrightness(p.x, p.y);
            } else {
                brightness = 0;
            }
            
            c.r*=brightness;
            c.g*=brightness;
            c.b*=brightness;
            
        }
    }
    
    ofSetLineWidth(1.5);
    mesh.draw();
    
    std::vector<ofFloatColor>& colours = mesh.getColors();
    
    for(size_t i= 0; i<colours.size(); i++) {
        colours[i].r*=0.4;
        colours[i].g*=0.4;
        colours[i].b*=0.4;
    }
    
    ofSetLineWidth(4);
    mesh.draw();
    
    
    if(showGuideImage && guideImage.isAllocated()) {
        ofSetColor(guideImageColour);
        guideImage.draw(0,0,width, height);
        
        
    }
    ofDisableBlendMode();
    // ofDisableSmoothing();
    
    ofPopMatrix();
    
    ofPopStyle();
    canvasPreviewFbo.end();
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    canvasPreviewFbo.draw(visibleRect.getTopLeft());
    ofPopStyle();
}

void Manager::setCanvasSize(int width, int height) {
    ManagerBase::setCanvasSize(width, height);
    setDefaultPreviewOffsetAndScale();
}

bool Manager ::toggleGui(){
    guiIsVisible = !guiIsVisible;
    return guiIsVisible;
}
void Manager ::setGuiVisible(bool visible){
    guiIsVisible = visible;
}
bool Manager::isGuiVisible() {
    return guiIsVisible;
}


void Manager::setDefaultHandleSize(float size) {
    defaultHandleSize = size;
    for(Laser* laser : lasers) {
        laser->setDefaultHandleSize(defaultHandleSize);
    }
    
}

void Manager::addCustomParameter(ofAbstractParameter& param, bool loadFromSettings){
    customParams.add(param);
    if(loadFromSettings){
        //loadJson.find("custom");
        if(!loadJson.empty()) {
            if(loadJson.contains("Laser")) {
                if(loadJson["Laser"].contains("CUSTOM_PARAMETERS")) {
                    //     auto value = loadJson["Laser"]["Custom"][param.getName()];
                    try {
                        ofDeserialize(loadJson["Laser"]["CUSTOM_PARAMETERS"], param);
                    } catch(...) {
                        
                    }
                    
                }
            }
        }
        //ofDeserialize(loadJson["custom"], param);
        //ofLogNotice(loadJson.dump(3));
        //ofLogNotice(loadJson["Laser"].dump(3));
        //ofLogNotice(loadJson["Laser"]["Custom"].dump(3));
        //ofLogNotice(loadJson["Laser"]["Custom"][param.getName()].dump(3));
        
    }
}


bool Manager::togglePreview(){
    showInputPreview = !showInputPreview;
    return showInputPreview;
};

glm::vec2 Manager::screenToLaserInput(glm::vec2& pos){
    
    glm::vec2 returnpos= pos - glm::vec2(guiSpacing, guiSpacing);
    returnpos/=previewScale;
    return returnpos;
    
}

void Manager::drawLaserGui() {
    
    ofxLaser::ManagerBase& laserManager = *this;
    
    int mainpanelwidth = 270;
    int laserpanelwidth = 280;
    int spacing = 8;
    
    // calculate x position of main window
    int x = ofGetWidth() - mainpanelwidth - spacing;
    
    UI::startWindow("Settings", ImVec2(x, spacing), ImVec2(mainpanelwidth, 0), ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize, true );
    
    
    // START BIG BUTTONS
    UI::largeItemStart();
    
    // the arm and disarm buttons
    bool useRedButton =laserManager.areAllLasersArmed();
    if(useRedButton) UI::secondaryColourButtonStart();
    // change the colour for the arm all button if we're armed
    int buttonwidth = (mainpanelwidth-(spacing*3))/2;
    if(ImGui::Button("ARM ALL LASERS", ImVec2(buttonwidth, 0.0f) )) {
        laserManager.armAllLasers();
    }
    if(useRedButton) UI::secondaryColourButtonEnd();
    
    ImGui::SameLine();
    if(ImGui::Button("DISARM ALL LASERS",  ImVec2(buttonwidth, 0.0f))) {
        laserManager.disarmAllLasers();
    }
    
    // change width of slider vs label
    ImGui::PushItemWidth(ImGui::GetWindowWidth() - 120.0f);
    
    // add intensity slide
    //UI::addFloatAsIntSlider(laser.masterIntensity, 100);
    ImGui::PushItemWidth(mainpanelwidth-(spacing*2));
    float multiplier = 100;
    int value = laserManager.globalBrightness*multiplier;
    if (ImGui::SliderInt("##int", &value, laserManager.globalBrightness.getMin()*multiplier, laserManager.globalBrightness.getMax()*multiplier, "GLOBAL BRIGHTNESS %d")) {
        laserManager.globalBrightness.set((float)value/multiplier);
        
    }
    ImGui::PopItemWidth();
    
    string label;
    if(lasers.size()>0) {
        label = showLaserSettings? "CLOSE LASER SETTINGS" : "OPEN LASER SETTINGS";
        
        if(ImGui::Button(label.c_str())) {
            showLaserSettings = !showLaserSettings;
        }
    }
    
    if(ImGui::Button("ADD LASER", ImVec2(buttonwidth, 0.0f))) {
        createAndAddLaser();
    }
    ImGui::SameLine();
    if(ImGui::Button("ADD ZONE", ImVec2(buttonwidth, 0.0f))) {
        addZone();
        lockInputZones = false;
    }
    // END BIG BUTTONS
    UI::largeItemEnd();
    
    
    
    // SHOW LIST OF LASERS
    
    for(int i = 0; i<getNumLasers(); i++) {
        
        ofxLaser::Laser& laserobject = laserManager.getLaser(i);
        string laserNumberString = ofToString(i+1);
        bool showsecondarycolour = false;
        
        // LASER BUTTONS
        if(ImGui::Button(laserNumberString.c_str())) {
            if(selectedLaser!=i) selectedLaser = i;
            else selectedLaser = -1;
        }
        ImGui::SameLine();
        
        // ARM BUTTONS
        if(laserobject.armed) {
            UI::secondaryColourButtonStart();
            showsecondarycolour = true;
        }
        string armlabel = "ARM##"+ofToString(i+1);
        if(ImGui::Button(armlabel.c_str())){
            laserobject.toggleArmed();
        }
        if(showsecondarycolour) UI::secondaryColourButtonEnd();
        
        // FRAME RATES
        
        ImGui::SameLine();
        label = "##framerate"+laserNumberString;
        ImGui::PushItemWidth(100);
        ImGui::PlotLines(label.c_str(), laserobject.frameTimeHistory, laserobject.frameTimeHistorySize, laserobject.frameTimeHistoryOffset, "", 0, 0.1f);
        ImGui::PopItemWidth();
        
        // DAC STATUSES
        ImGui::SameLine();
        
        
        
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 size = ImVec2(19,19); // ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
        
        int radius = 4;
        ImVec2 p = ImGui::GetCursorScreenPos();
        p.x+=radius-2;
        p.y+=radius+4;
        ImU32 col = UI::getColourForState(laserobject.getDacConnectedState());
        
        draw_list->AddCircleFilled(p,radius, col);
        ImGui::InvisibleButton("##invisible", ImVec2(radius*2, radius*2) - ImVec2(2,2));
        
        ImGui::SameLine();
        ImGui::Text("%s",ofToString(round(laserobject.getFrameRate())).c_str());
        
        
        
        
        
    }
    
    UI::addIntSlider(laserManager.testPattern);
    
    UI::addParameterGroup(laserManager.interfaceParams);
    
    UI::addParameter(previewNavigationEnabled);
    UI::toolTip("When this is enabled you can hit space to drag around the preview window. While space is pressed click the mouse with command/ctrl pressed to zoom in, with alt pressed to zoom out");
    if(guideImage.isAllocated()) {
        UI::addParameter(showGuideImage);
        UI::addParameter(guideImageColour);
    }
    
    if((!lockInputZones) && (selectedLaser ==-1)) {
        
        ImGui::Separator();
        
        for(Zone* zone : zones) {
            
            string buttonlabel ="DELETE "+zone->zoneLabel;
            string modallabel ="Delete "+zone->zoneLabel+"?";
            
            UI::secondaryColourButtonStart();
            if(ImGui::Button(buttonlabel.c_str())) {
                ImGui::OpenPopup(modallabel.c_str());
            }
            UI::secondaryColourButtonEnd();
            
            if (ImGui::BeginPopupModal(modallabel.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Are you sure? All the zone settings will be deleted.\n\n");
                ImGui::Separator();
                
                UI::secondaryColourButtonStart();
                if (ImGui::Button("DELETE", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                    deleteZone(zone);
                    
                }
                UI::secondaryColourButtonEnd();
                
                ImGui::SetItemDefaultFocus();
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                    
                }
                ImGui::EndPopup();
            }
            
            
            
            
            
        }
    }
    
    
    if(laserManager.customParams.size()>0) {
        ImGui::Separator();
        //ImGui::Text("CUSTOM PARAMETERS");
        UI::addParameterGroup(laserManager.customParams);
        
    }
    
    
    // ImGui::PopStyleVar(2);
    
    ImGui::PopItemWidth();
    
    UI::endWindow();
    // ImGui::End();
    
    
    // show laser settings :
    
    if(laserManager.showLaserSettings) {
        x-=(laserpanelwidth+spacing);
        
        int laserIndexToShow = selectedLaser;
        if(laserIndexToShow ==-1) laserIndexToShow = 0;
        drawLaserSettingsPanel(&getLaser(laserIndexToShow), laserpanelwidth, spacing, x);
        
        
    }
    
    // Show laser zone settings mute / solo / etc
    if(selectedLaser!=-1)  {
        
        
        // LASER ZONE SETTINGS
        Laser* laser = lasers[selectedLaser];
        
        glm::vec2 laserZonePos = previewOffset + (previewScale*glm::vec2(width, 0));
        
        UI::startWindow("Laser output zones", ImVec2(laserZonePos.x+spacing, laserZonePos.y), ImVec2(340,0));
        
        ImGui::Columns(3, "Laser zones columns");
        ImGui::SetColumnWidth(0, 80.0f);
        ImGui::SetColumnWidth(1, 80.0f);
        ImGui::SetColumnWidth(2, 180.0f);
        
        bool soloActive = laser->areAnyZonesSoloed();
        
        // MUTE SOLO
        vector<LaserZone*> activeZones = laser->getActiveZones();
        for(LaserZone* laserZone : laser->laserZones) {
            
            
            
            bool zonemuted = laserZone->muted;
            
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, soloActive?0.5f:1.0f);
            string muteLabel = "M##"+laserZone->getLabel();
            if(zonemuted) UI::secondaryColourButtonStart();
            if(ImGui::Button(muteLabel.c_str(), ImVec2(20,20))) {
                laserZone->muted = !laserZone->muted;
            };
            UI::addDelayedTooltip("Mute zone");
            
            
            if(zonemuted) UI::secondaryColourButtonEnd();
            ImGui::PopStyleVar();
            
            ImGui::SameLine();
            bool soloed = laserZone->soloed;
            if(soloed) UI::secondaryColourButtonStart();
            string soloLabel = "S##"+laserZone->getLabel();
            if(ImGui::Button(soloLabel.c_str(), ImVec2(20,20))){
                laserZone->soloed = !laserZone->soloed;
            }
            if(soloed) UI::secondaryColourButtonEnd();
            UI::addDelayedTooltip("Solo zone");
            
            ImGui::SameLine();
            ImGui::Text("%s",laserZone->getLabel().c_str());
            
        }
        ImGui::NextColumn();
        // ImGui::SetCursorPosX(200);
        for(Zone* zone : zones) {
            bool checked = laser->hasZone(zone);
            
            if(ImGui::Checkbox(zone->displayLabel.c_str(), &checked)) {
                if(checked) {
                    laser->addZone(zone, width, height);
                } else {
                    
                    laser->removeZone(zone);
                }
                
            }
        }
        ImGui::NextColumn();
        
        
        MaskManager& maskManager = laser->maskManager;
        if(ImGui::Button("ADD MASK")) {
            maskManager.addQuadMask();
        }
        ImDrawList*   draw_list = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos();
        draw_list->AddLine(ImVec2(p.x - 9999, p.y), ImVec2(p.x + 9999, p.y),  ImGui::GetColorU32(ImGuiCol_Border));
        ImGui::Dummy(ImVec2(0.0f, 2.0f));
        for(QuadMask* mask : maskManager.quads){
            string label = "##"+mask->displayLabel;
            ImGui::Text("MASK %s", mask->displayLabel.c_str());
            ImGui::SameLine();
            ImGui::PushItemWidth(40);
            int level = mask->maskLevel;
            if (ImGui::DragInt(label.c_str(),&level,1,0,100,"%d%%")) {
                mask->maskLevel = level;
            }
            
            ImGui::PopItemWidth();
            ImGui::SameLine();
            string buttonlabel = "DELETE "+mask->displayLabel+"##mask";
            if(ImGui::Button(buttonlabel.c_str())) {
                maskManager.deleteQuadMask(mask);
                
            }
            
        }
        
        
        
        
        
        
        
        ImGui::Columns();
        ImGui::Separator();
        UI::addIntSlider(laser->testPattern);
        UI::addCheckbox(laser->hideContentDuringTestPattern);
        UI::toolTip("Disable this if you want to see the laser content at the same time as the text patterns");
        
        UI::endWindow();
        
        
        // Laser Output Masks
        
        for(LaserZone* laserZone : laser->laserZones) {
            
            if(laserZone->zoneTransform.getSelected()) {
                ImVec2 pos(laserZone->zoneTransform.getRight(),laserZone->zoneTransform.getCentre().y);
                ImVec2 size(200,0);
                UI::startWindow(laserZone->getLabel()+"##"+laser->getLabel(),pos, size, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
                
                
                UI::addParameterGroup(laserZone->zoneTransform.params);
                ImGui::Text("Edge masks");
                UI::addFloatSlider(laserZone->bottomEdge);
                UI::addFloatSlider(laserZone->topEdge);
                UI::addFloatSlider(laserZone->leftEdge);
                UI::addFloatSlider(laserZone->rightEdge);
                
                UI::endWindow();
            }
        }
        
        
    }
    
}


void Manager :: drawLaserSettingsPanel(ofxLaser::Laser* laser, float laserpanelwidth, float spacing, float x) {
    
    UI::startWindow(laser->getLabel(), ImVec2(x, spacing), ImVec2(laserpanelwidth,0), ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |ImGuiWindowFlags_AlwaysAutoResize, true, (bool*)&showLaserSettings.get());
    
    
    
    UI::largeItemStart();
    // change width of slider vs label
    ImGui::PushItemWidth(140);
    
    UI::addCheckbox(laser->armed);
    ImGui::PushItemWidth(laserpanelwidth-(spacing*2));
    float multiplier = 100;
    int value = laser->intensity*multiplier;
    if (ImGui::SliderInt("##int", &value, laser->intensity.getMin()*multiplier, laser->intensity.getMax()*multiplier, "BRIGHTNESS %d")) {
        laser->intensity.set((float)value/multiplier);
        
    }
    ImGui::PopItemWidth();
    
    
    UI::largeItemEnd();
    
    UI::addIntSlider(laser->testPattern);
    //UI::addButton(resetDac);
    
    
    // THE DAC STATUS COLOUR - TO BE IMPROVED
    ImGui::Separator();
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 size = ImVec2(19,19); // ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
        
        ImVec2 p = ImGui::GetCursorScreenPos();
        int state = laser->getDacConnectedState();
        
        ImU32 col = UI::getColourForState(laser->getDacConnectedState());
        
        draw_list->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y), col);
        ImGui::InvisibleButton("##gradient2", size - ImVec2(2,2));
        
    }
    
    ImGui::SameLine();
    ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(0,2));
    //ImGui::Text("DAC:");
    //ImGui::SameLine();
    if(!laser->hasDac() && (laser->dacId.get()!="") ) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5);
        ImGui::Text("Waiting for %s", laser->dacId.get().c_str());
        ImGui::PopStyleVar();
    } else {
        ImGui::Text("%s", laser->getDacLabel().c_str());
    }
    
    ImGui::SameLine(laserpanelwidth-30);
    if(ImGui::Button("^", ImVec2(19,19))){
        
        
    }
    
    // DAC LIST -------------------------------------------------------------
    
    ImGui::PushItemWidth(laserpanelwidth-spacing*2);
    
    // get the dacs from the dacAssigner
    const vector<DacData>& dacList = dacAssigner.getDacList();
    
    if (ImGui::ListBoxHeader("##listbox", MIN(5, MAX(1,dacList.size())))){
        
        if(dacList.empty()) {
            
            ImGui::Selectable("No laser controllers found", false, ImGuiSelectableFlags_Disabled );
            
        } else {
            
            
            // add a combo box item for every element in the list
            for(const DacData& dacdata : dacList) {
                
                // get the dac label (usually type + unique ID)
                string itemlabel = dacdata.alias=="" ? dacdata.label : dacdata.alias;
                
                ImGuiSelectableFlags selectableflags = 0;
                
                if(!dacdata.available) {
                    // ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5);
                    //itemlabel += " - no longer available";
                    selectableflags|=ImGuiSelectableFlags_Disabled;
                } else {
                    //
                }
                // if this dac is assigned to a laser, show which laser
                //  - this could be done at the other end?
                
                if (ImGui::Selectable(itemlabel.c_str(), (dacdata.assignedLaser == laser), selectableflags)) {
                    // then select dac
                    // TODO : show a warning yes / no if :
                    //      - we already are connected to a DAC
                    //      - the chosen DAC is already being used by another laser
                    dacAssigner.assignToLaser(dacdata.label, *laser);
                }
                
                if(dacdata.assignedLaser!=nullptr) {
                    ImGui::SameLine(laserpanelwidth - 100);
                    string label =" > " + dacdata.assignedLaser->getLabel();
                    ImGui::Text("%s",label.c_str());
                }
                
                //ImGui::PopStyleVar();
            }
        }
        //    if (is_selected)
        //       ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
        //ImGui::EndCombo();
        ImGui::ListBoxFooter();
    }
    //ImGui::PopStyleVar();
    ImGui::PopItemWidth();
    
    if(laser->hasDac()) {
        if(ImGui::Button("Disconnect controller")) {
            dacAssigner.disconnectDacFromLaser(*laser);
        }
        ImGui::SameLine();
    }
    
    if(ImGui::Button("Refresh controller list")) {
        dacAssigner.updateDacList();
        
    }
    // ----------------------------------------------
    
    
    
    ImGui::Separator();
    ImGui::Text("OUTPUT / ZONE SETTINGS");
    
    UI::largeItemStart();
    ImGui::PushItemWidth(190);
    UI::addFloatAsIntPercentage(laser->speedMultiplier);
    UI::toolTip("Scanner speed adjustment (NB this works mathematically, it doesn't change the point rate)");
    ImGui::PopItemWidth();
    ImGui::PushItemWidth(170);
    UI::addFloatSlider(laser->colourChangeShift);
    UI::toolTip("Shifts the laser colours to match the scanner position (AKA blank shift)");
    ImGui::PopItemWidth();
    UI::largeItemEnd();
    
    
    
    // ZONES
    UI::addCheckbox(laser->flipX);
    UI::addCheckbox(laser->flipY);
    
    // FINE OUTPUT SETTINGS
    bool treevisible = ImGui::TreeNode("Fine position adjustments");
    
    UI::toolTip("These affect all output zones for this laser and can be used to re-align the output if moved slightly since setting them up");
    if (treevisible){
        
        
        ofParameter<float>& param = laser->rotation;
        if(ImGui::DragFloat("Rotation", (float*)&param.get(), 0.01f,-10,10)) { //  param.getMin(), param.getMax())) {
            param.set(param.get());
            
        }
        if(laser->rotation!=0) {
            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            if (ImGui::Button("Reset")) laser->rotation = 0;
        }
        
        ofParameter<glm::vec2>& param2 = laser->outputOffset;
        if(ImGui::DragFloat2("Position", (float*)&param2.get().x, 0.01f, -50.0f,50.0f)) { //  param.getMin(), param.getMax())) {
            param2.set(param2.get());
            
        }
        
        glm::vec2 zero2;
        if(laser->outputOffset.get()!=zero2) {
            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            if (ImGui::Button("Reset")) laser->outputOffset.set(zero2);
        }
        // ImGui::PopItemWidth();
        ImGui::TreePop();
    }
    
    
    
    
    // TODO IMPLEMENT LASER PROFILES
    /*
     // LASER PROFILE
     ImGui::Separator();
     ImGui::Text("LASER PROFILE");
     
     // TODO :
     // check if the settings are different from the preset, if they are
     // show a "save" button, also save as?
     //
     // when an option is selected, update all the params
     
     if (ImGui::BeginCombo("##combo", "LightSpace Unicorn RGB11000")) // The second parameter is the label previewed before opening the combo.
     {
     
     if (ImGui::Selectable("LightSpace Unicorn RGB11000", true)) {
     //item_current = items[n];
     }
     if (ImGui::Selectable("OPT PD4", false)) {
     //item_current = items[n];
     }
     if (ImGui::Selectable("OPT PD25", false)) {
     //item_current = items[n];
     }
     if (ImGui::Selectable("LaserCube 2W", false)) {
     //item_current = items[n];
     }
     if (ImGui::Selectable("LaserCube 1W", false)) {
     //item_current = items[n];
     }
     
     //    if (is_selected)
     //       ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
     ImGui::EndCombo();
     }
     */
    
    // SCANNER PROFILE SETTINGS
    
    
    ImGui::Separator();
    ImGui::Text("SCANNER SETTINGS");
    
    
    PresetManager& presetManager = *PresetManager::instance();
    // TODO :
    // check if the settings are different from the preset, if they are
    // show a "save" button, also save as?
    //
    // when an option is selected, update all the params
    const vector<string>& presets = presetManager.getPresetNames();
    string label =laser->scannerSettings.getLabel();
    ScannerSettings& currentPreset = *presetManager.getPreset(label);
    
    
    bool presetEdited = (laser->scannerSettings!=currentPreset);
    if (presetEdited){
        label+="(edited)";
        
        
    }
    
    
    if (ImGui::BeginCombo("##Scanner presets", label.c_str())) { // The second parameter is the label previewed before opening the combo.
        
        for(const string presetName : presets) {
            
            if (ImGui::Selectable(presetName.c_str(), presetName == laser->scannerSettings.getLabel())) {
                //get the preset and make a copy of it
                // uses operator overloading to create a clone
                laser->scannerSettings = *presetManager.getPreset(presetName);
            }
        }
        
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    
    if(ImGui::Button("EDIT")) {
        //ImGui::OpenPopup("Edit Scanner Preset");
        showEditScannerPreset = true;
    }
    
    //    ImGui::SetNextWindowPos,
    ImGui::SetNextWindowSize({760,0});
    // centre popup
    
    ImGui::SetNextWindowPos({(float)ofGetWidth()/2, (float)ofGetHeight()/2}, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    
    // SCANNER PRESET POPUP
    
    if (showEditScannerPreset && ImGui::Begin("Edit Scanner Preset", &showEditScannerPreset, ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoDocking))
    {
        ImGui::Text("SCANNER SETTINGS - %s",laser->getLabel().c_str());
        ImGui::Separator();
        if (ImGui::BeginCombo("Scanner presets", label.c_str())) { // The second parameter is the label previewed before opening the combo.
            
            for(const string presetName : presets) {
                
                if (ImGui::Selectable(presetName.c_str(), presetName == laser->scannerSettings.getLabel())) {
                    //get the preset and make a copy of it
                    // uses operator overloading to create a clone
                    laser->scannerSettings = *presetManager.getPreset(presetName);
                }
            }
            
            ImGui::EndCombo();
        }
        
        ImGui::SameLine();
        
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * (presetEdited ? 1.0f : 0.5f));
        if(ImGui::Button("SAVE")) {
            if(presetEdited)ImGui::OpenPopup("Save Preset");
            
        }
        ImGui::PopStyleVar();
        
        if (ImGui::BeginPopupModal("Save Preset", 0)){
            string presetlabel = laser->scannerSettings.getLabel();
            
            ImGui::Text("Are you sure you want to overwrite the preset \"%s\"?", presetlabel.c_str());
            ImGui::Separator();
            
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                PresetManager::addPreset(presetlabel, laser->scannerSettings);
                ImGui::CloseCurrentPopup();
                
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                
            }
            ImGui::EndPopup();
            
            
        }
        static char newPresetLabel[255]; // = presetlabel.c_str();
        
        
        ImGui::SameLine();
        if(ImGui::Button("SAVE AS")){
            strcpy(newPresetLabel, laser->scannerSettings.getLabel().c_str());
            ImGui::OpenPopup("Save Preset As");
            
        };
        
        if (ImGui::BeginPopupModal("Save Preset As", 0)){
            
            if(ImGui::InputText("1", newPresetLabel, IM_ARRAYSIZE(newPresetLabel))){
                
            }
            
            ImGui::Separator();
            
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                string presetlabel = newPresetLabel;
                // TODO CHECK PRESET EXISTS AND ADD POP UP
                PresetManager::addPreset(presetlabel, laser->scannerSettings);
                ImGui::CloseCurrentPopup();
                
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                
            }
            ImGui::EndPopup();
            
            
        }
        
        
        UI::addResettableFloatSlider(laser->scannerSettings.moveSpeed, currentPreset.moveSpeed,"How quickly the mirrors move between shapes", "%.1f", 3.0f);
        
        ImGui::Columns(2);
        UI::addResettableIntSlider(laser->scannerSettings.shapePreBlank, currentPreset.shapePreBlank, "The length of time that the laser is switched off and held at the beginning of a shape");
        
        UI::addResettableIntSlider(laser->scannerSettings.shapePreOn, currentPreset.shapePreOn, "The length of time that the laser is switched on and held at the beginning of a shape");
        ImGui::NextColumn();
        
        UI::addResettableIntSlider(laser->scannerSettings.shapePostBlank, currentPreset.shapePostBlank,"The length of time that the laser is switched off and held at the end of a shape" );
        UI::addResettableIntSlider(laser->scannerSettings.shapePostOn, currentPreset.shapePostOn,"The length of time that the laser is switched on and held at the end of a shape" );
        
        ImGui::Columns(1);
        
        ImGui::Text("Render profiles");
        UI::toolTip("Every scanner setting has three profiles for rendering different qualities of laser effects. Unless otherwise specified, the default profile is used. The fast setting is good for long curvy lines, the high detail setting is good for complex pointy shapes.");
        
        ImGui::Separator();
        //bool firsttreeopen = true;
        ImGui::Columns(3);
        ImGui::SetColumnWidth(0, 250);
        ImGui::SetColumnWidth(1, 250);
        ImGui::SetColumnWidth(1, 250);
        
        
        for (auto & renderProfilePair : laser->scannerSettings.renderProfiles) {
            ImGui::PushItemWidth(120);
            string name =renderProfilePair.first;
            RenderProfile& profile = renderProfilePair.second;
            
            RenderProfile& resetProfile = currentPreset.renderProfiles.at(name);
            
            ImGui::Text("%s", name.c_str());
            UI::addResettableFloatSlider(profile.speed,resetProfile.speed, "",  "%.1f", 3.0f);
            UI::addResettableFloatSlider(profile.acceleration,resetProfile.acceleration, "",  "%.2f", 3.0f);
            UI::addResettableIntSlider(profile.dotMaxPoints, resetProfile.dotMaxPoints);
            UI::addResettableFloatSlider(profile.cornerThreshold, resetProfile.cornerThreshold);
            
            ImGui::PopItemWidth();
            ImGui::NextColumn();
        }
        
        
        ImGui::End();
    }
    
    
    
    // COLOUR SETTINGS
    ImGui::Separator();
    ImGui::Text("COLOUR");
    
    if(ImGui::TreeNode("Colour calibration")){
        
        UI::addParameterGroup(laser->colourSettings.params);
        
        ImGui::TreePop();
    }
    
    
    
    ImGui::Separator();
    ImGui::Text("ADVANCED SETTINGS");
    
    // ADVANCED
    
    
    if(ImGui::TreeNode("Advanced")){
        // POINT RATE
        ImGui::PushItemWidth(laserpanelwidth-60);
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 26.0f);
        int ppsslider = laser->pps;
        string ppsstring = "Point rate : " + ofToString(ppsslider);
        if(ImGui::SliderInt("##Point rate", &ppsslider, laser->pps.getMin(), laser->pps.getMax(), ppsstring.c_str())){
            laser->pps.set(ppsslider/100*100);
            
        }
        UI::toolTip("The actual points sent to the laser - YOU DON'T NEED TO ADJUST THIS unless you want to actually change the data rate, or you need better resolution for very fast scanners. The speed of the scanners can be fully adjusted without changing the point rate. ");
        ImGui::PopItemWidth();
        ImGui::PopStyleVar(1);
        
        UI::addParameterGroup(laser->advancedParams);
        ImGui::TreePop();
    }
    
    
    // the arm and disarm buttons
    //int buttonwidth = (mainpanelwidth-(spacing*3))/2;
    UI::secondaryColourButtonStart();
    if(ImGui::Button("DELETE LASER")) {
        ImGui::OpenPopup("Delete?");
    }
    UI::secondaryColourButtonEnd();
    
    
    if (ImGui::BeginPopupModal("Delete?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Are you sure? All the zone settings will be deleted.\n\n");
        ImGui::Separator();
        
        UI::secondaryColourButtonStart();
        
        if (ImGui::Button("DELETE", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            deleteLaser(laser);
            
        }
        UI::secondaryColourButtonEnd();
        
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            
        }
        ImGui::EndPopup();
    }
    
    
    
    
    
    
    ImGui::PopItemWidth();
    
    
    // draw a flashing dot during saving
    if(laser->getSaveStatus() && (ofGetElapsedTimeMillis()%300)<150) {
        ImDrawList*   draw_list = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetWindowPos();
        p.x+=ImGui::GetContentRegionAvailWidth();
        p.y+=30;// + ImGui::GetScrollY();
        // if(ImGui::GetScrollY()>0) p.x-=14;
        //ImGui::GetContentRegionAvailWidth()
        draw_list->AddCircleFilled(p, 4, ImGui::GetColorU32(ImGuiCol_Border));
    }
    
    
    //ImGui::End();
    UI::endWindow();
}
