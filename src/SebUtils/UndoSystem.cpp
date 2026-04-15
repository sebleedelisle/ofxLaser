//
//  UndoSystem.cpp
//  Liberation
//
//  Created by Seb Lee-Delisle on 16/07/2024.
//

#include "UndoSystem.h"

bool UndoSystem :: pushUndoStackIfChanged(){
    //ofLogNotice(" UndoSystem :: pushUndoStackIfChanged  - undoStackSize ") << undoStack.size();
        
        string stateString = getStateString();
        
        
        if((undoStack.size()==0) || (undoStack.back()!=stateString)) {
            
            undoDiffs.emplace_back(undoStack.size()==0 ? "" : undoStack.back(), stateString);
            undoStack.push_back(stateString);
            
            redoStack.clear();
            redoDiffs.clear();
            
            ofLogNotice("pushUndoStackIfChanged() - Undo : ") << undoStack.size() << " Redo : " << redoStack.size();

            return true;
        } else {
            return false;
        }

}




bool UndoSystem :: undo(){
    
    if(undoStack.size()>1) {
        
        // get current state as stored on undo stack
        redoStack.push_back(undoStack.back());
        undoStack.pop_back();
        
        redoDiffs.push_back(undoDiffs.back());
        undoDiffs.pop_back();
        
        // now restore undo stack before that
        string stateString = undoStack.back();

        string testString = getStateStringFromDiffs();
        bool passed = (testString==stateString);
        if(!passed) {
            ofLogError("undo() - TEST FAILED ") << (testString==stateString);
            ofLogError() << testString; 
            restoreFromStateString(stateString);
        } else {
            restoreFromStateString(testString);
        }
        ofLogNotice("undo() - Undo : ") << undoStack.size() << " Redo : " << redoStack.size();
        return true;
    } else {
        
        ofLogError("PatchEditor :: popUndoStack() - nothing to undo!");
        return false;
    }
}



bool UndoSystem :: redo() {
    
    if(redoStack.size()>0) {
        
        undoStack.push_back(redoStack.back());
        string stateString = redoStack.back();
        redoStack.pop_back();
        
        undoDiffs.push_back(redoDiffs.back());
        redoDiffs.pop_back();
        
        string testString = getStateStringFromDiffs();
        bool passed = (testString==stateString);
        if(!passed) {
            ofLogError("redo() - TEST FAILED ") << (testString==stateString);
            restoreFromStateString(stateString);
            
        } else {
            restoreFromStateString(testString);
        }
        
        
        ofLogNotice("redo() - Undo : ") << undoStack.size() << " Redo : " << redoStack.size();
        return true;
    } else {
        
        ofLogError("PatchEditor :: redo() - nothing to redo!");
        return false;
    }
    
}
//
//string UndoSystem :: getStateString() {
//    if(patch!=nullptr) {
//        ofJson json;
//        patch->update(0,0); // reset time to zero so we don't compare params that change
//        patch->serialize(json, true);
//        
//        return json.dump();
//    } else {
//        ofLogError("PatchEditor :: getStateString() - null patch to edit");
//        return "";
//    }
//}
//
//bool UndoSystem :: restoreFromStateString(string statestring) {
//    if(patch!=nullptr) {
//        ofJson json = ofJson::parse(statestring);
//        return patch->deserialize(*json.begin());
//    } else {
//        ofLogError("PatchEditor :: restoreFromStateString() - null patch to edit");
//        return false;
//    }
//    
//}

bool UndoSystem :: canUndo() {
    return undoStack.size()>1;
}
bool UndoSystem :: canRedo() {
    return redoStack.size()>0;
}

void UndoSystem :: resetUndoStack() {
    
    redoStack.clear();
    undoStack.clear();
    
}


string UndoSystem :: getStateStringFromDiffs() {
    string str = "";
    for(Diff& diff : undoDiffs) {
        str = diff.patchForward.patchString(str);
        
    }
    return str;
    
}
