//
//  SVGLoader.h
//
//  Created by Seb Lee-Delisle on 22/02/2018.
//

#pragma once
#include "ofMain.h"
#include "ofxSvgExtra.h"
#include "ofxLaserGraphic.h"

class SVGLoader : public ofThread{
    
    public :
	
    ~SVGLoader();
    
    // returns number of SVGs to load
    int startLoad(string path);
    
    bool hasFinishedLoading();
    int getLoadedCount();
    int getLoadedPercent();
    int getTotalFileCount();
    
    void setLoadOptimisation(bool value);

    
	ofxLaser::Graphic& getLaserGraphic(int index);

	ofDirectory dir;
	vector<ofFile> files;
	
	vector<string> dataStrings;
	vector<ofxLaser::Graphic> frames;
	ofxLaser::Graphic empty;
	string svgData;
	
	ofxSVGExtra svg;

	void replaceAll( string& content, string toFind, string toReplace);
    
    // STATIC
    static deque<SVGLoader*> loadQueue;
    static bool isLoading;
    static void addToQueue(SVGLoader* svgloader);
    static void startQueueLoading();
    static void loadNext();

    static bool sortalgo(const ofFile& a, const ofFile& b);
    static int strcasecmp_withNumbers(const char *void_a, const char *void_b);
	
	protected:
	void threadedFunction();
    volatile int loadedCount = 0;
    int totalFileCount;
    
    bool useLoadOptimisation = true;
    

	
};


