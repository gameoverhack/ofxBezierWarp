//
//  ofxBezierWarp.h
//
//  Created by game over on 7/03/13.
//  Copyright (c) 2013 trace media. All rights reserved.
//

#ifndef _H_OFXBEZIERWARP
#define _H_OFXBEZIERWARP

#include "ofFbo.h"
#include "ofGraphics.h"
#include "ofEvents.h"

class ofxBezierWarp {
    
public:
	
    ofxBezierWarp();
    ~ofxBezierWarp();
    
    void allocate(int w, int h);
    void allocate(int w, int h, int numXPoints, int numYPoints, float pixelsPerGridDivision);
    
    void begin();
    void end();
    
    void draw();
    void draw(float x, float y);
    void draw(float x, float y, float w, float h);
    
    void setWarpGrid(int numXPoints, int numYPoints, bool forceReset = false);
    void setWarpGridPosition(float x, float y, float w, float h);
    
    void setWarpGridResolution(float pixelsPerGridDivision);
    void setWarpGridResolution(int gridDivisionsX, int gridDivisionsY);
    
    void resetWarpGrid();
    void resetWarpGridPosition();
    
    float getWidth();
    float getHeight();
    
    int getNumXPoints();
    int getNumYPoints();
    
    int getGridDivisionsX();
    int getGridDivisionsY();
    
    void toggleShowWarpGrid();
    void setShowWarpGrid(bool b);
    bool getShowWarpGrid();
    
    ofFbo& getFBO();
    
    ofTexture& getTextureReference();
    
    void setControlPoints(vector<GLfloat> controlPoints);
    
    vector<GLfloat> getControlPoints();
    vector<GLfloat>& getControlPointsReference();
    
    void mouseMoved(ofMouseEventArgs & e);
    void mouseDragged(ofMouseEventArgs & e);
    void mousePressed(ofMouseEventArgs & e);
    void mouseReleased(ofMouseEventArgs & e);
    
protected:
	
    void drawWarpGrid(float x, float y, float w, float h);
    
    bool bShowWarpGrid;
    bool bWarpPositionDiff;
    
    ofFbo fbo;
    
    float width;
    float height;
    
    float warpWidth;
    float warpHeight;
    float warpX;
    float warpY;
    
    int currentCntrlX;
    int currentCntrlY;
    
    int numXPoints;
    int numYPoints;
    
    int gridDivX;
    int gridDivY;
    
    vector<GLfloat> cntrlPoints;
    
private:
	
};

#endif
