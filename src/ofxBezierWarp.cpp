
/*
 * ofxBezierWarp.cpp
 *
 * Copyright 2013 (c) Matthew Gingold http://gingold.com.au
 * Adapted from: http://forum.openframeworks.cc/index.php/topic,4002.0.html
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * If you're using this software for something cool consider sending 
 * me an email to let me know about your project: m@gingold.com.au
 *
 */

#include "ofxBezierWarp.h"

GLfloat texpts [2][2][2] = {
    {
        {0, 0},
        {1, 0}
    },
    {
        {0, 1},
        {1, 1}
    }
};

//--------------------------------------------------------------

ofxBezierWarp::ofxBezierWarp() {
    currentCntrlY = -1;
    currentCntrlX = -1;
    numXPoints = 0;
    numYPoints = 0;
    warpX = 0;
    warpY = 0;
    warpWidth = 0;
    warpHeight = 0;
    gridResolution = -1;
    bShowWarpGrid = false;
    bWarpPositionDiff = false;
    bDoWarp = true;


}

//--------------------------------------------------------------

ofxBezierWarp::~ofxBezierWarp() {
    //fbo.destroy();
    cntrlPoints.clear();
}

//--------------------------------------------------------------

void ofxBezierWarp::allocate(int _w, int _h, int pixelFormat) {
    allocate(_w, _h, 2, 2, 100.0f, pixelFormat);
}

//--------------------------------------------------------------

void ofxBezierWarp::allocate(int _w, int _h, int _numXPoints, int _numYPoints, float pixelsPerGridDivision, int pixelFormat) {
    
    for (int i = 0; i < 4; i++) {
        showHelperOnCorner[i] = false;
    }

    //disable arb textures (so we use texture 2d instead)

    if (_w == 0 || _h == 0 || _numXPoints == 0 || _numYPoints == 0) {
        ofLogError("Cannot accept 0 as value for w, h numXPoints or numYPoints");
        return;
    }

    if (_w != fbo.getWidth() || _h != fbo.getHeight()) {

        fbo.allocate(_w, _h, pixelFormat);
        ofLogVerbose() << "Allocating bezier fbo texture as: " << fbo.getWidth() << " x " << fbo.getHeight();
    }

    setWarpGrid(_numXPoints, _numYPoints);

    //set up texture map for bezier surface
    glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2, 0, 1, 4, 2, &texpts[0][0][0]);
    glEnable(GL_MAP2_TEXTURE_COORD_2);
    glEnable(GL_MAP2_VERTEX_3);
    glEnable(GL_AUTO_NORMAL);

    setWarpGridResolution(pixelsPerGridDivision);

    //glShadeModel(GL_FLAT);


}

//--------------------------------------------------------------

void ofxBezierWarp::begin() {
    fbo.begin();
    ofPushMatrix();
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
}

//--------------------------------------------------------------

void ofxBezierWarp::end() {
    ofPopMatrix();
    fbo.end();
}

//--------------------------------------------------------------

void ofxBezierWarp::draw() {
    draw(0, 0, fbo.getWidth(), fbo.getHeight());
}

//--------------------------------------------------------------

void ofxBezierWarp::draw(float x, float y) {
    draw(x, y, fbo.getWidth(), fbo.getHeight());
}

//--------------------------------------------------------------

void ofxBezierWarp::draw(float x, float y, float w, float h) {

    if (!fbo.isAllocated()) return;

    ofPushMatrix();

    if (bDoWarp) {

        ofTranslate(x, y);
        ofScale(w / fbo.getWidth(), h / fbo.getHeight());

        ofTexture & fboTex = fbo.getTextureReference();

        // upload the bezier control points to the map surface
        // this can be done just once (or when control points change)
        // if there is only one bezier surface - but with multiple
        // it needs to be done every frame
        glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, numXPoints, 0, 1, numXPoints * 3, numYPoints, &(cntrlPoints[0]));

        fboTex.bind();

        glMatrixMode(GL_TEXTURE);
        glPushMatrix();
        glLoadIdentity();

        glScalef(fboTex.getWidth(), fboTex.getHeight(), 1.0f);
        glMatrixMode(GL_MODELVIEW);

        //      glEnable(GL_MAP2_VERTEX_3);
        //      glEnable(GL_AUTO_NORMAL);
        glEvalMesh2(GL_FILL, 0, gridDivX, 0, gridDivY);
        //      glDisable(GL_MAP2_VERTEX_3);
        //      glDisable(GL_AUTO_NORMAL);

        fboTex.unbind();

        glMatrixMode(GL_TEXTURE);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);

    } else {

        fbo.draw(x, y, w, h);

    }

    ofPopMatrix();

    if (bShowWarpGrid) {
        if (bWarpPositionDiff) {
            drawWarpGrid(warpX, warpY, warpWidth, warpHeight);
        } else {
            setWarpGridPosition(x, y, w, h);
        }
    }

    //    glFinish();
}

//--------------------------------------------------------------

void ofxBezierWarp::drawWarpGrid(float x, float y, float w, float h) {

    ofPushStyle();
    ofPushMatrix();

    ofSetColor(255, 255, 255);
    ofTranslate(x, y);
    ofScale(w / fbo.getWidth(), h / fbo.getHeight());

    //    glEnable(GL_MAP2_VERTEX_3);
    //    glEnable(GL_AUTO_NORMAL);
    if (
            any_of(
                std::begin(showHelperOnCorner),
                std::end(showHelperOnCorner),
                [](bool ii) { return ii; }
            ) ||
            any_of(
                std::begin(showHelperOnCornerMouse),
                std::end(showHelperOnCornerMouse),
                [](bool iii) { return iii; }
            )
        ) {
        ofSetColor(150);
    } else {
        ofSetColor(255);
    }
    glEvalMesh2(GL_LINE, 0, gridDivX, 0, gridDivY);
    //    glDisable(GL_MAP2_VERTEX_3);
    //    glDisable(GL_AUTO_NORMAL);

    for (int i = 0; i < numYPoints; i++) {
        for (int j = 0; j < numXPoints; j++) {
            ofFill();
            if (bRealignPlease) {
                ofSetColor(255, 0, 0);
            } else {
                ofSetColor(100, 255, 100);
            }
            ofCircle(cntrlPoints[(i * numXPoints + j)*3 + 0], cntrlPoints[(i * numXPoints + j)*3 + 1], 5);
            ofNoFill();
        }
    }
    ofSetColor(255);

    // check if a corner-Point is touched
    if (showHelperOnCorner[0] || showHelperOnCornerMouse[0]) {
        int x = cntrlPoints[(0 + 0)*3 + 0];
        int y = cntrlPoints[(0 + 0)*3 + 1];
        ofDrawLine(x-350, y, x+350, y);
        ofDrawLine(x, y-350, x, y+350);
        ofDrawLine(x-150, y-150, x+150, y+150);
        ofDrawLine(x+150, y-150, x-150, y+150);
    }
    if (showHelperOnCorner[1] || showHelperOnCornerMouse[1]) {
        int x = cntrlPoints[(0 + numYPoints)*3 + 0];
        int y = cntrlPoints[(0 + numYPoints)*3 + 1];
        ofDrawLine(x-350, y, x+350, y);
        ofDrawLine(x, y-350, x, y+350);
        ofDrawLine(x-150, y-150, x+150, y+150);
        ofDrawLine(x+150, y-150, x-150, y+150);
    }
    if (showHelperOnCorner[2] || showHelperOnCornerMouse[2]) {
        int x = cntrlPoints[(numXPoints*numYPoints -1)*3 + 0];
        int y = cntrlPoints[(numXPoints*numYPoints -1)*3 + 1];
        ofDrawLine(x-350, y, x+350, y);
        ofDrawLine(x, y-350, x, y+350);
        ofDrawLine(x-150, y-150, x+150, y+150);
        ofDrawLine(x+150, y-150, x-150, y+150);
    }
    if (showHelperOnCorner[3] || showHelperOnCornerMouse[3]) {
        int x = cntrlPoints[(numXPoints*(numYPoints-1))*3 + 0];
        int y = cntrlPoints[(numXPoints*(numYPoints-1))*3 + 1];
        ofDrawLine(x-350, y, x+350, y);
        ofDrawLine(x, y-350, x, y+350);
        ofDrawLine(x-150, y-150, x+150, y+150);
        ofDrawLine(x+150, y-150, x-150, y+150);
    }
    
    ofPopMatrix();
    ofPopStyle();
}

//--------------------------------------------------------------

void ofxBezierWarp::setWarpGrid(int _numXPoints, int _numYPoints, bool forceReset) {

    if (_numXPoints != numXPoints || _numYPoints != numYPoints) forceReset = true;

    if (_numXPoints < 2 || _numYPoints < 2) {
        ofLogError() << "Can't have less than 2 X or Y grid points";
        return;
    }

    if (forceReset) {

        numXPoints = _numXPoints;
        numYPoints = _numYPoints;

        // calculate an even distribution of X and Y control points across fbo width and height
        cntrlPoints.resize(numXPoints * numYPoints * 3);
        for (int i = 0; i < numYPoints; i++) {
            GLfloat x, y;
            y = (fbo.getHeight() / (numYPoints - 1)) * i;
            for (int j = 0; j < numXPoints; j++) {
                x = (fbo.getWidth() / (numXPoints - 1)) * j;
                cntrlPoints[(i * numXPoints + j)*3 + 0] = x;
                cntrlPoints[(i * numXPoints + j)*3 + 1] = y;
                cntrlPoints[(i * numXPoints + j)*3 + 2] = 0;
                //cout << x << ", " << y << ", " << "0" << endl;
            }
        }
    }

    glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, numXPoints, 0, 1, numXPoints * 3, numYPoints, &(cntrlPoints[0]));
}

//--------------------------------------------------------------

void ofxBezierWarp::setWarpGridPosition(float x, float y, float w, float h) {
    warpX = x;
    warpY = y;
    warpWidth = w;
    warpHeight = h;
    bWarpPositionDiff = true;
}

//--------------------------------------------------------------

void ofxBezierWarp::setWarpGridResolution(float pixelsPerGridDivision) {
    gridResolution = pixelsPerGridDivision;
    setWarpGridResolution(ceil(fbo.getWidth() / pixelsPerGridDivision), ceil(fbo.getHeight() / pixelsPerGridDivision));
}

//--------------------------------------------------------------

float ofxBezierWarp::getWarpGridResolution() {
    assert(gridResolution > 0); // tis since if set via gridDivY/X it won't be a single number...
    return gridResolution;
}

//--------------------------------------------------------------

void ofxBezierWarp::setWarpGridResolution(int gridDivisionsX, int gridDivisionsY) {
    // NB: at the moment this sets the resolution for all mapGrid
    // objects (since I'm not calling it every frame as it is expensive)
    // so if you try to set different resolutions
    // for different instances it won't work as expected

    gridDivX = gridDivisionsX;
    gridDivY = gridDivisionsY;
    glMapGrid2f(gridDivX, 0, 1, gridDivY, 0, 1);
}

//--------------------------------------------------------------

void ofxBezierWarp::resetWarpGrid() {
    setWarpGrid(numXPoints, numYPoints, true);
}

//--------------------------------------------------------------

void ofxBezierWarp::resetWarpGridPosition() {
    bWarpPositionDiff = false;
}

//--------------------------------------------------------------

float ofxBezierWarp::getWidth() {
    return fbo.getWidth();
}

//--------------------------------------------------------------

float ofxBezierWarp::getHeight() {
    return fbo.getHeight();
}

//--------------------------------------------------------------

int ofxBezierWarp::getNumXPoints() {
    return numXPoints;
}

//--------------------------------------------------------------

int ofxBezierWarp::getNumYPoints() {
    return numYPoints;
}

//--------------------------------------------------------------

int ofxBezierWarp::getGridDivisionsX() {
    return gridDivX;
}

//--------------------------------------------------------------

int ofxBezierWarp::getGridDivisionsY() {
    return gridDivY;
}

//--------------------------------------------------------------

void ofxBezierWarp::toggleShowWarpGrid() {
    setShowWarpGrid(!getShowWarpGrid());
}

//--------------------------------------------------------------

void ofxBezierWarp::setShowWarpGrid(bool b) {
    bShowWarpGrid = b;
	bRealignPlease = false;
    if (bShowWarpGrid) {
        ofRegisterMouseEvents(this);
        ofRegisterKeyEvents(this);
    } else {
        ofUnregisterMouseEvents(this);
        ofUnregisterKeyEvents(this);
    }
}

//--------------------------------------------------------------

bool ofxBezierWarp::getShowWarpGrid() {
    return bShowWarpGrid;
}

//--------------------------------------------------------------

void ofxBezierWarp::setDoWarp(bool b) {
    bDoWarp = b;
}

//--------------------------------------------------------------

bool ofxBezierWarp::getDoWarp() {
    return bDoWarp;
}

//--------------------------------------------------------------

void ofxBezierWarp::toggleDoWarp() {
    bDoWarp = !bDoWarp;
}

//--------------------------------------------------------------

ofFbo& ofxBezierWarp::getFBO() {
    return fbo;
}

//--------------------------------------------------------------

ofTexture& ofxBezierWarp::getTextureReference() {
    return fbo.getTextureReference();
}

//--------------------------------------------------------------

void ofxBezierWarp::setControlPoints(vector<GLfloat> _cntrlPoints) {
    cntrlPoints.clear();
    cntrlPoints = _cntrlPoints;
    glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, numXPoints, 0, 1, numXPoints * 3, numYPoints, &(cntrlPoints[0]));
}

//--------------------------------------------------------------

vector<GLfloat> ofxBezierWarp::getControlPoints() {
    return cntrlPoints;
}

//--------------------------------------------------------------

void ofxBezierWarp::setOffset(ofVec2f p) {
    offset = p;
}

//--------------------------------------------------------------

ofVec2f ofxBezierWarp::getOffset() {
    return offset;
}

//--------------------------------------------------------------

ofVec2f& ofxBezierWarp::getOffsetReference() {
    return offset;
}

//--------------------------------------------------------------

vector<GLfloat>& ofxBezierWarp::getControlPointsReference() {
    return cntrlPoints;
}

//--------------------------------------------------------------

void ofxBezierWarp::keyPressed(ofKeyEventArgs & e) {
    if (e.key == OF_KEY_SHIFT && bGrabbedACorner) {
        bRealignPlease = true;
    }
}

//--------------------------------------------------------------

void ofxBezierWarp::keyReleased(ofKeyEventArgs & e) {
    bRealignPlease = false;
}


//--------------------------------------------------------------

void ofxBezierWarp::mouseMoved(ofMouseEventArgs & e) {

    float x = e.x;
    float y = e.y;

    if (bWarpPositionDiff) {
        x = (e.x - warpX) * fbo.getWidth() / warpWidth;
        y = (e.y - warpY) * fbo.getHeight() / warpHeight;
    }

    float dist = 10.0f;

        for (int i = 0; i < numYPoints; i++) {
        for (int j = 0; j < numXPoints; j++) {
            if (x - cntrlPoints[(i * numXPoints + j)*3 + 0] >= -dist && x - cntrlPoints[(i * numXPoints + j)*3 + 0] <= dist &&
                    y - cntrlPoints[(i * numXPoints + j)*3 + 1] >= -dist && y - cntrlPoints[(i * numXPoints + j)*3 + 1] <= dist) {
                currentCntrlY_2 = i;
                currentCntrlX_2 = j;
            }
        }
    }

    // check if a corner-Point is touched
    // Upper Left
    if (
            (currentCntrlY_2 == 0 && currentCntrlX_2 == 0)
            ) {
        showHelperOnCornerMouse[0] = true;
    } else {
        showHelperOnCornerMouse[0] = false;
    }
    // Upper Right
    if (
            (currentCntrlY_2 == 0 && currentCntrlX_2 == numXPoints - 1)
            ) {
        showHelperOnCornerMouse[1] = true;
    } else {
        showHelperOnCornerMouse[1] = false;
    }
    // Lower Left
    if (
            (currentCntrlY_2 == numYPoints - 1 && currentCntrlX_2 == 0)
            ) {
        showHelperOnCornerMouse[3] = true;
    } else {
        showHelperOnCornerMouse[3] = false;
    }
    // Lower Right
        if (
            (currentCntrlY_2 == numYPoints - 1 && currentCntrlX_2 == numXPoints - 1)
            ) {
            showHelperOnCornerMouse[2] = true;
    } else {
            showHelperOnCornerMouse[2] = false;
    }

}

//--------------------------------------------------------------

void ofxBezierWarp::mouseDragged(ofMouseEventArgs & e) {

    if (!bShowWarpGrid) mouseReleased(e);

    float x = e.x;
    float y = e.y;

    if (bWarpPositionDiff) {
        x = (e.x - warpX) * fbo.getWidth() / warpWidth;
        y = (e.y - warpY) * fbo.getHeight() / warpHeight;
    }

    if (currentCntrlY != -1 && currentCntrlX != -1) {
        cntrlPoints[(currentCntrlY * numXPoints + currentCntrlX)*3 + 0] = x;
        cntrlPoints[(currentCntrlY * numXPoints + currentCntrlX)*3 + 1] = y;
        glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, numXPoints, 0, 1, numXPoints * 3, numYPoints, &(cntrlPoints[0]));
    }

}

//--------------------------------------------------------------

void ofxBezierWarp::mousePressed(ofMouseEventArgs & e) {

    if (!bShowWarpGrid) mouseReleased(e);

    float x = e.x;
    float y = e.y;

    if (bWarpPositionDiff) {
        x = (e.x - warpX) * fbo.getWidth() / warpWidth;
        y = (e.y - warpY) * fbo.getHeight() / warpHeight;
    }

    float dist = 10.0f;

    for (int i = 0; i < numYPoints; i++) {
        for (int j = 0; j < numXPoints; j++) {
            if (x - cntrlPoints[(i * numXPoints + j)*3 + 0] >= -dist && x - cntrlPoints[(i * numXPoints + j)*3 + 0] <= dist &&
                    y - cntrlPoints[(i * numXPoints + j)*3 + 1] >= -dist && y - cntrlPoints[(i * numXPoints + j)*3 + 1] <= dist) {
                currentCntrlY = i;
                currentCntrlX = j;
            }
        }
    }
    // check if a corner-Point is touched
    if (
            (currentCntrlY == 0 && currentCntrlX == 0) ||
            (currentCntrlY == 0 && currentCntrlX == numXPoints - 1) ||
            (currentCntrlY == numYPoints - 1 && currentCntrlX == 0) ||
            (currentCntrlY == numYPoints - 1 && currentCntrlX == numXPoints - 1)
            ) {
        bGrabbedACorner = true;
    } else {
        bGrabbedACorner = false;
    }
}

//--------------------------------------------------------------

void ofxBezierWarp::mouseReleased(ofMouseEventArgs & e) {
    if (bRealignPlease) {
        rearrangeAllPoints();
    }
    currentCntrlY = -1;
    currentCntrlX = -1;
    bGrabbedACorner = false;
}



//--------------------------------------------------------------

void ofxBezierWarp::rearrangeAllPoints() {
    ofLogNotice("rearrange all points!");
    // outer lines vertical
    //    ofVec2f topPoint = ofVec2f(cntrlPoints[0], );
    //    ofVec2f lowestPoint  = ofVec2f(cntrlPoints[0], );

    //    
    //    cntrlPoints[(currentCntrlY*numXPoints+currentCntrlX)*3+0] = x;
    //    cntrlPoints[(currentCntrlY*numXPoints+currentCntrlX)*3+1] = y;

    //                cntrlPoints[(i*numXPoints+j)*3+0] = x;
    //                cntrlPoints[(i*numXPoints+j)*3+1] = y;
    //            
    // i = yPoints, j = xPoints           

    // left upper corner
    int luNumber = (0 * numXPoints + 0);
    //ofLog() << luNumber;
    int luX = cntrlPoints[luNumber * 3 + 0];
    int luY = cntrlPoints[luNumber * 3 + 1];

    // right upper corner    
    int ruNumber = (numXPoints * 0 + (numXPoints - 1));
    //ofLog() << ruNumber;
    int ruX = cntrlPoints[ruNumber * 3 + 0];
    int ruY = cntrlPoints[ruNumber * 3 + 1];

    // left lower corner    
    int llNumber = ((numYPoints - 1) * numXPoints + 0);
    //ofLog() << llNumber;
    int llX = cntrlPoints[llNumber * 3 + 0];
    int llY = cntrlPoints[llNumber * 3 + 1];

    // right lower corner    
    int rlNumber = ((numYPoints - 1) * numXPoints + (numXPoints - 1));
    //ofLog() << rlNumber;
    int rlX = cntrlPoints[rlNumber * 3 + 0];
    int rlY = cntrlPoints[rlNumber * 3 + 1];

    int teilLeftX = (luX - llX) / (numYPoints-1);
    int teilRightX = (ruX - rlX) / (numYPoints-1);
    int teilLeftY = (luY - llY) / (numYPoints-1);
    int teilRightY = (ruY - rlY) / (numYPoints-1);

    // rearranging the vertical lines left and right
    for (int i = 0; i < numYPoints - 1; i++) {
        cntrlPoints[(i * numXPoints + 0)*3 + 0] = luX - (i * teilLeftX);
        cntrlPoints[(i * numXPoints + 0)*3 + 1] = luY - (i * teilLeftY);
//        ofLog() << (i * numXPoints + 0)*3 + 0 << ": " << luX - (i * teilLeftX);
//        ofLog() << (i * numXPoints + 0)*3 + 1 << ": " << luY - (i * teilLeftY) << " | " << llY;
        cntrlPoints[(i * numXPoints + numXPoints - 1)*3 + 0] = ruX - (i * teilRightX);
        cntrlPoints[(i * numXPoints + numXPoints - 1)*3 + 1] = ruY - (i * teilRightY);
    }
    
    // rearranging the horizontal lines
    // (we are not efficient here, yes - but we do not actually need to be 
    // at this point, so we prefer easier to understand code...)
    for (int i = 0; i < numYPoints; i++) {
        // in this row we do:
        // left point
        int lpX = cntrlPoints[(i * numXPoints + 0)*3 + 0];
        int lpY = cntrlPoints[(i * numXPoints + 0)*3 + 1];
        // right point
        int rpX = cntrlPoints[((i * numXPoints) + numXPoints-1)*3 + 0];
        int rpY = cntrlPoints[((i * numXPoints) + numXPoints-1)*3 + 1];
        // calculate parts to add
        int teilX = (lpX - rpX) / (numXPoints-1);
        int teilY = (lpY - rpY) / (numXPoints-1);
        // rearranging the app. horizontal line
        for (int j = 0; j < numXPoints-1; j++) {
            cntrlPoints[(i * numXPoints + j)*3 + 0] = lpX - (j * teilX);
            cntrlPoints[(i * numXPoints + j)*3 + 1] = lpY - (j * teilY);
       }        
    }

}


//--------------------------------------------------------------

void ofxBezierWarp::mouseScrolled(ofMouseEventArgs & e) {

}

//--------------------------------------------------------------

void ofxBezierWarp::mouseEntered(ofMouseEventArgs & e) {

}

//--------------------------------------------------------------

void ofxBezierWarp::mouseExited(ofMouseEventArgs & e) {

}


//--------------------------------------------------------------

void ofxBezierWarp::moveCorner(ofOeCorner corner, ofOeDirection direction) {
    int oePoint;
    // getting control points
    switch (corner) {
    case OE_UPPER_LEFT:
        oePoint = (0 * numXPoints + 0);
        break;
    case OE_UPPER_RIGHT:
        oePoint = (numXPoints * 0 + (numXPoints - 1));
        break;
    case OE_LOWER_LEFT:
        oePoint = ((numYPoints - 1) * numXPoints + 0);
        break;
    case OE_LOWER_RIGHT:
        oePoint = ((numYPoints - 1) * numXPoints + (numXPoints - 1));
        break;
    }

    switch (direction) {
    case OE_DIRECTION_UP:
        cntrlPoints[oePoint * 3 + 1]--;
        break;
    case OE_DIRECTION_DOWN:
        cntrlPoints[oePoint * 3 + 1]++;
        break;
    case OE_DIRECTION_LEFT:
        cntrlPoints[oePoint * 3 + 0]--;
        break;
    case OE_DIRECTION_RIGHT:
        cntrlPoints[oePoint * 3 + 0]++;
        break;

    }

}

