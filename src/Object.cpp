#include "Object.h"

//--------------------------------------------------------------
Object::Object(){
    //オブジェクトの色
    colors.push_back(ofColor(248, 134, 168)); //pink
    colors.push_back(ofColor(254, 141, 111)); //orange
    colors.push_back(ofColor(253, 196, 83)); //yellow
    colors.push_back(ofColor(252, 223, 61)); //yellow
    colors.push_back(ofColor(160, 221, 224)); //light blue
    colors.push_back(ofColor(154, 219, 197)); //medium aquamarine
}

//--------------------------------------------------------------
void Object::set(ofMesh _mesh, glm::vec2 _pos, float _width, float _height, float _scale){
    mesh = _mesh;
    int cr = (int)ofRandom(colors.size());
    col = colors[cr];
    malpha = 255;
    alpha = 255;
    pos = _pos;
    pos_copy = pos;
    width = _width;
    height = _height;
    scale = _scale;
    objectScale = 1;
    objectScale2 = ofRandom(1, 6); //オブジェクトの最終的な大きさ（〜倍にする）
    float r = ofRandom(1);
    if(r < 0.5){ //オブジェクトが地面についたあとの横移動の速さ オブジェクトの大きさに比例する　大きい：ゆっくり　小さい：速い
        addX = -1 * ofMap(objectScale2, 1, 6, 5, 1);
    } else {
        addX = ofMap(objectScale2, 1, 6, 5, 1);
    }
    
    //easing
    initTime = ofGetElapsedTimef(); //アプリケーションを開始してからの経過時間を取得
    endPosition = ofGetHeight() - height*scale*objectScale2/2;
}
//--------------------------------------------------------------
void Object::update(){
    //オブジェクトのy位置を小数点第２で四捨五入する(round)
    //四捨五入しないと、数値が1049.9999みたいになってしまう
    //ofGetHeight()の数値が1050の場合、次の処理に進まなくなる
    int py = round(pos.y + height*scale*objectScale/2);

    //オブジェクトを落下する
    if (py < ofGetHeight()){
        auto duration = 0.4f; //早い 0.1f ~ 1.0f ゆっくり
        auto endTime = initTime + duration;
        auto now = ofGetElapsedTimef();
        //イージング関数早見表　https://easings.net/ja
        pos.y = ofxeasing::map_clamp(now, initTime, endTime, pos_copy.y, endPosition, &ofxeasing::cubic::easeIn);
        //pos.y = addY;
        objectScale = ofxeasing::map_clamp(now, initTime, endTime, 1, objectScale2, &ofxeasing::cubic::easeIn);
        
    //オブジェクトが地面についたら横移動する
    } else if (py >= ofGetHeight()){
        if(pos.x == pos_copy.x){
            initTime2 = ofGetElapsedTimef();
        }
        pos.x += addX;
        if (addX<0 && pos.x < 0-width*scale*objectScale/2) {
            pos.x = ofGetWidth()+width*scale*objectScale/2;
        }
        if (addX>0 && pos.x > ofGetWidth()+width*scale*objectScale/2) {
            pos.x = 0-width*scale*objectScale/2;
        }
        
        //色をどんどん薄くする
        if(malpha > 0){
            auto duration = 70.0f;
            //auto duration = 5.0f;
            auto endTime = initTime2 + duration;
            auto now = ofGetElapsedTimef();
            malpha = 255 - ofxeasing::map_clamp(now, initTime2, endTime, 0, 255, &ofxeasing::quint::easeIn);
        
        } else {
            if(alpha == 255){
                initTime3 = ofGetElapsedTimef();
            }
            auto duration2 = 10.0f;
            auto endTime2 = initTime3 + duration2;
            auto now2 = ofGetElapsedTimef();
            alpha = 255 - ofxeasing::map_clamp(now2, initTime3, endTime2, 1, 255, &ofxeasing::linear  ::easeIn);
        }
    }
     
}
//--------------------------------------------------------------
void Object::draw(){
    //posの位置を描画
    //ofSetColor(255, 0, 0);
    //ofDrawEllipse(pos.x, pos.y, 10, 10);
    
    //オブジェクトを描画
    ofFill();
    
    //OF_BLENDMODE_MULTIPYでアルファ値を減らしていくとオブジェクトが白くなる
    //アルファ値が０になるとオブジェクトを消すプログラムにしているけど、白いオブジェクトが急に消えるようになってしまう
    //OF_BLENDMODE_MULTIPYでのアルファ値が０になったら、普通のブレンドモード（OF_BLENDMODE_ALPHA）で白いオブジェクトを変えてアルファ値を減らしていって自然に消えるようにする
    if(malpha > 0){
        ofEnableBlendMode(OF_BLENDMODE_MULTIPLY); //乗算
    } else {
        ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    }
    ofPushMatrix();
    ofTranslate(pos.x, pos.y);
    ofScale(scale, scale);
    ofScale(objectScale, objectScale);
    ofTranslate(-width/2, -height/2);
    if(malpha > 0){
        ofSetColor(col.r, col.g, col.b, malpha);
    } else {
        ofSetColor(255, 255, 255, alpha);
    }
    mesh.draw();
    ofPopMatrix();
    
    ofDisableBlendMode();
    ofSetColor(255);
}
