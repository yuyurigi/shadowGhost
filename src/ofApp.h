#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxGui.h"
#include "Object.h"

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
    
    void drawContour();
    void drawContour2();
    void createMesh();
    void createMesh2();
    
    //video
    vector<ofVideoPlayer> videos;
    int currentVideo;
    
    //opencv
    ofxCvColorImage image; //The current video frame
    ofxCvGrayscaleImage grayImage, monochro, blobImage, blobImage2;
    ofxCvContourFinder contourFinder, contourFinder2;
    int blobNum, blobNum2;
    
    // parameter of GUI
    ofxPanel gui;
    ofxIntSlider threshold;
    ofxFloatSlider min;
    bool bHide;
    
    ofFbo fbo, fbo2;
    float scale;
    int w_original, h_original, w, h;
    
    vector <Object> objects;
        
};
