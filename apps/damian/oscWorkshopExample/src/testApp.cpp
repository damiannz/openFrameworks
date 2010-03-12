#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	
	circleX = 0;
	circleY = 0;
	
	// listen on LISTEN_PORT
	static const int LISTEN_PORT = 33333;
	cout << "listening for osc messages on port " << LISTEN_PORT << "\n";
	receiver.setup( LISTEN_PORT );
	radius = 0;

	
	ofBackground( 30, 30, 130 );
	ofSetFrameRate( 60.0f );
	ofSetVerticalSync( true );
	
	// send to SEND_PORT
	static const int SEND_PORT = 33334;
	sender.setup( "localhost", SEND_PORT );
	
}

//--------------------------------------------------------------
void testApp::update(){
	
	// decrease the radius
	radius -= 4.0f*ofGetLastFrameTime();
	if ( radius < 0 )
		radius = 0;

	// check for waiting messages
	while( receiver.hasWaitingMessages() )
	{
		// get the next message
		ofxOscMessage m;
		receiver.getNextMessage( &m );

		printf("received osc: address is %s\n", m.getAddress().c_str() );
		
		// check for mouse moved message
		if ( m.getAddress() == "/triggered" )
		{
			radius = 1.0f;
		}
	}
}


//--------------------------------------------------------------
void testApp::draw(){

	ofSetColor( 255,0,0 );
	ofCircle( circleX, circleY, radius*radius*radius*200 );

}


//--------------------------------------------------------------
void testApp::keyPressed  (int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	circleX = x;
	circleY = y;
	
	ofxOscMessage m;
	m.setAddress( "/mouse" );
	m.addIntArg( x );
	m.addIntArg( y );
	sender.sendMessage( m );
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

