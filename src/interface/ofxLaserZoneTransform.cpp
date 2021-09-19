//
//  ofxLaserZoneTransform.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/02/2018.
//
//

#include "ofxLaserZoneTransform.h"

using namespace ofxLaser;

// static property / method
		 


ZoneTransform::ZoneTransform() {

	scale = 1;
	offset.set(0,0);
	initListeners();
	editable = true;
	isDirty = true;
	selected = false;
	
	dstHandles.resize(4);
	srcPoints.resize(4);
	editSubdivisions = false;
	
    // Used for serialize / deserialize
	params.setName("ZoneTransformParams");

	params.add(xDivisionsNew.set("x divisions", 1,1,6));
	params.add(yDivisionsNew.set("y divisions", 1,1,6));
	params.add(editSubdivisions.set("edit subdivisions", false));
	params.add(useHomography.set("perspective", true));

	xDivisions = 1;
	yDivisions = 1;
	setSrc(ofRectangle(0,0,100,100));
	setDst(ofRectangle(100,100,200,200));
   
    xDivisionsNew.addListener(this, &ZoneTransform::divisionsChanged);
    yDivisionsNew.addListener(this, &ZoneTransform::divisionsChanged);
    
    ofAddListener(params.parameterChangedE(), this, &ZoneTransform::paramChanged);

}

void ZoneTransform :: paramChanged(ofAbstractParameter& e) {
    isDirty= true; 
    
}
ZoneTransform::~ZoneTransform() {
    removeListeners();
    xDivisionsNew.removeListener(this, &ZoneTransform::divisionsChanged);
    yDivisionsNew.removeListener(this, &ZoneTransform::divisionsChanged);
    ofRemoveListener(params.parameterChangedE(), this, &ZoneTransform::paramChanged);
}


void ZoneTransform::init(ofRectangle& srcRect) {
	
		float srcwidth = srcRect.getWidth();
		float srcheight = srcRect.getHeight();
		
		setSrc(srcRect);
    
        // TODO - better default???

        ofRectangle destRect(200,200,400,400);
    
        //= srcRect;
		//destRect.scale(srcwidth/800, srcheight/800);
		//destRect.x*=srcwidth/800;
		//destRect.y*=srcheight/800;
		setDst(destRect);
		
		updateDivisions();

}


bool ZoneTransform::update(){
	if(isDirty) {
		updateQuads();
        
        isDirty = false;
        return true;
    } else {
        return false;
    }
	

}
void ZoneTransform :: setEditable(bool warpvisible){
	editable = warpvisible;
}
void ZoneTransform :: setVisible(bool warpvisible){
    visible = warpvisible;
}
void ZoneTransform::draw(string label) {
	
    if(!visible) return ;
	ofPushMatrix();
	ofTranslate(offset);
	ofScale(scale, scale);
	
	ofSetColor(150,150,255);
	ofDrawBitmapString(label,getCentre() - ofPoint(4*label.size(),5));
	
	for(size_t i= 0; i<dstHandles.size(); i++) {
		int x = i%(xDivisions+1);
		int y = (int)i/(xDivisions+1);
				
		ofColor edge = ofColor(255);
		ofColor inside  = editSubdivisions?ofColor(100,100,255):ofColor(0,0,255);
		
		if(x<xDivisions) {
			if((y>0)&&(y<yDivisions)) {
				ofSetColor(inside);
			} else {
				ofSetColor(edge);
			}
			UI::drawDashedLine(dstHandles[i], dstHandles[i+1], 6, scale);
		}
		if(y<yDivisions) {
			if((x>0)&&(x<xDivisions)) {
				ofSetColor(inside);
			} else {
				ofSetColor(edge);
			}
            UI::drawDashedLine(dstHandles[i], dstHandles[i+xDivisions+1], 6,scale );
		}
	}
	
	if(selected && editable) {
		for(size_t i = 0; i<dstHandles.size(); i++) {
			if((editSubdivisions) || (isCorner((int)i))) dstHandles[i].draw(mousePos,scale);
		}
	}
	ofPopMatrix();
}

ofPoint ZoneTransform::getCentre() {
	ofPoint centre = srcRect.getCenter();
	return getWarpedPoint(centre);
	
	
}
ofPoint ZoneTransform::getWarpedPoint(const ofPoint& p){
	ofPoint rp = p - srcRect.getTopLeft();
	
	int x = (rp.x / srcRect.getWidth()) * (float)(xDivisions);
	int y = (rp.y / srcRect.getHeight()) * (float)(yDivisions);
	
	//ofLog(OF_LOG_NOTICE, ofToString(x) + " " + ofToString(y));
	
	x = ofClamp(x,0,xDivisions-1);
	y = ofClamp(y,0,yDivisions-1);
	
	int quadnum = x + (y*xDivisions);
	Warper & quad = quadWarpers[quadnum];
	return quad.getWarpedPoint(p, useHomography);
	
};

ofPoint ZoneTransform::getUnWarpedPoint(const ofPoint& p){
	ofPoint rp = p - srcRect.getTopLeft();
	
	int x = (rp.x / srcRect.getWidth()) * (float)(xDivisions);
	int y = (rp.y / srcRect.getHeight()) * (float)(yDivisions);
	
	//ofLog(OF_LOG_NOTICE, ofToString(x) + " " + ofToString(y));
	
	x = ofClamp(x,0,xDivisions-1);
	y = ofClamp(y,0,yDivisions-1);
	
	int quadnum = x + (y*xDivisions);
	Warper & quad = quadWarpers[quadnum];
	return quad.getUnWarpedPoint(p);
	
};


ofxLaser::Point ZoneTransform::getWarpedPoint(const ofxLaser::Point& p){
	ofxLaser::Point rp = p;
	rp.x-=srcRect.getTopLeft().x;
	rp.y-=srcRect.getTopLeft().y;
	
	int x = (rp.x / srcRect.getWidth()) * (float)(xDivisions);
	int y = (rp.y / srcRect.getHeight()) * (float)(yDivisions);
	
	//ofLog(OF_LOG_NOTICE, ofToString(x) + " " + ofToString(y));
	
	x = ofClamp(x,0,xDivisions-1);
	y = ofClamp(y,0,yDivisions-1);
	
	int quadnum = x + (y*xDivisions);
	Warper & quad = quadWarpers[quadnum];
	return quad.getWarpedPoint(p, useHomography);
	
};
//
//Point getUnWarpedPoint(const Point& p){
//	return p;
//};
void ZoneTransform::setSrc(const ofRectangle& rect) {
	srcRect = rect;
	// update source points?
	int xpoints = xDivisions+1;
	int ypoints = yDivisions+1;
	
	int numpoints = xpoints*ypoints;
	
	// srcPoints should already have enough but let's check
	if((int)srcPoints.size()!= numpoints) {
		srcPoints.resize(numpoints);
	}
	
	for(int i= 0; i<numpoints; i++) {
		float x = ofMap(i%xpoints, 0, xDivisions, rect.getLeft(), rect.getRight());
		float y = ofMap(i/xpoints, 0, yDivisions, rect.getTop(), rect.getBottom());
		
		//ofLog(OF_LOG_NOTICE, ofToString(x) + " " +ofToString(y));
		
		srcPoints[i] = glm::vec3(x, y,0);
		
	}
	
}
void ZoneTransform::setDst(const ofRectangle& rect) {
	setDstCorners(rect.getTopLeft(), rect.getTopRight(), rect.getBottomLeft(), rect.getBottomRight());
	
}

void ZoneTransform :: setDstCorners(glm::vec3 topleft, glm::vec3 topright, glm::vec3 bottomleft, glm::vec3 bottomright) {
	// interpolate dst handle points?
	
	// ofLog(OF_LOG_NOTICE, "ZoneTransform::setDstCorners "+displayLabel);
	vector<cv::Point2f> srcCVPoints, dstCVPoints;
	srcCVPoints.resize(4);
	dstCVPoints.resize(4);
	
	srcCVPoints[0] = toCv(srcRect.getTopLeft());
	srcCVPoints[1] = toCv(srcRect.getTopRight());
	srcCVPoints[2] = toCv(srcRect.getBottomLeft());
	srcCVPoints[3] = toCv(srcRect.getBottomRight());
	
    //cout << topleft << " " << topright << " " << bottomleft << " " << bottomright << endl;
    
	dstCVPoints[0] = toCv(topleft);
	dstCVPoints[1] = toCv(topright);
	dstCVPoints[2] = toCv(bottomleft);
	dstCVPoints[3] = toCv(bottomright);
	
	// NB 8 is CV_RANSAC which isn't defined in all platforms for some reason.
	cv::Mat homography = cv::findHomography(cv::Mat(srcCVPoints), cv::Mat(dstCVPoints),8, 100);
	
	srcCVPoints.resize(srcPoints.size());
	dstCVPoints.resize(srcPoints.size());
	

	for(size_t i= 0; i<srcPoints.size(); i++) {
		srcCVPoints[i] = toCv(srcPoints[i]);
	
	}
	try {
		cv::perspectiveTransform(srcCVPoints, dstCVPoints, homography);
	} catch ( cv::Exception & e ) {
		ofLog(OF_LOG_ERROR, e.msg ); // output exception message
	}
		
	for(size_t i= 0; i<dstHandles.size(); i++) {
		dstHandles[i].set(toOf(dstCVPoints[i]));
        dstHandles[i].col = ofColor(100,100,255, 196);
        dstHandles[i].overCol = ofColor(196,196,255, 255);
       // ofLog(OF_LOG_NOTICE,"setting dstHandles["+ofToString(i)+"] to "+ofToString(dstCVPoints[i].x)+","+ofToString(dstCVPoints[i].y));
       // ofLog(OF_LOG_NOTICE," ------------------["+ofToString(i)+"] to "+ofToString(dstHandles[i].x)+","+ofToString(dstHandles[i].y));
       
	}
	
	
}

void ZoneTransform::resetFromCorners() {
	vector<ofPoint> corners = getCorners();
	setDstCorners(corners[0],corners[1],corners[2],corners[3]);

}
vector<ofPoint> ZoneTransform::getCorners(){
	vector<ofPoint> corners;
	corners.push_back(dstHandles[0]);
	corners.push_back(dstHandles[xDivisions]);
	corners.push_back(dstHandles[yDivisions*(xDivisions+1)]);
	corners.push_back(dstHandles[((xDivisions+1)*(yDivisions+1))-1]);
	return corners;
}

void ZoneTransform::getPerimeterPoints(vector<glm::vec3>& points) {
    points.clear();
    
    for(int i = 0; i<xDivisions; i++) {
        points.push_back(dstHandles[i]);
    }
    for(int i = xDivisions; i<(((xDivisions+1)*(yDivisions+1))-1); i+=(xDivisions+1) ) {
        points.push_back(dstHandles[i]);
    }
    int start =((xDivisions+1)*(yDivisions+1))-1;
    int end = yDivisions*(xDivisions+1);
    for(int i =start ; i>end; i--){
        points.push_back(dstHandles[i]);
    }
    start = yDivisions*(xDivisions+1);
    end = 0;
    for(int i =start ; i>=end; i-=(xDivisions+1)){
        points.push_back(dstHandles[i]);
    }
//    points.push_back(dstHandles[yDivisions*(xDivisions+1)]);
//
}



bool ZoneTransform :: isCorner(int i ) {
	return (i==0) || (i == xDivisions) || (i == yDivisions*(xDivisions+1)) || (i==((xDivisions+1)*(yDivisions+1))-1);
	
}

void ZoneTransform :: setDivisions(int xdivisions, int ydivisions) {
	xDivisionsNew = xdivisions;
	yDivisionsNew = ydivisions;
	
	
	updateDivisions();
	
}

void ZoneTransform:: divisionsChanged(int& e){
	if((xDivisionsNew!=xDivisions) || (yDivisionsNew!=yDivisions))
		updateDivisions();
}

void ZoneTransform:: updateDivisions(){
	//ofLogNotice("ZoneTransform::updateDivisions()");

	//ofLog(OF_LOG_NOTICE, "divisionsChanged");
	
    vector<ofPoint> corners  = getCorners();
    
	xDivisions = xDivisionsNew;
	yDivisions = yDivisionsNew;
	dstHandles.resize((xDivisions+1)*(yDivisions+1));
	srcPoints.resize((xDivisions+1)*(yDivisions+1));
	
	setSrc(srcRect);
	
	
	setDstCorners(corners[0], corners[1], corners[2], corners[3]);
	
	updateQuads();
	
	
}


void ZoneTransform::updateQuads() {
	//ofLogNotice("ZoneTransform::updateQuads()");
	
	int quadnum = xDivisions*yDivisions;
	quadWarpers.resize(quadnum);
	
	if(srcPoints.size()!=dstHandles.size()) {
		srcPoints.resize((xDivisions+1)*(yDivisions+1));
		
		setSrc(srcRect);
	}
	
	for(int i = 0; i<quadnum; i++) {
		
		int x = i%xDivisions;
		int y = i/xDivisions;
		
		int topleft = x+(y*(xDivisions+1));
		int topright =x+1+(y*(xDivisions+1));
		int bottomleft=x+((y+1)*(xDivisions+1));
		int bottomright=x+1+((y+1)*(xDivisions+1));
		
		//cout << i<< " " <<x<< " " << y << " " << topleft<< " " << topright<< " " << bottomleft << " " << bottomright<< endl;
		
		Warper & quad = quadWarpers[i];
		quad.updateHomography(srcPoints[topleft],
							  srcPoints[topright],
							  srcPoints[bottomleft],
							  srcPoints[bottomright],
							  dstHandles[topleft],
							  dstHandles[topright],
							  dstHandles[bottomleft],
							  dstHandles[bottomright]
							  );
		
	}
	
	

	
}


void ZoneTransform::initListeners() {
	
    ofAddListener(ofEvents().mouseMoved, this, &ZoneTransform::mouseMoved, OF_EVENT_ORDER_AFTER_APP);
    ofAddListener(ofEvents().mousePressed, this, &ZoneTransform::mousePressed, OF_EVENT_ORDER_AFTER_APP);
	ofAddListener(ofEvents().mouseReleased, this, &ZoneTransform::mouseReleased, OF_EVENT_ORDER_AFTER_APP);
	ofAddListener(ofEvents().mouseDragged, this, &ZoneTransform::mouseDragged, OF_EVENT_ORDER_AFTER_APP);
	
	
}

void ZoneTransform :: removeListeners() {
	
    ofRemoveListener(ofEvents().mouseMoved, this, &ZoneTransform::mouseMoved, OF_EVENT_ORDER_AFTER_APP);
    ofRemoveListener(ofEvents().mousePressed, this, &ZoneTransform::mousePressed, OF_EVENT_ORDER_AFTER_APP);
	ofRemoveListener(ofEvents().mouseReleased, this, &ZoneTransform::mouseReleased, OF_EVENT_ORDER_AFTER_APP);
	ofRemoveListener(ofEvents().mouseDragged, this, &ZoneTransform::mouseDragged, OF_EVENT_ORDER_AFTER_APP);
	
}

bool ZoneTransform :: mouseMoved(ofMouseEventArgs &e){
    
    
    if((!editable) || (!visible)) return false;

    mousePos = e;
    mousePos-=offset;
    mousePos/=scale;
    return false;

}

bool ZoneTransform :: mousePressed(ofMouseEventArgs &e){
	
	// TODO there is currently an issue where if a zone is on top of another
    // zone, you can't click on a handle underneath. Not sure of how to fix this...
    // but possibly needs some higher level logic than here.
    
	if((!editable) || (!visible)) return false;
    if(ofGetKeyPressed(' ')) return false; // for dragging around previews. 

	mousePos = e;
    mousePos-=offset;
    mousePos/=scale;
	
	
	bool hit = hitTest(mousePos);
	if((hit) &&(!selected)) {
		selected = true;
		return false; // keeps the event propogating
	}
	
	
	if(!selected) {
		return false;
	}
	
	bool handleHit = false;
	
	// this section of code if we click drag anywhere in the zone

	for(size_t i= 0; i<dstHandles.size(); i++) {
		if(dstHandles[i].hitTest(mousePos)) {
			
			if(!editSubdivisions && !isCorner((int)i)) continue;
			
			dstHandles[i].startDrag(mousePos);
			handleHit = true;
			
			if(!editSubdivisions) {
				
				DragHandle& currentHandle = dstHandles[i];
				
				vector<DragHandle*> corners;
				DragHandle& topLeft = dstHandles[0];
				DragHandle& topRight = dstHandles[xDivisions+1-1];
				DragHandle& bottomLeft = dstHandles[(yDivisions+1-1)*(xDivisions+1)];
				DragHandle& bottomRight = dstHandles.back();
				
				corners.push_back(&topLeft);
				corners.push_back(&topRight);
				corners.push_back(&bottomLeft);
				corners.push_back(&bottomRight);
			
				int handleIndex = 0;
				if(currentHandle == topLeft) handleIndex = 0;
				else if(currentHandle == topRight) handleIndex = 1;
				else if(currentHandle == bottomLeft) handleIndex = 2;
				else if(currentHandle == bottomRight) handleIndex =3;
				
				int x = ((handleIndex%2)+1)%2;
				int y = handleIndex/2;
				
				int xhandleindex = x+(y*2);
				
				x = handleIndex%2;
				y = ((handleIndex/2)+1)%2;
				int yhandleindex = x+(y*2);
				
				corners[xhandleindex]->startDrag(mousePos, false,true, true);
				corners[yhandleindex]->startDrag(mousePos, true,false, true);
				
//				bottomLeft.startDrag(mousePoint, false,true, true);
//				topRight.startDrag(mousePoint, true,false, true);
				
			}
		}
		
	}
	
	// drag all the handles!
	if(!handleHit && hit) {

		//centreHandle.startDrag(mousePoint);
		handleHit = true;
		for(size_t i= 0; i<dstHandles.size(); i++) {
			dstHandles[i].startDrag(mousePos);
		}


	}
	
	if(!handleHit && !hit) {
		selected = false;
	}
	return handleHit || hit;
	
}




bool ZoneTransform :: mouseDragged(ofMouseEventArgs &e){
	
    if((!editable) || (!visible)) return false;
	if(!selected) return false;

	ofPoint mousePoint;
    mousePoint.x = e.x;
    mousePoint.y = e.y;
	mousePoint-=offset;
	mousePoint/=scale;
	
	//ofRectangle bounds(centreHandle, 0, 0);
	int dragCount = 0;
	for(size_t i= 0; i<dstHandles.size(); i++) {
		if(dstHandles[i].updateDrag(mousePoint)) dragCount++;
		//bounds.growToInclude(handles[i]);
	}
//	if(!dragging) {
//		dragging = centreHandle.updateDrag(mousePoint);
//	} else {
//		updateCentreHandle();
//		
//	}
	
	isDirty |= (dragCount>0);
	if((dragCount>0)&&(!editSubdivisions)) resetFromCorners();
	
	return dragCount>0;
	
	
}


bool ZoneTransform :: mouseReleased(ofMouseEventArgs &e){
	
	//if(!editable) return false;
	if(!selected) return false;
	
	bool wasDragging = false;
	
	for(size_t i= 0; i<dstHandles.size(); i++) {
		if(dstHandles[i].stopDrag()) wasDragging = true;
	}
	
    // TODO mark as dirty so auto save ********************
	//saveSettings();
    isDirty|=wasDragging;
	return wasDragging;
	
}

bool ZoneTransform::hitTest(ofPoint mousePoint) {
	
	ofPolyline poly;
	for(int i = 0; i<=xDivisions; i++) {
		poly.addVertex(dstHandles[i]);
		//ofLog(OF_LOG_NOTICE, ofToString(i));
	}
	//ofLog(OF_LOG_NOTICE, "---");
	for(int i = 2; i<=yDivisions; i++) {
		poly.addVertex(dstHandles[(i*(xDivisions+1))-1]);
		//ofLog(OF_LOG_NOTICE, ofToString((i*(xDivisions+1))-1));
	}
	//ofLog(OF_LOG_NOTICE, "---");
	for(int i = ((xDivisions+1)*(yDivisions+1))-1; i>=(xDivisions+1)*(yDivisions); i--) {
		poly.addVertex(dstHandles[i]);
		//ofLog(OF_LOG_NOTICE, ofToString(i));
	}
	//ofLog(OF_LOG_NOTICE, "---");
	for(int i = yDivisions-1; i>=0; i--) {
		poly.addVertex(dstHandles[(i*(xDivisions+1))]);
		//ofLog(OF_LOG_NOTICE, ofToString((i*(xDivisions+1))));
	}

	return poly.inside(mousePoint);
	
}
//
//void ZoneTransform::saveSettings() {
//
//	//ofLog(OF_LOG_NOTICE, "ZoneTransform::saveSettings");
//
//	ofJson json;
//	serialize(json);
//	ofSavePrettyJson(getSaveLabel()+".json", json);
//
//}

void ZoneTransform::serialize(ofJson&json) {
	ofSerialize(json,params);
	ofJson& handlesjson = json["handles"];
	for(size_t i= 0; i<dstHandles.size(); i++) {
		DragHandle& pos = dstHandles[i];
		handlesjson.push_back({pos.x, pos.y});
	}
	//cout << json.dump(3) << endl;
	//deserialize(json);
}

bool ZoneTransform::deserialize(ofJson& jsonGroup) {
	//ofLogNotice("ZoneTransform::deserialize()");
    // note that ofDeserialize looks for the json group
    // with the same name as the parameterGroup
	ofDeserialize(jsonGroup, params);
    //cout << paramjson.dump(3) << endl;
    
	// number of handles could be different now
	int numhandles = (xDivisionsNew+1)*(yDivisionsNew+1);
	xDivisions = xDivisionsNew;
	yDivisions = yDivisionsNew;
	dstHandles.resize(numhandles);
    
    ofJson& handlejson = jsonGroup["handles"];
    
	if((int)handlejson.size()>=numhandles) {
		for(int i = 0; i<numhandles; i++) {
			ofJson& point = handlejson[i];
			dstHandles[i].x = point[0];
			dstHandles[i].y = point[1];
			dstHandles[i].z = 0;
            //cout << "setting handle " << i << " : " << dstHandles[i] << endl;
			
		}
	}
	//updateDivisions(); //< SHOULD BE called automatically I think
    
	return true; 
}


void ZoneTransform::setHandleSize(float size) {
	for(DragHandle& handle : dstHandles) {
		handle.setSize(size);
	}
	
}
