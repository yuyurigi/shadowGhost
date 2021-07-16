#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(60);
    ofHideCursor();
    
    //video
    ofDirectory dir;
    dir.listDir("movie/");
    dir.sort();
    if(dir.size()){
        videos.assign(dir.size(), ofVideoPlayer());
    }
    for (int i = 0; i < (int)dir.size(); i++) {
        videos[i].load(dir.getPath(i));
    }
    currentVideo = 0;
    videos[currentVideo].play();
    
    //ビデオの大きさ
    w_original = videos[0].getWidth();
    h_original = videos[0].getHeight();
    scale = 0.5; //動画をスクリーンに表示するときの大きさ（元の動画に掛ける数値）
    w = w_original*scale;
    h = h_original*scale;
    
    //opencvイメージの初期化
    //なくても動くけど、以下のコードを入れておくとコンソールに[warning]・[notaice]の文が出なくなる
    image.allocate(w_original, h_original);
    grayImage.allocate(w_original, h_original);
    monochro.allocate(w_original, h_original);
    blobImage.allocate(w_original, h_original);
    blobImage2.allocate(w_original, h_original);
    
    //fbo
    fbo.allocate(w_original, h_original);
    fbo.begin();
    ofBackground(255, 255, 255);
    fbo.end();
    
    fbo2.allocate(w_original, h_original);

    //set GUI
    gui.setup();
    gui.add(threshold.setup("threshold", 235, 0, 255));
    gui.add(min.setup("minArea", 0.01, 0.00001, 1.0));
    float gwidth = gui.getWidth();
    gui.setPosition(ofGetWidth()-10-gwidth, 10);
    bHide = true;
}

//--------------------------------------------------------------
void ofApp::update(){
    videos[currentVideo].update(); //必要に応じて新しいフレームをデコードします
    //新しいフレームが取得された場合にのみ計算を実行します
    if (videos[currentVideo].isFrameNew()) {
        
        //新しいフレームを取得
        image.setFromPixels(videos[currentVideo].getPixels());
        grayImage = image; //グレイスケールイメージに変換
        monochro = grayImage;
        monochro.threshold(threshold); //threshold 0~255の色要素を0,1に変換する
        
        fbo.begin();
        ofEnableBlendMode(OF_BLENDMODE_MULTIPLY); //乗算
        monochro.draw(0, 0);
        fbo.end();
        ofDisableBlendMode();
        
        fbo2.begin();
        monochro.draw(0, 0);
        fbo2.end();
    }
    
    for (int i = 0; i < objects.size(); i++) {
        float a = objects[i].alpha; //オブジェクトのアルファ値を取得
        if(a == 0){ //アルファ値が0になったらvectorから消す
            objects[i] = objects.back(); //最後の要素と交換
            objects.pop_back();
        }
        objects[i].update();
    }
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(255, 255, 255); //Set the background color
        
    //イメージの描画色を設定する
    ofSetColor(255, 255, 255);
    
    grayImage.draw(0, 0, w, h);
    monochro.draw(w+10, 0, w, h);
    fbo.draw(0, h+10, w, h);
    
    drawContour();
    drawContour2();
    
    for (int i = 0; i < objects.size(); i++) {
        objects[i].draw();
    }
    
    
    if(!bHide){
        gui.draw();
    }
}

//--------------------------------------------------------------
void ofApp::drawContour(){
    ofPixels pixels;
    fbo.readToPixels(pixels);
    pixels.setImageType(OF_IMAGE_GRAYSCALE);
    blobImage.setFromPixels(pixels);
    
    int totalArea = blobImage.width*blobImage.height;
    int minArea = totalArea * min;
    int maxArea = totalArea * 0.75;
    int nConsidered = 200;
    
    contourFinder.findContours(blobImage, minArea, maxArea, nConsidered, true);

    ofPushMatrix();
    ofTranslate(w+10, h+10);
    ofFill();
    ofSetHexColor(0x333333);
    ofDrawRectangle(0, 0, w, h);
    ofSetHexColor(0xffffff);
    ofScale(scale, scale);
    
    blobNum = contourFinder.nBlobs;
    
    for (int i = 0; i < blobNum; i++) {
        //検出した物体の幅を取得する
        ofRectangle rect;
        rect = contourFinder.blobs[i].boundingRect;
        float blobWidth = rect.getWidth();
        //ムービーに写ってる物体が画面外に見切れてる場合は探知をやり直す
        //見切れてない場合のみ輪郭抽出する
        if(blobWidth == w_original-2){
            fbo.begin();
            ofBackground(255, 255, 255);
            fbo.end();
        }
        
        //輪郭を描画
        contourFinder.blobs[i].draw(0, 0);
    }
    
    ofPopMatrix();
     
}
//--------------------------------------------------------------
void ofApp::createMesh(){
    
    for (int i = 0; i < blobNum; i++) {
        //検出した物体の幅、高さを取得する
        ofRectangle rect;
        rect = contourFinder.blobs[i].boundingRect;
        float blobWidth = rect.getWidth();
        float blobHeight = rect.getHeight();
        //物体の中心値を取得する
        glm::vec2 pos = contourFinder.blobs[i].centroid;
        
        ofPolyline poly;
        for (int j = 0; j < contourFinder.blobs[i].pts.size(); j++) {
            ofPoint ver = contourFinder.blobs[i].pts[j];
            ver.x = ofMap(ver.x, pos.x-blobWidth/2, pos.x+blobWidth/2, 0, blobWidth);
            ver.y = ofMap(ver.y, pos.y-blobHeight/2, pos.y+blobHeight/2, 0, blobHeight);
            poly.addVertex(ver.x, ver.y);
        }
        poly.close();
        
        ofMesh mesh;
        ofTessellator tess;
        tess.tessellateToMesh(poly, ofPolyWindingMode::OF_POLY_WINDING_ODD, mesh, true);
        
        Object o;
        glm::vec2 p = glm::vec2(w+10+pos.x*scale, h+10+pos.y*scale);
        o.set(mesh, p, blobWidth, blobHeight, scale);
        objects.push_back(o);
    }
}
//--------------------------------------------------------------
void ofApp::drawContour2(){
    ofPixels pixels;
    fbo2.readToPixels(pixels);
    pixels.setImageType(OF_IMAGE_GRAYSCALE);
    blobImage2.setFromPixels(pixels);
    
    int totalArea = blobImage2.width*blobImage2.height;
    int minArea = totalArea * min;
    int maxArea = totalArea * 0.75;
    int nConsidered = 200;
    
    contourFinder2.findContours(blobImage2, minArea, maxArea, nConsidered, true);

    ofPushMatrix();
    ofTranslate(w*2+10*2, 0);
    ofFill();
    ofSetHexColor(0x333333);
    ofDrawRectangle(0, 0, w, h);
    ofSetHexColor(0xffffff);
    ofScale(scale, scale);
    
    blobNum2 = contourFinder2.nBlobs;
    
    for (int i = 0; i < blobNum2; i++) {
        //検出した物体の幅を取得する
        ofRectangle rect;
        rect = contourFinder2.blobs[i].boundingRect;
        float blobWidth = rect.getWidth();
        //ムービーに写ってる物体が画面外に見切れてる場合は探知をやり直す
        //見切れてない場合のみ輪郭抽出する
        if(blobWidth == w_original-2){
            fbo2.begin();
            ofBackground(255, 255, 255);
            fbo2.end();
        }
        
        //輪郭を描画
        contourFinder2.blobs[i].draw(0, 0);
    }
    
    ofPopMatrix();
     
}
//--------------------------------------------------------------
void ofApp::createMesh2(){
    
    for (int i = 0; i < blobNum2; i++) {
        //検出した物体の幅、高さを取得する
        ofRectangle rect;
        rect = contourFinder2.blobs[i].boundingRect;
        float blobWidth = rect.getWidth();
        float blobHeight = rect.getHeight();
        //物体の中心値を取得する
        glm::vec2 pos = contourFinder2.blobs[i].centroid;
        
        ofPolyline poly;
        for (int j = 0; j < contourFinder2.blobs[i].pts.size(); j++) {
            ofPoint ver = contourFinder2.blobs[i].pts[j];
            ver.x = ofMap(ver.x, pos.x-blobWidth/2, pos.x+blobWidth/2, 0, blobWidth);
            ver.y = ofMap(ver.y, pos.y-blobHeight/2, pos.y+blobHeight/2, 0, blobHeight);
            poly.addVertex(ver.x, ver.y);
        }
        poly.close();
        
        ofMesh mesh;
        ofTessellator tess;
        tess.tessellateToMesh(poly, ofPolyWindingMode::OF_POLY_WINDING_ODD, mesh, true);
        
        Object o;
        glm::vec2 p = glm::vec2(w*2+10*2+pos.x*scale, pos.y*scale);
        o.set(mesh, p, blobWidth, blobHeight, scale);
        objects.push_back(o);
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    // h : gui表示/非表示
    // s : スクリーンショット
    // スペースバー：新しいオブジェクトを作る
    // c : 現在のフレームから新しいオブジェクトを作る
    // delete : オブジェクトをすべて消す
    // r : 探知をやり直す
    // 1 ~ 6 : ムービーを変える
    if (key == 'h' || key == 'H') { //gui 表示 / 非表示
        bHide = !bHide;
    }
    
    if (key == 'S' || key == 's') { //スクリーンショット
            ofImage myImage;
            myImage.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
            myImage.save(ofGetTimestampString("%Y%m%d%H%M%S")+"##.png");
        }

    if (key == ' ') { //新しいオブジェクトを作る
        fbo.begin();
        ofBackground(255, 255, 255);
        fbo.end();
        
        createMesh();
    }
    
    if (key == 'c' || key == 'C') { //現在のフレームからオブジェクトを作る
        createMesh2();
    }
    
    if (key == OF_KEY_DEL || key == OF_KEY_BACKSPACE) {
        objects.clear();
    }
    
    if (key == 'r' || key == 'R') { //探知をやり直す
        fbo.begin();
        ofBackground(255, 255, 255);
        fbo.end();
    }
    
    // 1~5 ムービーを変える
    if (key == '1') {
        videos[currentVideo].stop();
        currentVideo = 0;
        videos[currentVideo].play();
    }
    
    if (key == '2') {
        videos[currentVideo].stop();
        currentVideo = 1;
        videos[currentVideo].play();
    }
    
    if (key == '3') {
        videos[currentVideo].stop();
        currentVideo = 2;
        videos[currentVideo].play();
    }
    
    if (key == '4') {
        videos[currentVideo].stop();
        currentVideo = 3;
        videos[currentVideo].play();
    }
    
    if (key == '5') {
        videos[currentVideo].stop();
        currentVideo = 4;
        videos[currentVideo].play();
    }
    
    if (key == '6') {
        videos[currentVideo].stop();
        currentVideo = 5;
        videos[currentVideo].play();
    }
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
