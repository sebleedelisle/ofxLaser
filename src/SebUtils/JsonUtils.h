//
//  JsonUtils.h
//  Liberation
//
//  Created by Seb Lee-Delisle on 01/08/2024.
//

#include "ofMain.h"

class JsonUtils {
    
    
    
    public :
    
    static void serializeVec3(glm::vec3& vec, string label, ofJson& json) {
        json[label] = {vec.x, vec.y, vec.z};
    }
        
    static bool deserializeVec3(glm::vec3& vec, string label, ofJson& json) {
        
        if(json.contains(label) && ((json[label].size()>=3))) {
            vec.x = json[label][0].get<float>();
            vec.y = json[label][1].get<float>();
            vec.z = json[label][2].get<float>();
            return true;
        } else {
            return false;
        }
        
    }
        
    static bool deserializeBool(bool& value, string label, ofJson& json) {
        
        if(json.contains(label)) {
            value=json[label].get<bool>();
            return true;
        } else {
            return false;
        }
        
    }
    
}; 
