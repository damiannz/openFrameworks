#pragma once

#include <deque>
#include "ofThread.h"
#include "ofImage.h"
#include "ofURLFileLoader.h"
#include "ofTypes.h" 
#include <set>

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
	int id;
};

typedef deque<ofImageLoaderEntry>::iterator entry_iterator;

typedef struct {
	ofImage* image;
	bool wasCancelled;
} ofxThreadedImageLoaderEventArgs;

class ofxThreadedImageLoader : public ofThread {
public:
	ofxThreadedImageLoader();
	static ofxThreadedImageLoader* get(); // singleton instance
	
	/// once the queue is empty, how long to sleep before checking for new requests. default 100ms. 
	void setMaxLatency( int millis );
	
	void loadFromDisk(ofImage* image, string file);
	void loadFromURL(ofImage* image, string url);
	void cancelLoad(ofImage* image);
	
	/// start running
	void start();
	
	/// print the current status to the log
	void logStatus();

	/// url callback (i wish this could be private)
	void urlResponse(ofHttpResponse & response);
	
	/// event that fires on the main thread once the image is ready, or not
	ofEvent<ofxThreadedImageLoaderEventArgs> imageFinishedEv;
private:
	virtual void threadedFunction();
	
	
	void notifyImageDone( ofImage* image, bool wasCancelled );

	void update(ofEventArgs & a);
	entry_iterator getEntryFromAsyncQueue(string name);

	bool shouldLoadImages();
	ofImageLoaderEntry getNextImageToLoad();
	ofImageLoaderEntry getNextImageToUpdate();
	
	int num_loading;
	int latencyMillis;


	deque<ofImageLoaderEntry> images_loading_from_url; // keeps track of images which are loading async
	deque<ofImageLoaderEntry> images_to_load;
	deque<ofImageLoaderEntry> images_to_update;
	set<ofImage*> images_to_cancel;
	
	
};
