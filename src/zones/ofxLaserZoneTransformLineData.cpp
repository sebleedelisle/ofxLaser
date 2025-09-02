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
    
    
    setDirty(true);
    
    resetNodes();
    
    // Used for serialize / deserialize
    transformParams.setName("ZoneTransformLineDataParams");
    transformParams.add(zoneWidth.set("Width", 10,0,400));
   
    updateSrc(ofRectangle(0,0,100,100));
    setDefault();

    ofAddListener(transformParams.parameterChangedE(), this, &ZoneTransformLineData::paramChanged);
    
}

void ZoneTransformLineData :: setDefault() {
   
    resetNodes();
    autoSmooth = true;
    zoneWidth = 64;
    
}

void ZoneTransformLineData :: paramChanged(ofAbstractParameter& e) {
    setDirty(true);
    
}
ZoneTransformLineData::~ZoneTransformLineData() {
    
    ofRemoveListener(transformParams.parameterChangedE(), this, &ZoneTransformLineData::paramChanged);
}


void ZoneTransformLineData::init() {
    setDefault();
    resetNodes();

}

bool ZoneTransformLineData::setFromPoints(const vector<glm::vec2*> points) {
    vector<glm::vec2>points2;
    for(glm::vec2* p : points) {
        points2.push_back(*p);
    }
    bool changed = setFromPoints(points2);
   
    return changed;
}
bool ZoneTransformLineData::setFromPoints(vector<glm::vec2> points) {

    bool changed = false;
    
    int numAnchors = points.size()/3;
    
    if(nodes.size()!=numAnchors) {
        nodes.resize(numAnchors);
        changed = true;
    }
    
    for(size_t i = 0; i<points.size(); i+=3) {
        BezierNode& node = nodes[i/3];
        //const glm::vec2& point = points[i];
        changed = node.setFromAnchorAndControlPoints(points[i], points[i+1], points[i+2]) || changed;
    }
    
    if(autoSmooth) { //  && changed) {
        updateCurves();
    }
    
    
    if(changed) {
        setDirty(true); 
        glm::vec2 delta = getVectorToBringWithinBoundingBox();
        //ofLogNotice("delta : ") << delta;
        if(glm::length(delta)>0) {
            for(BezierNode& node : nodes) {
                node.translate(delta);
            }
            
            
        }
    }
    return changed;
    
}
bool ZoneTransformLineData :: moveHandle(int handleindex, glm::vec2 newpos) {
    int nodenum = floor(handleindex/3);
    int pointnum = handleindex%3;
    BezierNode& node = nodes[nodenum];
    glm::vec2& handleToMove = node.handles[pointnum];
    glm::vec2 diff = newpos - handleToMove;
    
    if(handleToMove!=newpos) {

        handleToMove+=diff;

        if(autoSmooth ) {
            updateCurves();
        } else {
            if(pointnum==0) {
                node.handles[1]+=diff;
                node.handles[2]+=diff;
            } else {
                int oppositeNodeHandleIndex = (1- (pointnum-1)) +1; // switches 1 to 2 and 2 to 1
                node.handles[oppositeNodeHandleIndex] = node.handles[0] - (node.handles[pointnum] - node.handles[0]);
                
            }
        }
        
        setDirty(true);
        return true;
    }
    
    return false;
}

void ZoneTransformLineData :: updateCurves () {
    
    // Cardinal spline algorithm.
    float scale = smoothLevel /3 * 2;
    
    for(int i = 0; i<nodes.size(); i++)  {
        
        BezierNode& node = nodes[i];
        glm::vec2 previous;
        if(i > 0) {
            previous = nodes[i-1].getPosition();
        } else {
            previous = node.getPosition() - (nodes[i+1].getPosition() - node.getPosition());
        }
        glm::vec2 next;
        if(i<nodes.size()-1) {
            next = nodes[i+1].getPosition();
        } else {
            next = node.getPosition() + ( node.getPosition() - nodes[i-1].getPosition());
        }
            
        glm::vec2 velocity = (next-previous) * scale;
       // ofLogNotice()<< velocity;
        node.setControlPoints(node.getPosition() - velocity, node.getPosition() + velocity );
        
    }
    
    // try and fix the ends
    BezierNode& nodeA = nodes[0];
    BezierNode& nodeB = nodes[1];
    
    glm::vec2 nodeBcpVector = nodeB.getControlPoint1() - nodeB.getPosition();
    glm::vec2 vecAB = glm::normalize(nodeB.getPosition()-nodeA.getPosition());
    glm::vec2 proj = glm::dot(nodeBcpVector, vecAB)* vecAB;
    nodeA.handles[2] = nodeA.getPosition() + nodeBcpVector - (proj * 2);
    nodeA.handles[1] = nodeA.getPosition();
    
    BezierNode& nodeD = nodes.back();
    BezierNode& nodeC = nodes[nodes.size()-2];
    
    glm::vec2 nodeDcpVector = nodeC.getControlPoint2() - nodeC.getPosition();
    glm::vec2 vecDC = glm::normalize(nodeC.getPosition()-nodeD.getPosition());
    glm::vec2 proj2 = glm::dot(nodeDcpVector, vecDC)* vecDC;
    nodeD.handles[1] = nodeD.getPosition() + nodeDcpVector - (proj2 * 2);
    nodeD.handles[2] = nodeD.getPosition();
    
}


void ZoneTransformLineData::resetNodes() {
    nodes.resize(2);
    nodes[0].reset(192,304);
    nodes[0].start = true;
    //nodes[1].reset(400,400);
    nodes[1].reset(608,304);
    nodes[1].end = true;
    
}


bool ZoneTransformLineData::update(){
    if(getIsDirty()) {

        if(autoSmooth) updateCurves();
        
        updateNodes();
        updatePolyline();
        updatePerimeter();
        setDirty(false);
        
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
    
    float polylength = linePoly.getPerimeter();
    float polyx = xunit*polylength;
    
    //ofPoint normal = getVectorNormal(handles[0], handles[1]);
    
    ofPoint posx = linePoly.getPointAtLength(polyx);
    float index = linePoly.getIndexAtLength(polyx);
    // fixing a stupid bug in ofPolyline
    if(index == 0) index = 1;
    if(index >= linePoly.size()-2) index = linePoly.size()-3;
    glm::vec3 normal = linePoly.getNormalAtIndexInterpolated(index);
    if(glm::length(normal)==0) {
        //cout << " " ;
        
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
    linePoly.clear();
    BezierNode* previousnode = &nodes[0];
    linePoly.addVertex(glm::vec3(previousnode->getPosition(),0));
    
    for(int i = 1; i<nodes.size(); i++) {
        BezierNode& node = nodes[i];
                    
        linePoly.bezierTo(glm::vec3(previousnode->getControlPoint2(),0), glm::vec3(node.getControlPoint1(),0), glm::vec3(node.getPosition(),0));
        
        previousnode = &node;
            
    }
    
    linePoly = linePoly.getResampledBySpacing(1);
    ofPath path;
}

void ZoneTransformLineData :: updatePerimeter() {
    
    int resolution = 1;
    perimeterPoly.clear();
    
    float polylength = linePoly.getPerimeter();
    for(float d = 0; d<=polylength; d+=resolution) {
        
        glm::vec3 p = linePoly.getPointAtLength(d);
        float index = linePoly.getIndexAtLength(d);
        glm::vec3 v = linePoly.getNormalAtIndexInterpolated(index);
        
        //polyPerimeter.addVertex(p);
     
        perimeterPoly.addVertex(p+(v*zoneWidth.get()));
        
        
    }
    for(float d = polylength; d>=0; d-=resolution) {
        
        glm::vec3 p = linePoly.getPointAtLength(d);
        float index = linePoly.getIndexAtLength(d);
        glm::vec3 v = linePoly.getNormalAtIndexInterpolated(index);
        
        //polyPerimeter.addVertex(p);
     
        perimeterPoly.addVertex(p-(v*zoneWidth.get()));
        
        
    }
    perimeterPoly.setClosed(true);
    
}

ofRectangle ZoneTransformLineData::getBoundingBox() {
    if(!boundingBoxDirty) return boundingBox;
    vector<glm::vec2> points;
    for(BezierNode& node : nodes) {
        points.push_back(node.getPosition());
    }
    if(points.size()==0) {
        boundingBox.set(0,0,0,0);
    } else {
        boundingBox.set(points[0], 0,0);
        for(glm::vec2& p : points) {
            
            boundingBox.growToInclude(p);
        }
    }
    
    boundingBoxDirty = false;
    return boundingBox;

}
void ZoneTransformLineData::updateNodes() {
    for(int i = 0; i<nodes.size(); i++) {
        BezierNode& node = nodes[i];
        
        node.start = (i==0);
        node.end = (i==nodes.size()-1);
    }
}

bool ZoneTransformLineData::deleteNode(int i){
    ofLogNotice("Deleting node at index ") << i << " out of " << nodes.size();
    if(nodes.size()<=2) return false;
    
    if((i>=0) && (i<nodes.size()) ) {
        nodes.erase(nodes.begin()+i);
        setDirty(true);
        return true;
    } else {
        return false;
    }
}

void ZoneTransformLineData :: addNode(glm::vec2 pos) {
    
    // ugh impossible to find the node index!
    // I have to make separate bezier curves from each segment
    // and iterate through to see if the new point is on that curve!!!!
    int insertionIndex = 0;
    ofPolyline curve;
    for(int i = 0; i<nodes.size()-1; i++) {
        curve.clear();
        BezierNode& node1 = nodes[i];
        BezierNode& node2 = nodes[i+1];
        curve.addVertex(glm::vec3(node1.getPosition(),0));
        curve.bezierTo(glm::vec3(node1.getControlPoint2(),0), glm::vec3(node2.getControlPoint1(),0), glm::vec3(node2.getPosition(),0));
        curve = curve.getResampledBySpacing(1);
        float curvelength = curve.getPerimeter();
        for(float l = 0; l<curvelength; l++) {
            glm::vec2 curvepoint = curve.getPointAtLength(l);
            if(glm::distance(curvepoint, pos)<2) {
                insertionIndex = i+1;
                break;
            }
            
        }
        
    }
    
    //glm::vec3 tangent = linePolyBezier.getTangentAtIndexInterpolated(indexInterpolated);
    ofLogNotice("Inserting node at index ") << insertionIndex << " out of " << nodes.size();
    BezierNode node;
    node.reset(pos.x, pos.y);
    nodes.insert(nodes.begin() + insertionIndex, node);

//    float distance = linePolyBezier.getClosestPoint(<#const vec<3, float> &target#>)
//    float indexinterpolated = linePolyBezier.indexAtLength;
//    
//    
//    glm::vec3 endpos = glm::vec3(nodes.back().getPosition(),0);
//    //int mode = nodes.back().mode;
//    glm::vec3 tangent = linePoly.getTangentAtIndexInterpolated(linePoly.getIndexAtPercent(0.99));
//    tangent*=100;
//    endpos+=tangent;
//    
//    nodes.resize(nodes.size()+1);
//    BezierNode& node = nodes.back();
//    node.reset(endpos.x, endpos.y);

    setDirty(true); 
    
}

void ZoneTransformLineData::getPerimeterPoints(vector<glm::vec2>& points) {
    
    points.clear();
    
   
    const ofPolyline& pline_ref= perimeterPoly;
    auto& verts= pline_ref.getVertices();
    auto vertices = pline_ref.getVertices();
    for(auto&  vertex : vertices) {
        points.push_back(vertex);
    }

}


ofPolyline& ZoneTransformLineData::getLinePoly() {
    return linePoly;
} 

void ZoneTransformLineData::serialize(ofJson&json) const {
    ofSerialize(json, transformParams);
    ofJson& nodesjson = json["nodes"];
    for(size_t i= 0; i<nodes.size(); i++) {
        const BezierNode& node = nodes[i];
        
        ofJson nodejson;
        node.serialize(nodejson);
        nodesjson.push_back(nodejson);

    }
    json["smoothlevel"] = smoothLevel;
    json["autosmooth"] = autoSmooth;
    
    //cout << json.dump(3) << endl;
    
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
    if(jsonGroup.contains("smoothlevel") ) {
        smoothLevel = jsonGroup["smoothlevel"].get<float>();
    }
    if(jsonGroup.contains("autosmooth") ) {
        autoSmooth = jsonGroup["autosmooth"].get<bool>();
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
