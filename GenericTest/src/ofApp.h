#pragma once

#include "ofMain.h"
#include "PresetManager.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    	
};
//
//template <typename T>
//class PresetManager {
//
//
//private:
//    vector<T*> presets;
//
//
//public:
//    PresetManager();
//    void print();
//};
//
//template <typename T>
//PresetManager<T>::PresetManager()
//{
////    ptr = new T[s];
////    size = s;
////    for (int i = 0; i < size; i++)
////        ptr[i] = arr[i];
//}
//
//template <typename T>
//void PresetManager<T>::print()
//{
////    for (int i = 0; i < size; i++)
////        cout << " " << *(ptr + i);
////    cout << endl;
//}
