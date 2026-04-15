//
//  SebUtils.h
//  Liberation
//
//  Created by Seb Lee-Delisle on 02/10/2024.
//

#pragma once

class SebUtils {
    
    public :
    
    template <typename T>
    static bool moveElementByIndex(vector<T>& vec, int from, int to) {
        if(from==to) return false;
        if((from<0) || (from>=vec.size()) || (to<0) || (to>=vec.size())) {
            return false;
        }
        
        const auto from_it = vec.begin() + from;
        const auto to_it   = vec.begin() + to;
        
        if(from < to) {
            std::rotate(from_it, from_it + 1, to_it + 1);
        }
        else if(to < from)
        {
            // We're rotating the element toward the front,
            // so we want the new front of the range to be the "from" iterator.
            std::rotate(to_it, from_it, from_it + 1);
        }
        return true;
    }
    
    template <typename T>
    
    static int indexOfElementInVector(vector<std::shared_ptr<T>>& vec, std::shared_ptr<T> element) {
        
        typename vector<std::shared_ptr<T>>::iterator it = std::find_if(vec.begin(), vec.end(), [element](const std::shared_ptr<T> elementToCompare){
            
            return element.get() == elementToCompare.get();;
            
        });
        
        if(it==vec.end()) {
            return -1;
        } else {
            return it-vec.begin();
        }
        
    }
    
    template <typename T>
    static int indexOfElementInVector(vector<T*>& vec, T*element) {
        typename vector<std::shared_ptr<T>>::iterator it = std::find(vec.begin(), vec.end(), element);
        
        if(it==vec.end()) {
            return -1;
        } else {
            return it-vec.begin();
        }
    }
    
    template <typename T>
    static int indexOfElementInVector(vector<T>& vec, T& element) {
        typename vector<T>::iterator it = std::find(vec.begin(), vec.end(), element);
        
        if(it==vec.end()) {
            return -1;
        } else {
            return it-vec.begin();
        }
    }
//    static int indexOfElementInVector(vector<float>& vec, float element) {
//        typename vector<float>::iterator it = std::find_if(vec.begin(), vec.end(), [element](const float elementToCompare){
//            bool eq = fabs(element - elementToCompare) < std::numeric_limits<float>::epsilon();
//            ofLogNotice("comparing floats : " ) << element << " " << elementToCompare << " " << eq;
//            return eq;
//            
//        });
//        
//        if(it==vec.end()) {
//            return -1;
//        } else {
//            return vec.begin()-it;
//        }
//    }
    
    template <typename T>
    static bool elementInVector(vector<std::shared_ptr<T>>& vec, std::shared_ptr<T> element) {
        
        typename vector<std::shared_ptr<T>>::iterator it = std::find_if(vec.begin(), vec.end(), [element](const std::shared_ptr<T> elementToCompare){
            return element.get() == elementToCompare.get();
        });
        
        return it!=vec.end();
    }
    
    template <typename T>
    static bool elementInVector(vector<T*>& vec, T*element) {
        typename vector<T*>::iterator it = std::find(vec.begin(), vec.end(), element);
        
        return it!=vec.end();
    }
    
    template <typename T>
    static bool elementInVector(vector<T>& vec, T&element) {
        typename vector<T>::iterator it = std::find(vec.begin(), vec.end(), element);
        
        return it!=vec.end();
    }
    
    template <typename T>
    static bool removeElementFromVector(vector<std::shared_ptr<T>>& vec, std::shared_ptr<T> element) {
        
        typename vector<std::shared_ptr<T>>::iterator it = std::find_if(vec.begin(), vec.end(), [element](const std::shared_ptr<T> elementToCompare){
            return element.get() == elementToCompare.get();
        });
        
        if(it!=vec.end()) {
            vec.erase(it);
            return true;
        } else {
            return false;
        }
    }
    
    template <typename T>
    static bool removeElementFromVector(vector<T*>& vec, T*element) {
        typename vector<T*>::iterator it = std::find(vec.begin(), vec.end(), element);
        if(it!=vec.end()) {
            vec.erase(it);
            return true;
        } else {
            return false;
        }
    }
    
    template <typename T>
    static bool removeElementFromVector(vector<T>& vec, T&element) {
        typename vector<std::shared_ptr<T>>::iterator it = std::find(vec.begin(), vec.end(), element);
        if(it!=vec.end()) {
            vec.erase(it);
            return true;
        } else {
            return false;
        }
    }
    

};
