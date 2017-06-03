//
//  FlappyBird.cpp
//  LaserShow
//
//  Created by Seb Lee-Delisle on 18/03/2014.
//
//

#include "FlappyBird.h"


FlappyBird::FlappyBird() {
	gameWidth = 800;
	gameHeight = 600;
	gameRect.set(0,0,800,600);
	reset();
	
	top = 0;
	bottom = 600;
	ypos = top;
	xpos = 100;
	yvel = 0;
	radius = 20;
	
	sensitivity = 1;
	
	columnsPerSecond= 0.2;
	
	birdSVGs.push_back(ofxSVG());
	birdSVGs.back().load("flappy bird.svg");
	birdSVGs.push_back(ofxSVG());
	birdSVGs.back().load("flappy wing up.svg");
	birdSVGs.push_back(ofxSVG());
	birdSVGs.back().load("flappy bird.svg");
	birdSVGs.push_back(ofxSVG());
	birdSVGs.back().load("flappy wing down.svg");
	
	currentBirdSVG = 0;
	
	textWriter.lineRandomness = 0;
	textWriter.colourFlickerMin = 1;
	
	reset();
	
	
};


void FlappyBird :: update(float deltatime, float inputlevel) {

	elapsedTime+=deltatime;
	
	if((alive) && (elapsedTime > 0) && (columnsMade / elapsedTime < columnsPerSecond)) {
		// make column
		columns.push_back(Column(800, top + ((bottom-top)/2), ofMap(columnsMade, 0, 5, 100, 50, true)));
		columnsMade++; 
		
	}
	
	// bird physics
	// sound controlled version :
	
	inputlevel = ofClamp(inputlevel, 0, 1) * sensitivity;
	
	if(alive) {
		yvel*=0.9;
		float target = ofMap(inputlevel, 0, 1, bottom, top, true);
		yvel += (target-ypos) * 0.02;
	} else {
		yvel +=2;
	}
	ypos+=yvel;
	
	/*
	
	// button version
	//yvel -=inputlevel*deltatime*500;
	ypos += yvel;
	// drag
	yvel*=0.99;
	// gravity
	yvel+=deltatime*30;
	*/

	
	
	if(ypos > bottom - radius) {
		if(alive) yvel *= -0.2;
		else yvel = 0; 
		ypos = bottom - radius;
	}

	if(alive) {
		for(int i = 0; i<columns.size(); i++) {
			Column & column = columns[i];
			
			if((column.xpos < xpos + radius) && (column.xpos+50 > xpos-radius)){
				if((ypos-radius < column.gapYPosition - column.gapSize) || (ypos+radius > column.gapYPosition + column.gapSize)) {
					alive = false;
					yvel = 10;
				}
				
			}
			
			float lastpos = column.xpos;
			
			if(alive) column.xpos -=deltatime*200;
			
			if((lastpos+50>=xpos-radius) && (column.xpos+50<xpos-radius)) score++;
			
			if(column.xpos <-50) {
				columns.erase(columns.begin()+i);
				i--;
			}
			
			
			
		}
	}
	
	elapsedTime += deltatime; 

}

void FlappyBird :: draw(ofxLaser::Manager &laserManager) {
	
	
	
	for(int i = 0; i<columns.size(); i++) {
		Column & column = columns[i];

		laserManager.addLaserRect(ofRectangle(ofPoint(column.xpos, top), ofPoint(column.xpos+50,top + column.gapYPosition - column.gapSize)), ofColor::green);
		laserManager.addLaserRect(ofRectangle(ofPoint(column.xpos, top + column.gapYPosition + column.gapSize), ofPoint(column.xpos+50,bottom)), ofColor::green);
		//laserManager.addLaserRect(ofRectangle(ofPoint(column.xpos, column.gapHeight + column.gapSize), ofPoint(50,bottom - (column.gapHeight + column.gapSize))), ofColor::green);
		
	}
	
	
	renderBird(laserManager);
	showScore(laserManager);
}

void FlappyBird :: renderBird(ofxLaser::Manager &laserManager){
	
	
	//laserManager.addLaserCircle(ofPoint(xpos, ypos),radius, ofColor:: yellow);
	//laserManager.addLaserDot(ofPoint(xpos, ypos),ofColor:: yellow);
	ofPoint rotation = ofPoint(0,0,ofMap(yvel, -15,15,-45,45, true));
	ofPoint scale = ofPoint(1,1);
	ofPoint pos = ofPoint(xpos, ypos);
	
	ofxSVG & svg = birdSVGs[currentBirdSVG];
	
	//laserManager.addLaserSVG(birdSVGs[currentBirdSVG], ofPoint(xpos, ypos), ofPoint(1,1), ofPoint(0,0,rotation));
	
	ofVec3f centrePoint = ofVec3f(svg.getWidth()/2, svg.getHeight()/2);
	
	for(int i=0; i<svg.getNumPath(); i++ ) {
		
		const vector<ofPolyline>& lines = svg.getPathAt(i).getOutline();
		ofColor col = svg.getPathAt(i).getStrokeColor();
		
		for(int j=0; j<lines.size(); j++) {
			ofPolyline line = lines[j];
			
			vector<ofVec3f>& vertices = line.getVertices();
			for(int i = 0; i<vertices.size(); i++) {
				ofVec3f& v = vertices[i];
				v-=centrePoint;
				v.rotate(rotation.x, ofPoint(1,0,0));
				v.rotate(rotation.y, ofPoint(0,1,0));
				v.rotate(rotation.z, ofPoint(0,0,1));
				v*=scale;
				v+=pos;
			}
			line.simplify(0.1);
			//cout << "brightness : " << brightness << endl;
			laserManager.addLaserPolyline(line,col, 3);
		}
	}
	

	
	if((yvel<1) &&(ypos<bottom-radius*2) ) {
		currentBirdSVG++;
		if(currentBirdSVG>=birdSVGs.size()) currentBirdSVG = 0;
		
	} else {
		currentBirdSVG = 0;
	}
	
}
void FlappyBird:: showScore(ofxLaser::Manager &laserManager) {

	// todo - fix this clusterfuck :
	
	textWriter.colour = ofColor::magenta;
	ofMesh laserWordMesh;
	
	if(!alive) laserWordMesh = textWriter.getMesh("SCORE "+ofToString(score), ofPoint(400,300), 10, true);
	else laserWordMesh = textWriter.getMesh(ofToString(score), ofPoint(50,top+50), 6, true);
	
	vector<ofVec3f>& vertices = laserWordMesh.getVertices();

	for(int i = 0; i<vertices.size(); i+=2) {
		if(i+1>=vertices.size()) break;
		laserManager.addLaserLine(vertices[i], vertices[i+1], laserWordMesh.getColors()[i]);
	}
	
}


void FlappyBird :: flap() {
	
	if(alive) yvel -=12;
}

void FlappyBird :: reset() {
	
	alive = true;
	elapsedTime = -1;
	columnsMade = 0;
	score = 0;
	columns.clear();

}
