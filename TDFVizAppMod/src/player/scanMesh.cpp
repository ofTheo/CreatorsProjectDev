extern "C" {
#include "EnvMap.h"
}

#include "ofMain.h"
#include "ofx3DUtils.h"
#include "ofxVectorMath.h"
#include "scanMesh.h"
#include "testApp.h"



//---------------------------------------------------------------------------------
ofxVec3f normalForTriangle(ofxVec3f a, ofxVec3f b, ofxVec3f c){
	ofxVec3f v1 = b - a;
	ofxVec3f v2 = c - a;
	ofxVec3f faceNormal = v1.getCrossed(v2);
	return faceNormal;
}


void scanMesh::setup(int srcWidth, int srcHeight, float sizeScaling){	
	this->srcWidth  = srcWidth;
	this->srcHeight = srcHeight;
	this->sizeScaling = sizeScaling;
	numTotal = srcWidth * srcHeight;
	nVertices		= srcWidth*srcHeight;
	nFaces			= (srcWidth-1)*(srcHeight-1)*2;
	vertices		= new ofxVec3f[numTotal];
	faces			= new face[(srcWidth)*(srcHeight)*2];
	normals			= new ofxVec3f[numTotal];
	normalsSmoothed = new ofxVec3f[numTotal];
	
	mask.assign(srcWidth*srcHeight, 0);
	depth.assign(srcWidth*srcHeight, 0.0);
	depthReal.assign(srcWidth*srcHeight, 0.0);
	
	int k = 0;
	for (int i = 0; i < srcWidth-1; i++){
		for (int j = 0; j < srcHeight-1; j++){
			
			int nw = j*srcWidth+i;
			int ne = j*srcWidth+(i+1);
			int sw = (j+1)*srcWidth+i;
			int se = (j+1)*srcWidth+(i+1);
			
			faces[ (j * srcWidth + i) * 2 + 0].v0 = nw;
			faces[ (j * srcWidth + i) * 2 + 0].v1 = sw;
			faces[ (j * srcWidth + i) * 2 + 0].v2 = ne;
			
			faces[ (j * srcWidth + i) * 2 + 1].v0 = sw;
			faces[ (j * srcWidth + i) * 2 + 1].v1 = se;
			faces[ (j * srcWidth + i) * 2 + 1].v2 = ne;
			
		}
	}
	
	panelPtr = &((testApp *)ofGetAppPtr())->panel;
	
}


//ghetto ripped from drawMesh - we use this in draw ball to do the normal calc and smoothing
//---------------------------------------------------------------------------------
void scanMesh::calcDepth(ofImage & currentFrame) {
	
	unsigned char * pixelsColorDepth = currentFrame.getPixels();
	//unsigned char * pixelsDepth = TSL.depthFrames[currentFrame].getPixels();
	//unsigned char * color = pixelsColor;
	
	int x = 0; 
	int y = 0;
	int k = 0;
	int faceOffset= panelPtr->getValueI("faceOffset");
	float scaling = alphaScaling * sizeScaling;
	for (int i = 0; i < srcWidth*srcHeight*4; i+=4){
		
		depthReal[k] = pixelsColorDepth[i+3];
		
		if( x >= srcWidth ){
			y++;
			x = 0;
		}
		
		depth[k] = depthReal[k] * scaling - faceOffset;
		
		if( depth[k] > 0 ){
			mask[k] = false;
			ofSetColor(pixelsColorDepth[i], pixelsColorDepth[i+1], pixelsColorDepth[i+2], 255);
			glVertex3f(-srcWidth/2+x, -srcHeight/2+y, depth[k]);
			
		} else {
			mask[k]  = true;
			depth[k] = 0.0;
		}
		
		x++;
		k++;
	}
	
	for(int i = 0; i < nVertices; i++){
		normals[i].set(0,0,0);
		vertices[i].x = i%srcWidth * 1;
		vertices[i].y = (int)(i/(float)srcWidth) * 1;
		vertices[i].z = depth[i];
	}
	
	for(int i = 0; i < nFaces; i++){
		
		if (!mask[faces[i].v0] && !mask[faces[i].v1] && !mask[faces[i].v2]){
			
			ofxVec3f v1 = vertices[faces[i].v0];
			ofxVec3f v2 = vertices[faces[i].v1];
			ofxVec3f v3 = vertices[faces[i].v2];
			
			faces[i].nrml = normalForTriangle(v1, v2, v3);
			faces[i].nrml.normalize();
		}
	}
	
	for(int i = 0; i < nFaces; i++){
		
		if (!mask[faces[i].v0] && !mask[faces[i].v1] && !mask[faces[i].v2]){
			normals[faces[i].v0] += faces[i].nrml;
			normals[faces[i].v1] += faces[i].nrml;
			normals[faces[i].v2] += faces[i].nrml;
		}
	}
	
	
	ofxVec3f nForward(0, 0, 1);
	
	int xx = 0; 
	int yy = 0;
	int nn = 0;
	
	float smoothAmnt = panelPtr->getValueF("normalSmooth");
	float iSmoothAmnt = 1.0-smoothAmnt;
	
	ofxVec3f smoothNormal;
	for(int i = 0; i < nVertices; i++){
		if( xx >= srcWidth ){
			xx = 0;
			yy++;
		}
		
		if (mask[i]){
			normals[i].set(0,0,0);
		}else{
			
			//normalize
			normals[i] /= 3.0;
			nn = 1;
			
			smoothNormal = normals[i];
			
			if( xx > 0 ){
				smoothNormal += normals[i-1];
				nn++;
			}
			if( xx < srcWidth-1 ){
				smoothNormal += normals[i+1];
				nn++;
			}
			if( yy > 0 ){
				smoothNormal += normals[i-srcWidth*1];
				nn++;
			}
			if( yy < srcHeight-1 ){
				smoothNormal += normals[i+srcWidth*1];
				nn++;
			}			
			if( xx > 1 ){
				smoothNormal += normals[i-2];
				nn++;
			}
			if( xx < srcWidth-2 ){
				smoothNormal += normals[i+2];
				nn++;
			}
			if( yy > 1 ){
				smoothNormal += normals[i-srcWidth*2];
				nn++;
			}
			if( yy < srcHeight-2 ){
				smoothNormal += normals[i+srcWidth*2];
				nn++;
			}
			
			smoothNormal /= (float)nn;
			
			normalsSmoothed[i] *= smoothAmnt;
			normalsSmoothed[i] += smoothNormal*iSmoothAmnt;
			
			//for even more smoothing lets write back to the normals
			normals[i] = normalsSmoothed[i];
		}
		
		xx++;
	}	
}


//---------------------------------------------------------------------------------
void scanMesh::drawMesh(ofImage & currentFrame, float dx) {
	
	unsigned char * pixelsColorDepth = currentFrame.getPixels();
	
	glEnable(GL_NORMALIZE);
	
	glPushMatrix();
	
	glTranslated(300, 400, 0);
	glScalef(2.0, 2.0, 2.0);
	glRotatef(dx, 0, 1, 0);
	glTranslatef(-srcWidth / 2, -srcHeight / 2, 0);
	glPointSize(2);
	glBegin(GL_POINTS);
	int m = 0;
	int n = 0;
	int faceOffset= panelPtr->getValueI("faceOffset");
	float scaling = alphaScaling * sizeScaling;
	for(int y = 0; y < srcHeight; y++) {
		for(int x = 0; x < srcWidth; x++) {
			depthReal[m] = pixelsColorDepth[n + 3];			
			if(depthReal[m] > faceOffset){
				mask[m] = false;
				depth[m] = depthReal[m] * scaling - faceOffset;
				ofSetColor(pixelsColorDepth[n], pixelsColorDepth[n + 1], pixelsColorDepth[n + 2], 255);
				glVertex3f(x, y, depth[m]);
			} else {
				mask[m]  = true;
				depth[m] = 0;
			}
			m++;
			n += 4;
		}
	}
	glEnd();
	glPopMatrix();
	
	for(int i = 0; i < nVertices; i++){
		normals[i].set(0,0,0);
		vertices[i].x = i%srcWidth * 1;
		vertices[i].y = (int)(i/(float)srcWidth) * 1;
		vertices[i].z = depth[i];
	}
	
	for(int i = 0; i < nFaces; i++){
		
		if (!mask[faces[i].v0] && !mask[faces[i].v1] && !mask[faces[i].v2]){
			
			ofxVec3f v1 = vertices[faces[i].v0];
			ofxVec3f v2 = vertices[faces[i].v1];
			ofxVec3f v3 = vertices[faces[i].v2];
			
			faces[i].nrml = normalForTriangle(v1, v2, v3);
			faces[i].nrml.normalize();
		}
	}
	
	for(int i = 0; i < nFaces; i++){
		
		if (!mask[faces[i].v0] && !mask[faces[i].v1] && !mask[faces[i].v2]){
			normals[faces[i].v0] += faces[i].nrml;
			normals[faces[i].v1] += faces[i].nrml;
			normals[faces[i].v2] += faces[i].nrml;
		}
	}
	
	
	ofxVec3f nForward(0, 0, 1);
	
	int xx = 0; 
	int yy = 0;
	int nn = 0;
	
	float smoothAmnt = panelPtr->getValueF("normalSmooth");
	float iSmoothAmnt = 1.0-smoothAmnt;
	
	ofxVec3f smoothNormal;
	for(int i = 0; i < nVertices; i++){
		if( xx >= srcWidth ){
			xx = 0;
			yy++;
		}
		
		if (mask[i]){
			normals[i].set(0,0,0);
		}else{
			
			//normalize
			normals[i] /= 3.0;
			nn = 1;
			
			smoothNormal = normals[i];
			
			if( xx > 0 ){
				smoothNormal += normals[i-1];
				nn++;
			}
			if( xx < srcWidth-1 ){
				smoothNormal += normals[i+1];
				nn++;
			}
			if( yy > 0 ){
				smoothNormal += normals[i-srcWidth*1];
				nn++;
			}
			if( yy < srcHeight-1 ){
				smoothNormal += normals[i+srcWidth*1];
				nn++;
			}			
			if( xx > 1 ){
				smoothNormal += normals[i-2];
				nn++;
			}
			if( xx < srcWidth-2 ){
				smoothNormal += normals[i+2];
				nn++;
			}
			if( yy > 1 ){
				smoothNormal += normals[i-srcWidth*2];
				nn++;
			}
			if( yy < srcHeight-2 ){
				smoothNormal += normals[i+srcWidth*2];
				nn++;
			}
			
			smoothNormal /= (float)nn;
			
			normalsSmoothed[i] *= smoothAmnt;
			normalsSmoothed[i] += smoothNormal*iSmoothAmnt;
			
			//for even more smoothing lets write back to the normals
			normals[i] = normalsSmoothed[i];
		}
		
		xx++;
	}
	
	ofxVec3f nrm;
	ofxVec3f pos;
	int startColor;
	
	//THIS CODE IS WHERE THE ACTUAL DRAWING HAPPENS!
	
	if( panelPtr->getValueB("doShader") ){
		startShader();
	}
	
	glPushMatrix();
	
	glTranslated(400+260, 400, 0);
	glRotatef(dx, 0,1,0);
	glScalef(3.5, 3.5, 3.5);
	
	glBegin(GL_TRIANGLES);
	
	for(int i = 0; i < nFaces; i++){
		
		if (mask[faces[i].v0] || mask[faces[i].v1] || mask[faces[i].v2]){
			continue;
		}
		
		nrm = normalsSmoothed[faces[i].v0];
		pos = vertices[faces[i].v0];
		glNormal3f(nrm.x, nrm.y, nrm.z);
		startColor = faces[i].v0*4;
		glColor3f(pixelsColorDepth[startColor]/255.0f, pixelsColorDepth[startColor+1]/255.0f, pixelsColorDepth[startColor+2]/255.0f);
		glVertex3f(pos.x-srcWidth/2, pos.y-srcHeight/2, pos.z);
		
		nrm = normalsSmoothed[faces[i].v1];
		pos = vertices[faces[i].v1];
		startColor = faces[i].v1*4;
		glColor3f(pixelsColorDepth[startColor]/255.0f, pixelsColorDepth[startColor+1]/255.0f, pixelsColorDepth[startColor+2]/255.0f);
		glNormal3f(nrm.x, nrm.y, nrm.z);
		glVertex3f(pos.x-srcWidth/2, pos.y-srcHeight/2, pos.z);
		
		nrm = normalsSmoothed[faces[i].v2];
		pos = vertices[faces[i].v2];
		startColor = faces[i].v2*4;
		glColor3f(pixelsColorDepth[startColor]/255.0f, pixelsColorDepth[startColor+1]/255.0f, pixelsColorDepth[startColor+2]/255.0f);
		glNormal3f(nrm.x, nrm.y, nrm.z);
		glVertex3f(pos.x-srcWidth/2, pos.y-srcHeight/2, pos.z);
		
	}
	
	glEnd();
	
	glPopMatrix();
	
	if( panelPtr->getValueB("doShader") ){
		endShader();
	}		
	
	
}