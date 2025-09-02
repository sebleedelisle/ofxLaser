//
//  SebUtils.h
//  Liberation
//
//  Created by Seb Lee-Delisle on 02/10/2024.
//

#pragma once
//
#include "SebUtils.h"
#include "ofxImGui.h"
//
class SebImGui {
    public :
    
    static int dropDownMenu(string label, vector<string>& items, string selectedlabel ) {
        
        int index = SebUtils::indexOfElementInVector(items, selectedlabel);
        return dropDownMenu(label, items, index, selectedlabel);
        
    }
    // returns -1 if nothing changed, index if it has
    // missing label lets you show a non-existent label, when selectedindex == -1
    static int dropDownMenu(string label, const vector<string>& items, int selectedindex, string selectedlabel = "") {
        
        bool missing = false; // !SebUtils::elementInVector(items, selectedlabel) ;
        //    int selectedindex =
        
        if((selectedindex>=0) && (selectedindex<items.size())) {
            selectedlabel = items.at(selectedindex);
        } else {
            missing = true;
            selectedlabel = "Not selected";
        }
        
        int selection = -1;
        if (ImGui::BeginCombo(label.c_str(), selectedlabel.c_str())) { // The second parameter is the label previewed before opening the combo.
            
            for(size_t i =0 ; i<items.size(); i++) {
           
                const string& item = items.at(i);
                string itemlabel = item+"###"+ofToString(label) + ofToString(i);//+"#"+ofToString(i)+label; // to ensure unique
                
                if (ImGui::Selectable(itemlabel.c_str(), i==selectedindex)) {
                    selection = i;
                    ofLogNotice("dropDownMenu selected : " ) << selection << " " << item;
                    
                }
            }
            
            ImGui::EndCombo();
        }
        return selection;
        
    }
    
    template <typename T>
    static int dropDownMenu(string label, vector<std::pair<string, T>>& items, T selectionvalue) {

        
        int selectionindex = -1;
        string selectionstring = "";
        
        // find the value in the items
        typename vector<std::pair<string, T>>::iterator it = std::find_if(items.begin(), items.end(), [selectionvalue](const std::pair<string, T> itemtocompare){
            return selectionvalue == itemtocompare.second;
        });

        // if we found it then use that pair to get the values
        if(it!=items.end()) {
            selectionindex = it - items.begin();
            selectionstring = it->first;
        }
        
        int selection = -1;
        if (ImGui::BeginCombo(label.c_str(), selectionstring.c_str())) { // The second parameter is the label previewed before opening the combo.
            
            for(size_t i =0 ; i<items.size(); i++) {
           
                string& item = items.at(i).first;
                string itemlabel = item+"##"+ofToString(i)+label; // to ensure unique
                
                if (ImGui::Selectable(itemlabel.c_str(), i==selectionindex)) {
                    selection = i;
                    
                }
            }
            
            ImGui::EndCombo();
        }
        return selection;
        
    }
    
    static bool TextInput(string label, string& value, int width) {
        static const int maxstringlength = 2000;
        static char textinputcache[maxstringlength] ="";
        
        ImGui::PushStyleColor(ImGuiCol_FrameBg, {0,0,0,0} );
        
        ImGui::SetNextItemWidth(width);
        bool changed = false;
        if(ImGui::InputText(label.c_str(), textinputcache, maxstringlength, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue  )){
            changed = true;
            
        }
        if(ImGui::IsItemDeactivated()) {
            changed = true;
        }
        if(!ImGui::IsItemFocused() ) {
            strcpy(textinputcache, value.c_str());
        }
        ImGui::PopStyleColor();
        if(changed) {
            value = textinputcache;
            return true;
        } else {
            return false;
        }
        
    }
    
    
};
