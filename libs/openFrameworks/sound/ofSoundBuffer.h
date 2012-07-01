//
//  ofSoundBuffer.h
//  openFrameworksLib
//
//  Created by Damian Stewart on 01.07.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#pragma once

#import "ofTypes.h"

class ofSoundBuffer
{
public:
	ofSoundBuffer();
	~ofSoundBuffer();
	
	// make a copy of data
	void copyFromRawData( uint16_t* data, int nFrames, int nChannels, int sampleRate, bool isInterleaved );
	void copyFromRawDataNativeEndian( void* data, int nBytesPerSample, int nFrames, int nChannels, int sampleRate, bool isInterleaved );
	
	void copyTo( vector<short>& buffer );
	
	float getDuration();
	
	unsigned long getNumFrames() { return nFrames; }
	int getNumChannels() { return nChannels; }
	int getSampleRate() { return sampleRate; }
	
private:
	
	ofPtr<uint16_t> data;
	int nFrames;
	int nChannels;
	int sampleRate;
	bool isInterleaved;
	
};


