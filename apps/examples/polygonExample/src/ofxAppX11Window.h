
#ifndef ofxAppX11Window_h_
#define ofxAppX11Window_h_

#include "ofAppBaseWindow.h"

class ofxAppX11Window: public ofAppBaseWindow
{
	public:
		ofxAppX11Window() {};
		virtual ~ofxAppX11Window() {};

		void setupOpenGL( int w, int h, int screenMode );
		void initializeWindow();

		void runAppViaInfiniteLoop( ofPtr<ofBaseApp> appPtr );

};



#endif


