//
//  SVGLoader.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 22/02/2018.
//

#include "SVGLoader.h"

bool SVGLoader::isLoading = false;
deque<SVGLoader*> SVGLoader::loadQueue;

SVGLoader::~SVGLoader() {
    //stopThread();
    if(isThreadRunning()) waitForThread();
    
};

int SVGLoader:: startLoad(string path) {
    
    dir = ofDirectory(path);
	dir.listDir();
	const vector<ofFile>& allFiles = dir.getFiles();

	//only show svg files
	dir.allowExt("svg");
	//populate the directory object
	dir.listDir();

	files = dir.getFiles();

	dir.close();
	
	frames.resize(files.size());
	
	loadedCount = 0;
	
    totalFileCount = (int)files.size();
    
    
	SVGLoader::addToQueue(this); 
	SVGLoader::startQueueLoading();
    
    loadStarted = true;
    
	return totalFileCount;
}
void SVGLoader::threadedFunction() {
	
	ofSort(files, sortalgo);
	
	string dataString;
	int dupeCount = 0;
	
	// load the file
	for(size_t i = 0; i<files.size();i++) {
		ofFile & file = files.at(i);
		

		// TODO compare resulting graphics rather than original files?
//		bool dupe = false;
//		string &data1 = dataStrings[i];
//		for(int j = 0; j<i;j++) {
//			string & data2 = dataStrings[j];
//			if(data1.size()!=data2.size()) {
//				continue;
//			}else if(data1==data2) {
//				ofLog(OF_LOG_NOTICE, "duplicate detected " + ofToString(i));
//				dupe = true;
//				break;
//			}
//
//		}
		
		
//		if(dupe) {
//			ofLog(OF_LOG_NOTICE, "duplicate detected " + ofToString(i));
//			dupeCount++;
//
//		} else {

		//svgData = dataStr;
		//cout << file.getExtension() << endl;
		
		bool loadOptimised = false;
        string optimisedFileName = file.getEnclosingDirectory()+"optimised/"+file.getBaseName()+".ofxlg";
		ofFile ofxlgfile(optimisedFileName);
		if(ofxlgfile.exists()) {
            if(!useLoadOptimisation) {
                ofxlgfile.remove();
            } else {
                std::filesystem::file_time_type ofxlgfiletime = std::filesystem::last_write_time(ofxlgfile);
                std::filesystem::file_time_type originalfiletime = std::filesystem::last_write_time(file);
                if(ofxlgfiletime>originalfiletime) {
                    loadOptimised = true;
                }

            }
        }
		
		if(!loadOptimised) {
			
			//ofLogNotice("Loading svg : " + file.getAbsolutePath());
//			ofBuffer buffer = ofBufferFromFile(file.getAbsolutePath());
//
//			dataString = buffer.getText();
//			buffer.clear();
//
//
//			//ofLog(OF_LOG_NOTICE, "Loading frame #"+ofToString(i));
//			//if(!isThreadRunning()) break;
//
////			while(!lock()){
////				sleep(1);
////			}
////			//ofLog(OF_LOG_NOTICE,file.getFileName());
////			unlock();
//
//			try {
//				svg.loadFromString(dataString);
//			} catch (const std::exception& e) {
//				ofLog(OF_LOG_ERROR, ofToString(e.what()));
//			}
//
			while(!lock()){
				sleep(1);
			}
			frames[i].addSvgFromFile(file.getAbsolutePath(), true, true);
			
            if(useLoadOptimisation) {
                ofJson json;
                frames[i].serialize(json);
                //cout << "Saving optimised file : " << file.getEnclosingDirectory()+file.getBaseName()+".ofxlg" << endl;
                ofSavePrettyJson(optimisedFileName, json);
            }
            
            unlock();

		} else {
			
			//ofLogNotice("Loading ofxlg : " + optimisedFileName);
			ofJson json = ofLoadJson(optimisedFileName);
			while(!lock()){
				sleep(1);
			}
			frames[i].deserialize(json);
			unlock();

		}
		file.close();
        lock();
		loadedCount=(int)i+1;
        unlock();
        

	}
	dir.close();
	while(!lock()){
		sleep(1);
	}
	ofLog(OF_LOG_NOTICE, ofToString(loadedCount) + " svgs finished loading "+ ofToString(dupeCount)+ " duplicates");
	//svgs.clear();
	dataStrings.clear();
	files.clear();
	//fileNames.clear();
	unlock();
	stopThread();
	
	//ofLog(OF_LOG_NOTICE, "SVGLoader finished : " + dir.getOriginalDirectory());
	SVGLoader::loadNext(); 
	
}

bool SVGLoader::hasFinishedLoading() {
    return ((getLoadedCount() == totalFileCount) && (loadStarted));
}
int SVGLoader :: getLoadedPercent(){
    return ofMap(getLoadedCount(), 0, totalFileCount, 0, 100);
}
int SVGLoader :: getTotalFileCount(){
    return totalFileCount;
    
}

int SVGLoader :: getLoadedCount(){
    int count;
    
    if(!isThreadRunning()) return loadedCount;
       
    if(lock()) {
        count = loadedCount;
        unlock();
        return count;
    } else {
        return -1;
    }
}



void SVGLoader :: setLoadOptimisation(bool value) {
    useLoadOptimisation = value;
}

void SVGLoader :: replaceAll(string& data, string stringToFind, string stringToReplace){
	
	std::string::size_type n = 0;
	while ( ( n = data.find( stringToFind, n ) ) != std::string::npos )
	{
		data.replace( n, stringToFind.size(), stringToReplace );
		n += stringToReplace.size();
	}
	

}

ofxLaser::Graphic&  SVGLoader::getLaserGraphic(int index) {
	if(isThreadRunning() && !lock()) {
		return empty;
	} else {
		
		ofxLaser::Graphic* returngraphic = &empty;
		
		if(index>=(int)frames.size()) index = (int)frames.size()-1;
		if(index<0) index = 0;
		
		if((frames.size()!=0) && (loadedCount>index)) {
			returngraphic = &(frames.at(index));
		}
		
		if(isThreadRunning()) unlock();
		
		return *returngraphic;
		
	}
}

// STATIC :
void SVGLoader::addToQueue(SVGLoader* svgloader) {
    loadQueue.push_back(svgloader);
}
void SVGLoader::startQueueLoading() {
    if(!isLoading) {
        loadNext();
        
    }
}
void SVGLoader :: loadNext() {
    if(loadQueue.size()>0) {
        loadQueue[0]->startThread();
        ofLog(OF_LOG_NOTICE, "SVGLoader load starting : " + loadQueue[0]->dir.getOriginalDirectory());
        loadQueue.pop_front();
        isLoading = true;
    } else {
        isLoading = false;
    }
}

bool SVGLoader :: sortalgo(const ofFile& a, const ofFile& b) {
    string aname = a.getBaseName(), bname = b.getBaseName();
    return strcasecmp_withNumbers(aname.c_str(), bname.c_str()) < 0;
}

int SVGLoader :: strcasecmp_withNumbers(const char *void_a, const char *void_b) {
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
