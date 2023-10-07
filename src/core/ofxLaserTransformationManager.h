//
//  ofxLaserTransformationManager.h
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 06/10/2023.
//
#pragma once

#include "ofMain.h" // maybe only need glm ?

namespace ofxLaser {

class TransformationManager {
    
    public :
    
    TransformationManager() {
        reset();
    }
    
    void reset() {
        currentMatrix = glm::mat4(1.0f); // identity matrix;
        matrixStack.clear();
    }
     
    glm::vec3 getTransformed(glm::vec3 point) {
        glm::vec4 p4(point.x, point.y, point.z, 1);
        glm::vec4 transformed = currentMatrix * p4;
        return (glm::vec3(transformed.x, transformed.y, transformed.z));
    }
    
    void pushMatrix() {
        matrixStack.push_back(currentMatrix);
    }
    void popMatrix() {
        if(matrixStack.size()>0) {
            currentMatrix = matrixStack.back();
            matrixStack.pop_back();
        } else {
            ofLogError("ofxLaser::Manager::popMatrix called too many times!");
        }
    }
    
    
    /// \brief Translate by (x,y,z) vector of our coordinate system.
    /// The call of ofTranslate() modifies graphics positions.
    ///
    /// Use ofPushMatrix() and ofPopMatrix() to save and restore the untranslated
    /// coordinate system.
    /// ~~~~{.cpp}
    /// void ofApp::draw(){
    ///     ofTranslate(100, 100, 0);   // move the coordinate system to position x 100 and y 100 and make that zero.
    ///     ofDrawRectangle(0, 0, 10, 10);       // draw a rect at that position
    /// }
    /// ~~~~
    void translate(float x, float y, float z = 0) {
        translate(glm::vec3(x,y,z));
    }

    void translate(const glm::vec3 & p) {
        glm::mat4 translationMatrix = glm::translate(glm::mat4(), p);
        currentMatrix =  currentMatrix * translationMatrix;
    }
    void translate(const glm::vec2 & p) {
        translate(p.x, p.y);
    }

    void scale(float x, float y, float z = 1) {
        scale(glm::vec3(x, y, z));
    }

    /// \brief Scale along the X, Y and Z axis with the same amount.
    void scale(float amount) {
        scale(amount, amount, amount);
    }

    void scale(const glm::vec3 & p) {
        glm::mat4 scalingMatrix = glm::scale(p);
        currentMatrix = currentMatrix * scalingMatrix;
    }

 

    /// \brief Produces a rotation around the vector (vecX,vecY,vecZ).
    ///
    /// All graphics drawn after ofRotate is called are rotated. Use ofPushMatrix()
    /// and ofPopMatrix() to save and restore the unrotated coordinate system.
    /// ~~~~{.cpp}
    /// void ofApp::draw(){
    ///     ofRotateDeg(50, 1, 0.5, 0); //rotates the coordinate system 50 degrees along the x-axis and 25 degrees on the y-axis
    ///     ofDrawRectangle(20,20,100,100);
    /// }
    /// ~~~~
    ///
    /// \param degrees Specifies the angle of rotation, in degrees.
    /// \param vecX specifies the x coordinates of a vector
    /// \param vecY specifies the y coordinates of a vector
    /// \param vecZ specifies the z coordinates of a vector
    void rotateDeg(float degrees, float vecX, float vecY, float vecZ){
        rotateRad(glm::radians(degrees), vecX, vecY, vecZ);
    }
    /// \brief Rotate around the z-axis
    void rotateDeg(float degrees) {
        rotateZDeg(degrees);
    }

    /// \brief Produces a rotation around the X-axis of our coordinate
    /// system represented by the vector (1,0,0).
    /// ~~~~{.cpp}
    /// void ofApp::draw(){
    ///     ofRotateXDeg(45); //rotates the coordinate system 45 degrees around the x-axis
    ///     ofDrawRectangle(20,20,100,100);
    /// }
    /// ~~~~
    /// \param degrees Specifies the angle of rotation, in degrees.
    void rotateXDeg(float degrees){
        rotateXRad(glm::radians(degrees));
    }

    /// \brief Produces a rotation around the Y-axis of our coordinate
    /// system represented by the vector (0,1,0).
    /// ~~~~{.cpp}
    /// void ofApp::draw(){
    ///     ofRotateYDeg(45); //rotates the coordinate system 45 degrees around the y-axis
    ///     ofDrawRectangle(20,20,100,100);
    /// }
    /// ~~~~
    /// \param degrees Specifies the angle of rotation, in degrees.
    void rotateYDeg(float degrees){
        rotateYRad(glm::radians(degrees));
    }

    /// \brief Produces a rotation around the Z-axis of our coordinate
    /// system represented by the vector (0,0,1).
    /// ~~~~{.cpp}
    /// void ofApp::draw(){
    ///     ofRotateZDeg(45); //rotates the coordinate system 45 degrees around the z-axis
    ///     ofDrawRectangle(20,20,100,100);
    /// }
    /// ~~~~
    /// \param degrees Specifies the angle of rotation, in degrees.
    void rotateZDeg(float degrees) {
        rotateZRad(glm::radians(degrees));
    }

    /// \brief Produces a rotation around the vector (vecX,vecY,vecZ).
    ///
    /// All graphics drawn after ofRotate is called are rotated. Use ofPushMatrix()
    /// and ofPopMatrix() to save and restore the unrotated coordinate system.
    /// ~~~~{.cpp}
    /// void ofApp::draw(){
    ///     ofRotateRad(M_PI / 2.0f, 1, 0.5, 0); //rotates the coordinate system M_PI / 2.0f radians along the x-axis and M_PI / 4.0f degrees on the y-axis
    ///     ofDrawRectangle(20,20,100,100);
    /// }
    /// ~~~~
    ///
    /// \param radians Specifies the angle of rotation, in radians.
    /// \param vecX specifies the x coordinates of a vector
    /// \param vecY specifies the y coordinates of a vector
    /// \param vecZ specifies the z coordinates of a vector
    void rotateRad(float radians, float vecX, float vecY, float vecZ) {
        currentMatrix = glm::rotate(currentMatrix, radians, glm::vec3(vecX, vecY, vecZ));
    }

    /// \brief Rotate around the z-axis
    void rotateRad(float radians) {
        rotateZRad(radians);
    }

    /// \brief Produces a rotation around the X-axis of our coordinate
    /// system represented by the vector (1,0,0).
    /// ~~~~{.cpp}
    /// void ofApp::draw(){
    ///     ofRotateXRad(M_PI / 4.0f); //rotates the coordinate system M_PI / 4.0f radians around the x-axis
    ///     ofDrawRectangle(20,20,100,100);
    /// }
    /// ~~~~
    /// \param degrees Specifies the angle of rotation, in radians.
    void rotateXRad(float radians) {
        currentMatrix = glm::rotate(currentMatrix, radians, glm::vec3(1,0,0));
    }

    /// \brief Produces a rotation around the Y-axis of our coordinate
    /// system represented by the vector (0,1,0).
    /// ~~~~{.cpp}
    /// void ofApp::draw(){
    ///     ofRotateYRad(M_PI / 4.0f); //rotates the coordinate system M_PI / 4.0f radians around the y-axis
    ///     ofDrawRectangle(20,20,100,100);
    /// }
    /// ~~~~
    /// \param degrees Specifies the angle of rotation, in radians.
    void rotateYRad(float radians) {
        currentMatrix = glm::rotate(currentMatrix, radians, glm::vec3(0,1,0));
    }

    /// \brief Produces a rotation around the Z-axis of our coordinate
    /// system represented by the vector (0,0,1).
    /// ~~~~{.cpp}
    /// void ofApp::draw(){
    ///     ofRotateZRad(M_PI / 4.0f); //rotates the coordinate system M_PI / 4.0f radians degrees around the z-axis
    ///     ofDrawRectangle(20,20,100,100);
    /// }
    /// ~~~~
    /// \param degrees Specifies the angle of rotation, in radians.
    void rotateZRad(float radians) {
        currentMatrix = glm::rotate(currentMatrix, radians, glm::vec3(0,0,1));
    }
    
    vector<glm::mat4> matrixStack;
    glm::mat4 currentMatrix;

};

}
