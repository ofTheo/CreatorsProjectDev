#pragma once

#include "guiBaseObject.h"
#include "guiColor.h"
#include "simpleColor.h"
#include "guiValue.h"


class drawableStacker : public ofBaseDraws{
    public:

    void addDrawer(ofBaseDraws * drawer){
        drawers.push_back(drawer);
    }

    void setWidth(float w){
        width = w;
    }

    void setHeight(float h){
        height = h;
    }

    float getWidth(){
        return width;
    }

    float getHeight(){
        return height;
    }

    void draw(float x, float y, float w, float h){
        ofSetColor(0xFFFFFF);
        for(int i = 0; i < drawers.size(); i++){
            drawers[i]->draw(x, y, w, h);
        }
    }

    void draw(float x, float y){
        ofSetColor(0xFFFFFF);
        for(int i = 0; i < drawers.size(); i++){
            drawers[i]->draw(x, y, width, height);
        }
    }

    vector <ofBaseDraws *> drawers;
    float width;
    float height;
};

class guiTypeDrawable : public guiBaseObject{

     public:

        guiTypeDrawable(){
            vid = NULL;
            bgColor.setSelectedColor(0, 0, 0, 255);
            bgColor.setColor(0, 0, 0, 255);
        }

        //------------------------------------------------
        void setup(string videoName, ofBaseDraws * vidIn, float videoWidth, float videoHeight){
            vid = vidIn;
            name = videoName;
            updateText();

            setDimensions(videoWidth, videoHeight);

        }

        //-----------------------------------------------.
        void updateGui(float x, float y, bool firstHit, bool isRelative){

        }

        //-----------------------------------------------.
        void render(){
            ofPushStyle();

                glPushMatrix();
                //glTranslatef(boundingBox.x, boundingBox.y, 0);
                    guiBaseObject::renderText();

                    //draw the background
                    ofFill();
                    glColor4fv(bgColor.getColorF());
                    ofRect(hitArea.x, hitArea.y, hitArea.width, hitArea.height);

                    ofDisableAlphaBlending();

                    ofSetColor(0xFFFFFF);
                    vid->draw(hitArea.x, hitArea.y, hitArea.width, hitArea.height);

                glPopMatrix();

            ofPopStyle();
        }

        ofBaseDraws * vid;
};



class guiTypeVideo : public guiTypeDrawable{
	
	public:
		 //------------------------------------------------
        void setup(string videoName, ofVideoPlayer * vidIn, float videoWidth, float videoHeight){
            video		= vidIn;
			playPause	= false;
			
			guiTypeDrawable::setup(videoName, video, videoWidth, videoHeight);
			
			
		}
		
		//-----------------------------------------------.
        void updateGui(float x, float y, bool firstHit, bool isRelative){
			
			if( firstHit && state == SG_STATE_SELECTED && video != NULL ){
				if ( x >= pButtonX && x <= pButtonX + pButtonW && y >= pButtonY && y < pButtonY + pButtonH){
					
					playPause = !playPause;
									
					if( playPause )video->setPaused(true);
					else video->setPaused(false);
					
				}
			}
			
			if( state == SG_STATE_SELECTED && video != NULL ){
				if ( x >= scX && x <= scX + scW && y >= scY && y < scY + scH){
					scrubPct = ofMap(x, scX, scX + scW, 0.0, 0.99);
					video->setPosition(scrubPct);
				}
			}
			
        }
		
		//-----------------------------------------------.
        void render(){
		
						
			pButtonX = hitArea.x + 4;
			pButtonY =  hitArea.y + hitArea.height - 20;
			pButtonW = 16;
			pButtonH = 16;
			
			scX  = pButtonX + pButtonW + 6;
			scY  = pButtonY+1;
			scW  = hitArea.width - (pButtonW + 16);
			scH  = pButtonH-2;
		
            ofPushStyle();

				if( video != NULL ){
					scrubPct = video->getPosition();
				}else scrubPct = 0.0;

                glPushMatrix();
                //glTranslatef(boundingBox.x, boundingBox.y, 0);
                    guiBaseObject::renderText();

                    //draw the background
                    ofFill();
                    glColor4fv(bgColor.getColorF());
                    ofRect(hitArea.x, hitArea.y, hitArea.width, hitArea.height);

                    ofDisableAlphaBlending();

                    ofSetColor(0xFFFFFF);
                    vid->draw(hitArea.x, hitArea.y, hitArea.width, hitArea.height);
					
					ofEnableAlphaBlending();
					
					//ofSetColor(20, 90, 220, 100);
					glColor4fv( outlineColor.getColorF() );
					ofNoFill();
					ofRect(pButtonX, pButtonY, pButtonW, pButtonH);
					
					ofFill();
					
					
					glColor4fv( fgColor.getColorF() );
					if( playPause ){
						ofRect(pButtonX + 4, pButtonY + 3, 2, 9);
						ofRect(pButtonX + 9, pButtonY + 3, 2, 9);
					}else{
						float dx = pButtonX + 5;
						float dy = pButtonY + 3;
						
						ofTriangle( dx, dy, dx + 7, dy + 5, dx, dy + 10); 
					}

					glColor4fv( outlineColor.getColorF() );
					ofNoFill();
					
					//ofSetColor(20, 90, 220, 100);
					ofRect(scX, scY, scW, scH);
					
					ofFill();
					glColor4fv( fgColor.getColorF() );
					ofRect( scX + 2, scY + 2, scrubPct * (scW-4.0), scH - 4);

                glPopMatrix();

            ofPopStyle();
        }
		
	float pButtonX, pButtonY, pButtonW, pButtonH;	
	float scX, scY, scW, scH;	
	
	float scrubPct;
	bool playPause;
	ofVideoPlayer * video;
};

