//
//  ofxLaser.cpp
//
//  Created by Seb Lee-Delisle on 13/08/2015.
//
//

#include "ofxLaserManager.h"


using namespace ofxLaser;

Manager :: Manager() {
	

}


void Manager :: setup (int width, int height) {
	
	//ofSetFrameRate(20);
	appWidth = width;
	appHeight = height;
	maskRectangle.set(0,0,width, height);
	
	//shouldBeConnected = false;
	
	
	//restartCount = 0;
	
	//white.set(1,1,1);
	//black.set(0,0,0);
	
	
	pmin.set(0,0);
	pmax.set(appWidth, appHeight);
	
	testPattern = 0;
	numTestPatterns = 5;
	
	
	setupParameters();

	
	laserHomePosition = maskRectangle.getCenter();
}



void Manager :: connectButtonPressed(){
	
//	if(!shouldBeConnected) {
//		connectToEtherdream();
//	} else {
//		disconnectFromEtherdream();
//	}
	
	
}

void Manager :: update() {
	
	minPoints = pps / 50;

	//resetIldaPoints();
	
	// clear all pre-existing points.
	ildaPoints.clear();
	laserPoints.clear();
	// clear the previewPathMesh - the blue and white laser preview
	previewPathMesh.clear();
	
//	
//	string etherdreamstate = etherdream.getStateString();
//	
//	etherdreamStatus = etherdream.getStateString();//etherdream.getDeviceStateString() + " " + (etherdreamstate) + " "+ ofToString(shouldBeConnected)+" " +ofToString(restartCount);
//	
//	if(etherdream.state==ETHERDREAM_DISCONNECTED){
//		
//		restartCount++;
//		disconnectFromEtherdream();
//		beep.play();
//		connectToEtherdream();
//	}
//	
//	

	
	etherdream.checkConnection();
	
	// at this point we should report if the laser has disconnected.
	
	//lastState = etherdreamstate;
	
}

void Manager :: draw() {
	
	
	//ofPoint startPosition(maskRectangle.getCenter());
	
	if(testPattern>0) drawTestPattern();
	
	
	//updateRenderDelay();
	
	
	
	drawShapes();
	
	
	
	//ofRectangle boundsRectangle(maskRectangle.getCenter(), 0,0);
	
	// update the bounds rectangle
	//if(i == 0) boundsRectangle.set(
	
	if(renderLaserPreview) {
		renderPreview();
	}
	renderLaserPath();

	
	
	vector<ofxIlda::Point> adjustedPoints;
	
	
	// Make adjustments for the colour change delay
	
	for(int i = 0; i<ildaPoints.size(); i++) {
		
		ofxIlda::Point p = ildaPoints.at(i);
		
		
		
		int colourPointIndex = i+colourChangeDelay;
		while(colourPointIndex<0) colourPointIndex+=ildaPoints.size();
		
		ofxIlda::Point colourPoint = ildaPoints.at(colourPointIndex%ildaPoints.size());
		
		p.r = round(colourPoint.r);
		p.g = round(colourPoint.g);
		p.b = round(colourPoint.b);
		
		adjustedPoints.push_back(p);
		
	}
	
	etherdream.setPoints(adjustedPoints);
	etherdream.setPPS(pps);
	
	//showMask();
	
	// TODO if we're not using the delay system, let's
	// delete all the shapes
//	if(delay==0) {
		for(int i = 0; i<shapes.size(); i++) {
			delete shapes[i];
}
//
//	}
	
	// clear the shapes vector no matter what
	shapes.clear();
	
	warp.draw();
	
	
	
}



void Manager :: drawShapes() {
	
	ofPoint currentPosition = laserHomePosition;
	vector<ofxLaser::Point> shapepoints;
	
	vector<ofxLaser::Shape*> sortedShapes;
	
	// sort the shapes by nearest neighbour
	
	if(shapes.size()>0) {

		ofFloatColor c = ofColor::white;
		
		shapes.push_front(new ofxLaser::Dot(currentPosition, c, 0, 0));
		
		
		for(int i =0; i<shapes.size(); i++ ) {
			shapes[i]->tested = false;
			shapes[i]->reversed = false;
			
		}
		
		bool reversed = false;
		int currentIndex = 0;
		
		float shortestDistance = INFINITY;
		
		
		int numberSorted = 0;
		
		
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
				
				if(shape1.getEndPos().distanceSquared(shape2.getStartPos()) < shortestDistance) {
					shortestDistance = shape1.getEndPos().distanceSquared(shape2.getStartPos());
					nextDotIndex = j;
					reversed = false;
				}
				
				if((shape2.reversable) && (shape1.getEndPos().distanceSquared(shape2.getEndPos()) < shortestDistance)) {
					shortestDistance = shape1.getEndPos().distanceSquared(shape2.getEndPos());
					nextDotIndex = j;
					reversed = true;
				}
				
				
			}
			
			currentIndex = nextDotIndex;
			
			
			
		} while (currentIndex>-1);
		
		offScreen = false;
		
		
		addPoint(currentPosition, ofColor::black);
		
		
		
		for(int i = 0; i<sortedShapes.size(); i++) {
			
			ofxLaser::Shape* shape = sortedShapes.at(i);
			
			// TODO add shape pre points switched on.
			if(!currentPosition.match(shape->getStartPos(), 0.01)) {
				addPointsForMoveTo(currentPosition, shape->getStartPos());
				
				// PRE BLANK
				for(int i = 0; i<shapePreBlank; i++) {
					addPoint(shape->getStartPos(), ofColor::black);
				}
			} else {
				// no blanks if we're there already?
				// PRE BLANK
				//for(int i = 0; i<shapePreBlank; i++) {
				//	addIldaPoint(shape->getStartPos(), shape.getC, 1);
				//}
			}
			
			shapepoints.clear();
			shape->appendPointsToVector(shapepoints);
			addPoints(shapepoints);
			
			
			// POST BLANK
			
			for(int i = 0; i<shapePostBlank; i++) {
				addPoint(shape->getEndPos(), ofColor::black);
			}
			currentPosition = shape->getEndPos();
			
		}
		
		
	}
	
	addPointsForMoveTo(currentPosition, laserHomePosition);
	
	while(laserPoints.size()<minPoints) {
		//const ofPoint p =currentPosition + ofPoint(0,10);
		ofxLaser::Circle blank(laserHomePosition + ofPoint(0,10), 10, ofFloatColor(0), 2,2,0);
		shapepoints.clear();
		blank.appendPointsToVector(shapepoints);
		addPoints(shapepoints);
	}

	processIldaPoints();
	

	
	// the "Home" position is the start and end point for the laser.
	// It tends towards the last position in all the shapes.
	if(sortedShapes.size()>1) {
		currentPosition =sortedShapes[1]->getStartPos();
		// get rid of the dummy shape
		shapes.pop_front();
	}
	laserHomePosition += (currentPosition-laserHomePosition)*0.01;
	
	
	
}


void Manager :: renderLaserPath() {
	
	ofPushStyle();
	
	bool overrideSettings = false;
	ofRectangle previewRectangle = maskRectangle;
	
	if((renderLaserPathPreview)||(overrideSettings)) {
		
		ofPushMatrix();
		
		ofTranslate(previewRectangle.getTopLeft());
		float scale =  previewRectangle.width / appWidth;
		ofScale(scale, scale);
		
		//ofDisableBlendMode();
		ofNoFill();
		ofSetLineWidth(1);
		ofSetColor(0,0,255);
		previewPathMesh.setMode(OF_PRIMITIVE_LINE_LOOP);
		previewPathMesh.draw();
		
		
		ofSetColor(255,255,255);
		previewPathMesh.setMode(OF_PRIMITIVE_POINTS);
		previewPathMesh.draw();
		ofPopMatrix();
		
		// THIS CODE HIGHLIGHTS A POINT DEPENDENT ON THE MOUSE POS
//		
		if(previewPathMesh.getNumVertices()>0) {
			
			int pointindex =floor(ofMap(ofGetMouseX(), previewRectangle.x, previewRectangle.getRight(), 0, previewPathMesh.getNumVertices()-1, true));
			
			//if(pointindex>=pathMesh.getNumVertices()) pointindex =pathMesh.getNumVertices();
			
			ofPoint p = previewPathMesh.getVertex(pointindex);
			ofSetColor(0,255,0);
			ofCircle(ofMap(p.x, 0, appWidth, previewRectangle.x, previewRectangle.getRight()), ofMap(p.y, 0, appHeight, previewRectangle.y, previewRectangle.getBottom()), 5);
			
		}
	}
	
	// TODO - this needs to go somewhere else!
//	warp.visible = showWarpPoints || (overrideSettings);
//	warp.draw();

	ofPopStyle();
	
}


void Manager :: renderPreview() {
	
	// TODO push the preview mesh generation into the individual laser shapes
	
	ofPushStyle();
	
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	
	ofMesh mesh;
	

	
	deque<ofxLaser::Shape*>& newshapes = shapes; // (shapesHistory.size()>0) ? shapesHistory.back() : shapes;
	
	mesh.clear();
	mesh.setMode(OF_PRIMITIVE_LINE_STRIP);
	
	for(int i = 0; i<newshapes.size(); i++) {
		
		ofxLaser::Shape* shape = newshapes[i];
		
		// Is it a dot?
		ofxLaser::Dot * dot = dynamic_cast<ofxLaser::Dot*>(shape);
		if(dot) {
			float radius = 1.5;
			
			ofVec3f v(0,-radius);
			mesh.addColor(ofColor::black);
			mesh.addVertex(v + dot->getStartPos());
			ofColor c(dot->colour);
			
			for(int i = 0; i<=360; i+=30) {
				v.set(0, -radius);
				v.rotate(i, ofVec3f(0,0,1));
				mesh.addColor(c);
				mesh.addVertex(v+dot->getStartPos());
			}
			
			v.set(0, -radius);
			mesh.addColor(ofColor::black);
			mesh.addVertex(v+dot->getStartPos());
		}
		
		// Is it a circle?
		ofxLaser::Circle * circle = dynamic_cast<ofxLaser::Circle*>(shape);
		if(circle) {
			
			ofVec3f v(0,-circle->radius);
			mesh.addColor(ofColor::black);
			mesh.addVertex(v + circle->pos);
			
			for(int i = 0; i<=360; i+=10) {
				v.set(0, -circle->radius);
				v.rotate(i, ofVec3f(0,0,1));
				mesh.addColor(circle->colour);
				mesh.addVertex(v+circle->pos);
			}
			
			v.set(0, -circle->radius);
			mesh.addColor(ofColor::black);
			mesh.addVertex(v+circle->pos);
		}
		
//		// Is it a line?
		ofxLaser::Line * line = dynamic_cast<ofxLaser::Line*>(shape);
		if(line) {
			mesh.addColor(ofColor::black);
			mesh.addVertex(line->getStartPos());
			
			mesh.addColor(line->colour);
			mesh.addVertex(line->getStartPos());
			mesh.addColor(line->colour);
			mesh.addVertex(line->getEndPos());
			
			mesh.addColor(ofColor::black);
			mesh.addVertex(line->getEndPos());
		}

		// is it a poly?
		ofxLaser::Polyline* laserpoly = dynamic_cast<ofxLaser::Polyline*>(shape);
		if(laserpoly) {
			ofPolyline& poly = laserpoly->polyline;
			
			vector<ofPoint> & vertices = poly.getVertices();
			mesh.addColor(ofColor::black);
			mesh.addVertex(vertices.front());
			
			for(int i = 0; i<vertices.size(); i++) {
				
				mesh.addColor(laserpoly->colour);
				mesh.addVertex(vertices[i]);
			}
			
			
			mesh.addColor(ofColor::black);
			mesh.addVertex(vertices.back());

			

		}
		
		
	}
	
	ofSetLineWidth(1.5);
	mesh.draw();
	
	vector<ofFloatColor>& colours = mesh.getColors();
	
	for(int i = 0; i<colours.size(); i++) {
		colours[i].r*=0.4;
		colours[i].g*=0.4;
		colours[i].b*=0.4;
	}
	
	ofSetLineWidth(4);
	mesh.draw();
	
	
	ofPopStyle();
	
}



// TODO for all shapes, re-implement 3D to 2D conversion

void Manager :: addLaserDot(const ofPoint& ofpoint, ofFloatColor colour, float dotintensity, int maxpoints){
	
	// TODO re-implement 3D to 2D conversion
	//shapes.push_back(new ofxLaser::Shape(gLProject(ofpoint), colour, intensity));
	shapes.push_back(new ofxLaser::Dot(ofpoint, colour, dotintensity, (maxpoints<0) ? (int)dotMaxPoints : maxpoints));
	
}

// TODO allow specification of acceleration and speed

void Manager :: addLaserLine(const ofPoint&startpoint, const ofPoint&endpoint, ofFloatColor colour, float speed, float acceleration) {
	shapes.push_back(new ofxLaser::Line(startpoint, endpoint, colour, speed<0 ? (float)defaultLineSpeed : speed, acceleration<0 ? (float) defaultLineAcceleration : acceleration));
}

// TODO allow specification of acceleration, speed, and overlap
void Manager :: addLaserCircle(const ofPoint& centre, float radius, ofFloatColor colour, float speed, float acceleration, float overlap){
	
	//ofPoint p = gLProject(ofpoint);
	//float scalar = gLGetScaleForZ(ofpoint.z);
	shapes.push_back(new ofxLaser::Circle(centre, radius, colour, speed<0 ? (float)defaultCircleSpeed : speed, acceleration<0 ? (float)defaultCircleAcceleration : acceleration, overlap<0 ? (float)defaultCircleOverlap : overlap));
}


// TODO implement multicolour, and allow custom acceleration and speed

void Manager ::addLaserPolyline(const ofPolyline& line, ofColor colour, float speed, float acceleration, float cornerthreshold){
	
	// quick error check to make sure our line has any data!
	// (useful for dynamically generated lines, or empty lines
	// that are often found in poorly compiled SVG files)
	
	if((line.getVertices().size()==0)||(line.getPerimeter()<0.1)) return;
	
	//***
	// convert to 2D TODO - reimplement
//	ofPolyline& polyline = tmpPoly;
//	polyline.clear();
//	
//	for(int i = 0; i<line.getVertices().size(); i++) {
//		polyline.addVertex(gLProject(line.getVertices()[i]));
//		
//	}
	
	shapes.push_back(new ofxLaser::Polyline(line, colour, acceleration< 0 ? (float) defaultPolylineAcceleration : acceleration, speed<0 ? (float) defaultPolylineSpeed : speed, cornerthreshold<0 ? (float) defaultPolylineCornerThreshold : cornerthreshold));
	
	
}


void Manager ::drawTestPattern() {
//	
	if(testPattern==1) {
		
		
		ofPoint v = maskRectangle.getBottomRight() - maskRectangle.getTopLeft();
		for(float y = 0; y<=1; y+=0.333333333) {
			
			addLaserLine(ofPoint(maskRectangle.getLeft(), maskRectangle.getTop()+v.y*y),ofPoint(maskRectangle.getRight(), maskRectangle.getTop()+v.y*y), ofColor::white );
		}

		for(float x =0 ; x<=1; x+=0.3333333333) {
			
			
			addLaserLine(ofPoint(maskRectangle.x + v.x*x, maskRectangle.getTop()),ofPoint(maskRectangle.x + v.x*x, maskRectangle.getBottom()), ofColor::red );
			
		}
		
		addLaserCircle(maskRectangle.getCenter(), maskRectangle.getWidth()/12, ofColor::blue);
		addLaserCircle(maskRectangle.getCenter(), maskRectangle.getWidth()/6, ofFloatColor(0,1,0));
		
		
		
	}
	else if((testPattern>=2) && (testPattern<=5)) {
		ofColor c;
		
		ofRectangle rect(appWidth*0.3, appHeight*0.5, appWidth*0.4, appHeight*0.1);
		
		//addPointsForMoveTo(currentPosition,rect.getTopLeft());
		
		for(int row = 0; row<5; row ++ ) {
			
			
			float y =(rect.getHeight()*row/4);
			
			ofPoint left = ofPoint(rect.getLeft(), y);
			
			ofPoint right = ofPoint(rect.getRight(), y);
			
			if(testPattern == 2) c.set(255,0,0);
			else if(testPattern == 3) c.set(0,255,0);
			else if(testPattern == 4) c.set(0,0,255);
			else if(testPattern == 5) c.set(255,255,255);
			
			switch (row) {
				case 0 :
					c.r *= red100;
					c.g *= green100;
					c.b *= blue100;
					break;
				case 1 :
					c.r *= red75;
					c.g *= green75;
					c.b *= blue75;
					break;
				case 2 :
					c.r *= red50;
					c.g *= green50;
					c.b *= blue50;
					break;
				case 3 :
					c.r *= red25;
					c.g *= green25;
					c.b *= blue25;
					break;
				case 4 :
					c.r *= red0;
					c.g *= green0;
					c.b *= blue0;
					break;
			}
			
			float speed = 15 * ( 1- (row*0.25));
			if(speed<5) speed = 5;

			
			addLaserLine(rect.getTopLeft()+ofPoint(0,y),rect.getTopRight()+ofPoint(0,y),c, speed,200);
			
//
			
			// 0 = normalspeed;
			// 1 = normalspeed * 0.75
			// 2 = normalspeed * 0.5
			// 3 = normalspeed * 0.25
			
			
			// 0 = 1;
			// 1 = 0.75
			// 2 = 0.5;
			// 3 = 0.25
		}
		
		
	}
	
	
}
//
//void Manager :: showMask() {
//	
//	ofPushStyle();
//	if(maskRectangleBrightness>0) {
//		ofNoFill();
//		ofSetColor(maskRectangleBrightness * 255);
//		
//		ofRect(maskRectangle);
//		maskRectangleBrightness-=0.01;
//		
//	}
//	
//	if(showMaskBitmap) {
//		ofPushStyle();
//		//		ofEnableAlphaBlending();
//		ofEnableBlendMode(OF_BLENDMODE_SCREEN);
//		ofSetColor(50,0,0);
//		maskBitmap.draw(0,0,appWidth, appHeight);
//		ofPopStyle();
//	}
//	ofPopStyle();
//
//}

void Manager :: addPointsForMoveTo(const ofPoint & currentPosition, const ofPoint & targetpoint){
	
	ofPoint target = targetpoint;
	ofPoint start = currentPosition;
	
	ofPoint v = target-start;
	
	float blanknum = v.length()/moveSpeed + movePointsPadding;
	
	for(int j = 0; j<blanknum; j++) {
		
		float t = Quint::easeInOut((float)j, 0.0f, 1.0f, blanknum);
		
		ofPoint c = (v* t) + start;
		addPoint(c, (laserOnWhileMoving && j%2==0) ? ofColor(200,0,0) : ofColor::black);
		
	}
	
}



void Manager :: addPoint(ofPoint p, ofFloatColor c, float pointIntensity, bool useCalibration) {

	
	addPoint(ofxLaser::Point(p, c, pointIntensity, useCalibration));
	
}


void Manager :: addPoints(vector<ofxLaser::Point>&points) {
	
	
	for(int i = 0; i<points.size();i++) {
		
		addPoint(points[i]);
		
		
	}
	
	
}

void Manager :: addPoint(ofxLaser::Point p) {
	
	
//	p.r*=pointIntensity;
//	p.g*=pointIntensity;
//	p.b*=pointIntensity;
	
	//	if(useCalibration) {
	//		p.r = calculateCalibratedBrightness(c.r, intensity, red100, red75, red50, red25, red0);
	//		p.g = calculateCalibratedBrightness(c.g, intensity, green100, green75, green50, green25, green0);
	//		p.b = calculateCalibratedBrightness(c.b, intensity, blue100, blue75, blue50, blue25, blue0);
	//	}
	
	laserPoints.push_back(p);
	
	if(!showPostTransformPreview) previewPathMesh.addVertex(ofPoint(p.x, p.y));
	
	
}



//
//void Manager :: addIldaPoint(ofPoint p, ofFloatColor c, float pointIntensity, bool useCalibration){
//	
//	
//	// TODO should be smarter about this. Ideally we should calculate
//	// the point that it crosses the line and use that as the last point.
//	
//	
//	// inside doesn't work because I need points on the edge of the rect to work.
//
//	if(p.x<maskRectangle.getLeft() ||
//		p.x>maskRectangle.getRight() ||
//		p.y<maskRectangle.getTop() ||
//		p.y>maskRectangle.getBottom()) {
//		
//		p.x = ofClamp(p.x, maskRectangle.getLeft(), maskRectangle.getRight());
//		p.y = ofClamp(p.y, maskRectangle.getTop(), maskRectangle.getBottom());
//		
//		
//		if(!offScreen) {
//			offScreen = true;
//			offScreenPoint = p;
//			addPoint(offScreenPoint, c, useCalibration);
//			
//		}
//		lastClampedOffScreenPoint = p;
//		
//		
//	} else {
//		
//		if(offScreen){
//			// ease between offScreenPoint and lastClampedOffScreenPoint
//			offScreen = false;
//			
//			ofPoint target = lastClampedOffScreenPoint;
//			ofPoint start = offScreenPoint;
//			
//			ofPoint v = target-start;
//			
//			float blanknum = v.length()/moveSpeed + movePointsPadding;
//			
//			for(int j = 0; j<blanknum; j++) {
//				
//				float t = Quint::easeInOut((float)j, 0.0f, 1.0f, blanknum);
//				
//				ofPoint c = (v* t) + start;
//				addPoint(c, (laserOnWhileMoving && j%2==0) ? ofColor(100,0,0) : ofColor::black, useCalibration);
//				
//			}
//		}
//		
//		c.r*=pointIntensity;
//		c.g*=pointIntensity;
//		c.b*=pointIntensity;
//		
////		if(useMaskBitmap) {
////			float brightness = maskBitmap.getColor(p.x/appWidth* (float)maskBitmap.getWidth(), p.y/appHeight * (float)maskBitmap.getHeight()).getBrightness();
////			c*= brightness/255.0f;
////			
////			
////		}
//		
//		
//		addPoint(p, c, useCalibration);
//		
//	}
//}


void  Manager :: processIldaPoints() {
	
	// TODO implement proper masking and clamping.
	
	for(int i = 0; i<laserPoints.size(); i++) {
		
		
		ofxLaser::Point &p = laserPoints[i];

		// TODO currently just scales using a rectangle. QuadWarp will
		// eventually do proper perspective correction.
		
		p.x = ofMap(p.x, 0, appWidth, warp.handles[0].x, warp.handles[2].x);
		p.y = ofMap(p.y, 0, appHeight, warp.handles[0].y, warp.handles[2].y);
		if(flipY) p.y= appHeight-p.y;
		if(flipX) p.x= appWidth-p.x;
	
		// shouldn't need to do this once we have proper clamping
		p.x = ofClamp(p.x, 0, appWidth);
		p.y = ofClamp(p.y, 0, appHeight);
	
	
		
		if(showPostTransformPreview){
			ofPoint previewpoint(p.x, p.y);
			previewPathMesh.addVertex(previewpoint);
			
		}
		
		
		// TODO proper colour calibration
		ofColor c(p.r, p.g, p.b);
		c*=intensity;
		
		ildaPoints.push_back(ofxIlda::Point(p, c, pmin, pmax));
		
	}
	
}
//
//void Manager :: addIldaPointRaw(ofPoint p, ofFloatColor c, bool useCalibration) {
//	
////	if(useCalibration) {
////		c.r = calculateCalibratedBrightness(c.r, intensity, red100, red75, red50, red25, red0);
////		c.g = calculateCalibratedBrightness(c.g, intensity, green100, green75, green50, green25, green0);
////		c.b = calculateCalibratedBrightness(c.b, intensity, blue100, blue75, blue50, blue25, blue0);
////	}
//	
//	if(!showPostTransformPreview) pathMesh.addVertex(p);
//	
//	
//	ofPoint warpedpoint = p; // warp.getWarpedPoint(p);
//	
//	ofxIlda::Point ildapoint = ofPointToIldaPoint(warpedpoint, c);
//	ildaPoints.push_back(ildapoint);
//	
//	if(showPostTransformPreview){
//		ofPoint previewpoint;
//		previewpoint.x = ofMap(ildapoint.x, kIldaMinPoint, kIldaMaxPoint, 0, appWidth);
//		previewpoint.y = ofMap(ildapoint.y, kIldaMinPoint, kIldaMaxPoint, 0, appHeight);
//		pathMesh.addVertex(previewpoint);
//		
//	}
//	
//}
//
//
//ofxIlda::Point Manager :: ofPointToIldaPoint(const ofPoint& ofpoint, ofFloatColor colour){
//	
//	ofPoint p = ofpoint;
//	
//	ofxIlda::Point ildapoint;
//	
//	
//	if(flipY) p.y= appHeight-p.y;
//	if(flipX) p.x= appWidth-p.x;
//	
//	p.x = ofClamp(p.x, 0, appWidth);
//	p.y = ofClamp(p.y, 0, appHeight);
//	
//	
//	ildapoint.set(p, colour, pmin, pmax);
//	return ildapoint;
//	
//}
//
//ofPoint Manager :: ildaPointToOfPoint(const ofxIlda::Point& ildapoint){
//	
//	ofxIlda::Point p = ildapoint;
//	
//	p.x = ofMap(p.x, kIldaMinPoint, kIldaMaxPoint, 0, appWidth);
//	p.y = ofMap(p.y, kIldaMinPoint, kIldaMaxPoint, 0, appHeight);
//	if(flipY) p.y= appHeight-p.y;
//	if(flipX) p.x= appWidth-p.x;
//	
//	//	return  warp.getUnWarpedPoint(ofVec3f(p.x, p.y));
//	return  ofVec3f(p.x, p.y);
//
//
//}
//



void Manager :: connectToEtherdream() {
	
	etherdream.setup();
	//shouldBeConnected = true;
	//etherdream.setPPS(10000);
}
void Manager :: disconnectFromEtherdream() {
	
	etherdream.kill();
	//shouldBeConnected = false;
	
}


//void Manager :: updateRenderDelay() {
//	
//	// if we're using the clever laser render delay
//	// system
//	
//	if(delay > 0) {
//		float currentTime = ofGetElapsedTimef();
//		
//		// add the current shapes and time to the
//		// histories
//		
//		shapesHistory.push_back(shapes);
//		frameTimes.push_back(currentTime);
//		
//		//start from the oldest shapes and while the next set of
//		// shapes are due, delete the oldest
//		int numDeleted = 0;
//		while((frameTimes.size()>1) && (frameTimes[1]+delay < currentTime)) {
//			
//			shapes = shapesHistory.front();
//			
//			// DELETE ALL SHAPES IN HISTORY.FRONT
//			for(int i = 0; i<shapes.size(); i++) {
//				delete shapes[i];
//			}
//			
//			shapesHistory.pop_front();
//			frameTimes.pop_front();
//			
//			//cout << "deleting oldest " << endl;
//			numDeleted++;
//			
//		}
//		shapes = shapesHistory.front();
//		if(numDeleted == 0 ) {
//			//cout << "NONE DELETED" << endl;
//			
//		}
//		
//		
//	} else if(shapesHistory.size()!=0) {
//		// TODO need to also delete shapes otherwise memory leak
//		for(int i = 0; i<shapesHistory.size(); i++) {
//			
//			for(int j = 0; j<shapesHistory[i].size(); j++) {
//				delete shapesHistory[i][j];
//			}
//		}
//		
//		shapesHistory.clear();
//		frameTimes.clear();
//	}
//	
//}

void Manager :: setupParameters() {
	
	float x = appWidth * 0.1;
	float w = appWidth * 0.8;
	float y = appHeight * 0.1;
	float h = appHeight * 0.8;
	warp.set(x, y, w, h);
	
//	warp.label = "laserWarp";
//	warp.setDstPoint(0, ofVec2f(x1,y1));
//	warp.setDstPoint(1, ofVec2f(x2,y1));
//	warp.setDstPoint(2, ofVec2f(x2,y2));
//	warp.setDstPoint(3, ofVec2f(x1,y2));
//
//	warp.setSrcPoint(0, ofVec2f(0,0));
//	warp.setSrcPoint(1, ofVec2f(appWidth,0));
//	warp.setSrcPoint(2, ofVec2f(appWidth,appHeight));
//	warp.setSrcPoint(3, ofVec2f(0,appHeight));
//
//	//warp.loadSettings();
//	warp.visible = true;
//	
//	warp.dragSpeed = 1;
	

	
	parameters.setName("Laser Manager");
	
	connectButton.setup("Etherdream connect");
	connectButton.addListener(this, &Manager ::connectButtonPressed);
	
	//parameters.add(&connectButton);
	
	//parameters.add(etherdreamStatus.set("", "test"));
	
	parameters.add(intensity.set("intensity", 1, 0, 1));
	
	parameters.add(testPattern.set("test pattern", 0, 0, numTestPatterns));
	//parameters.add(delay.set("sync delay", 0, 0, 0.4));
	parameters.add(pps.set("points per second", 30000, 5000, 100000));
	
	pps.addListener(this, &Manager ::roundPPS);
	
	
	//parameters.add(maskMarginTop.set("mask margin top", 0, 0, appHeight));
	//parameters.add(maskMarginBottom.set("mask margin bottom", 0, 0, appHeight));
	//parameters.add(maskMarginLeft.set("mask margin left", 0, 0, appWidth));
	//parameters.add(maskMarginRight.set("mask margin right", 0, 0, appWidth));
	//parameters.add(useMaskBitmap.set("use mask bitmap", true));
	//parameters.add(showMaskBitmap.set("show mask bitmap", true));
	
	//parameters.add(showWarpPoints.set("show warp points", false));
	
	
	parameters.add(colourChangeDelay.set("colour change offset", -6, -15, 15));
	
	parameters.add(flipX.set("flip x", true));
	parameters.add(flipY.set("flip y", true));
	
	parameters.add(laserOnWhileMoving.set("laser on during move", false));
	
	parameters.add(renderLaserPreview.set("preview laser", true));
	parameters.add(renderLaserPathPreview.set("preview laser path", true));
	
	parameters.add(showPostTransformPreview.set("show post transform path", false));
	
	
	parameters.add(moveSpeed.set("move speed", 6,2,20));
	parameters.add(movePointsPadding.set("move points padding", 1,0,20));
	
	parameters.add(shapePreBlank.set("shape pre blank points", 1, 0, 20));
	parameters.add(shapePostBlank.set("shape post blank points", 1, 0, 20));
	
	parameters.add(dotMaxPoints.set("dot max points", 7, 0, 100));
	
	parameters.add(defaultLineAcceleration.set("line acceleration", 0.5, 0.01, 4));
	parameters.add(defaultLineSpeed.set("line speed", 20,2, 40));
	
	parameters.add(defaultCircleOverlap.set("circle overlap", 20,0, 100));
	parameters.add(defaultCircleAcceleration.set("circle acceleration", 0.5, 0.01, 4));
	parameters.add(defaultCircleSpeed.set("circle speed", 20,2, 40));
	
	parameters.add(defaultPolylineCornerThreshold.set("polyline corner threshold", 90,0, 100));
	parameters.add(defaultPolylineAcceleration.set("polyline acceleration", 0.5, 0.01, 4));
	parameters.add(defaultPolylineSpeed.set("polyline speed", 20,2, 40));
	
	//parameters.add(speedEasedLine.set("eased line speed", 8, 2, 20));
	//parameters.add(paddingEasedLine.set("eased line padding", 1,0, 20));
	//parameters.add(spiralSpacing.set("spiral spacing", 10,1, 20));
	
	// where is this getting set up?
	//	homographyParameters.setName("Laser Homography");
	//
	//
		redParams.add(red100.set("red 100", 1,0,1));
		redParams.add(red75.set("red 75", 0.75,0,1));
		redParams.add(red50.set("red 50", 0.5,0,1));
		redParams.add(red25.set("red 25", 0.25,0,1));
		redParams.add(red0.set("red 0", 0,0,0.5));
	
		greenParams.add(green100.set("green 100", 1,0,1));
		greenParams.add(green75.set("green 75", 0.75,0,1));
		greenParams.add(green50.set("green 50", 0.5,0,1));
		greenParams.add(green25.set("green 25", 0.25,0,1));
		greenParams.add(green0.set("green 0", 0,0,0.5));
	
		blueParams.add(blue100.set("blue 100", 1,0,1));
		blueParams.add(blue75.set("blue 75", 0.75,0,1));
		blueParams.add(blue50.set("blue 50", 0.5,0,1));
		blueParams.add(blue25.set("blue 25", 0.25,0,1));
		blueParams.add(blue0.set("blue 0", 0,0,0.5));
	
		redParams.setName("Laser red calibration");
		greenParams.setName("Laser green calibration");
		blueParams.setName("Laser blue calibration");
	//
	//	colourChangeDelay = -6;
	
	//	warp.visible = true;
	
	//	warp.dragSpeed = 1;
	
	//	maskMarginTop.addListener(this, &LaserManager::updateMaskRectangleParam);
	//	maskMarginBottom.addListener(this, &LaserManager::updateMaskRectangleParam);
	//	maskMarginLeft.addListener(this, &LaserManager::updateMaskRectangleParam);
	//	maskMarginRight.addListener(this, &LaserManager::updateMaskRectangleParam);
	
	//	updateMaskRectangle();
	//	if(!maskBitmap.loadImage("img/LaserMask.png")) {
	//		maskBitmap.allocate(appWidth, appHeight, OF_IMAGE_COLOR);
	//	};
	
}
void Manager :: roundPPS(int& v) {
	pps = round((float)pps/1000.0f)* 1000;

}