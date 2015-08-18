/*
 *  ofxBezierWarp.cpp
 *
 *  Created by Patrick Saint-Denis on 12-03-05.
 *
 *  Forked by Teruaki Tsubokura on 13-08-28.
 *
 */

#include "ofxBezierWarp.h"

void ofxBezierWarp::setup() {
    setup(800, 600, 10, 0);
}


void ofxBezierWarp::setup(int _width, int _height) {
//    width = _width;
//	height = _height;
    setup(_width, _height, 10, 0);
}

void ofxBezierWarp::setup(int _width, int _height, int grid, int _layer) {
    width = _width;
	height = _height;
    no = _layer;

	mouseON = true;
	spritesON = true;
    anchorControl = false;
    gridRes = grid;
    prev_gridRes = gridRes;
	rad = 10;
    showGrid = false;
    
	defaults();
	
	ofAddListener(ofEvents().mousePressed, this, &ofxBezierWarp::mousePressed, OF_EVENT_ORDER_BEFORE_APP);
	ofAddListener(ofEvents().mouseDragged, this, &ofxBezierWarp::mouseDragged, OF_EVENT_ORDER_BEFORE_APP);
//	ofAddListener(ofEvents().mouseReleased, this, &ofxBezierWarp::mouseReleased);

}

// resets control points to default position
void ofxBezierWarp::defaults() {
    selectedCenter = 0;
	for(int i = 0; i < 4; i++) {
		selectedSprite[i] = 0;
	}
	for(int i = 0; i < 8; i++) {
		selectedControlPoint[i] = 0;
	}
	
    float rnd_x = ofRandom(0.1, 0.6);
    float rnd_y = ofRandom(0.1, 0.6);
	
	
	// top left first then clockwise
	corners[0] = ofPoint(width * (0.0 + rnd_x), height * (0.0 + rnd_y));
	corners[1] = ofPoint(width * (0.3 + rnd_x), height * (0.0 + rnd_y));
	corners[2] = ofPoint(width * (0.3 + rnd_x), height * (0.3 + rnd_y));
	corners[3] = ofPoint(width * (0.0 + rnd_x), height * (0.3 + rnd_y));
	
	// top left first then clockwise
	anchors[0] = ofPoint(width * (0.0 + rnd_x), height * (0.1 + rnd_y));
	anchors[1] = ofPoint(width * (0.1 + rnd_x), height * (0.0 + rnd_y));
	anchors[2] = ofPoint(width * (0.2 + rnd_x), height * (0.0 + rnd_y));
	anchors[3] = ofPoint(width * (0.3 + rnd_x), height * (0.1 + rnd_y));
	anchors[4] = ofPoint(width * (0.3 + rnd_x), height * (0.2 + rnd_y));
	anchors[5] = ofPoint(width * (0.2 + rnd_x), height * (0.3 + rnd_y));
	anchors[6] = ofPoint(width * (0.1 + rnd_x), height * (0.3 + rnd_y));
	anchors[7] = ofPoint(width * (0.0 + rnd_x), height * (0.2 + rnd_y));
}

void ofxBezierWarp::resetAnchors(){
    for(int i = 0; i < 4; i++) {
        anchors[i * 2] = corners[i] + (corners[(i + 3) % 4] - corners[i]) / 3;
        anchors[i * 2 + 1] = corners[i] + (corners[(i + 1) % 4] - corners[i]) / 3;
    }
}

void ofxBezierWarp::draw() {
    if(spritesON){
        ofPushStyle();

        drawGrid(gridRes, gridRes);
		sprites();
        ofPopStyle();
    }
	
	
}

void ofxBezierWarp::draw(ofTexture tex) {

    ofPoint ** bezSurfPoints = new ofPoint*[gridRes+1];
    for( int i=0; i<gridRes+1; i++ ) {
        bezSurfPoints[i] = new ofPoint[gridRes+1];
    }
	
	for(int i = 0; i <= gridRes; i++) {
		for(int j = 0; j <= gridRes; j++) {
			float start_x = bezierPoint(corners[0].x, anchors[0].x, anchors[7].x, corners[3].x, (float)j/gridRes);
			float end_x = bezierPoint(corners[1].x, anchors[3].x, anchors[4].x, corners[2].x, (float)j/gridRes);
			float start_y = bezierPoint(corners[0].y, anchors[0].y, anchors[7].y, corners[3].y, (float)j/gridRes);
			float end_y = bezierPoint(corners[1].y, anchors[3].y, anchors[4].y, corners[2].y, (float)j/gridRes);
			
			float x = bezierPoint(start_x, ((anchors[1].x - anchors[6].x) * (1.0f - (float)j/gridRes)) + anchors[6].x, ((anchors[2].x - anchors[5].x) * (1.0f - (float)j/gridRes)) + anchors[5].x, end_x, (float)i/gridRes);
			float y = bezierPoint(start_y, ((anchors[1].y - anchors[6].y) * (1.0f - (float)j/gridRes)) + anchors[6].y, ((anchors[2].y - anchors[5].y) * (1.0f - (float)j/gridRes)) + anchors[5].y, end_y, (float)i/gridRes);
			
			bezSurfPoints[i][j] = ofPoint(x, y);
		}
	}
	for(int i = 0; i < gridRes; i++) {
		for(int j = 0; j < gridRes; j++) {
			tex.bind();
			glBegin(GL_QUADS);  
			
			glTexCoord2f((i) * (tex.getWidth()/gridRes), (j) * (tex.getHeight()/gridRes));
			glVertex2f(bezSurfPoints[i][j].x, bezSurfPoints[i][j].y);
			
			glTexCoord2f((i+1) * (tex.getWidth()/gridRes), (j) * (tex.getHeight()/gridRes));  
			glVertex2f(bezSurfPoints[i+1][j].x, bezSurfPoints[i+1][j].y);  
			
			glTexCoord2f((i+1) * (tex.getWidth()/gridRes), (j+1) * (tex.getHeight()/gridRes)); 
			glVertex2f(bezSurfPoints[i+1][j+1].x, bezSurfPoints[i+1][j+1].y);  
			
			glTexCoord2f((i) * (tex.getWidth()/gridRes), (j+1) * (tex.getHeight()/gridRes)); 
			glVertex2f(bezSurfPoints[i][j+1].x, bezSurfPoints[i][j+1].y);
            
			glEnd();
			tex.unbind();
		}
	}
	sprites();
    
    for( int i=0; i<gridRes+1; i++ ) {
        delete[] bezSurfPoints[i];
    }
    delete[] bezSurfPoints;
}


void ofxBezierWarp::sprites() {
	if(spritesON) {
		ofEnableSmoothing();
		for(int i = 0; i < 4; i++) {
			if(selectedSprite[i] == 1) {
                if(anchorControl){
                    ofPoint m = ofPoint(mousePosX,mousePosY);
                    anchors[i * 2] += m - corners[i];
                    anchors[i * 2 + 1] += m - corners[i];
                }else{
                    anchors[i * 2] = corners[i] + (corners[(i + 3) % 4] - corners[i]) / 3;
                    anchors[i * 2 + 1] = corners[i] + (corners[(i + 1) % 4] - corners[i]) / 3;
                    anchors[(i * 2 + 7) % 8] = corners[i] + (corners[(i + 3) % 4] - corners[i]) / 3 * 2;
                    anchors[(i * 2 + 2) % 8] = corners[i] + (corners[(i + 1) % 4] - corners[i]) / 3 * 2;
                }
                
				corners[i] = ofPoint(mousePosX,mousePosY);
				ofPushStyle();
                ofEnableAlphaBlending();
				ofSetColor(0, 255, 0, 150);
				ofFill();
                ofSetLineWidth(2);
				ofCircle(corners[i].x, corners[i].y, rad);
				ofDisableAlphaBlending();
				ofPopStyle();
				ofPushStyle();
				ofSetColor(255, 100, 0);
				ofNoFill();
                ofSetLineWidth(2);
				ofCircle(corners[i].x, corners[i].y, rad);
				ofLine(corners[i].x, corners[i].y - (rad) - 5, corners[i].x, corners[i].y + (rad) + 5);
				ofLine(corners[i].x - (rad) - 5, corners[i].y, corners[i].x + (rad) + 5, corners[i].y);
				ofPopStyle();
			} else {
				ofPushStyle();
				ofSetColor(0, 255, 0);
                if(selectedCenter) ofSetColor(255, 100, 0);
				ofNoFill();
                ofSetLineWidth(2);
				ofCircle(corners[i].x, corners[i].y, rad);
				ofLine(corners[i].x, corners[i].y - (rad) - 5, corners[i].x, corners[i].y + (rad) + 5);
				ofLine(corners[i].x - (rad) - 5, corners[i].y, corners[i].x + (rad) + 5, corners[i].y);
				ofPopStyle();
			}		      
		}
        for(int i = 0; i < 8; i++) {		    
            if(selectedControlPoint[i] == 1) {
                anchors[i] = ofPoint(mousePosX,mousePosY);
                
                if(anchorControl){
                    ofPushStyle();
                    ofEnableAlphaBlending();
                    ofSetColor(0, 255, 0, 150);
                    ofFill();
                    ofSetLineWidth(2);
                    ofCircle(anchors[i].x, anchors[i].y, rad/2);
                    ofDisableAlphaBlending();
                    ofPopStyle();

                    ofPushStyle();
                    ofNoFill();
                    ofSetLineWidth(2);
                    ofSetColor(255, 100, 0);
                    ofCircle(anchors[i].x, anchors[i].y, rad/2);
                    ofLine(corners[i/2].x, corners[i/2].y, anchors[i].x, anchors[i].y);
                    ofPopStyle();
                }
                
                ofPushStyle();
                ofNoFill();
                ofSetLineWidth(2);
                ofSetColor(0, 255, 0);
                if((i % 2) == 0) {
                    if(anchorControl){
                        ofBezier(corners[i/2].x, corners[i/2].y, anchors[(i+1) % 8].x, anchors[(i+1) % 8].y, anchors[(i+2) % 8].x, anchors[(i+2) % 8].y, corners[((i/2)+1) % 4].x, corners[((i/2)+1) % 4].y);
                    }else{
                        ofLine(corners[i/2].x, corners[i/2].y, corners[((i/2)+1) % 4].x, corners[((i/2)+1) % 4].y);
                    }
                }
                ofPopStyle();
            } else {
                if(anchorControl){
                    ofPushStyle();
                    ofNoFill();
                    ofSetLineWidth(2);
                    ofSetColor(0, 255, 255);
                    if(selectedCenter) ofSetColor(255, 100, 0);
                    ofCircle(anchors[i].x, anchors[i].y, rad/2);
                    ofLine(corners[i/2].x, corners[i/2].y, anchors[i].x, anchors[i].y);
                    ofPopStyle();
                }
                
                ofPushStyle();
                ofNoFill();
                ofSetLineWidth(2);
                ofSetColor(0, 255, 0);
                if(selectedCenter) ofSetColor(255, 100, 0);
                if((i % 2) == 0) {
                    if(anchorControl){
                        ofBezier(corners[i/2].x, corners[i/2].y, anchors[(i+1) % 8].x, anchors[(i+1) % 8].y, anchors[(i+2) % 8].x, anchors[(i+2) % 8].y, corners[((i/2)+1) % 4].x, corners[((i/2)+1) % 4].y);
                    }else{
                        ofLine(corners[i/2].x, corners[i/2].y, corners[((i/2)+1) % 4].x, corners[((i/2)+1) % 4].y);
                    }
                }
                ofPopStyle();
            }
        }
        center = ofPoint();
        for(int i = 0; i < 4; i++) {
            center += corners[i];
        }
        center /= 4;
        ofPushStyle();
        ofEnableAlphaBlending();
        ofFill();
        ofSetLineWidth(2);

        if(selectedCenter){
            ofSetColor(255, 100, 0, 200);
            ofPoint m = ofPoint(mousePosX,mousePosY);
            for(int i = 0; i < 4; i++) {
                corners[i] += m - center;
            }
            for(int i = 0; i < 8; i++) {
                anchors[i] += m - center;
            }
        }else{
            ofSetColor(0, 255, 0, 200);
        }
        ofCircle(center, rad * 2);
        ofDisableAlphaBlending();
        ofPopStyle();
        
		ofDisableSmoothing();
	}
}

float ofxBezierWarp::bezierPoint(float x0, float x1, float x2, float x3, float t) {
	float   ax, bx, cx;
	float   ay, by, cy;
	float   t2, t3;
	float   x;
	
	// polynomial coefficients
	cx = 3.0f * (x1 - x0);
	bx = 3.0f * (x2 - x1) - cx;
	ax = x3 - x0 - cx - bx;
	
	t2 = t * t;
	t3 = t2 * t;
	x = (ax * t3) + (bx * t2) + (cx * t) + x0;
	
	return x;
}

//saves sprites positions
void ofxBezierWarp::save() {
	
	// open and write data to the file
    string _name= "presets_" + ofToString(no) + ".bin";
	std::fstream ofs( _name.c_str(), std::ios::out | std::ios::binary );
	for(int i = 0; i < 4; i++) {
		ofs.write( (const char*) &corners[i].x, sizeof(corners[i].x) );
		ofs.write( (const char*) &corners[i].y, sizeof(corners[i].y) );
	}
	for(int i = 0; i < 8; i++) {
		ofs.write( (const char*) &anchors[i].x, sizeof(anchors[i].x) );
		ofs.write( (const char*) &anchors[i].y, sizeof(anchors[i].y) );
	}
	ofs.close();	
}

//reloads last saved sprites positions
void ofxBezierWarp::load() {
	// re-open the file, but this time to read from it
    string _name= "presets_" + ofToString(no) + ".bin";
	std::fstream ifs( _name.c_str(), std::ios::in | std::ios::binary );
	for(int i = 0; i < 4; i++) {
		ifs.read( (char*) &corners[i].x, sizeof(corners[i].x) );
		ifs.read( (char*) &corners[i].y, sizeof(corners[i].y) );
	}
	for(int i = 0; i < 8; i++) {
		ifs.read( (char*) &anchors[i].x, sizeof(anchors[i].x) );
		ifs.read( (char*) &anchors[i].y, sizeof(anchors[i].y) );
	}
	ifs.close();
    ofLogNotice("ofxBezierWarp::load() : \n"
                 + ofToString(corners[0].x) + "," + ofToString(corners[0].y) + "\n"
                 + ofToString(corners[1].x) + "," + ofToString(corners[1].y) + "\n"
                 + ofToString(corners[2].x) + "," + ofToString(corners[2].y) + "\n"
                 + ofToString(corners[3].x) + "," + ofToString(corners[3].y) + "\n"
                );
};

//handles mouse events 1
bool ofxBezierWarp::mousePressed(ofMouseEventArgs &e) {
	
	int x = e.x;
	int y = e.y;
	int button = e.button;
	
	bool objectClicked = false;
	
    if(mouseON){
        mousePosX = x;
        mousePosY = y;
        
        selectedCenter = 0;
        for(int i = 0; i < 4; i++) {
            selectedSprite[i] = 0;
        }
        for(int i = 0; i < 8; i++) {
            selectedControlPoint[i] = 0;
        }
        
        if(spritesON) {
            for(int i = 0; i < 4; i++) {
                //
                if(selectedSprite[i] == 0) {
                    if((x > corners[i].x - (rad)) && (x < corners[i].x + (rad)) && (y >  corners[i].y - (rad)) && (y <  corners[i].y + (rad))) {
						
						objectClicked = true;
						
                        if(button == 0){
                            selectedSprite[i] = 1;
                            mousePosX = corners[i].x;
                            mousePosY = corners[i].y;
                        }else{
							anchorControl = !anchorControl;
						}
                    }
                }
            }
            for(int i = 0; i < 8; i++) {
                //
                if(selectedControlPoint[i] == 0 && anchorControl) {
                    if((x > anchors[i].x - (rad/2)) && (x < anchors[i].x + (rad/2)) && (y >  anchors[i].y - (rad/2)) && (y <  anchors[i].y + (rad/2))) {
                        selectedControlPoint[i] = 1;
                        mousePosX = anchors[i].x;
                        mousePosY = anchors[i].y;
						objectClicked = true;
                    }
                }
            }
            if(selectedCenter == 0){
                if((x > center.x - (rad*2)) && (x < center.x + (rad*2)) && (y >  center.y - (rad*2)) && (y <  center.y + (rad*2))) {
					objectClicked = true;
                    if(button == 0){
                        selectedCenter = 1;
                        mousePosX = center.x;
                        mousePosY = center.y;
                    }else{
						anchorControl = !anchorControl;
                    }
                }
            }
        }
        /*
        if(anchorControl == 0){
            gridRes = 1;
        }else{
            gridRes = prev_gridRes;
        }
         */
    }
	draggingSomething = objectClicked;
	return objectClicked;
}

//handles mouse events 3
bool ofxBezierWarp::mouseDragged(ofMouseEventArgs &e) {
    if(mouseON){
        mousePosX = e.x;
        mousePosY = e.y;
	} else {
	
	}
	return draggingSomething;
}

//handles keyboard events
void ofxBezierWarp::keyPressed(int key) {
	switch(key) {
        case OF_KEY_LEFT:
            mousePosX--;
            break;
        case OF_KEY_RIGHT:
            mousePosX++;
            break;
        case OF_KEY_UP:
            mousePosY--;
            break;
        case OF_KEY_DOWN:
            mousePosY++;
            break;
	}
}

Boolean ofxBezierWarp::isSelected(){
    int _sum = 0;
    _sum += selectedCenter;
    for (int i=0; i<4; i++) {
        _sum += selectedSprite[i];
    }
    if(_sum > 0){
        return true;
    }else{
        return false;
    }
}

void ofxBezierWarp::setCanvasSize(int _width, int _height){
    if(_width > 0 && _height > 0){
        width = _width;
        height = _height;
    }else{
        ofLogWarning("[ofxBezierWarp] setCanvasSize : Width and height must be higher than 0.");
    }
}

void ofxBezierWarp::setWarpResolution(int _res){
    if(_res > 0 && _res < 100){
        gridRes = _res;
        prev_gridRes = gridRes;
    }else{
        ofLogWarning("[ofxBezierWarp] setGridResolution : Resolution must be between 0 - 100.");
    }
}

void ofxBezierWarp::setGridVisible(bool _visible){
    showGrid = _visible;
    spritesON = showGrid;
}

void ofxBezierWarp::drawGrid(float _stepX, float _stepY)
{
    float w = width;
    float h = height;
    
    float perX = w / _stepX;
    float perY = h / _stepY;
    
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    for( int y = 0; y <= h; y+=perY){
        ofSetColor(255, 255, 255, 100);
        ofSetLineWidth(4);
        ofLine(0, y, w, y);
    }
    for( int x = 0; x <= w; x+=perX){
        ofSetColor(255, 255, 255, 100);
        ofSetLineWidth(4);
        ofLine(x, 0, x, h);
    }
    ofPopStyle();
}