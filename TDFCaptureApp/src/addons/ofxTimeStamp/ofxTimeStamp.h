#pragma once

#include "ofMain.h"
#include <time.h>

class ofxTimeStamp{
    public:

    ofxTimeStamp(){
        year        = 0;
        month       = 0;
        day         = 0;
        hour        = 0;
        minute      = 0;
        second      = 0;
        weekday     = 0;
        unixTime    = 0;
		setTimestampToCurrentTime();
    }

    //Setters

    //--------------------------------------
    //--------------------------------------

    //------------------------------------------------
    void setTimestampToCurrentTime(){
        time_t rawtime;
        time ( &rawtime );
        setTimestamp( (int)rawtime );
    }

    //------------------------------------------------
    bool setTimestamp(string ts, string delimilator){

        vector <string> elements;

        //first we check for this format 20091231235959
        if(delimilator == "" && ts.length() == 14 ){
            elements.assign(6, "");

            elements[0] = ts.substr( 0, 4 );
            elements[1] = ts.substr( 4, 2 );
            elements[2] = ts.substr( 6, 2 );
            elements[3] = ts.substr( 8, 2 );
            elements[4] = ts.substr( 10, 2 );
            elements[5] = ts.substr( 12, 2 );

        //now we check for this format 2009-12-31-23-59-59
        }else if( delimilator != "" ){
            vector <string> results;
            StringExplode(ts, delimilator, &results);

            if(results.size() == 6){
                elements = results;
            }
        }

        if( elements.size() == 6 ){
            year    = atoi(elements[0].c_str());
            month   = atoi(elements[1].c_str());
            day     = atoi(elements[2].c_str());
            hour    = atoi(elements[3].c_str());
            minute  = atoi(elements[4].c_str());
            second  = atoi(elements[5].c_str());

            return  updateTime();
        }

        ofLog(OF_LOG_ERROR, "setTimestamp - timestamp was not formatted correctly!");
        return false;
    }

    bool setTimestamp(int _year, int _month, int _day, int _hour, int _minute, int _second){
        year    = _year;
        month   = _month;
        day     = _day;
        hour    = _hour;
        minute  = _minute;
        second  = _second;

        return updateTime();
    }

    bool setTimestamp(int _unixtime){
        if( unixTime < 0 )return false;

        unixTime = _unixtime;
        tm * timeinfo = localtime(&unixTime);

        year    = timeinfo->tm_year + 1900;
        month   = timeinfo->tm_mon;
        day     = timeinfo->tm_mday;
        weekday = timeinfo->tm_wday;
        hour    = timeinfo->tm_hour;
        minute  = timeinfo->tm_min;
        second  = timeinfo->tm_sec;

        return true;
    }

    //-----------------------------
    string getTimestampAsString(string seper = ""){
        return ofToString( year ) +seper+ toLZString(month,2) + seper+ toLZString(day,2) + seper + toLZString(hour,2) + seper + toLZString(minute,2) + seper + toLZString(second,2);
    }

    //Getters

    //--------------------------------------
    //--------------------------------------
    int getUnixTime(){
        return unixTime;
    }

    string getUnixTimeAsString(){
      return ofToString((int)unixTime);
    }

    int getYear(){
        return year;
    }

    int getMonth(){
        return month;
    }

    int getDay(){
        return day;
    }

    int getWeekday(){
        return weekday;
    }

    int getHours(){
        return hour;
    }

    int getMinutes(){
        return minute;
    }

    int getSeconds(){
        return second;
    }

    protected:

        int year, month, day, hour, minute, second;
        int weekday;
        time_t unixTime;
		
		string toLZString(int numIn, int numUnits){
			string result = ofToString(numIn);
			if( result.length() < numUnits ){
				int numToAdd = numUnits-result.length();
				for(int k = 0; k < numToAdd; k++){
					result = "0" + result;
				}
			}			
			return result;
		}

        //-----------------------------
        bool updateTime(){
            tm timeinfo;

            timeinfo.tm_year    = year - 1900;
            timeinfo.tm_mon     = month;
            timeinfo.tm_mday    = day;
            timeinfo.tm_hour    = hour;
            timeinfo.tm_min     = minute;
            timeinfo.tm_sec     = second;

            timeinfo.tm_isdst   = false; //eek?

            unixTime = mktime(&timeinfo);

            if( unixTime != -1){
                tm * timeinfo2  = localtime(&unixTime);
                weekday         = timeinfo2->tm_wday;
            }else{
                ofLog(OF_LOG_ERROR, "mktime error!");
                return false;
            }

            return true;
        }

        //from http://www.infernodevelopment.com/perfect-c-string-explode-split
        static void StringExplode(string str, string separator, vector<string>* results){
            int found;
            found = str.find_first_of(separator);
            while(found != string::npos){
                if(found > 0){
                    results->push_back(str.substr(0,found));
                }
                str = str.substr(found+separator.length());
                found = str.find_first_of(separator);
            }
            if(str.length() > 0){
                results->push_back(str);
            }
        }
};
