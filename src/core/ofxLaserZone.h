//
//  ofxLaserZone.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 06/11/2017.
//
//

#pragma once
#include "ofMain.h"
#include "ofxLaserShape.h"
#include "ofxLaserDot.h"
#include "ofxLaserDragHandle.h"

namespace ofxLaser {

	class Zone  {
		
		public :
		
		Zone();
		Zone(int index, float x, float y, float w, float h);
		~Zone();

		void set(float x, float y, float w, float h);
		void set(ofRectangle newrect);
		void setHandleSize(float size); 
		
		bool update();
		bool addShape(Shape* s);
		void draw();
		
		ofPoint& addSortedShapesToVector(vector<Shape*>& shapes, ofPoint& currentPosition);

		void initListeners();
		void removeListeners();
		
		bool mousePressed(ofMouseEventArgs &e);
		bool mouseDragged(ofMouseEventArgs &e);
		bool mouseReleased(ofMouseEventArgs &e);
		void startDragging(int handleIndex, ofPoint clickPos);

		bool loadSettings();
		void saveSettings();
		
		void serialize(ofJson&json);
		void deserialize(ofJson&jsonGroup);

		
		ofRectangle rect;

		deque<Shape*> shapes;
		string label;
		bool editable;
		vector<DragHandle> handles;
		ofPoint offset;
		float scale = 1;
		bool visible = true;
		bool active = true; 
		bool isDirty = true;
		bool snapToPixels = true;
		
		int index = 0;
		
		
	};
}
