
#include "scanPlayer.h"
extern "C" {
#include "EnvMap.h"
}
#include "ofxVec3f.h"

#include "testApp.h"
ofxControlPanel * panelPtr;

	int srcWidth  = 160;
	int srcHeight = 120;
	
	int numTotal = srcWidth * srcHeight;

//---------------------------------------------------------------------------------
void scanPlayer::setup(){
	maxNumFrames = 15 * 20; // 15 secs 2
	imageData.allocate(srcWidth,srcHeight, GL_RGB);
	currentFrame = 0;
	totalNumFrames = 0;
	
	TSL.setup(maxNumFrames, srcWidth, srcHeight);
	
	FBO.allocate(1024, 768, GL_RGBA, 4);
	
	ofSetFrameRate(30);
	
	// setup the shader: 
	doSomething();
	endShader();
	
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
	
	//G H E T T O :) 
	panelPtr = &((testApp *)ofGetAppPtr())->panel;
}

//---------------------------------------------------------------------------------
void scanPlayer::update(){
	
	if (TSL.state == TH_STATE_JUST_LOADED) {
		currentFrame	= 0;
		totalNumFrames	= TSL.totalNumFrames;
		TSL.state = TH_STATE_LOADED;			// make sure, but maybe not needed.
	}
	if (TSL.state != TH_STATE_LOADED){
		totalNumFrames = 0;
	} else {
		totalNumFrames = TSL.totalNumFrames;
	}
	
	if (totalNumFrames > 0){
		currentFrame++;
		
		// this is because some scans sequences were missing chunks of frames (ie output2) 
		// so I just make sure that we really do have something on this frame. 
		
		//while (TSL.bLoaded[currentFrame] == false){
		//	currentFrame++;
		//	currentFrame %= totalNumFrames;
		//}
		currentFrame %= totalNumFrames;
	}
	
	if( TSL.state != TH_STATE_LOADING ){
		histogram.setPixels(&depthReal[0], srcWidth, srcHeight);
		histogramAfter.setPixels(&depth[0], srcWidth, srcHeight);
	}
	
}

//---------------------------------------------------------------------------------
void scanPlayer::draw(){
	

	//if (TSL.state == TH_STATE_LOADED){
		if (totalNumFrames > 0){
			
			FBO.clear();
			FBO.begin();
				ofSetColor(50, 50, 50);
				ofRect(0, 0, 1024, 768);
		
				glEnable(GL_DEPTH_TEST);
				ofSetColor(255, 255, 255);
				
				
				if( panelPtr->getValueB("showFaceOnBall") ){
					drawBall();
				}else {
					drawMesh();
				}

					
			
				glDisable(GL_DEPTH_TEST);
			
			FBO.end();
		}
	//}

	ofSetColor(255, 255, 255);
	FBO.draw(0, 0, 1024, 768);
	FBO.draw(1024, 0, 1024, 768);
		
}

//---------------------------------------------------------------------------------
void scanPlayer::loadDirectory(string pathName){
	TSL.start(pathName);
};

//---------------------------------------------------------------------------------
ofxVec3f normalForTriangle(ofxVec3f a, ofxVec3f b, ofxVec3f c){
	ofxVec3f v1 = b - a;
	ofxVec3f v2 = c - a;
	ofxVec3f faceNormal = v1.getCrossed(v2);
	return faceNormal;
}

//ghetto ripped from drawMesh - we use this in draw ball to do the normal calc and smoothing
//---------------------------------------------------------------------------------
void scanPlayer::calcDepth() {
	
	unsigned char * pixelsColorDepth = TSL.depthImageFrames[currentFrame].getPixels();
	//unsigned char * pixelsDepth = TSL.depthFrames[currentFrame].getPixels();
	//unsigned char * color = pixelsColor;
		
	int x = 0; 
	int y = 0;
	int k = 0;
	for (int i = 0; i < srcWidth*srcHeight*4; i+=4){
		
		depthReal[k] = pixelsColorDepth[i+3];
		
		if( x >= srcWidth ){
			y++;
			x = 0;
		}
		
		depth[k] = ofMap(pixelsColorDepth[i+3], panelPtr->getValueF("minZ"),  panelPtr->getValueF("maxZ"), 1.0, 255.0, true);
		depth[k] -= panelPtr->getValueI("minZCutoff");
		
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
	
	float scaleZAmnt = (1.0/255.0) * panelPtr->getValueI("topZ");
	
	for(int i = 0; i < nVertices; i++){
		normals[i].set(0,0,0);
		vertices[i].x = i%srcWidth * 1;
		vertices[i].y = (int)(i/(float)srcWidth) * 1;
		vertices[i].z = depth[i]*scaleZAmnt;
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
	
}

void scanPlayer::drawBall(){
	
	//we use this to smooth our normals
	calcDepth();

	if( panelPtr->getValueB("doShader") ){
		startShader();
	}
	
	glEnable(GL_NORMALIZE);
	
	glPushMatrix();
	glTranslatef(ofGetWidth()/2, ofGetHeight()/2, -100);
	glRotatef(dx, 0, 1, 0);
	
	glScalef(200, 200, 200);
	//ofRotate(ofGetElapsedTimef()*100, 1,1,0.2);
	//ofRotate(180, 0, 1, 0);
	//ofRotate(90, 1, 0, 0);
	
	float theta,nextTheta, phi, x, y, z;
	int sphereResolution = 240;
	
	float scale = 190.0;
	float relief = 1.0;

	
	float depthAmnt = 0.0;
	
	float bPX = 0.35;
	float tPX = 0.65;

	float bPY = 0.4;
	float tPY = 0.6;	
	
	ofxVec3f normal;
	ofxVec3f smoothedN;
	
	float zFaceScale = panelPtr->getValueF("zPercent") * 0.008;
	
	for(int i=0; i<sphereResolution; i++){
		theta = (float)i/(sphereResolution-1) * TWO_PI;
		nextTheta = (float)((i+1)%sphereResolution)/(sphereResolution-1) * TWO_PI;
		glBegin(GL_QUAD_STRIP);
		for(int j=0; j<sphereResolution; j++){
			
			phi		= (float)j/(sphereResolution-1) * PI;
						
			//relief	= 1.0 + amnt * ofSignedNoise(phi*scaleX, theta*scaleY, ofGetElapsedTimef() * speed);

			float pctJ = ofMap(j, 0, sphereResolution, 0.0, 1.0, true);
			float pctI = ofMap(i, 0, sphereResolution, 1.0, 0.0, true);

			if( pctJ >= bPX && pctJ < tPX && pctI >= bPY && pctI < tPY ){
				
				int depthIX = ofMap(pctJ, bPX, tPX, (float)srcWidth * 0.3, (float)srcWidth * 0.7, true);
				int depthIY = ofMap(pctI, bPY, tPY, 0, srcHeight, true);
			
				depthAmnt = depth[depthIX + srcWidth*depthIY];
				smoothedN = normalsSmoothed[depthIX + srcWidth*depthIY];
				
				relief =  1.0 + depthAmnt * zFaceScale;
			}else {
				relief = 1.0;
			}
				
			x		= cos(theta) * sin(phi);
			y		= sin(theta) * sin(phi);
			z		= cos(phi);
			
			float noise = ofNoise(x/3.0,y/3.0,z/3.0,ofGetElapsedTimef()*0.2);
			noise = 2*noise - 1;
			noise = fabs(noise) ;
			//noise = 0.0;
			noise = powf(noise, 1);
			
			//TODO: note - we use the sphere's normal if we are in the mask
			// if we are in the face we use the face's normal - but it doens't match the angle of the face on the ball
			//so there is some funky reversing going on - I think its right????
			if( relief == 1.0 ){
				normal.set(x + x*noise*0.5, y + y*noise*0.5, z + z*noise*0.5);
				normal.normalize();
			}else{
				normal.set(-smoothedN.z, smoothedN.y, -smoothedN.x);
			}

			x *= relief;
			y *= relief;
			z *= relief;
			
			glNormal3fv(normal.v);
			glVertex3f( (x + x*noise*0.5), (y + y*noise*0.5), (z + z*noise*0.5));
			

			pctI = ofMap(i+1, 0, sphereResolution, 1.0, 0.0, true);
			
			if( pctJ >= bPX && pctJ < tPX && pctI >= bPY && pctI < tPY ){
				
				int depthIX = ofMap(pctJ, bPX, tPX, (float)srcWidth * 0.3, (float)srcWidth * 0.7, true);
				int depthIY = ofMap(pctI, bPY, tPY, 0, srcHeight, true);
				
				depthAmnt = depth[depthIX + srcWidth*depthIY];
				smoothedN = normalsSmoothed[depthIX + srcWidth*depthIY];

				relief =  1.0 + depthAmnt * zFaceScale;
			}else {
				relief = 1.0;
			}			
			
			x = cos(nextTheta) * sin(phi);
			y = sin(nextTheta) * sin(phi);
			z = cos(phi);
			
			noise = ofNoise(x/3.0,y/3.0,z/3.0,ofGetElapsedTimef()*0.2);
			noise = 2*noise - 1;
			noise = fabs(noise) ;
			noise = powf(noise, 1);

			//TODO: note - we use the sphere's normal if we are in the mask
			// if we are in the face we use the face's normal - but it doens't match the angle of the face on the ball
			//so there is some funky reversing going on - I think its right????			
			if( relief == 1.0 ){
				normal.set(x + x*noise*0.5, y + y*noise*0.5, z + z*noise*0.5);
				normal.normalize();
			}else{
				normal.set(-smoothedN.z, smoothedN.y, -smoothedN.x);
			}		
			
			x *= relief;
			y *= relief;
			z *= relief;
			
			//noise = powf(noise, 1);
			
			glNormal3fv(normal.v);
			glVertex3f( (x + x*noise*0.5), (y + y*noise*0.5), (z + z*noise*0.5));
		}
		glEnd();
	}	
	
	glPopMatrix();

	if( panelPtr->getValueB("doShader") ){
		endShader();
	}	
}

//---------------------------------------------------------------------------------
void scanPlayer::drawMesh() {
	
	unsigned char * pixelsColorDepth = TSL.depthImageFrames[currentFrame].getPixels();
	//unsigned char * pixelsDepth = TSL.depthFrames[currentFrame].getPixels();
	//unsigned char * color = pixelsColor;
	
	glEnable(GL_NORMALIZE);

	glPushMatrix();
	
	glTranslated(300, 400, 0);
	glScalef(2.0, 2.0, 2.0);
	glRotatef(dx,0,1,0);
		
		glPointSize(2);
		glBegin(GL_POINTS);
		
			int x = 0; 
			int y = 0;
			int k = 0;
			for (int i = 0; i < srcWidth*srcHeight*4; i+=4){
				
				depthReal[k] = pixelsColorDepth[i+3];
				
				if( x >= srcWidth ){
					y++;
					x = 0;
				}

				depth[k] = ofMap(pixelsColorDepth[i+3], panelPtr->getValueF("minZ"),  panelPtr->getValueF("maxZ"), 1.0, 255.0, true);
				depth[k] -= panelPtr->getValueI("minZCutoff");
				
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

		glEnd();
	
	glPopMatrix();
		
	float scaleZAmnt = (1.0/255.0) * panelPtr->getValueI("topZ");
	
	for(int i = 0; i < nVertices; i++){
		normals[i].set(0,0,0);
		vertices[i].x = i%srcWidth * 1;
		vertices[i].y = (int)(i/(float)srcWidth) * 1;
		vertices[i].z = depth[i]*scaleZAmnt;
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