#pragma once

#include "ofMain.h"
#include "ofxEasing.h"

class Object {
private:
    vector<ofColor> colors;
    ofMesh mesh;
    float scale, speed;
    float addX;
    float initTime, endPosition, initTime2, initTime3;
    float objectScale, objectScale2;
    float malpha;
    glm::vec2 pos_copy;
public:
    Object(); //コンストラクタ
    void set(ofMesh _mesh, glm::vec2 _pos, float _width, float _height, float _scale);
    void update();
    void draw();
    
    glm::vec2 pos;
    ofColor col;
    float width, height, alpha;
};
