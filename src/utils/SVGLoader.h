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
	
	~SVGLoader(){
		//stopThread();
		waitForThread();
		
	};
    int getLoadCount(){
        int count;
        
        if(!isThreadRunning()) return loadCount;
           
        if(lock()) {
            count = loadCount;
            unlock();
            return count;
        } else {
            return -1;
        }
    }
	static deque<SVGLoader*> loadQueue;
	static bool isLoading;
	
	static void addToQueue(SVGLoader* svgloader) {
		//deque<SVGLoader*>& loadQueue = *loadQueuePointer;
		loadQueue.push_back(svgloader);
	}
	static void startQueueLoading() {
		if(!isLoading) {
			loadNext();
			
		}
	}
	static void loadNext() {
		//deque<SVGLoader*>& loadQueue = *SVGLoader::loadQueuePointer;
		
		if(loadQueue.size()>0) {
			loadQueue[0]->startThread();
			ofLog(OF_LOG_NOTICE, "SVGLoader load starting : " + loadQueue[0]->dir.getOriginalDirectory());
			loadQueue.pop_front();
			isLoading = true;
		} else {
			isLoading = false;
		}
	}
	
	// returns number of SVGs to load
    int startLoad(string path);
	

    
    //ofxSVGFromString & getSvg(int index);
	ofxLaser::Graphic& getLaserGraphic(int index);

	//string& getSvgFilename(int index);
	ofDirectory dir;
	vector<ofFile> files;
	
	//vector<ofxSVGFromString> svgs;
	//vector<string> fileNames;
	vector<string> dataStrings;
	vector<ofxLaser::Graphic> frames;
	ofxLaser::Graphic empty;
	string svgData;
	
	ofxSVGExtra svg;

   
	void replaceAll( string& content, string toFind, string toReplace);

    static bool sortalgo(const ofFile& a, const ofFile& b) {
        string aname = a.getBaseName(), bname = b.getBaseName();
        return strcasecmp_withNumbers(aname.c_str(), bname.c_str()) < 0;
        
    }
    
    static int strcasecmp_withNumbers(const char *void_a, const char *void_b) {
        const char *a = void_a;
        const char *b = void_b;
        
        if (!a || !b) { // if one doesn't exist, other wins by default
            return a ? 1 : b ? -1 : 0;
        }
        if (isdigit(*a) && isdigit(*b)) { // if both start with numbers
            char *remainderA;
            char *remainderB;
            long valA = strtol(a, &remainderA, 10);
            long valB = strtol(b, &remainderB, 10);
            if (valA != valB)
                return valA - valB;
            // if you wish 7 == 007, comment out the next two lines
            else if (remainderB - b != remainderA - a) // equal with diff lengths
                return (remainderB - b) - (remainderA - a); // set 007 before 7
            else // if numerical parts equal, recurse
                return strcasecmp_withNumbers(remainderA, remainderB);
        }
        if (isdigit(*a) || isdigit(*b)) { // if just one is a number
            return isdigit(*a) ? -1 : 1; // numbers always come first
        }
        while (*a && *b) { // non-numeric characters
            if (isdigit(*a) || isdigit(*b))
                return strcasecmp_withNumbers(a, b); // recurse
            if (tolower(*a) != tolower(*b))
                return tolower(*a) - tolower(*b);
            a++;
            b++;
        }
        return *a ? 1 : *b ? -1 : 0;
    }
	
	protected:
	void threadedFunction();
    volatile int loadCount;
    

	
};


