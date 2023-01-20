//
//  ofxLaserZoneTransformLineData.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/02/2018.
//
//

#include "ofxLaserZoneTransformLineData.h"


using namespace ofxLaser;

// static property / method

ZoneTransformLineData :: ZoneTransformLineData() {
    
    
    isDirty = true;
    
    resetNodes();
    
    // Used for serialize / deserialize
    transformParams.setName("ZoneTransformLineDataParams");
    transformParams.add(zoneWidth.set("Width", 10,0,400));
   
    updateSrc(ofRectangle(0,0,100,100));

    ofAddListener(transformParams.parameterChangedE(), this, &ZoneTransformLineData::paramChanged);
    
}

void ZoneTransformLineData :: paramChanged(ofAbstractParameter& e) {
    isDirty= true;
    
}
ZoneTransformLineData::~ZoneTransformLineData() {
    
    ofRemoveListener(transformParams.parameterChangedE(), this, &ZoneTransformLineData::paramChanged);
}


void ZoneTransformLineData::init(ofRectangle& srcRect) {

    updateSrc(srcRect);
    resetNodes();

    
}

void ZoneTransformLineData::resetNodes() {
    nodes.resize(3);
    nodes[0].reset(300,400);
    nodes[0].start = true;
    nodes[1].reset(400,400);
    nodes[2].reset(500,400);
    nodes[2].end = true;
    
}


bool ZoneTransformLineData::update(){
    if(isDirty) {
        
       
        updateNodes();
        updatePolyline();
        updatePerimeter();
        
        isDirty = false;
        
        return true;
    } else {
        return false;
    }
}

vector<BezierNode>& ZoneTransformLineData::getNodes() {
    return nodes;
}


glm::vec3 ZoneTransformLineData::getVectorNormal(glm::vec3 v1, glm::vec3 v2) {
    glm::vec3 normal = v2-v1;
    normal = glm::normalize(normal);
    normal.x *=-1;
    float x = normal.x;
    normal.x = normal.y;
    normal.y = x;
   
    return normal;
    
}


glm::vec2 ZoneTransformLineData::getCentre() {
    glm::vec2 average;
    for(BezierNode& node : nodes) {
        average+= node.getPosition();
    }
    average/=nodes.size();
    return average;
}

ofPoint ZoneTransformLineData::getWarpedPoint(const ofPoint& p){
    
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

ofPoint ZoneTransformLineData::getUnWarpedPoint(const ofPoint& p){
   
    return p;
    
};


ofxLaser::Point ZoneTransformLineData::getWarpedPoint(const ofxLaser::Point& p){
    ofxLaser::Point rp = p;
    ofPoint ofpoint = p;
    ofpoint = getWarpedPoint(ofpoint);
    rp.set(ofpoint);
    
    return rp;
    
};

void ZoneTransformLineData::updatePolyline() {
    poly.clear();
    BezierNode* previousnode = &nodes[0];
    poly.addVertex(glm::vec3(previousnode->getPosition(),0));
    
    for(int i = 1; i<nodes.size(); i++) {
        BezierNode& node = nodes[i];
                    
        poly.bezierTo(glm::vec3(previousnode->getControlPoint2(),0), glm::vec3(node.getControlPoint1(),0), glm::vec3(node.getPosition(),0));
        
        previousnode = &node;
            
    }

    
    poly = poly.getResampledBySpacing(1);
    
}

void ZoneTransformLineData :: updatePerimeter() {
    
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

void ZoneTransformLineData::updateNodes() {
    for(int i = 0; i<nodes.size(); i++) {
        BezierNode& node = nodes[i];
        
        node.start = (i==0);
        node.end = (i==nodes.size()-1);
    }
}
bool ZoneTransformLineData::deleteNode(int i){
    
    if(nodes.size()<=2) return false;
    
    if((i>0) && (i<nodes.size()) ) {
        nodes.erase(nodes.begin()+i);
        isDirty = true;
        return true;
    } else {
        return false;
    }
}

void ZoneTransformLineData :: addNode() {

    glm::vec3 endpos = glm::vec3(nodes.back().getPosition(),0);
    int mode = nodes.back().mode;
    glm::vec3 tangent = poly.getTangentAtIndexInterpolated(poly.getIndexAtPercent(0.99));
    tangent*=10;
    endpos+=tangent;
    
    nodes.resize(nodes.size()+1);
    BezierNode& node = nodes.back();
    node.reset(endpos.x, endpos.y);
    node.mode = mode;
    
    isDirty = true;
    
}


void ZoneTransformLineData::getPerimeterPoints(vector<glm::vec2>& points) {
    
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




bool ZoneTransformLineData::serialize(ofJson&json) {
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

bool ZoneTransformLineData::deserialize(ofJson& jsonGroup) {
    //ofLogNotice("ZoneTransformLineData::deserialize()");
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

    
   // ofLogNotice("ZoneTransformLineData::deserialize");
    return true;
}

//
//bool ZoneTransformLineData::setSelected(bool v) {
//    
//    if(ZoneTransformBase::setSelected(v)) {
//        if(!selected) {
//            for(size_t i= 0; i<nodes.size(); i++) {
//                nodes[i].stopDrag();
//            }
//        }
//        return true;
//    } else {
//        return false;
//    }
//    
//};
//
