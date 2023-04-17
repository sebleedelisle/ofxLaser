#include "ofxSvgExtra.h"
#include "ofConstants.h"
#include <regex>

using namespace std;

extern "C"{
	#include "svgtiny.h"
}
ofxSVGExtra::~ofxSVGExtra(){
	paths.clear();
}

float ofxSVGExtra::getWidth() const {
	return width;
}

float ofxSVGExtra::getHeight() const {
	return height;
}

void ofxSVGExtra::setImprovedCompatibilityMode(bool mode) {
	increaseCompatibility = mode;
}

int ofxSVGExtra::getNumPath(){
	return paths.size();
}
ofPath & ofxSVGExtra::getPathAt(int n){
	return paths[n];
}

void ofxSVGExtra::load(std::string path){
	path = ofToDataPath(path);

	if(path.compare("") == 0){
		ofLogError("ofxSVGExtra") << "load(): path does not exist: \"" << path << "\"";
		return;
	}

	ofBuffer buffer = ofBufferFromFile(path);
	
	loadFromString(buffer.getText(), path);
	
}

void ofxSVGExtra::loadFromString(std::string stringdata, std::string urlstring){
    if(increaseCompatibility) fixSvgText(stringdata);
    if(stringdata.empty()) return ;
    
	const char* data = stringdata.c_str();
	int size = stringdata.size();
	const char* url = urlstring.c_str();

	struct svgtiny_diagram * diagram = svgtiny_create();
	svgtiny_code code = svgtiny_parse(diagram, data, size, url, 0, 0);

	if(code != svgtiny_OK){
		string msg;
		switch(code){
		 case svgtiny_OUT_OF_MEMORY:
			 msg = "svgtiny_OUT_OF_MEMORY";
			 break;

		 /*case svgtiny_LIBXML_ERROR:
			 msg = "svgtiny_LIBXML_ERROR";
			 break;*/

		 case svgtiny_NOT_SVG:
			 msg = "svgtiny_NOT_SVG";
			 break;

		 case svgtiny_SVG_ERROR:
			 msg = "svgtiny_SVG_ERROR: line " + ofToString(diagram->error_line) + ": " + diagram->error_message;
			 break;

		 default:
			 msg = "unknown svgtiny_code " + ofToString(code);
			 break;
		}
		ofLogError("ofxSVGExtra") << "load(): couldn't parse \"" << urlstring << "\": " << msg;
	}

	setupDiagram(diagram);

	svgtiny_free(diagram);
}

void ofxSVGExtra::fixSvgText(std::string& xmlstring) {
	
	ofXml xml;
	
	xml.parse(xmlstring);
	
	// so it turns out that if the stroke width is <1 it rounds it down to 0,
	// and makes it disappear because svgtiny stores strokewidth as an integer!
    ofXml::Search strokeWidthElements = xml.find("//*[@stroke-width]");
	if(!strokeWidthElements.empty()) {
		
		for(ofXml & element: strokeWidthElements){
			//cout << element.toString() << endl;
			float strokewidth = element.getAttribute("stroke-width").getFloatValue();
			//cout << strokewidth << endl;
			strokewidth = MAX(1,round(strokewidth));
			element.getAttribute("stroke-width").set(strokewidth);
			//cout << strokewidth << endl;
		}
	}
    
    // now a bunch of extra code to find and replace stroke-widths defined
    // inside of style tags!
    ofXml::Search styleElements = xml.find("//*[@style]");
    if(!styleElements.empty()) {
        //cout << styleElements.size() << endl;
        //cout << xmlstring<< endl;
        for(int i = 0; i<styleElements.size(); i++) {
            ofXml element = styleElements[i];
            //cout << element.toString() << endl;
        //for(ofXml element: styleElements){
  
            //ofXml::Attribute att = element.getAttribute("style");
            //if(att.)
            string style = element.getAttribute("style").getValue();
           
            //cout << style << endl;
            size_t startpos = style.find("stroke-width");
            if(startpos!=string::npos) {
                
                // remove white space
                if(style.find(" ")!=string::npos) ofJoinString(ofSplitString(style, " "), style);
     
                
                vector<string>styleelements = ofSplitString(style, ";");
                bool changed = false;
                for(string& styleelement : styleelements) {
                    if(styleelement.find("stroke-width")!=string::npos) {
                        //cout << styleelement << endl;
                        startpos = style.find(':', startpos)+1;
                        
                        
                        string strokewidthstring = style.substr(startpos, string::npos);
                        //cout << strokewidthstring << endl;
                        float strokewidth = stof(strokewidthstring);
                        if(strokewidth <1) {
                            styleelement = "stroke-width=1";
                            changed = true;
                        }
                    }
                }
                if(changed) {
                    style = ofJoinString(styleelements, ";");
                    //cout << style << endl;
                    element.setAttribute("style", style);
                    //cout << element.getAttribute("style");
                }
            }

        }
    }
    
	
    // for some reason libsvgtiny fails if there is a viewBox attribute in the
    // svg header but no width and height, or if there's a percentage in the width / height, so let's strip it out!
   
    ofXml::Search viewBoxElements = xml.find("//*[@viewBox]");
    
    if(!viewBoxElements.empty()) {
        for(ofXml & element: viewBoxElements){
            if((element.getAttribute("width").getValue()=="") || (element.getAttribute("width").getValue().find("%")!=string::npos )) {

                element.removeAttribute("viewBox");
            }
        }
    }
        
    
	//lib svgtiny doesn't remove elements with display = none, so this code fixes that
	
	bool finished = false;
	while(!finished) {
		
        ofXml::Search invisibleElements  = xml.find("//*[@display=\"none\"]");
		
		if(invisibleElements.empty()) {
			finished = true;
		} else {
			const ofXml& element = invisibleElements[0];
            ofXml parent = element.getParent();
			if(parent && element) parent.removeChild(element);
		}
		
	}
	
	// implement the SVG "use" element by expanding out those elements into
	// XML that svgtiny will parse correctly.

	finished = false;
	
	while(!finished) {
        ofXml::Search useElements = xml.find("//use");
		if(!useElements.empty()) {
			
			for(ofXml & element: useElements){
				
				// get the id attribute
				string id = element.getAttribute("xlink:href").getValue();
				// remove the leading "#" from the id
				id.erase(id.begin());
				
				// find the original definition of that element - TODO add defs into path?
				string searchstring ="//*[@id='"+id+"']";
                ofXml idelement = xml.findFirst(searchstring);
				
				// if we found one then use it! (find first returns an empty xml on failure)
				if(idelement.getAttribute("id").getValue()!="") {
					
					// make a copy of that element
					element.appendChild(idelement);
					
					// TODO - maybe give the element a new name to avoid id conflicts?
					
					// then turn the use element into a g element
					element.setName("g");
					
				}
			}
		} else {
			finished = true;
		}
		
		//cout << xml.toString()<<endl;
		//cout << "------------------------------------------------------"<< endl;
	}
	//cout << xml.toString()<<endl;
	
	
	
	xmlstring = xml.toString();
	
}

void ofxSVGExtra::draw(bool useColour){
	for(int i = 0; i < (int)paths.size(); i++){
        paths[i].setUseShapeColor(useColour); 
		paths[i].draw();
	}
}

void ofxSVGExtra::setupDiagram(struct svgtiny_diagram * diagram){

    // diagram width and height always seem to be 0! svgtiny - the gift that keeps giving :|
//	width = diagram->width;
//	height = diagram->height;

    ofRectangle bounds;
    bool firstShape = true;
	paths.clear();

	for(int i = 0; i < (int)diagram->shape_count; i++){
		if(diagram->shape[i].path){
			paths.push_back(ofPath());
			setupShape(&diagram->shape[i],paths.back());
            if(firstShape){
                bounds = getBoundingBoxOfPath(paths.back());
                firstShape = false;
            } else {
                bounds.growToInclude(getBoundingBoxOfPath(paths.back()));
            }
            
		}else if(diagram->shape[i].text){
			ofLogWarning("ofxSVGExtra") << "setupDiagram(): text: not implemented yet";
		}
	}
    width = bounds.getWidth();
    height = bounds.getHeight();
    boundingBox = bounds; 
}

    
    
void ofxSVGExtra::setupShape(struct svgtiny_shape * shape, ofPath & path){
	float * p = shape->path;

	path.setFilled(false);

	if(shape->fill != svgtiny_TRANSPARENT){
		path.setFilled(true);
		path.setFillHexColor(shape->fill);
		path.setPolyWindingMode(OF_POLY_WINDING_NONZERO);
    }

	if(shape->stroke != svgtiny_TRANSPARENT){
		// abs the stroke width because apparently sometimes
		// the transformation can cause the stroke width to become
		// negative #facepalm
		path.setStrokeWidth(abs(shape->stroke_width));
		path.setStrokeHexColor(shape->stroke);
	}

	for(int i = 0; i < (int)shape->path_length;){
		if(p[i] == svgtiny_PATH_MOVE){
			path.moveTo(p[i + 1], p[i + 2]);
           //cout << "MOVE " << p[i+1] << " " << p[i+2] << endl;
			i += 3;
		}
		else if(p[i] == svgtiny_PATH_CLOSE){
			path.close();

			i += 1;
		}
		else if(p[i] == svgtiny_PATH_LINE){
			path.lineTo(p[i + 1], p[i + 2]);
            //cout << "LINE " << p[i+1] << " " << p[i+2] << endl;
			i += 3;
		}
		else if(p[i] == svgtiny_PATH_BEZIER){
			path.bezierTo(p[i + 1], p[i + 2],
						   p[i + 3], p[i + 4],
						   p[i + 5], p[i + 6]);
			i += 7;
		}
		else{
			ofLogError("ofxSVGExtra") << "setupShape(): SVG parse error";
			i += 1;
		}
	}
}

const vector <ofPath> & ofxSVGExtra::getPaths() const{
    return paths;
}
