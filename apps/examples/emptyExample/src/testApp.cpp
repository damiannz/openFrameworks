#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
}

//--------------------------------------------------------------
void testApp::update(){
}

//--------------------------------------------------------------
void testApp::draw(){
/*
	ofDrawBitmapString("hello", 10, 10 );
	char buf[1024];
	sprintf( buf, "fps: %f", ofGetFrameRate() );
	ofDrawBitmapString( buf, 10, 30 );
*/	
	printf("draw(): %i\n", glGetError() );

	ofSetColor( 0x80, 0x30, 0x20 );
	
	printf("draw(1): %i\n", glGetError() );
	GLfloat vertices[] = { 5, 5, 100, 100, 30, 30 };
	glEnableClientState( GL_VERTEX_ARRAY );
	printf("draw(2): %i\n", glGetError() );
	
	glVertexPointer( 2, GL_FLOAT, 0, vertices );
	printf("draw(3): %i\n", glGetError() );
	glDrawArrays( GL_LINE_LOOP, 0, 2 );
	printf("draw(4): %i\n", glGetError() );
	glDisableClientState( GL_VERTEX_ARRAY );
	printf("draw(5): %i\n", glGetError() );

}
	

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

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

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}
