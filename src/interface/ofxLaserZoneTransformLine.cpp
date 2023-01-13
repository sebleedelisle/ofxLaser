//
//  ofxLaserZoneTransformLine.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/02/2018.
//
//

#include "ofxLaserZoneTransformLine.h"


using namespace ofxLaser;

// static property / method

ZoneTransformLine :: ZoneTransformLine() {
    
    initListeners();
    isDirty = true;
    
    resetNodes();
    
    // Used for serialize / deserialize
    transformParams.setName("ZoneTransformLineParams");
    transformParams.add(zoneWidth.set("Width", 10,0,400));
   
    updateSrc(ofRectangle(0,0,100,100));

    ofAddListener(transformParams.parameterChangedE(), this, &ZoneTransformLine::paramChanged);
    
  
    updateHandleColours();
}

void ZoneTransformLine :: paramChanged(ofAbstractParameter& e) {
    isDirty= true;
    
}
ZoneTransformLine::~ZoneTransformLine() {
    removeListeners();
    
    ofRemoveListener(transformParams.parameterChangedE(), this, &ZoneTransformLine::paramChanged);
}


void ZoneTransformLine::init(ofRectangle& srcRect) {

    updateSrc(srcRect);
    resetNodes(); 

    
}

void ZoneTransformLine::resetNodes() {
    nodes.resize(3);
    nodes[0].reset(300,400);
    nodes[0].start = true;
    nodes[1].reset(400,400);
    nodes[2].reset(500,400);
    nodes[2].end = true;
    
    for(BezierNode& node : nodes) {
       // node.setGrid(snapToGrid, gridSize);
    }
    
}


bool ZoneTransformLine::update(){
    if(isDirty) {
        
        updateHandleColours();
        updateNodes();
        updatePolyline();
        updatePerimeter();
        
        isDirty = false;
        
        return true;
    } else {
        return false;
    }
}

vector<BezierNode>& ZoneTransformLine::getNodes() {
    return nodes;
} 

void ZoneTransformLine::draw(string label) {
    
    if(!visible) return ;
    
    ofPushMatrix();
    ofTranslate(offset);
    ofScale(scale, scale);
    ofSetColor(selected?uiZoneFillColourSelected : uiZoneFillColour);
   
   
    
    ofFill();
    const ofPolyline& polyconst = polyPerimeter;
    polyPerimeter.draw();
    ofBeginShape();
    const auto& vertices = polyconst.getVertices();
    for(auto& vertex : vertices) {
        ofVertex(vertex);
    }
  
    ofEndShape();
    ofNoFill();
    
    ofSetColor(uiZoneStrokeColour);
    polyPerimeter.draw();
    poly.draw();
//    ofDrawLine(handles[0], handles[1]);
    
//    ofDrawLine(handles[0]-(tangent1*zoneWidth.get()), handles[0]+(tangent1*zoneWidth.get()));
//    ofDrawLine(handles[1]-(tangent1*zoneWidth.get()), handles[1]+(tangent1*zoneWidth.get()));

    ofDrawBitmapString(label,getCentre() - ofPoint(4*label.size(),5));
    
    if(selected && editable) {
        for(size_t i = 0; i<nodes.size(); i++) {
            nodes[i].draw(mousePos,scale);
        }
    }
    
    ofPopMatrix();
    
   
}

glm::vec3 ZoneTransformLine::getVectorNormal(glm::vec3 v1, glm::vec3 v2) {
    glm::vec3 normal = v2-v1;
    normal = glm::normalize(normal);
    normal.x *=-1;
    float x = normal.x;
    normal.x = normal.y;
    normal.y = x;
   
    return normal;
    
}


ofPoint ZoneTransformLine::getCentre() {
    glm::vec3 average;
    for(BezierNode& node : nodes) {
        average+= node.getPosition();
    }
    average/=nodes.size();
    return average;
}

ofPoint ZoneTransformLine::getWarpedPoint(const ofPoint& p){
    
    float xunit = ofMap(p.x, srcRect.getLeft(), srcRect.getRight(), 0, 1);
    float yunit = ofMap(p.y, srcRect.getTop(), srcRect.getBottom(), 1, -1);
    
    float polylength = poly.getPerimeter();
    float polyx = xunit*polylength;
    
    //ofPoint normal = getVectorNormal(handles[0], handles[1]);
    
    ofPoint posx = poly.getPointAtLength(polyx);
    float index = poly.getIndexAtLength(polyx);
    // fixing a stupid bug in ofPolyline
    if(index == 0) index = 1;
    if(index >= poly.size()-2) index = poly.size()-3;
    glm::vec3 normal = poly.getNormalAtIndexInterpolated(index);
    if(glm::length(normal)==0) {
        cout << " " ; 
        
    }
    
    
    
    return posx+(normal*yunit*zoneWidth.get());

    
};

ofPoint ZoneTransformLine::getUnWarpedPoint(const ofPoint& p){
   
    return p;
    
};


ofxLaser::Point ZoneTransformLine::getWarpedPoint(const ofxLaser::Point& p){
    ofxLaser::Point rp = p;
    ofPoint ofpoint = p;
    ofpoint = getWarpedPoint(ofpoint);
    rp.set(ofpoint);
    
    return rp;
    
};

void ZoneTransformLine::updatePolyline() {
    poly.clear();
    BezierNode* previousnode = &nodes[0];
    poly.addVertex(previousnode->getPosition());
    
    for(int i = 1; i<nodes.size(); i++) {
        BezierNode& node = nodes[i];
                    
        poly.bezierTo(previousnode->getControlPoint2(), node.getControlPoint1(), node.getPosition());
        
        previousnode = &node;
            
    }

    
    poly = poly.getResampledBySpacing(1);
    
}

void ZoneTransformLine :: updatePerimeter() {
    
    int resolution = 1;
    polyPerimeter.clear();
    
    float polylength = poly.getPerimeter();
    for(float d = 0; d<=polylength; d+=resolution) {
        
        glm::vec3 p = poly.getPointAtLength(d);
        float index = poly.getIndexAtLength(d);
        glm::vec3 v = poly.getNormalAtIndexInterpolated(index);
        
        //polyPerimeter.addVertex(p);
     
        polyPerimeter.addVertex(p+(v*zoneWidth.get()));
        
        
    }
    for(float d = polylength; d>=0; d-=resolution) {
        
        glm::vec3 p = poly.getPointAtLength(d);
        float index = poly.getIndexAtLength(d);
        glm::vec3 v = poly.getNormalAtIndexInterpolated(index);
        
        //polyPerimeter.addVertex(p);
     
        polyPerimeter.addVertex(p-(v*zoneWidth.get()));
        
        
    }
    polyPerimeter.setClosed(true);
    
}

void ZoneTransformLine::updateNodes() {
    for(int i = 0; i<nodes.size(); i++) {
        BezierNode& node = nodes[i];
        
        node.start = (i==0);
        node.end = (i==nodes.size()-1);
    }
}
bool ZoneTransformLine::deleteNode(int i){
    
    if(nodes.size()<=2) return false;
    
    if((i>0) && (i<nodes.size()) ) {
        nodes.erase(nodes.begin()+i);
        isDirty = true;
        return true;
    } else {
        return false;
    }
}

void ZoneTransformLine :: addNode() {

    glm::vec3 endpos = nodes.back().getPosition();
    int mode = nodes.back().mode;
    glm::vec3 tangent = poly.getTangentAtIndexInterpolated(poly.getIndexAtPercent(0.99));
    endpos+=(tangent*10);
    
    nodes.resize(nodes.size()+1);
    BezierNode& node = nodes.back();
    node.reset(endpos.x, endpos.y);
    node.mode = mode;
    node.setGrid(snapToGrid, gridSize); 
    
    isDirty = true;
    
}


void ZoneTransformLine::getPerimeterPoints(vector<glm::vec3>& points) {
    
    points.clear();
    
   
    const ofPolyline& pline_ref= polyPerimeter;
    auto& verts= pline_ref.getVertices();
    auto vertices = pline_ref.getVertices();
    for(auto&  vertex : vertices) {
        points.push_back(vertex);
    }
//    glm::vec3 tangent1 = handles[1]-handles[0];
//    tangent1 = glm::normalize(tangent1);
//    tangent1.x *=-1;
//    tangent1.z = tangent1.x;
//    tangent1.x = tangent1.y;
//    tangent1.y = tangent1.z;
//    tangent1.z = 0;
//    points.push_back(handles[0]-(tangent1*zoneWidth.get()));
//    points.push_back(handles[0]+(tangent1*zoneWidth.get()));
//    points.push_back(handles[1]+(tangent1*zoneWidth.get()));
//    points.push_back(handles[1]-(tangent1*zoneWidth.get()));
}



void ZoneTransformLine::initListeners() {
    
    ofAddListener(ofEvents().mouseMoved, this, &ZoneTransformLine::mouseMoved, OF_EVENT_ORDER_AFTER_APP);
    ofAddListener(ofEvents().mousePressed, this, &ZoneTransformLine::mousePressed, OF_EVENT_ORDER_AFTER_APP);
    ofAddListener(ofEvents().mouseReleased, this, &ZoneTransformLine::mouseReleased, OF_EVENT_ORDER_AFTER_APP);
    ofAddListener(ofEvents().mouseDragged, this, &ZoneTransformLine::mouseDragged, OF_EVENT_ORDER_AFTER_APP);
    
    
}

void ZoneTransformLine :: removeListeners() {
    
    ofRemoveListener(ofEvents().mouseMoved, this, &ZoneTransformLine::mouseMoved, OF_EVENT_ORDER_AFTER_APP);
    ofRemoveListener(ofEvents().mousePressed, this, &ZoneTransformLine::mousePressed, OF_EVENT_ORDER_AFTER_APP);
    ofRemoveListener(ofEvents().mouseReleased, this, &ZoneTransformLine::mouseReleased, OF_EVENT_ORDER_AFTER_APP);
    ofRemoveListener(ofEvents().mouseDragged, this, &ZoneTransformLine::mouseDragged, OF_EVENT_ORDER_AFTER_APP);
    
}

void ZoneTransformLine :: mouseMoved(ofMouseEventArgs &e){
    
    
//    if((!editable) || (!visible)) return;
//
    mousePos = e;
    mousePos-=offset;
    mousePos/=scale;
    //return false;
    
}

bool ZoneTransformLine :: mousePressed(ofMouseEventArgs &e){
    //ofLogNotice("ZoneTransformLine::mousePressed");
    // TODO there is currently an issue where if a zone is on top of another
    // zone, you can't click on a handle underneath. Not sure of how to fix this...
    // but possibly needs some higher level logic than here.
    
    if((!editable) || (!visible)) return false;
    //if(ofGetKeyPressed(' ')) return ; // for dragging around previews.
    
    mousePos = e;
    mousePos-=offset;
    mousePos/=scale;
    
    
    bool hit = hitTest(mousePos);
    if((hit) &&(!selected)) {
        selected = true;
        return false; //  propogates
    }
    
    
    if(!selected) {
        return false; // propogates
    }
    
    bool handleHit = false;
    
    if(!locked) {
        // this section of code if we click drag anywhere in the zone
        
        for(int i= nodes.size()-1; i>=0 && !handleHit; i--) {
            
            if(nodes[i].hitTest(mousePos, scale)) {
                nodes[i].startDrag(mousePos, scale);
                handleHit = true;
                    
            }
            
        }
        
        // if the middle is clicked then drag all the handles!
        if(!handleHit && hit) {
            
            //centreHandle.startDrag(mousePoint);
            handleHit = true;
            DragHandle* controlHandle = &nodes[0].handles[0];
            nodes[0].startDragAll(mousePos);

            for(size_t i= 1; i<nodes.size(); i++) {
                
                nodes[i].startDragAll(mousePos, controlHandle);
            }
            
            
        }
    }
    if(!handleHit && !hit) {
        selected = false;
    }
    
    return false; // propogates, was : handleHit || hit;
    
}




void ZoneTransformLine :: mouseDragged(ofMouseEventArgs &e){
    
    if((!editable) || (!visible)) return ;
    if(!selected) return ;
    
    ofPoint mousePoint;
    mousePoint.x = e.x;
    mousePoint.y = e.y;
    mousePoint-=offset;
    mousePoint/=scale;
    
    //ofRectangle bounds(centreHandle, 0, 0);
    int dragCount = 0;
    for(size_t i= 0; i<nodes.size(); i++) {
        if(nodes[i].updateDrag(mousePoint)) dragCount++;
        //bounds.growToInclude(handles[i]);
    }
    //	if(!dragging) {
    //		dragging = centreHandle.updateDrag(mousePoint);
    //	} else {
    //		updateCentreHandle();
    //
    //	}
    
    isDirty |= (dragCount>0);
    //if((dragCount>0)&&(!editSubdivisions)) resetFromCorners();
    
    //return dragCount>0;
    
    
}


void ZoneTransformLine :: mouseReleased(ofMouseEventArgs &e){
    
    //if(!editable) return false;
    if(!selected) return;
    
    bool wasDragging = false;
    
    for(size_t i= 0; i<nodes.size(); i++) {
        if(nodes[i].stopDrag()) wasDragging = true;
    }
    
    // TODO mark as dirty so auto save ********************
    //saveSettings();
    isDirty|=wasDragging;
    //return wasDragging;
    
}

bool ZoneTransformLine::hitTest(ofPoint mousePoint) {
    
//    ofPolyline poly;
//    for(int i = 0; i<handles.size(); i++) {
//        poly.addVertex(handles[i]);
//    }

    for(BezierNode& node : nodes) {
        if(node.hitTest(mousePoint, scale) ) return true;
        
    }
    
    if(polyPerimeter.inside(mousePoint)) {
        return true;
    }
    
    float distance = MAX(zoneWidth.get()/scale, 2.0f);
    
    ofPoint closest = poly.getClosestPoint(mousePoint);
    if((zoneWidth*scale<2) &&
        (closest.distance(mousePoint)<(distance))) {
        return true;
    } else {
        return false;
    }

}


bool ZoneTransformLine::serialize(ofJson&json) {
    ofSerialize(json, transformParams);
    ofJson& nodesjson = json["nodes"];
    for(size_t i= 0; i<nodes.size(); i++) {
        BezierNode& node = nodes[i];
        
        ofJson nodejson;
        node.serialize(nodejson);
        nodesjson.push_back(nodejson);

    }
    
    //cout << json.dump(3) << endl;
    
    return true;
}

bool ZoneTransformLine::deserialize(ofJson& jsonGroup) {
    //ofLogNotice("ZoneTransformLine::deserialize()");
    // note that ofDeserialize looks for the json group
    // with the same name as the parameterGroup
    ofDeserialize(jsonGroup, transformParams);
    
    if(jsonGroup.contains("nodes")) {
        ofJson& nodesjson = jsonGroup["nodes"];
        int numnodes = nodesjson.size();
        nodes.resize(numnodes);
    
        for(int i = 0; i<numnodes; i++) {
            ofJson& nodejson = nodesjson[i];
            nodes[i].deserialize(nodejson);
            nodes[i].setGrid(snapToGrid, gridSize);
            
        }
    }

//    dstHandles.resize(numhandles);
//
//    ofJson& handlejson = jsonGroup["handles"];
//    cout << handlejson.dump(3) << endl;
//    if((int)handlejson.size()>=numhandles) {
//        for(int i = 0; i<numhandles; i++) {
//            ofJson& point = handlejson[i];
//            dstHandles[i].x = point[0];
//            dstHandles[i].y = point[1];
//            dstHandles[i].z = 0;
//            cout << "setting handle " << i << " : " << dstHandles[i] << endl;
//
//        }
//    }

    
   // ofLogNotice("ZoneTransformLine::deserialize");
    return true;
}




bool ZoneTransformLine :: setGrid(bool snapstate, int gridsize) {
    if(ZoneTransformBase :: setGrid(snapstate, gridsize)) {
        for(auto& node : nodes) {
            node.setGrid(snapToGrid, gridSize);
        }
        return true;
    } else {
        return false;
    }
}
void ZoneTransformLine::setHue(int hue) {
    ZoneTransformBase :: setHue(hue);
    updateHandleColours();
  
}
void ZoneTransformLine::updateHandleColours() {
    
    for(size_t i= 0; i<nodes.size(); i++) {
        nodes[i].setColour(uiZoneHandleColour, uiZoneHandleColourOver);
    }

   
}


bool ZoneTransformLine::setSelected(bool v) {
    
    if(ZoneTransformBase::setSelected(v)) {
        if(!selected) {
            for(size_t i= 0; i<nodes.size(); i++) {
                nodes[i].stopDrag();
            }
        }
        return true;
    } else {
        return false;
    }
    
};

