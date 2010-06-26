/*

File: EnvMap.m

Abstract: EnvMap Exhibit

 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Computer, Inc. ("Apple") in consideration of your agreement to the
 following terms, and your use, installation, modification or
 redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use,
 install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software. 
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple.  Except
 as expressly stated in this notice, no other rights or licenses, express
 or implied, are granted by Apple herein, including but not limited to
 any patent rights that may be infringed by your derivative works or by
 other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

  Copyright (c) 2004-2006 Apple Computer, Inc., All rights reserved.

*/

#import "EnvMap.h"
#import "Exhibit.h"


/* The EnvMap */
@interface EnvMap : Exhibit {
	/* Textures */
	GLuint house_texture;
}




- (id) init;
- (void) initLazy;
- (void) dealloc;
- (NSString *) name;
- (NSString *) descriptionFilename;
- (void) renderFrame;
- (void) start : (float) red : (float) green : (float) blue : (float) envRatio : (float) solidRatio : (float) lightX : (float) lightY : (float) lightZ;
- (void) end;

@end

static EnvMap * myMap;

void doSomething(){
	
	// [ NSApp hide:NSApp ];
	
	
	
	myMap = nil;
	myMap = [[EnvMap alloc] init];
	if (myMap){
		[myMap initLazy];
	}
}

void doSomthingElse(){
	if (myMap){
		[myMap renderFrame];
	}
}


void startShader(float red, float green, float blue,
								 float envRatio, float solidRatio,
								 float lightX, float lightY, float lightZ){
	if (myMap){
		[myMap start: red : green : blue : envRatio : solidRatio : lightX : lightY : lightZ];
	}
}
void endShader(){	
	if (myMap){
		[myMap end];
	}
}




@implementation EnvMap

- (id) init
{
	[super init];
	
	return self;
}

- (void) initLazy
{
	[super initLazy];
	
	/* Setup GLSL */
   {
      NSBundle *bundle;
      NSString *string, *vertex_string, *fragment_string;
		NSBitmapImageRep *bitmapimagerep;
		NSRect rect;

      bundle = [NSBundle bundleForClass: [self class]];

		/* house texture */
		glGenTextures(1, &house_texture);
		string = [bundle pathForResource: @"hdr" ofType: @"jpg"];
		bitmapimagerep = LoadImage(string, 1);
		rect = NSMakeRect(0, 0, [bitmapimagerep pixelsWide], [bitmapimagerep pixelsHigh]);
		
		glBindTexture(GL_TEXTURE_2D, house_texture);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, rect.size.width, rect.size.height, 0,
						 (([bitmapimagerep hasAlpha])?(GL_RGBA):(GL_RGB)), GL_UNSIGNED_BYTE, 
						 [bitmapimagerep bitmapData]);

		/* Load vertex and fragment shader */
      vertex_string   = [bundle pathForResource: @"EnvMap" ofType: @"vert"];
      vertex_string   = [NSString stringWithContentsOfFile: vertex_string];
      fragment_string = [bundle pathForResource: @"EnvMap" ofType: @"frag"];
      fragment_string = [NSString stringWithContentsOfFile: fragment_string];
		if ([self loadVertexShader: vertex_string fragmentShader: fragment_string])
			NSLog(@"Failed to load EnvMap");
			
		/* Setup uniforms */
		glUseProgramObjectARB(program_object);
		glUniform3fARB(glGetUniformLocationARB(program_object, "LightPos"), 0.0, 314.0, -400.0);
		glUniform3fARB(glGetUniformLocationARB(program_object, "BaseColor"), 1, 1, 1);
		 glUniform1fARB(glGetUniformLocationARB(program_object, "envRatio"), 0.2);
		 glUniform1fARB(glGetUniformLocationARB(program_object, "solidRatio"), 0.2);
		glUniform1iARB(glGetUniformLocationARB(program_object, "EnvMap"), 0);

	}

}

- (void) dealloc
{
	[super dealloc];
	glDeleteTextures(1, &house_texture);
}

- (NSString *) name
{
	return @"EnvMap";
}

- (NSString *) descriptionFilename
{
	NSBundle *bundle;
	NSString *string;
	bundle = [NSBundle bundleForClass: [self class]];
   string = [bundle pathForResource: @"EnvMap" ofType: @"rtf"];
	
	return string;
}

- (void) renderFrame
{
	[super renderFrame];
	
	
	/*glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-0.3, 0.3, 0.0, 0.6, 1.0, 8.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 1.0, -5.0);
	*/
		
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glEnable( GL_DEPTH_TEST );
	
	// front- or back-facing facets can be culled
	
	//glEnable( GL_CULL_FACE );
	
	glUseProgramObjectARB(program_object);

	glBindTexture(GL_TEXTURE_2D, house_texture);
	
	teapot(16, 0.2, GL_FILL);
	
	glUseProgramObjectARB(NULL);
}


static float temp = 0;
- (void) start : (float) red : (float) green : (float) blue : (float) envRatio : (float) solidRatio : (float) lightX : (float) lightY : (float) lightZ
{
	[super renderFrame];
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable( GL_DEPTH_TEST );
	
	// front- or back-facing facets can be culled
	
	// glEnable( GL_CULL_FACE );
	
	
	glUseProgramObjectARB(program_object);
	
	glUniform3fARB(glGetUniformLocationARB(program_object, "BaseColor"), red, green, blue);
	glUniform1fARB(glGetUniformLocationARB(program_object, "envRatio"), envRatio);
	glUniform1fARB(glGetUniformLocationARB(program_object, "solidRatio"), solidRatio);
	glUniform3fARB(glGetUniformLocationARB(program_object, "LightPos"), lightX, lightY, lightZ);
	
	glBindTexture(GL_TEXTURE_2D, house_texture);
	
	temp+= 0.05f;
	
	//glUniform1iARB(glGetUniformLocationARB(program_object, "EnvMap"), temp);
	
	//glUseProgramObjectARB(NULL);
}

- (void) end
{
	
	glDisable( GL_DEPTH_TEST );
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgramObjectARB(NULL);
}

@end
