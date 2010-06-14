/*
 *  videoUtils.h
 *  openFrameworks
 *
 *  Created by theo on 10/10/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */


static void flipPixelsHRGB( unsigned char * inPixels, unsigned char * outPixels, float w, float h){
	int src = 0;
	int dst = 0;
	
	for(int x = 0; x < w; x++){
		for(int y = 0; y < h; y++){
			src = y * w * 3 + x *3;
			dst = y * w * 3 + (w-x-1) * 3;
			
			outPixels[dst] = inPixels[src];
			outPixels[dst+1] = inPixels[src+1];
			outPixels[dst+2] = inPixels[src+2];
		}
	}
}