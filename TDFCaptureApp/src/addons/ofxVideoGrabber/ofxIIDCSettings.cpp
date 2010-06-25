#include "ofxIIDCSettings.h"
#include "ofxVideoGrabberSDK.h"


inline void spaceReplace(std::string & myStr)
{
        /* replace all spaces in the name to satisfy XML*/
        size_t found;
        found=myStr.find_first_of(" ");
        while (found!=string::npos)
        {
            //myStr.erase(found,1);
            myStr[found]='_';

            found=myStr.find_first_of(" ",found+1);
        }
}

ofxIIDCSettings::ofxIIDCSettings()
{

}

ofxIIDCSettings::~ofxIIDCSettings()
{
	ofRemoveListener(panel.guiEvent, this, &ofxIIDCSettings::eventsIn);
}



void ofxIIDCSettings::setupGUI()
{
    videoGrabber->getAllFeatureValues();

    panel.setup("ofxIIDCSettings", 0, 0, 480, ofGetHeight()/2);
//	panel.loadFont("myFont.ttf", 10);
    panel.addPanel("Settings", 2, false);
	panel.setWhichPanel("Settings");
    panel.setWhichColumn(0);

    /* set the gui callback */
    //callback.SetCallback( this, &ofxIIDCSettings::parameterCallback);

	int tmp_index = -1;
	int tmp_featureID = -1;
	bool whitebalance = false;
	int featureID = -1;

    /* setup feature gui sliders */
    for (int i=0; i < videoGrabber->availableFeatureAmount; ++i)
    {
        if(videoGrabber->featureVals[i].isPresent) {
            featureID = videoGrabber->featureVals[i].feature;

            if( featureID == FEATURE_WHITE_BALANCE) {
                tmp_index = i;
                tmp_featureID = featureID;
                whitebalance = true;
            }
            else if(featureID == FEATURE_TRIGGER) {
                //not using any features above this
                break;
            }
            else {
                string name = videoGrabber->featureVals[i].name;
                string xmlName = name;
                transform(xmlName.begin(), xmlName.end(), xmlName.begin(), ::toupper);

                float minVal = videoGrabber->featureVals[i].minVal;
                float maxVal = videoGrabber->featureVals[i].maxVal;
                float currVal = videoGrabber->featureVals[i].currVal;
                //panel.addButtonSlider(name, xmlName, currVal, minVal, maxVal, true, &callback, featureID);
				panel.addButtonSlider(name, xmlName, currVal, minVal, maxVal, true);
            }
        }
    }

    /* setup white balance 2d slider */
    if(whitebalance)
    {
        string name = videoGrabber->featureVals[tmp_index].name;
        string xmlName = name;
        transform(xmlName.begin(), xmlName.end(), xmlName.begin(), ::toupper);
        spaceReplace(xmlName);


        float minVal = videoGrabber->featureVals[tmp_index].minVal;
        float maxVal = videoGrabber->featureVals[tmp_index].maxVal;
        float currVal = videoGrabber->featureVals[tmp_index].currVal;
        float currVal2 = videoGrabber->featureVals[tmp_index].currVal2;
        //panel.addSlider2D(name,xmlName, currVal, currVal2, minVal, maxVal, minVal, maxVal, false, &callback,tmp_featureID);
        panel.addSlider2D(name,xmlName, currVal, currVal2, minVal, maxVal, minVal, maxVal, false);
    }

    panel.setWhichColumn(1);

    /* setup features modes */
    for (int i=0; i < videoGrabber->availableFeatureAmount; ++i)
    {
        if(videoGrabber->featureVals[i].isPresent) {
            featureID = videoGrabber->featureVals[i].feature;

            if (featureID == FEATURE_TRIGGER)
            {
                //not using any features above this
                break;
            }
            if( featureID != FEATURE_WHITE_BALANCE)
            {
                string name = videoGrabber->featureVals[i].name + " mode";
                string xmlName = name;

                transform(xmlName.begin(), xmlName.end(), xmlName.begin(), ::toupper);
                spaceReplace(xmlName);

                int defaultval = FEATURE_MODE_MANUAL;

//                if(videoGrabber->featureVals[i].hasAutoModeActive) {
//                    defaultval = FEATURE_MODE_AUTO;
//					videoGrabber->setFeatureMode(FEATURE_MODE_AUTO, featureID );
//                } else {
					videoGrabber->setFeatureMode(FEATURE_MODE_MANUAL, featureID );
//				}
                vector <string> modelist;
                modelist.push_back("MANUAL");


                if(videoGrabber->featureVals[i].hasAutoMode)
                {
                    modelist.push_back("AUTO");
                    if(videoGrabber->featureVals[i].hasOnePush )
                    {
                        modelist.push_back("ONE PUSH AUTO");
                    }
                }

                //panel.addTextDropDown(name,xmlName, defaultval, modelist,&callback,featureID);
                panel.addTextDropDown(name,xmlName, defaultval, modelist);

            }
        }
    }

    /* setup whitebalance modes */
    if(whitebalance)
    {
                featureID = FEATURE_WHITE_BALANCE;
                string name = videoGrabber->featureVals[tmp_index].name + " mode";
                string xmlName = name;
                transform(xmlName.begin(), xmlName.end(), xmlName.begin(), ::toupper);
                spaceReplace(xmlName);

                int defaultval = FEATURE_MODE_MANUAL;
//                if(videoGrabber->featureVals[tmp_index].hasAutoModeActive) {
//                    defaultval = FEATURE_MODE_AUTO;
//					videoGrabber->setFeatureMode(FEATURE_MODE_AUTO, featureID );
//                } else {
					videoGrabber->setFeatureMode(FEATURE_MODE_MANUAL, featureID );
//				}
                vector <string> modelist;
                modelist.push_back("MANUAL");

                if(videoGrabber->featureVals[tmp_index].hasAutoMode)
                {
                    modelist.push_back("AUTO");
                    if(videoGrabber->featureVals[tmp_index].hasOnePush )
                    {
                        modelist.push_back("ONE PUSH AUTO");
                    }
                }
                //panel.addTextDropDown(name,xmlName, defaultval, modelist,&callback,tmp_featureID);
                panel.addTextDropDown(name,xmlName, defaultval, modelist);

    }
	
	panel.setupEvents();
	panel.enableEvents();
	ofAddListener(panel.guiEvent, this, &ofxIIDCSettings::eventsIn);

	if(panel.currentXmlFile != ""){
		panel.loadSettings(panel.currentXmlFile);
	}
	
    //Example of how to set up toggles
    /*
    string name = "myToggle";
    string xmlName = "MY_TOGGLE";
    bool currVal = false;
    panel.addToggle(name,xmlName,currVal,&callback,666);

    name = "myMultiToggle";
    xmlName = "MY_MULTI_TOGGLE";
    currVal = 2;
    vector<string> boxnames;
    boxnames.push_back("box1");
    boxnames.push_back("box2");
    boxnames.push_back("box3");
    panel.addMultiToggle(name,xmlName,currVal,boxnames, &callback,667);
    */
}

//--------------------------------------------------------------------
void ofxIIDCSettings::draw()
{
    panel.draw();
}

//--------------------------------------------------------------------
void ofxIIDCSettings::update()
{
    panel.update();
}

//--------------------------------------------------------------------
void ofxIIDCSettings::eventsIn(guiCallbackData & data)
{
	//printf("data.groupname is %s\n", data.groupName.c_str());	

	//if this is a feature mode setting
	string guiName = data.getXmlName();
	if( guiName.find("_MODE") != string::npos ){
		
		int param_id = titleToCameraFeature( guiName.substr(0, guiName.size()-5) );
		if( param_id != NULL_FEATURE ){

			//printf("mode is %s\n", cameraFeatureToTitle( mode ).c_str() );
			//printf("value[%i] is %s \n",  data.getInt(0), data.getString(0).c_str() );		
			videoGrabber->setFeatureMode(data.getInt(0), param_id);
		}
		
	}else{
		//its a value setting
		int param_id = titleToCameraFeature(data.getXmlName());
		if( param_id != NULL_FEATURE ){
			if( param_id == FEATURE_WHITE_BALANCE ){
				videoGrabber->setFeatureValue(data.getFloat(0), data.getFloat(1), param_id);
			}else{
				videoGrabber->setFeatureValue(data.getInt(0), param_id);
			}
		}
	}
	
	//old code 

//    if(param_mode != NULL_MODE)
//    {
//        //cout << "set mode called: "  << param_mode << " param_id = " << param_id << endl;
//        videoGrabber->setFeatureMode(param_mode, param_id);
//    }
//    else {
//        if(param_id == FEATURE_WHITE_BALANCE) {
//            videoGrabber->setFeatureValue(param1, param2, param_id);
//        }
//        else {
//            //cout << "set feature called: " << param1 << " " << param_id << endl;
//            videoGrabber->setFeatureValue(param1, param_id);
//        }
//    }

}





