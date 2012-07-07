//
//  ofSoundBuffer.cpp
//  openFrameworksLib
//
//  Created by Damian Stewart on 01.07.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "ofSoundBuffer.h"
#include "ofLog.h"

ofSoundBuffer::ofSoundBuffer()
: nFrames(0), nChannels(0), sampleRate(44100), isInterleaved(false)
{
}

ofSoundBuffer::~ofSoundBuffer()
{
}

void ofSoundBuffer::copyFromRawData( int16_t *_data, int _nFrames, int _nChannels, int _sampleRate, bool _isInterleaved )
{
	nFrames = _nFrames;
	nChannels = _nChannels;
	sampleRate = _sampleRate;
	isInterleaved = _isInterleaved;
	data = ofPtr<int16_t>(new int16_t[nFrames*nChannels]);
	// make a copy of the data
	memcpy( data.get(), _data, _nChannels*_nFrames*sizeof(int16_t));
}

template < class T >
void fillBufferFromRawDataNativeEndian( T* _data, int count, ofPtr<int16_t> output )
{
	unsigned long sampleMax = numeric_limits<T>::max();
	unsigned long outputMax = numeric_limits<int16_t>::max();
	int nBytesPerChannel = sizeof(T);
	// convert the data to float
	int16_t* out = output.get();
	T* in = _data;
	while( count-- )
	{
		(*out++) = outputMax*(float(*in++)/sampleMax);
	}
}

void ofSoundBuffer::copyFromRawDataNativeEndian( void* _data, int nBytesPerSample, int _nFrames, int _nChannels, int _sampleRate, bool _isInterleaved )
{
	if ( nBytesPerSample != 1 && nBytesPerSample != 4 )
	{
		ofLogError("ofSoundBuffer") << "sorry, " << nBytesPerSample << " bytes per sample not supported";
		return;
	}
	nFrames = _nFrames;
	nChannels = _nChannels;
	sampleRate = _sampleRate;
	isInterleaved = _isInterleaved;
	data = ofPtr<int16_t>(new int16_t[nFrames*nChannels]);
	
	int count = nFrames*nChannels;
	if ( nBytesPerSample == 1 )
		fillBufferFromRawDataNativeEndian( (uint8_t*)_data, count, data );
	else if ( nBytesPerSample == 4 )
		fillBufferFromRawDataNativeEndian( (uint32_t*)_data, count, data );
		
}

void ofSoundBuffer::copyTo(vector<short> &buffer)
{
	buffer.resize( getNumFrames() );
	for ( int i=0; i<buffer.size(); i++ )
	{
		buffer[i] = data.get()[i];
	}
}

float ofSoundBuffer::getDuration() {
	return float(nFrames)/sampleRate;
}
