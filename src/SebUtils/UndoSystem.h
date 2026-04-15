//
//  UndoSystem.h
//  Liberation
//
//  Created by Seb Lee-Delisle on 16/07/2024.
//
#pragma once
#include "ofMain.h"

struct DiffPatch {
    
    void set(string newstr, int start, int end) {
        startIndex = start;
        numChars = 0;
        stringReplacement = newstr;
        
    }
    string patchString(string stringToPatch) {
        string left = stringToPatch.substr(0, startIndex);
        string right = stringToPatch.substr(startIndex+numChars);
        return left + stringReplacement + right;
        
    }
    string stringReplacement;
    int startIndex, numChars;
};

class Diff {
    
    public :
    Diff(){};
    Diff(string string1, string string2){
        set(string1, string2);
    }
    void set(string string1, string string2) {
        //ofLogNotice( "Diff set ---------------------------------------------");
        //ofLogNotice() << "|" << string1 << "| |" << string2 << "|";
        if(string1 == string2) {
            patchForward.stringReplacement = patchBackward.stringReplacement = "";
            patchForward.startIndex = patchBackward.startIndex = 0;
            patchForward.numChars = patchBackward.numChars = 0;
            return;
        }
        
        int i = 0;
        while(i<string1.size() && i<string2.size() && string1.at(i)==string2.at(i)) {
            i++;
        }
        
        int startpos = patchForward.startIndex = patchBackward.startIndex = i;
        
        // if we're at the end of the first string
        if(i==string1.size()) {
            patchForward.numChars = 0;
            patchBackward.numChars = string2.size()-i; // TO CHECK
        } else if(i==string2.size()) {
            patchBackward.numChars = 0;
            patchForward.numChars = string1.size()- i; // TO CHECK!
        } else {
            
            i = 0;
            ofLogNotice() << (int)string2.size()-1-i << " " << startpos;
            while(((int)string1.size()-i>startpos) && ((int)string2.size()-i>startpos) && string1.at(string1.size()-1-i)==string2.at(string2.size()-1-i)) {
                i++;
            }
            
            patchForward.numChars = string1.size()- i - startpos;
            patchBackward.numChars = string2.size()- i - startpos;

        }
    
        patchForward.stringReplacement = string2.substr(patchBackward.startIndex, patchBackward.numChars);
        patchBackward.stringReplacement = string1.substr(patchForward.startIndex, patchForward.numChars);

    }
    
    
    bool test(string str1, string str2) {
        set(str1, str2);
        string patched1 = patchForward.patchString(str1);
        string patched2 = patchBackward.patchString(str2);
        //ofLogNotice() << patched2 << " " << str1 << " " << patched1 << " " << str2;
        
        Diff& diff = *this;
        ofLogNotice() << "|" << diff.patchForward.stringReplacement << "| " << diff.patchForward.startIndex << " " << diff.patchForward.numChars << " |" << diff.patchBackward.stringReplacement << "| " << diff.patchBackward.startIndex << " " << diff.patchBackward.numChars;
        
        bool passed = (patched2==str1) && (patched1==str2);
        if(passed) {
            ofLogNotice("Test passed");
        } else {
            ofLogError("TEST FAILED : ") << "patch1 : " << (patched1==str2) << " patch2 : " << (patched2==str1);
            
            for(int i = 0; i<str1.size(); i++) {
                if(i>=patched2.size()) break;
                if(str1.at(i)!=patched2.at(i)) {
                    ofLogNotice("Difference found at : ") << i << " " << (uint8_t)str1.at(i) << " " << (uint8_t)patched2.at(i);
                    ofLogNotice(str1.substr(i-10,20));
                    ofLogNotice(patched2.substr(i-10,20));
                    
                }
            }
                
    
            
            
//            ofFile file("patched1.txt",ofFile::WriteOnly);
//            file << patched1;
//            ofFile file2("orig1.txt",ofFile::WriteOnly);
//            file2 << str2;
        }
        
        return passed;
    }
        
    
    DiffPatch patchForward;
    DiffPatch patchBackward;
    
    static void Test() {
      
        Diff diff;
        string str1 = "This is a test of the diff system. ";
        string str2 = "This is a different test of the diff system. ";

        diff.test(str1, str2);
          
        diff.test("Same string", "Same string");
   
        diff.test("only change is at the end", "only change is at the ending");

        diff.test("1", "12");
 
        diff.test("12", "1");
        
        diff.test("the only change is at the beginning", "only change is at the beginning");
        diff.test("only change is at the beginning", "the only change is at the beginning");
        
        diff.test("23", "123");
        diff.test("123", "23");
   
        diff.test("two completely different strings", "make up this test");
     
        diff.test("12", "34");
   
        diff.test("12", "345");
    
        diff.test("123", "45");
    
        diff.test("12", "3456");
   
        diff.test("1234", "56");
     
        
        diff.test("1234", "125");
     
        diff.test("34", "123456");

        diff.test("123456", "34");
       
        diff.test("comparing to empty string", "");
    
        diff.test("", "comparing to empty string");
    
        
        diff.test("123456", "1256");
        
        
        // seems to fail when it's the same ending but different beginning
        // FAILING
        diff.test("5634", "1234");
        ofLogNotice() << "|" << diff.patchForward.stringReplacement << "| " << diff.patchForward.startIndex << " " << diff.patchForward.numChars << " |" << diff.patchBackward.stringReplacement << "| " << diff.patchBackward.startIndex << " " << diff.patchBackward.numChars;
        ofLogNotice("|12| 0 2 |56| 0 2");
        
        // FAILING
        diff.test("54", "1234");
        ofLogNotice() << "|" << diff.patchForward.stringReplacement << "| " << diff.patchForward.startIndex << " " << diff.patchForward.numChars << " |" << diff.patchBackward.stringReplacement << "| " << diff.patchBackward.startIndex << " " << diff.patchBackward.numChars;
        ofLogNotice("|123| 0 1 |5| 0 3");
        
        // FAILING
        diff.test("1234", "54");
        ofLogNotice() << "|" << diff.patchForward.stringReplacement << "| " << diff.patchForward.startIndex << " " << diff.patchForward.numChars << " |" << diff.patchBackward.stringReplacement << "| " << diff.patchBackward.startIndex << " " << diff.patchBackward.numChars;
        ofLogNotice("|5| 0 3 |123| 0 1");
        
        
        
        
    }
    
    
    
};


class UndoSystem {
    public :
    
    UndoSystem() {
        //Diff::Test();
    }
    virtual bool pushUndoStackIfChanged();
    bool undo();
    bool redo();
    virtual string getStateString() = 0;
    virtual bool restoreFromStateString(string statestring) = 0;
    void resetUndoStack();
    bool canUndo();
    bool canRedo();
    
    string getStateStringFromDiffs(); 

    protected:
    vector<string> undoStack;
    vector<string> redoStack;
    
    vector<Diff> undoDiffs;
    vector<Diff> redoDiffs;
    

    
};
