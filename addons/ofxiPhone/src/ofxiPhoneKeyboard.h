/*
 *  ofIphoneKeyboard.h
 *  iPhone UIKeyboard Example
 *
 *  Created by Zach Gage on 3/1/09.
 *  Copyright 2009 stfj. All rights reserved.
 *
 */

#import <UIKit/UIKit.h>
#import "ofMain.h"
#import "ofxiPhoneExtras.h"
#pragma once

@interface ofxiPhoneKeyboardDelegate : NSObject <UITextFieldDelegate>
{
	UITextField*			_textField;
	bool					open;
	char *					_ctext;
	int						_x;
	int						_y;
	int						_w;
	int						_h;
	int						_xOriginal;
	int						_yOriginal;
	int						fieldLength;
	
	/// event -- the string returned is autoreleased
	ofEvent<string>			textUpdatedEv;
}

@property (copy) NSString* fontName;

- (id) init: (int)x y:(int)y width:(int)w height:(int)h;
- (void) showText;
- (void) showTextFade:(float)fadeTime;
- (void) hideText;
- (void) hideTextFade:(float)fadeTime;
- (char *) getText;
- (const char*) getLabelText;
- (void) setText: (NSString *)text;
- (void) setFont:(NSString*)fontName;
- (void) setFontSize: (int)size;
- (void) setFontColorRed: (int)r green: (int)g blue:(int)b alpha:(int)a;
- (void) setBgColorRed: (int)r green: (int)g blue:(int)b alpha:(int)a;
- (bool) isKeyboardShowing;
- (void) setFrame: (CGRect) rect;
- (void) setPlaceholder: (NSString *)text;
- (void) openKeyboard;
- (void) closeKeyboard;
- (void) updateOrientation;
- (void) makeSecure;
- (void) setFieldLength: (int)len;
- (ofEvent<string>&) textUpdatedEvent;


@end

class ofxiPhoneKeyboard 
{
	
public:
	
	ofxiPhoneKeyboard(int _x, int _y, int _w, int _h);
	~ofxiPhoneKeyboard();
	
	void setVisible(bool visible, float fadeTime = 0 );
	
	void setPosition(int _x, int _y);
	void setSize(int _w, int _h);
	void setFontSize(int ptSize);
	void setFont(string fontName);
	void setFontColor(int r, int g, int b, int a);
	void setBgColor(int r, int g, int b, int a);
	void setText(string _text);
	void setPlaceholder(string _text);
	void openKeyboard();
	void closeKeyboard();
	void updateOrientation();
	void makeSecure();
	void setMaxChars(int max);
	
	string getText();
	string getLabelText();
	bool isKeyboardShowing();
	
	ofEvent<string>& getTextUpdatedEvent() { return [keyboard textUpdatedEvent]; }
	
protected:
	
	ofxiPhoneKeyboardDelegate *	keyboard;
	int x,y,w,h;
};