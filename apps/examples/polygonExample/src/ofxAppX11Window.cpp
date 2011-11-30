#include <assert.h>
#include <GLES/gl.h>
#include <GLES/egl.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <EGL/egl.h>

#include "glu.h"

#include "ofMain.h"
#include "ofxAppX11Window.h"


EGLDisplay			egldisplay;
EGLConfig			eglconfig;
EGLSurface			eglsurface;
EGLContext			eglcontext;

Display 			*display;


ofPtr<ofBaseApp> appPtr;

void render();



void ofxAppX11Window::setupOpenGL( int w, int h, int screenMode )
{
	printf("setupOpenGL\n");

	static const EGLint s_configAttribs[] =
	{
		EGL_RED_SIZE, 5,
		EGL_GREEN_SIZE, 6,
		EGL_BLUE_SIZE, 5,
		EGL_ALPHA_SIZE, 	0,
		EGL_SAMPLES, 0, 
		EGL_NONE
	};

	EGLint numconfigs;

	//Open X display
	printf("XOpenDisplay \n");
	display = XOpenDisplay(NULL);
	assert(display != NULL);	

	//Get the default screen from display
	int screen;
	printf("DefaultScreen [");
	screen = DefaultScreen(display);
	printf("%d]\n", screen);

	//get root window
	Window window, rootwindow; 
	printf("RootWindow \n");
	rootwindow = RootWindow(display,screen); 
	
	//get egldisplay from X display
	printf("eglGetDisplay (%d)\n", (int)display);
	egldisplay = eglGetDisplay ( display);
	assert(eglGetError() == EGL_SUCCESS);

	printf("Disp = %d \n",(int) egldisplay);

	//initializa egl display
	printf("eglInitialize \n");
	eglInitialize(egldisplay, NULL, NULL);
	assert(eglGetError() == EGL_SUCCESS);

	//tell the driver we are using OpenGL ES
	printf("eglBindAPI \n");
	eglBindAPI(EGL_OPENGL_ES_API);

	//pass our egl configuration to egl
	printf("eglChooseConfig \n");
	eglChooseConfig(egldisplay, s_configAttribs, &eglconfig, 1, &numconfigs);
	assert(eglGetError() == EGL_SUCCESS);

	assert(numconfigs == 1);

	//Create and x window
	printf("XCreateSimpleWindow \n");
	window = XCreateSimpleWindow(display, rootwindow, 0, 0, w, h, 0, 0, WhitePixel (display, screen));

	XMapWindow(display, window);

	//get an eglsurface from the display using our egl configuration
	printf("*eglCreateWindowSurface \n");
	eglsurface = eglCreateWindowSurface(egldisplay, eglconfig, window, NULL);
	assert(eglGetError() == EGL_SUCCESS);

	//create the egl graphics context
	eglcontext = eglCreateContext(egldisplay, eglconfig, NULL, NULL);
	printf("creatcontext, \n");
	assert(eglGetError() == EGL_SUCCESS);

	//make the context current
	eglMakeCurrent(egldisplay, eglsurface, eglsurface, eglcontext);
	printf("makecurrent, \n");
	assert(eglGetError() == EGL_SUCCESS);

	/*set perspective*/
	EGLint e_h, e_w;
	/*get width and height from egl*/
    	eglQuerySurface(egldisplay, eglsurface, EGL_WIDTH, &e_w);
    	eglQuerySurface(egldisplay, eglsurface, EGL_HEIGHT, &e_h);
	printf("got w/h %i/%i\n", e_w, e_h );



	// setup opengl bits
	/* Enable smooth shading */
    	glShadeModel(GL_SMOOTH);

    	/* Set the background black */
    	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
    	/* Depth buffer setup */
    	glClearDepthf(1.0f);
	
    	/* Enables Depth Testing */
    	glEnable(GL_DEPTH_TEST);

    	/* The Type Of Depth Test To Do */
    	glDepthFunc(GL_LEQUAL);

    	/* Really Nice Perspective Calculations */
    	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	/*change to projection matrix*/
    	glMatrixMode(GL_PROJECTION);
	/*reset the projection matrix*/
    	glLoadIdentity();
	/*set the viewport*/
    	glViewport(0, 0, e_w, e_h);

	//GLUmat4 perspective;
	/*use glu to set perspective*/
    	gluPerspective( 45.0f,((GLfloat)e_w/(GLfloat)e_h), 1.0f, 100.0f);
	
	/*get back to model view matrix*/
    	glMatrixMode(GL_MODELVIEW);
	/*reset modevl view matrix*/
    	glLoadIdentity();

		printf("about to render() from inside setup\n");
				render();

}


void ofxAppX11Window::initializeWindow()
{
	printf("ofxXllAppWindow::initializeWindow\n");

}

void render()
{
    GLfloat vertices[4][3];

    /* Clear The Screen And The Depth Buffer */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	eglSwapBuffers(egldisplay, eglsurface );	

    /* Enable VERTEX array */
    glEnableClientState(GL_VERTEX_ARRAY);

    /* Setup pointer to  VERTEX array */
    glVertexPointer(3, GL_FLOAT, 0, vertices);

    /* Move Left 1.5 Units And Into The Screen 6.0 */
    glLoadIdentity();
    glTranslatef(-1.5f, 0.0f, -6.0f);

    /* Top Of Triangle */
    vertices[0][0]=0.0f; vertices[0][1]=1.0f; vertices[0][2]=0.0f;
    /* Left Of Triangle */
    vertices[1][0]=-1.0f; vertices[1][1]=-1.0f; vertices[1][2]=0.0f;
    /* Right Of Triangle */
    vertices[2][0]=1.0f; vertices[2][1]=-1.0f; vertices[2][2]=0.0f;

    /* Drawing Using Triangles, draw triangles using 3 vertices */
    glDrawArrays(GL_TRIANGLES, 0, 3);

    /* Move Right 3 Units */
    glLoadIdentity();
    glTranslatef(1.5f, 0.0f, -6.0f);

    /* Top Right Of The Quad    */
    vertices[0][0]=1.0f;  vertices[0][1]=1.0f;  vertices[0][2]=0.0f;
    /* Top Left Of The Quad     */
    vertices[1][0]=-1.0f; vertices[1][1]=1.0f;  vertices[1][2]=0.0f;
    /* Bottom Left Of The Quad  */
    vertices[2][0]=1.0f;  vertices[2][1]=-1.0f; vertices[2][2]=0.0f;
    /* Bottom Right Of The Quad */
    vertices[3][0]=-1.0f; vertices[3][1]=-1.0f; vertices[3][2]=0.0f;

    /* Drawing using triangle strips, draw triangles using 4 vertices */
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    /* Disable vertex array */
    glDisableClientState(GL_VERTEX_ARRAY);

    /* Flush all drawings */
    glFinish();
}


//------------------------------------------------------------
void ofxAppX11Window::runAppViaInfiniteLoop(ofPtr<ofBaseApp> _appPtr){

	appPtr = _appPtr;

printf("about to ofNotifySetup and Update\n");
	ofNotifySetup();
	ofNotifyUpdate();
printf(	"about to notifyDraw/in a loop\n");
	for ( int i=0; i<100; i++ )
	{
		render();
	//	ofNotifyDraw();
	//	ofNotifyUpdate();
	}

printf("glutMainLoop() done\n");

}


