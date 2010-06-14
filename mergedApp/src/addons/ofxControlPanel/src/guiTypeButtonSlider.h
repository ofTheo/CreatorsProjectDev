#pragma once


#include "guiBaseObject.h"
#include "guiColor.h"
#include "simpleColor.h"
#include "guiValue.h"

#define INCREMENT 0.005f

class guiTypePlusButton : public guiTypeToggle
{
    public:
        //-----------------------------------------------.
        void render(){
            ofPushStyle();
                glPushMatrix();
                    guiBaseObject::renderText();

                    //draw the background
                    ofFill();
                    glColor4fv(bgColor.getColorF());
                    ofRect(hitArea.x, hitArea.y, hitArea.width, hitArea.height);

                    //draw the outline
                    ofNoFill();
                    glColor4fv(outlineColor.getColorF());
                    ofRect(hitArea.x, hitArea.y, hitArea.width, hitArea.height);

                    if( value.getValueI() == 1){
                        ofFill();
                    }else{
                        ofNoFill();
                    }

                    glColor4fv(fgColor.getColorF());
                    ofSetLineWidth(1.0f);
                    ofLine(hitArea.x+hitArea.width/2.0f, hitArea.y, hitArea.x+hitArea.width/2.0f, hitArea.y + hitArea.height -1.0f);
                    ofLine(hitArea.x +1.0f, hitArea.y+hitArea.height/2.0f, hitArea.x+hitArea.width, hitArea.y + hitArea.height/2.0f);

                glPopMatrix();
            ofPopStyle();
        }
};

class guiTypeMinusButton : public guiTypeToggle
{
    public:
        //-----------------------------------------------.
        void render(){
            ofPushStyle();
                glPushMatrix();
                    guiBaseObject::renderText();

                    //draw the background
                    ofFill();
                    glColor4fv(bgColor.getColorF());
                    ofRect(hitArea.x, hitArea.y, hitArea.width, hitArea.height);

                    //draw the outline
                    ofNoFill();
                    glColor4fv(outlineColor.getColorF());
                    ofRect(hitArea.x, hitArea.y, hitArea.width, hitArea.height);

                    if( value.getValueI() == 1){
                        ofFill();
                    }else{
                        ofNoFill();
                    }

                    glColor4fv(fgColor.getColorF());
                    ofSetLineWidth(1.0f);
                    ofLine(hitArea.x +1.0f, hitArea.y+hitArea.height/2.0f, hitArea.x+hitArea.width, hitArea.y + hitArea.height/2.0f);

                glPopMatrix();
            ofPopStyle();
        }
};

class guiTypeButtonSlider : public guiBaseObject{


     public:

        guiTypeButtonSlider(){
            slider = NULL;
            leftButton = NULL;
            rightButton = NULL;
            bgColor.setSelectedColor(255, 255, 255, 255);
            bgColor.setColor(255, 255, 255, 255);
        }

        ~guiTypeButtonSlider() {
            if(slider != NULL) {
                delete slider;
            }
            if(leftButton != NULL){
                delete leftButton;
            }
            if(rightButton != NULL){
                delete rightButton;
            }
        }

        //------------------------------------------------
        void setup(string buttonSliderName, float panelWidth, float panelHeight, float defaultVal, float min, float max, bool isInt){

            /* slider setup */
            slider = new guiTypeSlider();
            slider->setup(buttonSliderName, defaultVal, min, max);
            value.addValue(defaultVal, min, max);

            slider->setDimensions(180, 10);
            if(isInt){
                slider->setTypeInt();
            }else{
                slider->setTypeFloat();
            }
			slider->setFont(displayText.ourFont);

            /* left button setup */
            leftButton = new guiTypeMinusButton();
            leftButton->setup(buttonSliderName+string("1"), false);
            leftButton->setDimensions(10, 10);
            leftButton->setTypeBool();
            leftButton->setShowText(false);

            /* right button setup */
            rightButton = new guiTypePlusButton();
            rightButton->setup(buttonSliderName+string("2"), false);
            rightButton->setDimensions(10, 10);
            rightButton->setTypeBool();
            rightButton->setShowText(false);

            name = buttonSliderName;

            setDimensions(panelWidth, panelHeight);

        }

        //-----------------------------------------------
        virtual void saveSettings(){

        }

        //-----------------------------------------------
        virtual void saveSettings(string filename){

        }

        //-----------------------------------------------
        virtual void loadSettings(string filename){

        }

        //-----------------------------------------------
        virtual void reloadSettings(){

        }

        //-----------------------------------------------
        virtual void setValue(float _value, int whichParam) {
            slider->value.setValue(_value, whichParam);
        }

        //-----------------------------------------------
        virtual void updateValue() {

            if(parameterCallback != NULL) {
                parameterCallback->Execute(slider->value.getValueF(),-1, -1, callback_id);
            }
        }


        //-----------------------------------------------
        virtual void update(){
            updateText();
            slider->setShowText(true);

            slider->setPosition(boundingBox.x + 15, boundingBox.y );
            leftButton->setPosition(boundingBox.x,boundingBox.y + slider->displayText.getTextHeight() + leftButton->titleSpacing);
            rightButton->setPosition(boundingBox.x + boundingBox.width - rightButton->getWidth(),boundingBox.y + slider->displayText.getTextHeight() + leftButton->titleSpacing);

            slider->update();
            leftButton->update();
            rightButton->update();

            timeNow = ofGetElapsedTimef();
            if( (timeNow-timeThen) > 0.1f) {
                if((leftButton->state == SG_STATE_SELECTED)||(rightButton->state == SG_STATE_SELECTED))
                    updateGui(prevMouse.x, prevMouse.y, true, false );
                timeThen = timeNow;
            }
        }

        //-----------------------------------------------.
        void updateGui(float x, float y, bool firstHit, bool isRelative){


            if(state != SG_STATE_SELECTED)return;

            if(this->readOnly) return;

            if( isInsideRect(x, y, slider->hitArea))
            {
                slider->state = SG_STATE_SELECTED;
                slider->setSelected();
                slider->updateGui(x,y,firstHit,isRelative);
                prevMouse.set(x, y);
            }
            else
            if ((slider->state == SG_STATE_SELECTED) && (!firstHit&&isRelative))
            {
                slider->updateGui(x,y,firstHit,isRelative);
            }

            if( isInsideRect(x, y, leftButton->hitArea) ){
                if(!firstHit) return;
                leftButton->state = SG_STATE_SELECTED;
                leftButton->setSelected();
                leftButton->updateGui(x,y,firstHit,isRelative);
                slider->setSelected();

                float pct = slider->value.getPct();
                pct = pct - INCREMENT;
                slider->value.setValueAsPct( pct );
                prevMouse.set(x, y);
                timeThen = timeNow;
            }
            if( isInsideRect(x, y, rightButton->hitArea) ){
                if(!firstHit) return;
                leftButton->state = SG_STATE_SELECTED;
                rightButton->setSelected();
                rightButton->updateGui(x,y,firstHit,isRelative);
                slider->setSelected();

                float pct = slider->value.getPct();
                pct = pct + INCREMENT;
                slider->value.setValueAsPct( pct );
                prevMouse.set(x, y);
                timeThen = timeNow;
            }

            value.setValue(slider->value.getValueF()); // for xml settings saving
            if(parameterCallback != NULL) {
                parameterCallback->Execute(slider->value.getValueF(),-1, -1, callback_id);
            }

        }

        //should  be called on mouse up
        //-------------------------------------------
        virtual void release(){
            state = SG_STATE_NORMAL;
            setNormal();

            slider->state = SG_STATE_NORMAL;
            slider->setNormal();
            leftButton->state = SG_STATE_NORMAL;
            leftButton->setNormal();
            leftButtonOn = false;
            rightButton->state = SG_STATE_NORMAL;
            rightButton->setNormal();
         }

        //-----------------------------------------------.
        void render(){

            ofPushStyle();
                glPushMatrix();

                    slider->render();
                    leftButton->render();
                    rightButton->render();

                glPopMatrix();
            ofPopStyle();
        }

        guiTypeSlider*      slider;
        guiTypeMinusButton* leftButton;
        guiTypePlusButton*  rightButton;
        ofPoint             prevMouse;
        bool                leftButtonOn;

        float 	            timeNow, timeThen;

};
