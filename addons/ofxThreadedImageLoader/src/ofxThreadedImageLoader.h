#pragma once

#include <deque>
#include "ofThread.h"
#include "ofImage.h"
#include "ofURLFileLoader.h"
#include "ofTypes.h" 

using namespace std;

// Where to load form?
enum ofLoaderType {
	 OF_LOAD_FROM_DISK
	,OF_LOAD_FROM_URL
};


// Entry to load.
struct ofImageLoaderEntry {
	ofImageLoaderEntry() {
		image = NULL;
	}
	
	ofImageLoaderEntry(ofImage* pImage, ofLoaderType nType) {
		image = pImage;
		type = nType;
	}
	ofImage* image;
	ofLoaderType type;
	string filename;
	string url;
	string name;
	long timeoutTime;
	int id;
};

typedef deque<ofImageLoaderEntry>::iterator entry_iterator;


class ofxThreadedImageLoader : public ofThread {
public:
	ofxThreadedImageLoader();
	
	/// once the queue is empty, how long to sleep before checking for new requests. default 100ms. 
	void setMaxLatency( int millis );
	/// set the timeout on image loading, default 10s
	void setTimeout( float seconds );
	
	void loadFromDisk(ofImage* image, string file);
	void loadFromURL(ofImage* image, string url);

	void start();
	void update(ofEventArgs & a);
	virtual void threadedFunction();
	void urlResponse(ofHttpResponse & response);
	entry_iterator getEntryFromAsyncQueue(string name);
	friend ostream& operator<<(ostream& os, const ofxThreadedImageLoader& loader);

	deque<ofImageLoaderEntry> images_async_loading; // keeps track of images which are loading async
	deque<ofImageLoaderEntry> images_to_load;
	deque<ofImageLoaderEntry> images_to_update;
	
	/// print the current status to the log
	void logStatus();
	
private:
	bool shouldLoadImages();
	ofImageLoaderEntry getNextImageToLoad();
	ofImageLoaderEntry getNextImageToUpdate();
	
	int num_loading;
	int latencyMillis;
	float timeoutSeconds;

	
};
