#include "ofMain.h"
#include "testApp.h"
//#include "ofAppGlutWindow.h"
#include "ofxAppX11Window.h"

#include <assert.h>
#include <GLES/gl.h>
#include <GLES/egl.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <EGL/egl.h>

#include "glu.h"

#include "ofMain.h"
#include "ofxAppX11Window.h"
void ofSoundShutdown()
{
	printf("****** HACK *** remove ofSoundShutdown from main()\n");

}


//========================================================================
int main( ){

    ofxAppX11Window x11window;
	printf("calling window.setup..\n");
	x11window.setupOpenGL( 1024, 768, OF_WINDOW );
	printf("now calling inside\n");
	ofSetupOpenGL(&x11window, 1024,768, OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp( new testApp());

}
