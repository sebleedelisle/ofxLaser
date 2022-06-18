//
//  PresetBase.cpp
//  GenericTest
//
//  Created by Seb Lee-Delisle on 17/06/2022.
//

#include "PresetBase.h"

PresetBase :: PresetBase(){
    
    
}
void PresetBase ::setLabel(string _label) {
    label = _label;
};


const string& PresetBase :: getLabel() {
    return label.get();
}
