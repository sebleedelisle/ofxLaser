/*
 *  ofxBezierWarp.h
 *
 *  Created by Patrick Saint-Denis on 12-03-05.
 *	A Bezier Warp made of multiple homographies
 *  
 *  Forked by Teruaki Tsubokura on 13-08-28.
 *
 */

#pragma once

#include "ofMain.h"

class ofxBezierWarp{
			
public:
    int no = 0;
    int layer = 0;
	
	ofPoint corners[4];
	ofPoint anchors[8];
    ofPoint center;
	int gridRes = 0;
    int prev_gridRes = 0;
    int mouseON, spritesON, anchorControl;
    int selectedSprite[4], selectedControlPoint[8], selectedCenter;
    bool showGrid;
		
	ofxBezierWarp(){}
	
    void setup();
	void setup(int _width, int _height);
	
    void draw();
	
    void resetAnchors();
	void save();
	void load();
	bool mousePressed(ofMouseEventArgs &e);
	bool mouseDragged(ofMouseEventArgs &e);
	bool draggingSomething = false; 
	void keyPressed(int clef);
    Boolean isSelected();
    
    void setCanvasSize(int _width, int _height);
    void setWarpResolution(int _res);
    void setGridVisible(bool _visible);
    bool bGradient;
    
//private:
	int mousePosX, mousePosY, rad;
	float width, height;
    
   // void setup();
     void setup(int _width, int _height, int grid, int _layer);
    void draw(ofTexture texture);
    void defaults();
	void sprites();
	float bezierPoint(float x0, float x1, float x2, float x3, float t);
    void drawGrid(float _stepX, float _stepY);
    
    
};