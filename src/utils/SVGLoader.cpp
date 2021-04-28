//
//  SVGLoader.cpp
//  ofxLaser
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

	dir.close();
	
	frames.resize(files.size());
	
	loadCount = 0;
	
	int size = (int)files.size();

	SVGLoader::addToQueue(this); 
	SVGLoader::startQueueLoading();
	
	return size; 
}
void SVGLoader::threadedFunction() {
	
	ofSort(files, sortalgo);
	
	string dataString;
	int dupeCount = 0;
	int loadedCount = 0;
	
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
		ofFile ofxlgfile(file.getEnclosingDirectory()+file.getBaseName()+".ofxlg");
		if(ofxlgfile.exists()) {
            if(!useLoadOptimisation) {
                ofxlgfile.remove();
            } else {
                time_t ofxlgfiletime = std::filesystem::last_write_time(ofxlgfile);
                time_t originalfiletime = std::filesystem::last_write_time(file);
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
                ofSavePrettyJson(file.getEnclosingDirectory()+file.getBaseName()+".ofxlg", json);
            }
            
            unlock();

		} else {
			
			//ofLogNotice("Loading ofxlg : " + file.getAbsolutePath());
			ofJson json = ofLoadJson(file.getEnclosingDirectory()+file.getBaseName()+".ofxlg");
			while(!lock()){
				sleep(1);
			}
			frames[i].deserialize(json);
			unlock();

		}
		file.close();
        lock();
		loadCount=(int)i+1;
        unlock();
        
		loadedCount++;
       

	
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
//
//string& SVGLoader::getSvgFilename(int index) {
//	//if(svgs.size()==0) return;
//	index = index %fileNames.size();
//	return fileNames.at(index);
//}

ofxLaser::Graphic&  SVGLoader::getLaserGraphic(int index) {
	if(isThreadRunning() && !lock()) {
		return empty;
	} else {
		
		ofxLaser::Graphic* returngraphic = &empty;
		
		if(index>=(int)frames.size()) index = (int)frames.size()-1;
		if(index<0) index = 0;
		
		if((frames.size()!=0) && (loadCount>index)) {
			returngraphic = &(frames.at(index));
		}
		
		if(isThreadRunning()) unlock();
		
		return *returngraphic;
		
	}
}

