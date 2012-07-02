#include "ofxThreadedImageLoader.h"
#include "ofAppRunner.h"
#include <sstream>
ofxThreadedImageLoader::ofxThreadedImageLoader() 
:ofThread()
{
	num_loading = 0;
	ofAddListener(ofEvents().update, this, &ofxThreadedImageLoader::update);
	ofRegisterURLNotification(this);
	latencyMillis = 100;
	timeoutSeconds = 10.0f;
}

void ofxThreadedImageLoader::setMaxLatency(int millis)
{
	latencyMillis = max(0,millis);
}

ofxThreadedImageLoader* ofxThreadedImageLoader::get(){
	static ofxThreadedImageLoader* instance = new ofxThreadedImageLoader();
	return instance;
}
void ofxThreadedImageLoader::setTimeout(float seconds)
{
	timeoutSeconds = max(0.0f,seconds);
}

// Load an image from disk.
//--------------------------------------------------------------
void ofxThreadedImageLoader::loadFromDisk(ofImage* image, string filename) {
	lock();	
	
	num_loading++;
	ofImageLoaderEntry entry(image, OF_LOAD_FROM_DISK);
	entry.filename = filename;
	entry.id = num_loading;
	entry.image->setUseTexture(false);
	images_to_load.push_back(entry);
	
	unlock();
}


// Load an url asynchronously from an url.
//--------------------------------------------------------------
void ofxThreadedImageLoader::loadFromURL(ofImage* image, string url) {
	lock();
	
	num_loading++;
	ofImageLoaderEntry entry(image, OF_LOAD_FROM_URL);
	entry.url = url;
	entry.id = num_loading;
	entry.image->setUseTexture(false);	
	
	stringstream ss;
	ss << "image" << entry.id;
	entry.name = ss.str();
	images_to_load.push_back(entry);
	
	unlock();
	
}


// Reads from the queue and loads new images.
//--------------------------------------------------------------
void ofxThreadedImageLoader::threadedFunction() {
	while(isThreadRunning()) {
		lock();
		for ( int i=0; i<images_loading_from_url.size(); i++ )
		{
			ofImageLoaderEntry& entry = images_loading_from_url[i];
			if ( entry.type == OF_LOAD_FROM_URL && entry.timeoutTime < ofGetElapsedTimeMillis() )
			{
				ofRemoveURLRequest(entry.id);
				images_loading_from_url.erase( images_loading_from_url.begin()+i );
				ofLogNotice("ofxThreadedImageLoader") << "loading image from url " << entry.url << " timed out, cancelling";
				// still need to notify finished, but let's say we were cancelled
				images_to_cancel.insert(entry.image);
				images_to_update.push_back(entry);
				i--;
			}
		}
		unlock();
		
		if(shouldLoadImages()) {
			ofImageLoaderEntry entry = getNextImageToLoad();
			
			if(entry.image == NULL) {
				continue;
			}
			lock();
			// has this image been cancelled already?
			if (images_to_cancel.count(entry.image)) {
				images_to_update.push_back(entry);
				unlock();
				continue;
			}
			unlock();
		
			if(entry.type == OF_LOAD_FROM_DISK) {
				ofLogVerbose("ofxThreadedImageLoader", "loading image from disk: " + entry.filename );
				entry.image->loadImage(entry.filename);
				lock();
				images_to_update.push_back(entry);
				unlock();
			}
			else if(entry.type == OF_LOAD_FROM_URL) {
				entry.timeoutTime = ofGetElapsedTimeMillis()+timeoutSeconds*1000;
				lock();
				images_loading_from_url.push_back(entry);
				ofLogNotice("ofxThreadedImageLoader", "loading image from url: " + entry.url );
				int id = ofLoadURLAsync(entry.url, entry.name);
				images_loading_from_url.back().id = id;
				unlock();	
			}
		}
		else {
			// sleep until a new request arrives
			ofSleepMillis(latencyMillis);
		}
	}
}


// When we receive an url response this method is called; 
// The loaded image is removed from the async_queue and added to the
// update queue. The update queue is used to update the texture.
//--------------------------------------------------------------
void ofxThreadedImageLoader::urlResponse(ofHttpResponse & response) {
	if(response.status == 200) {
		lock();
		
		// Get the loaded url from the async queue and move it into the update queue.
		entry_iterator it = getEntryFromAsyncQueue(response.request.name);
		if(it != images_loading_from_url.end()) {
			if ( images_to_cancel.count((*it).image) )
			{
				images_to_cancel.erase((*it).image);
			}
			else {
				(*it).image->loadImage(response.data);
				images_to_update.push_back((*it));
			}
			images_loading_from_url.erase(it);
		}
		
		unlock();

	}
	else {
		// log error.
		stringstream ss;
		ss << "Could not image from url, response status: " << response.status;
		ofLog(OF_LOG_ERROR, ss.str());
		
		// remove the entry from the queue
		lock();
		entry_iterator it = getEntryFromAsyncQueue(response.request.name);
		if(it != images_loading_from_url.end()) {
			images_loading_from_url.erase(it);
		}
		else {
			ofLog(OF_LOG_WARNING, "Cannot find image in load-from-url queue");
		}
		unlock();
	}
}

// Find an entry in the aysnc queue.
//--------------------------------------------------------------
entry_iterator ofxThreadedImageLoader::getEntryFromAsyncQueue(string name) {
	entry_iterator it = images_loading_from_url.begin();		
	while(it != images_loading_from_url.end()) {
		if((*it).name == name) {
			return it;			
		}
	}
	return images_loading_from_url.end();
}


// Check the update queue and update the texture
//--------------------------------------------------------------
void ofxThreadedImageLoader::update(ofEventArgs & a){
	ofImageLoaderEntry entry = getNextImageToUpdate();
	if (entry.image != NULL) {
		lock();
		if ( images_to_cancel.count(entry.image) ){
			images_to_cancel.erase(entry.image);
			unlock();
			notifyImageDone( entry.image, true );
		}
		else {
			unlock();
			const ofPixels& pix = entry.image->getPixelsRef();
			entry.image->getTextureReference().allocate(
					 pix.getWidth()
					,pix.getHeight()
					,ofGetGlInternalFormat(pix)
			);
			
			entry.image->setUseTexture(true);
			entry.image->update();
			notifyImageDone( entry.image, false );
			
		}
	}
}


// Pick an entry from the queue with images for which the texture
// needs to be update.
//--------------------------------------------------------------
ofImageLoaderEntry ofxThreadedImageLoader::getNextImageToUpdate() {
	lock();
	ofImageLoaderEntry entry;
	if(images_to_update.size() > 0) {
		entry = images_to_update.front();
		images_to_update.pop_front();
	}	
	unlock();
	return entry;
}

// Pick the next image to load from disk.
//--------------------------------------------------------------
ofImageLoaderEntry ofxThreadedImageLoader::getNextImageToLoad() {
	lock();
	ofImageLoaderEntry entry;
	if(images_to_load.size() > 0) {
		entry = images_to_load.front();
		images_to_load.pop_front();
	}
	unlock();
	return entry;
}

// Check if there are still images in the queue.
//--------------------------------------------------------------
bool ofxThreadedImageLoader::shouldLoadImages() {
	lock();
	bool tf = (images_to_load.size() > 0);
	unlock();
	return tf;
}



void ofxThreadedImageLoader::start()
{
	startThread( false, false );
}

bool ofxThreadedImageLoader::cancelLoad(ofImage *image)
{
	lock();
	
	bool found = false;
	// first check the queue
	for ( int i=0; i<images_to_load.size(); i++ )
	{
		if ( images_to_load[i].image == image )
		{
			found = true;
			break;
		}
	}
	if ( !found )
	{
		// not in queue, check images loading from URL
		for ( int i=0; i<images_loading_from_url.size(); i++ )
		{
			if ( images_loading_from_url[i].image == image )
			{
				found = true;
				break;
			}
		}
	}
	if ( !found )
	{
		// not in queue or loading from URL, check images ready to be updated
		for ( int i=0; i<images_to_update.size(); i++ )
		{
			if ( images_to_update[i].image == image )
			{
				found = true;
				break;
			}
		}
	}
	// found? great!
	if ( found )
		images_to_cancel.insert( image );
	else {
		ofLogWarning("ofxThreadedImageLoader") << "couldn't cancel image* " << (unsigned long long)image << " because it was not found in any queue";
	}
	unlock();
	return found;
}

void ofxThreadedImageLoader::logStatus()
{
	deque<ofImageLoaderEntry>::const_iterator it = images_loading_from_url.begin();
	string chan =  "ofxThreadedImageLoader";
	if(it != images_loading_from_url.end()) {
		ofLogNotice( chan, "Currently loading from url\n-----------------------" );
		while(it != images_loading_from_url.end()) {
			ofLogNotice( chan, "Loading: " + (*it).url );
			++it;
		}
	}
	
	ofLogNotice( chan, "To be loaded from disk\n-----------------------" );
	it = images_to_load.begin();
	if(it != images_to_load.end()) {
		
		while(it != images_to_load.end()) {
			if ((*it).type == OF_LOAD_FROM_DISK) {
				ofLogNotice( chan, (*it).filename );
			}
			++it;
		}
	}
	
	it = images_to_load.begin();
	if(it != images_to_load.end()) {
		ofLogNotice( chan, "To be loaded from url\n-----------------------" );
		while(it != images_to_load.end()) {
			if((*it).type == OF_LOAD_FROM_URL) {
				ofLogNotice( chan, (*it).url );
			}
			++it;
		}
	}
}

void ofxThreadedImageLoader::notifyImageDone( ofImage* image, bool wasCancelled )
{
	ofxThreadedImageLoaderEventArgs args;
	args.image = image;
	args.wasCancelled = wasCancelled;
	ofNotifyEvent( imageFinishedEv, args );
}

