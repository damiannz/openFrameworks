/***********************************************************************
 
 Copyright (c) 2009
 Memo Akten, http://www.memo.tv
 Marek Bareza http://mrkbrz.com/
 
 Updated 2012 by Dan Wilcox <danomatika@gmail.com>
 
 references:
	- http://michael.tyson.id.au/2008/11/04/using-remoteio-audio-unit/
 
 ***********************************************************************/

#include "ofxiPhoneSoundStream.h"

#ifdef OF_SOUNDSTREAM_IPHONE

#include "ofSoundStream.h"
#include "ofMath.h"
#include "ofUtils.h"
#import "ofxiPhone.h"

// intermediate buffer for sample scaling
#define MAX_BUFFER_SIZE 4096
float scaleBuffer[MAX_BUFFER_SIZE];

#define kOutputBus	0
#define kInputBus	1

//------------------------------------------------------------------------------

// returns true on error
//
// see general error codes here:
// http://www.opensource.apple.com/source/CarbonHeaders/CarbonHeaders-18.1/MacErrors.h
//
// error string conversion from:
// http://stackoverflow.com/questions/2196869/how-do-you-convert-an-iphone-osstatus-code-to-something-useful
//
bool checkStatus(OSStatus error) {
	if(error != noErr) {
		char* str = new char[32];
		// see if it appears to be a 4-char-code
		*(UInt32 *)(str + 1) = CFSwapInt32HostToBig(error);
		if(isprint(str[1]) && isprint(str[2]) && isprint(str[3]) && isprint(str[4])) {
			str[0] = str[5] = '\'';
			str[6] = '\0';
		} else {
			// no, format it as an integer
			sprintf(str, "%d", (int) error);
		}
		ofLog(OF_LOG_ERROR, "ofxiPhoneSoundStream: OS status error code %s", str);
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------

// called when the audio system is interrupted (backgrounded, etc)
static void rioInterruptionListener(void *inClientData, UInt32 inInterruption) {
	if(inInterruption == kAudioSessionBeginInterruption)
		ofLog(OF_LOG_VERBOSE, "ofxiPhoneSoundStream: Audio session interrupted");
	else if(inInterruption == kAudioSessionEndInterruption)
		ofLog(OF_LOG_VERBOSE, "ofxiPhoneSoundStream: Audio session resumed");
}

OSStatus ofxiPhoneSoundStream::playbackCallback(void *inRefCon,
								 AudioUnitRenderActionFlags *ioActionFlags, 
								 const AudioTimeStamp *inTimeStamp, 
								 UInt32 inBusNumber, 
								 UInt32 inNumberFrames, 
								 AudioBufferList *ioData) {
	
	//ofLogNotice("ofxiPhoneSoundStream") << " playbackCallback " << (unsigned long long)soundOutputPtr;

	ofxiPhoneSoundStream* stream = (ofxiPhoneSoundStream*)inRefCon;
	
	if(stream->soundOutputPtr == NULL)
		return noErr;
	
	for(int i = 0; i < ioData->mNumberBuffers; i++) {
		
		short int *buffer = (short int *)ioData->mBuffers[i].mData;
		
		// check to see if our buffer is big enough to store the data:
		if(ioData->mBuffers[i].mDataByteSize > MAX_BUFFER_SIZE*2) {
			ofLogError("ofxiPhoneSoundStream") << "operating system gave us a buffer size of " << ioData->mBuffers[i].mDataByteSize << " but our MAX_BUFFER_SIZE is " << MAX_BUFFER_SIZE*2;
			ofLogError("ofxiPhoneSoundStream") << " but damian thinks this calculation is dodgy, as it doesn't seem to take into account sample sizes or numbers of channels";
			ofLogError("ofxiPhoneSoundStream") << " -> setting buffer to 0 and you won't hear anything";
			int len = ioData->mBuffers[i].mDataByteSize/2;
			for(int j = 0; j < len; j++) {
				buffer[j] = 0;
			}
		}
		else {
			// get floats from app
			//memset( scaleBuffer, 0, sizeof(float)*MAX_BUFFER_SIZE );
			
			int nFrames = ioData->mBuffers[i].mDataByteSize/(ioData->mBuffers[i].mNumberChannels*2);
			int nChannels = ioData->mBuffers[i].mNumberChannels;
			
			if ( stream->newBuffersNeededForOutput ){
				stream->soundOutputPtr->audioOutBuffersChanged( nFrames, nChannels, stream->sampleRate );
				stream->newBuffersNeededForOutput = false;
			}

			ofSoundBuffer& outBuffer = stream->outputBuffer;
			// resize the output buffer if necessary
			if ( outBuffer.size() != nFrames*nChannels || outBuffer.getNumChannels()!=nChannels ){
				outBuffer.setNumChannels(nChannels);
				outBuffer.resize(nChannels*nFrames);
			}
			outBuffer.set(0);
			stream->applySoundStreamOriginInfo(&outBuffer);
			stream->soundOutputPtr->audioOut(outBuffer);
			//stream->soundOutputPtr->audioOut(scaleBuffer,nFrames, nChannels, 0, stream->tickCount);
			
			// truncate to 16bit fixed point data
			int len = outBuffer.size();
			for(int j = 0; j < len; j++) {
				buffer[j] = (int) (outBuffer[j] * 32767.f);
			}
		}
	}

	stream->tickCount++;
	
    return noErr;
}

OSStatus ofxiPhoneSoundStream::recordingCallback(void *inRefCon, 
                                  AudioUnitRenderActionFlags *ioActionFlags, 
                                  const AudioTimeStamp *inTimeStamp, 
                                  UInt32 inBusNumber, 
                                  UInt32 inNumberFrames, 
								  AudioBufferList *ioData) {
	
	ofxiPhoneSoundStream* stream = (ofxiPhoneSoundStream*)inRefCon;

	// set input buffer params
	stream->inputBufferList.mBuffers[0].mDataByteSize = 2*inNumberFrames*stream->inputBufferList.mBuffers[0].mNumberChannels;
	ioData = &(stream->inputBufferList);
	
    // obtain recorded samples
	OSStatus status = AudioUnitRender(stream->audioUnit, ioActionFlags, inTimeStamp, 1, inNumberFrames, ioData);
    if(checkStatus(status)) {
		ofLog(OF_LOG_ERROR, "ofxiPhoneSoundStream: Couldn't render input audio samples");
		return status;
	}
	
	// send data to app
	if(stream->soundInputPtr != NULL) {

		for(int i = 0; i < ioData->mNumberBuffers; ++i) {
			short int *buffer = (short int *) ioData->mBuffers[i].mData;
			for(int j = 0; j < ioData->mBuffers[i].mDataByteSize/2; ++j) {
				scaleBuffer[j] = (float) buffer[j] / 32767.f;	// convert each sample into a float
			}
			
			int nFrames = ioData->mBuffers[i].mDataByteSize/(ioData->mBuffers[i].mNumberChannels*2);
			int nChannels = ioData->mBuffers[i].mNumberChannels;
			
			
			ofSoundBuffer& inBuffer = stream->inputBuffer;
			
			if ( stream->newBuffersNeededForInput ){
				stream->soundInputPtr->audioInBuffersChanged( nFrames, nChannels, stream->sampleRate );
				stream->newBuffersNeededForInput = false;
			}
			inBuffer.set( scaleBuffer, nFrames, nChannels );
			stream->applySoundStreamOriginInfo(&inBuffer);
			stream->soundInputPtr->audioIn(inBuffer);
	//		stream->soundInputPtr->audioIn(scaleBuffer,	nFrames, nChannels, 0, stream->tickCount);
		}
		
	}
	return noErr;
}

//------------------------------------------------------------------------------
ofxiPhoneSoundStream::ofxiPhoneSoundStream(){
	nInputChannels = 0;
	nOutputChannels = 0;
	newBuffersNeededForInput = true;
	newBuffersNeededForOutput = true;
	soundOutputPtr = NULL;
	soundInputPtr = NULL;
	audioUnit = NULL;
}

//------------------------------------------------------------------------------
ofxiPhoneSoundStream::~ofxiPhoneSoundStream(){
}

//------------------------------------------------------------------------------
void ofxiPhoneSoundStream::listDevices(){
}

//------------------------------------------------------------------------------
void ofxiPhoneSoundStream::setDeviceID(int _deviceID){
}

//------------------------------------------------------------------------------
void ofxiPhoneSoundStream::setInput(ofBaseSoundInput * soundInput){
	soundInputPtr = soundInput;
	newBuffersNeededForInput = true;
}

//------------------------------------------------------------------------------
void ofxiPhoneSoundStream::setOutput(ofBaseSoundOutput * soundOutput){
	soundOutputPtr = soundOutput;
	newBuffersNeededForOutput = true;
}

//------------------------------------------------------------------------------
bool ofxiPhoneSoundStream::setup(int outChannels, int inChannels, int _sampleRate, int _nFramesPerBuffer, int _nBuffers){
	
	nInputChannels = inChannels;
	nOutputChannels = outChannels;
	tickCount = 0;
	sampleRate = _sampleRate;
	nFramesPerBuffer = _nFramesPerBuffer;

	inputBuffer.setSampleRate(sampleRate);	
	outputBuffer.setSampleRate(sampleRate);

	
	// nBuffers is always 1  (see CoreAudio AudioBuffer struct)
	// this may change in the future ...
	// @TODO: FIFO to implement our own buffering
	nBuffers = 1;
	
	OSStatus status;
	
	// initialize and configure the audio session
	status = AudioSessionInitialize(NULL, NULL, rioInterruptionListener, NULL);
	if(checkStatus(status)) {
		ofLog(OF_LOG_ERROR, "ofxiPhoneSoundStream: Couldn't initialize audio session");
		return false;
	}
	status = AudioSessionSetActive(true);
	if(checkStatus(status)) {
		ofLog(OF_LOG_ERROR, "ofxiPhoneSoundStream: Couldn't set audio session active");
		return false;
	}
	
	Float32 preferredBufferSize = (float) nFramesPerBuffer/sampleRate; 
	
	
	status = AudioSessionSetProperty(kAudioSessionProperty_PreferredHardwareIOBufferDuration, sizeof(preferredBufferSize), &preferredBufferSize);
	if(checkStatus(status)) {
		ofLog(OF_LOG_ERROR, "ofxiPhoneSoundStream: Couldn't set i/o buffer duration");
	}
	
	
	// describe audio component
	AudioComponentDescription desc;
	desc.componentType = kAudioUnitType_Output; // this is for output, input, or input-output
	desc.componentSubType = kAudioUnitSubType_RemoteIO;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	
	// get component
	AudioComponent inputComponent = AudioComponentFindNext(NULL, &desc);
	
	// get audio units
	status = AudioComponentInstanceNew(inputComponent, &audioUnit);
	if(checkStatus(status)) {
		ofLog(OF_LOG_ERROR, "ofxiPhoneSoundStream: Couldn't create audio unit");
		close();
		return false;
	}
	
	// this is supposed to make the audio come out of the speaker rather
	// than the receiver, but I don't think it works when using the microphone as well.
	//	UInt32 category = kAudioSessionOverrideAudioRoute_Speaker;
	//	AudioSessionSetProperty(kAudioSessionProperty_OverrideAudioRoute, sizeof(category), &category);
	
	UInt32 category = 1;
	status = AudioSessionSetProperty(kAudioSessionOverrideAudioRoute_Speaker, sizeof(category), &category);
	if(checkStatus(status)) {
		ofLog(OF_LOG_ERROR, "ofxiPhoneSoundStream: Couldn't set ignore speaker routing");
	}
	
	// describe format
	audioFormat.mSampleRate			= (double)sampleRate;
	audioFormat.mFormatID			= kAudioFormatLinearPCM;
	audioFormat.mFormatFlags		= kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
	audioFormat.mFramesPerPacket	= 1;
	audioFormat.mBitsPerChannel		= 16;
	
	AURenderCallbackStruct callbackStruct;
	UInt32 flag = 1;
	
	if(outChannels > 0) {
	
		// enable IO for playback
		status = AudioUnitSetProperty(audioUnit, 
									  kAudioOutputUnitProperty_EnableIO, 
									  kAudioUnitScope_Output, 
									  kOutputBus,
									  &flag, 
									  sizeof(flag));
		if(checkStatus(status)) {
			ofLog(OF_LOG_ERROR, "ofxiPhoneSoundStream: Couldn't enable audio output");
			close();
			return false;
		}
		
		// set output format
		audioFormat.mChannelsPerFrame	= outChannels;
		audioFormat.mBytesPerPacket		= outChannels*2;
		audioFormat.mBytesPerFrame		= outChannels*2;
		
		status = AudioUnitSetProperty(audioUnit, 
									  kAudioUnitProperty_StreamFormat, 
									  kAudioUnitScope_Input, 
									  kOutputBus, 
									  &audioFormat, 
									  sizeof(audioFormat));
		if(checkStatus(status)) {
			ofLog(OF_LOG_ERROR, "ofxiPhoneSoundStream: Couldn't set output format");
			close();
			return false;
		}
		
		// set output callback
		callbackStruct.inputProc = playbackCallback;
		callbackStruct.inputProcRefCon = (void*)this;
		status = AudioUnitSetProperty(audioUnit, 
									  kAudioUnitProperty_SetRenderCallback, 
									  kAudioUnitScope_Global, 
									  kOutputBus,
									  &callbackStruct, 
									  sizeof(callbackStruct));
		if(checkStatus(status)) {
			ofLog(OF_LOG_ERROR, "ofxiPhoneSoundStream: Couldn't set output callback");
			close();
			return false;
		}
	}
	
	if(inChannels > 0) {
		
		// enable IO for recording
		status = AudioUnitSetProperty(audioUnit, 
									  kAudioOutputUnitProperty_EnableIO, 
									  kAudioUnitScope_Input, 
									  kInputBus,
									  &flag, 
									  sizeof(flag));
		if(checkStatus(status)) {
			ofLog(OF_LOG_ERROR, "ofxiPhoneSoundStream: Couldn't enable audio input");
			close();
			return false;
		}
		
		audioFormat.mChannelsPerFrame	= inChannels;
		audioFormat.mBytesPerPacket		= inChannels*2;
		audioFormat.mBytesPerFrame		= inChannels*2;
		
		// set input format
		status = AudioUnitSetProperty(audioUnit, 
									  kAudioUnitProperty_StreamFormat, 
									  kAudioUnitScope_Output, 
									  kInputBus, 
									  &audioFormat, 
									  sizeof(audioFormat));
		if(checkStatus(status)) {
			ofLog(OF_LOG_ERROR, "ofxiPhoneSoundStream: Couldn't enable set input format");
			close();
			return false;
		}
		
		// setup input buffer
		inputBufferList.mNumberBuffers = nBuffers;
		for(int i = 0; i < inputBufferList.mNumberBuffers; ++i) {
			inputBufferList.mBuffers[i].mData = (short int*) malloc(MAX_BUFFER_SIZE*2*inChannels);
			inputBufferList.mBuffers[i].mDataByteSize = audioFormat.mBytesPerFrame;
			inputBufferList.mBuffers[i].mNumberChannels = inChannels;
		}
		
		// set input callback
		callbackStruct.inputProc = recordingCallback;
		callbackStruct.inputProcRefCon = (void*)this;
		status = AudioUnitSetProperty(audioUnit, 
									  kAudioOutputUnitProperty_SetInputCallback, 
									  kAudioUnitScope_Global, 
									  kInputBus, 
									  &callbackStruct, 
									  sizeof(callbackStruct));
		if(checkStatus(status)) {
			ofLog(OF_LOG_ERROR, "ofxiPhoneSoundStream: Couldn't set input callback");
			close();
			return false;
		}
	}
	
	UInt32 shouldAllocateBuffer = 1;
	AudioUnitSetProperty(audioUnit, kAudioUnitProperty_ShouldAllocateBuffer,
						 kAudioUnitScope_Global, 1, &shouldAllocateBuffer,
						 sizeof(shouldAllocateBuffer));
	
	// Initialise
	status = AudioUnitInitialize(audioUnit);
	if(checkStatus(status)) {
		ofLog(OF_LOG_ERROR, "ofxiPhoneSoundStream: Couldn't initialize audio unit");
		close();
		return false;
	}
	
	return true;
}

//------------------------------------------------------------------------------
bool ofxiPhoneSoundStream::setup(ofBaseApp * app, int outChannels, int inChannels, int sampleRate, int nFramesPerBuffer, int nBuffers){
	setInput(app);
	setOutput(app);
	return setup(outChannels, inChannels, sampleRate, nFramesPerBuffer, nBuffers);
}

//------------------------------------------------------------------------------
void ofxiPhoneSoundStream::start(){
	if(audioUnit != NULL) {
		OSStatus status = AudioOutputUnitStart(audioUnit);
		checkStatus(status);
	}
}

//------------------------------------------------------------------------------
void ofxiPhoneSoundStream::stop(){
	OSStatus status = AudioOutputUnitStop(audioUnit);
	checkStatus(status);
}

//------------------------------------------------------------------------------
void ofxiPhoneSoundStream::close(){
	if(audioUnit != NULL)
		AudioUnitUninitialize(audioUnit);
	audioUnit = NULL;
	
	// clear input buffer
	for(int i = 0; i < inputBufferList.mNumberBuffers; ++i) {
		if(inputBufferList.mBuffers[i].mData != NULL)
			free(inputBufferList.mBuffers[i].mData);
	}
}

//------------------------------------------------------------------------------
long unsigned long ofxiPhoneSoundStream::getTickCount(){
	return tickCount;
}

//------------------------------------------------------------------------------
int ofxiPhoneSoundStream::getNumOutputChannels(){
	return nOutputChannels;
}

//------------------------------------------------------------------------------
int ofxiPhoneSoundStream::getNumInputChannels(){
	return nInputChannels;
}

//------------------------------------------------------------------------------
int ofxiPhoneSoundStream::getSampleRate(){
    return sampleRate;
}

//------------------------------------------------------------------------------
int ofxiPhoneSoundStream::getBufferSize(){
    return nFramesPerBuffer;
}

#endif
