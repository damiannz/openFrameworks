
/*
 
adapted from Finch, under the following license terms:

Copyright (c) 2009, 2010 Tomáš Znamenáček

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the “Software”), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 
 */

#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import <CoreFoundation/CoreFoundation.h>
#include "ofSampleReaderApple.h"

#include "ofMain.h"


ofSoundBuffer ofSampleReaderApple::readSample( string path )
{

	path = ofToDataPath(path);
    AudioFileID fileId = 0;
	OSStatus errcode = noErr;
	ofSoundBuffer buffer;
	
	// convert path to pathCFURLRef
	CFURLRef pathCFURLRef = CFURLCreateFromFileSystemRepresentation( NULL, (const unsigned char*)path.c_str(), path.length(), false );
	// attempt to open the file
	errcode = AudioFileOpenURL( pathCFURLRef, kAudioFileReadPermission, 0, &fileId );
	// free path ref
	CFRelease( pathCFURLRef );
	if (errcode){
		ofLogError("ofSampleReaderApple") << "error " << errcode << " opening '" << path << "'";
		return buffer;
	}
	
	// fetch stream description
    AudioStreamBasicDescription fileFormat;
    unsigned long propertySize = sizeof(fileFormat);
    errcode = AudioFileGetProperty(fileId, kAudioFilePropertyDataFormat, &propertySize, &fileFormat);
    if (errcode) {
		ofLogError("ofSampleReaderApple") << "error " << errcode << " getting stream description from '" << path << "'";
        AudioFileClose(fileId);
        return buffer;
    }
	
	bool isNativeEndian = TestAudioFormatNativeEndian(fileFormat);
	if ( !isNativeEndian ){
		ofLogError("ofSampleReaderApple") << "sorry, audio with non-native endianness is not supported reading from '" << path << "'";
		AudioFileClose(fileId);
		return buffer;
	}
    if (fileFormat.mFormatID != kAudioFormatLinearPCM) {
		ofLogWarning("ofSampleReaderApple") << "format is not linear PCM, may not work reading from '" << path << "'";
    }
	
    UInt64 fileSize = 0;
    propertySize = sizeof(fileSize);
    errcode = AudioFileGetProperty(fileId, kAudioFilePropertyAudioDataByteCount, &propertySize, &fileSize);
    if (errcode) {
		ofLogError("ofSampleReaderApple") << "error " << errcode << " getting data byte count from '" << path << "'";
        AudioFileClose(fileId);
        return buffer;
    }
	
    double sampleLength = -1;
    propertySize = sizeof(sampleLength);
    errcode = AudioFileGetProperty(fileId, kAudioFilePropertyEstimatedDuration, &propertySize, &sampleLength);
    if (errcode) {
		ofLogError("ofSampleReaderApple") << "error " << errcode << " getting estimated audio duration from '" << path << "'";
        AudioFileClose(fileId);
        return buffer;
    }
	
    UInt32 dataSize = (UInt32) fileSize;
    void *data = malloc(dataSize);
    if (!data) {
		ofLogError("ofSampleReaderApple") << "out of memory loading '" << path << "'";
        AudioFileClose(fileId);
        return buffer;
    }
	
    errcode = AudioFileReadBytes(fileId, false, 0, &dataSize, data);
    if (errcode) {
		ofLogError("ofSampleReaderApple") << "error " << errcode << " reading bytes from '" << path << "'";
        AudioFileClose(fileId);
        free(data);
        return buffer;
    }
	
    AudioFileClose(fileId);
	
	unsigned long nFrames = dataSize/fileFormat.mBytesPerFrame;
	int nChannelsPerFrame = fileFormat.mChannelsPerFrame;
	int sampleRate = fileFormat.mSampleRate;
	bool isInterleaved = true;

	int nBytesPerChannel = fileFormat.mBitsPerChannel/8;
	if ( nBytesPerChannel == 2 )
		buffer.copyFromRawData( (uint16_t*)data, nFrames, nChannelsPerFrame, sampleRate, isInterleaved ); 
	else
		buffer.copyFromRawDataNativeEndian( (void*)data, nBytesPerChannel, nFrames, nChannelsPerFrame, sampleRate, isInterleaved );
/*	
	if ( fileFormat.mBytesPerFrame == 1 ){
		ofLogNotice("ofSampleReaderApple") << "1 byte per sample, assuming uint8_t";
		buffer.copyFromRawDataNativeEndian( (uint8_t*)data, nFrames, nChannelsPerFrame, sampleRate, isInterleaved );
	}
	else if ( fileFormat.mBytesPerFrame == 2 ){
		ofLogNotice("ofSampleReaderApple") << "1 byte per sample, assuming uint16_t";
		buffer.copyFromRawDataNativeEndian( (uint16_t*)data, nFrames, nChannelsPerFrame, sampleRate, isInterleaved );
	}
	else if ( fileFormat.mBytesPerFrame == 3 ) {
		ofLogError("ofSampleReaderApple") << "sorry, 24 bit reading not supported";
	}
	else if ( fileFormat.mBytesPerFrame == 4 ) {
		ofLogNotice("ofSampleReaderApple") << "4 bytes per sample, assuming float";
		buffer.copyFromRawData( (float*)data, nFrames, nChannelsPerFrame, sampleRate, isInterleaved );
	}*/
	
	free(data);
	
    /*
	
	FISample *sample = [[FISample alloc] init];
    [sample setNumberOfChannels:fileFormat.mChannelsPerFrame];
    [sample setHasNativeEndianity:TestAudioFormatNativeEndian(fileFormat)];
    [sample setBitsPerChannel:fileFormat.mBitsPerChannel];
    [sample setSampleRate:fileFormat.mSampleRate];
    [sample setDuration:sampleLength];
    [sample setData:[NSData dataWithBytesNoCopy:data length:dataSize freeWhenDone:YES]];
	
    return sample;
	 */
	
	return buffer;
}

