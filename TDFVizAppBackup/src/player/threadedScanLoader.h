/*
 *  threadMovieLoader.h
 *  emptyExample
 *
 *  Created by itotaka on 1/13/10.
 *  Copyright 2010 YCAM. All rights reserved.
 *
 */

#ifndef _THREAD_MOVIE_LOADER
#define _THREAD_MOVIE_LOADER

#include "ofMain.h"
#include "ofxThread.h"
#include "ofxDirList.h"

enum state_TH{	
	TH_STATE_LOADING, TH_STATE_JUST_LOADED, TH_STATE_LOADED, TH_STATE_UNLOADED
};

class threadedScanLoader : public ofxThread {
	
	public:
	
		threadedScanLoader();
		~threadedScanLoader();
	
		void	setup(int maxFrames, int resizeToW, int resizeToH);
		
		bool    start(string & _folderName);
		void    stop();
		void    threadedFunction();
		void    loadScans(string pathName);
		void	unload ();

		int				nFrames;
		int				state;
		int				resizeW;
		int				resizeH;
		string			folderName;
	
	
//		ofImage *	depthImageFrames;
				
		vector <ofImage> depthImageFrames;
		
//		vector <ofImage> * currentFrame
		
		//ofImage *	imageFrames;
		bool	*	bLoaded;
		int			maxNumFrames;	// we should do this at some point
		
		int			totalNumFrames;
	
	protected:
	
	
};

#endif





