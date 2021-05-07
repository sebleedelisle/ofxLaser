//
//  ofxLaserZone.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 06/11/2017.
//
//

#include "ofxLaserZone.h"

using namespace ofxLaser;

//Zone::Zone() {
//	ofLog(OF_LOG_NOTICE, "Zone() constructor") ;
////	editable = false;
//}

Zone::Zone(float x, float y, float w, float h) : QuadGui::QuadGui() {
    setIndex(0);
	set(x, y, w,h);
    setColours(ofColor(200,20,200), ofColor(200,20,200), ofColor(200,20,200));
	
    lockPerpendicular = true;
    for(DragHandle& handle : handles) {
        handle.gridSize = 1;
        handle.snapToGrid = true;
        
    }
    
    setConstrained(rect);
}

void Zone:: setIndex(int _index)  {
    index = _index;
    setName("Z"+ofToString(index+1));
    zoneLabel = "ZONE " + ofToString(index+1); 
   
    
}

Zone::~Zone() {
	removeListeners();
	
}


void Zone:: set(float x, float y, float w, float h) {
    QuadGui::set(x, y, w, h);
    rect.set(x,y,w,h); 

}
bool Zone::update() {

	if(isDirty) {
		rect.set(handles[0], handles[3]);
		isDirty = false;
		return true;
		
	}
	return false;
}

bool Zone::addShape(Shape* s){
	
	if(s->intersectsRect(rect)){
		shapes.push_back(s);
		return true;
	} else {
		return false;
	}
	
}



ofPoint& Zone::addSortedShapesToVector(vector<ofxLaser::Shape*>& shapesContainer, ofPoint& currentPosition){
	
	deque<ofxLaser::Shape*> sortedShapes;
	
	// sort the shapes by nearest neighbour
	
	if(shapes.size()>0) {
		
		ofFloatColor c = ofColor(255);
		
		// add a dummy shape to fix the start position
		shapes.push_front(new ofxLaser::Dot(currentPosition, c, 1, ""));
		
		for(size_t i=0; i<shapes.size(); i++ ) {
			shapes[i]->tested = false;
			shapes[i]->reversed = false;
			
		}
		
		bool reversed = false;
		int currentIndex = 0;
		
		float shortestDistance = INFINITY;
		
		int nextDotIndex = NULL;
		
		// SORT THE SHAPES
		
		do {
			
			ofxLaser::Shape& shape1 = *shapes[currentIndex];
			
			shape1.tested = true;
			sortedShapes.push_back(&shape1);
			shape1.reversed = reversed;
			
			shortestDistance = INFINITY;
			nextDotIndex = -1;
			
			
			for(int j = 0; j<shapes.size(); j++) {
				
				ofxLaser::Shape& shape2 = *shapes[j];
				if((&shape1==&shape2) || (shape2.tested)) continue;
				
				shape2.reversed = false;
				
				if(shape1.getEndPos().squareDistance(shape2.getStartPos()) < shortestDistance) {
					shortestDistance = shape1.getEndPos().squareDistance(shape2.getStartPos());
					nextDotIndex = j;
					reversed = false;
				}
				
				if((shape2.reversable) && (shape1.getEndPos().squareDistance(shape2.getEndPos()) < shortestDistance)) {
					shortestDistance = shape1.getEndPos().squareDistance(shape2.getEndPos());
					nextDotIndex = j;
					reversed = true;
				}
				
				
			}
			
			currentIndex = nextDotIndex;
			
			
			
		} while (currentIndex>-1);
		
		// TODO - delete the dummy shape at the start?
		delete sortedShapes[0];
		sortedShapes.pop_front();
		shapes.pop_front();
		
		for(unsigned int i= 0; i<sortedShapes.size(); i++) {
			shapesContainer.push_back(sortedShapes[i]);
			
		}
		
		
	}

	
	
	return currentPosition;
	
}



//
//
//bool Zone ::  loadSettings() {
//
//
//	ofFile jsonfile(label+".json");
//	if(jsonfile.exists()) {
//		ofJson json = ofLoadJson(label+".json");
//		deserialize(json);
//		return true;
//	}
//
//
//	return false;
//
//
//}
//void Zone::deserialize(ofJson& jsonGroup) {
//
//	ofJson& pointjson = jsonGroup["points"];
//
//	for(int i = 0; i<handles.size(); i++) {
//		ofJson& point = pointjson[i];
//		handles[i].x = point[0];
//		handles[i].y = point[1];
//		handles[i].z = 0;
//
//	}
//
//	isDirty = true;
//
//}
//void Zone :: saveSettings() {
//	ofJson json;
//	serialize(json);
//	ofSavePrettyJson(label+".json", json);
//
//
//}
//
//
//void Zone :: serialize(ofJson&json) {
//
//	ofJson& pointsjson = json["points"];
//	for(int i = 0; i<handles.size(); i++) {
//		DragHandle& pos = handles[i];
//		pointsjson.push_back({pos.x, pos.y});
//	}
//	cout << json.dump(3) << endl;
//	//deserialize(json);
//}

void Zone::setHandleSize(float size) {
	//for(DragHandle& handle : handles) {
	//	handle.setSize(size);
	//}
	
}
