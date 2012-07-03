#pragma once


#include <vector>
#include "ofPoint.h"
#include "ofRectangle.h"
#include "ofConstants.h"
#include "ofPath.h"
#include "ofTexture.h"
#include "ofMesh.h"

//--------------------------------------------------
typedef struct {
	int character;
	int height;
	int width;
	int setWidth;
	int topExtent;
	int leftExtent;
	float tW,tH;
	float x1,x2,y1,y2;
	float t1,t2,v1,v2;
} charProps;


typedef ofPath ofTTFCharacter;

//--------------------------------------------------
#define NUM_CHARACTER_TO_START		32		// 0 - 31 are control characters, no graphics needed.

class ofTrueTypeFont{

public:


	ofTrueTypeFont();
	virtual ~ofTrueTypeFont();
	
	/// set the default glpyh dpi for all typefaces.
	static void setGlobalDpi(int newDpi, bool alsoSetDisplayDpi = true );
	/// set the dpi of the display device for all typefaces. useful for retina.
	/// if the global glpyh dpi is 144 and the display dpi is 72, fonts will be rendered at half their internal scale.
	static void setGlobalDisplayDpi( int displayDpi );
	/// set the default simplify amount. initially 0.3f. 0.0f is no simplification.
	static void setGlobalSimplify( float simplifyAmt );
	
			
	/// default: non-full char set, anti aliased, not full char set, no contours, simplifyAmount set from global default, dpi set from global DPI:
	bool 		loadFont(string filename, int fontsize, bool _bAntiAliased=true, bool _bFullCharacterSet=false, bool makeContours=false, float simplifyAmt=-1, int dpi=0);
	
	bool		isLoaded();
	bool		isAntiAliased();
	bool		hasFullCharacterSet();

    int         getSize();
    float       getLineHeight();
  	void 		setLineHeight(float height); // in OpenGL/screen/display units
	float 		getLetterSpacing(); 
	void 		setLetterSpacing(float spacing); // scale factor relative to glyph width
	float 		getSpaceSize();
	void 		setSpaceSize(float size); // scale factor relative to normal space width
	float 		stringWidth(string s);
	float 		stringHeight(string s);
	
	ofRectangle    getStringBoundingBox(string s, float x, float y);
	
	void 		drawString(string s, float x, float y);
	void		drawStringAsShapes(string s, float x, float y);
	
	//			get the num chars in the loaded char set
	int			getNumCharacters();	
	
	ofTTFCharacter getCharacterAsPoints(int character);
	vector<ofTTFCharacter> getStringAsPoints(string str);

	/// begin collecting font drawing commands
	void bind( ofPoint initialOffset=ofPoint(0,0,0) );
	/// font drawing, if any, actually happens in unbind();
	void unbind();

protected:
	
	inline float	getGlyphScale();
	
	bool			bLoadedOk;
	bool 			bAntiAliased;
	bool 			bFullCharacterSet;
	int 			nCharacters;
	
	vector <ofTTFCharacter> charOutlines;

	float 			lineHeight;
	float			letterSpacing;
	float			spaceSize;
	
	vector<charProps> 	cps;			// properties for each character

	float				fontSize;
	bool			bMakeContours;
	float 			simplifyAmt;
	int 			dpi;

	void 			drawChar(int c, float x, float y);
	void			drawCharAsShape(int c, float x, float y);
	
	int				border;//, visibleBorder;
	string			filename;

	ofTexture texAtlas;
	bool binded;
	ofPoint bindInitialOffset;
	float bindedGlyphScale; // getGlyphScale() at the time we called bind()
	ofMesh stringQuads;

private:
#if defined(TARGET_ANDROID) || defined(TARGET_OF_IPHONE)
	friend void ofUnloadAllFontTextures();
	friend void ofReloadAllFontTextures();
#endif
#ifdef TARGET_OPENGLES
	GLint blend_src, blend_dst;
	GLboolean blend_enabled;
	GLboolean texture_2d_enabled;
#endif
	void		unloadTextures();
	void		reloadTextures();
};


