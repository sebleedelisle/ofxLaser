//
//  SVGLoader.cpp
//  SVGPlayerFlux
//
//  Created by Seb Lee-Delisle on 22/02/2018.
//

#include "SVGLoader.h"

bool SVGLoader::isLoading = false;
deque<SVGLoader*> SVGLoader::loadQueue;

int SVGLoader:: startLoad(string path) {
    
    dir = ofDirectory(path);
	dir.listDir();
	const vector<ofFile>& allFiles = dir.getFiles();

	//only show svg files
	dir.allowExt("svg");
	//populate the directory object
	dir.listDir();

	files = dir.getFiles();

	ofSort(files, sortalgo);
	
	for(int j = 0; j<files.size(); j++) {
		string newname = path+"/"+files[j].getFileName();
		fileNames.push_back(newname);
	}

	frames.resize(files.size());
	
	loadCount = 0;
	
	int size = files.size();

	SVGLoader::addToQueue(this); 
	SVGLoader::startQueueLoading();
	
	return size; 
}
void SVGLoader::threadedFunction() {
	
	
	dataStrings.resize(files.size());
	int dupeCount = 0;
	int loadedCount = 0;
	
	for(int i = 0; i<files.size();i++) {
		const ofFile & file = files.at(i);
		ofBuffer buffer = ofBufferFromFile(file.getAbsolutePath());
		
		dataStrings[i] = buffer.getText();
		buffer.clear();
	}
	for(int i = 0; i<dataStrings.size();i++) {

		//ofLog(OF_LOG_NOTICE, "Loading frame #"+ofToString(i));
		//if(!isThreadRunning()) break;
		
		ofFile& file = files[i];
		
		while(!lock()){
			sleep(1);
		}
		//ofLog(OF_LOG_NOTICE,file.getFileName());
		unlock();
		
		bool dupe = false;
		string &data1 = dataStrings[i];
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
		
		
		if(dupe) {
			ofLog(OF_LOG_NOTICE, "duplicate detected " + ofToString(i));
			dupeCount++;
			
		} else {

			svgData = data1;
			
			try {
				svg.loadFromString(svgData);
			} catch (const std::exception& e) {
				ofLog(OF_LOG_ERROR, ofToString(e.what()));
			}
			
			while(!lock()){
				sleep(1);
			}
			frames[i].addSvg(svg);

			loadCount=i+1;
			
			loadedCount++;
			unlock();
		}
	
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
	
	ofLog(OF_LOG_NOTICE, "SVGLoader finished : " + dir.getOriginalDirectory());
	SVGLoader::loadNext(); 
	
}

void SVGLoader::replaceAll(string& data, string stringToFind, string stringToReplace){
	
	std::string::size_type n = 0;
	while ( ( n = data.find( stringToFind, n ) ) != std::string::npos )
	{
		data.replace( n, stringToFind.size(), stringToReplace );
		n += stringToReplace.size();
	}
	

}

string& SVGLoader::getSvgFilename(int index) {
	//if(svgs.size()==0) return;
	index = index %fileNames.size();
	return fileNames.at(index);
}

ofxLaser::Graphic&  SVGLoader::getLaserGraphic(int index) {
	if(isThreadRunning() && !lock()) {
		return empty;
	} else {
		
		ofxLaser::Graphic* returngraphic = &empty;
		
		if(index>=frames.size()) index = frames.size()-1;
		if(index<0) index = 0;
		
		if((frames.size()!=0) && (loadCount>index)) {
			returngraphic = &(frames.at(index));
		}
		
		if(isThreadRunning()) unlock();
		
		return *returngraphic;
		
	}
}

