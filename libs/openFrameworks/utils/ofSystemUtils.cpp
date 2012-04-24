
#include "ofConstants.h"
#include "ofSystemUtils.h"
#include "ofFileUtils.h"
#include "ofLog.h"

#ifdef TARGET_WIN32
#include <winuser.h>
#include <commdlg.h>
#define _WIN32_DCOM

#include <windows.h>
#include <shlobj.h>
#include <tchar.h>
#include <stdio.h>

#endif

#ifdef TARGET_OSX
	// ofSystemUtils.cpp is configured to build as
	// objective-c++ so as able to use NSAutoreleasePool.
	// This is done with this compiler flag
	//		-x objective-c++
	// http://www.yakyak.org/viewtopic.php?p=1475838&sid=1e9dcb5c9fd652a6695ac00c5e957822#p1475838

	#include <Carbon/Carbon.h>
	#include <sys/param.h> // for MAXPATHLEN
	#include <Cocoa/Cocoa.h>  // for NSAutoreleasePool
#endif

#ifdef TARGET_WIN32
#include <locale>
#include <sstream>
#include <string>

std::string convertWideToNarrow( const wchar_t *s, char dfault = '?',
                      const std::locale& loc = std::locale() )
{
  std::ostringstream stm;

  while( *s != L'\0' ) {
    stm << std::use_facet< std::ctype<wchar_t> >( loc ).narrow( *s++, dfault );
  }
  return stm.str();
}
#endif

#if defined( TARGET_LINUX ) && defined (OF_USING_GTK)
#include <gtk/gtk.h>
static gboolean closeGTK(GtkWidget *widget){
	//gtk_widget_destroy(widget);
    gtk_main_quit();
    return (FALSE);
}
static void initGTK(){
	int argc=0; char **argv = NULL;
	gtk_init (&argc, &argv);

}
static void startGTK(GtkWidget *dialog){
	gtk_init_add( (GSourceFunc) closeGTK, NULL );
	gtk_quit_add_destroy(1,GTK_OBJECT(dialog));
	//g_timeout_add(10, (GSourceFunc) destroyWidgetGTK, (gpointer) dialog);
	gtk_main();
}

static string gtkFileDialog(GtkFileChooserAction action,string windowTitle,string defaultName=""){
	initGTK();
	string results;

	const gchar* button_name = "";
	switch(action){
	case GTK_FILE_CHOOSER_ACTION_OPEN:
		button_name = GTK_STOCK_OPEN;
		break;
	case GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER:
		button_name = GTK_STOCK_SELECT_ALL;
		break;
	case GTK_FILE_CHOOSER_ACTION_SAVE:
		button_name = GTK_STOCK_SAVE;
		break;
	default:
		return "";
		break;
	}

	GtkWidget *dialog = gtk_file_chooser_dialog_new (windowTitle.c_str(),
						  NULL,
						  action,
						  button_name, GTK_RESPONSE_ACCEPT,
						  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						  NULL);

	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog),defaultName.c_str());

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		results = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	}
	startGTK(dialog);
	return results;
}

#endif
#ifdef TARGET_ANDROID
#include "ofxAndroidUtils.h"
#endif

//------------------------------------------------------------------------------
ofFileDialogResult::ofFileDialogResult(){
	filePath = "";
	fileName = "";
	bSuccess = false;
}

//------------------------------------------------------------------------------
string ofFileDialogResult::getName(){
	return fileName;
}

//------------------------------------------------------------------------------
string ofFileDialogResult::getPath(){
	return filePath;
}


//------------------------------------------------------------------------------
void ofSystemAlertDialog(string errorMessage){


	#ifdef TARGET_WIN32
		// we need to convert error message to a wide char message.
		// first, figure out the length and allocate a wchar_t at that length + 1 (the +1 is for a terminating character)
		int length = strlen(errorMessage.c_str());
		wchar_t * widearray = new wchar_t[length+1];
		memset(widearray, 0, sizeof(wchar_t)*(length+1));
		// then, call mbstowcs:
		// http://www.cplusplus.com/reference/clibrary/cstdlib/mbstowcs/
		mbstowcs(widearray, errorMessage.c_str(), length);
		// launch the alert:
		MessageBoxW(NULL, widearray, L"alert", MB_OK);
		// clear the allocated memory:
		delete widearray;
	#endif


	#ifdef TARGET_OSX
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];  // The StandardAlert requires a NSAutoreleasePool to avoid memory leaks

		CFStringRef msgStrRef = CFStringCreateWithCString(NULL, errorMessage.c_str(), kCFStringEncodingASCII);
		DialogRef theItem;
		DialogItemIndex itemIndex;
		CreateStandardAlert(kAlertNoteAlert, msgStrRef, NULL, NULL, &theItem);
		RunStandardAlert(theItem, NULL, &itemIndex);

		[pool drain];
	#endif

	#if defined( TARGET_LINUX ) && defined (OF_USING_GTK)
		initGTK();
		GtkWidget* dialog = gtk_message_dialog_new (NULL, (GtkDialogFlags) 0, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, errorMessage.c_str());
		gtk_dialog_run (GTK_DIALOG (dialog));
		startGTK(dialog);
	#endif

	#ifdef TARGET_ANDROID
		ofxAndroidAlertBox(errorMessage);
	#endif
}


//----------------------------------------------------------------------------------------
//------------------------------------------------------------------------------       OSX
//----------------------------------------------------------------------------------------
#ifdef TARGET_OSX
//---------------------------------------------------------------------
// Gets a file to open from the user. Caller must release the CFURLRef.
CFURLRef GetOpenFileFromUser(bool bFolder)
{
	NavDialogCreationOptions dialogOptions;
	NavDialogRef dialog;
	NavReplyRecord replyRecord;
	CFURLRef fileAsCFURLRef = NULL;
	FSRef fileAsFSRef;
	OSStatus status;

	// Get the standard set of defaults
	status = NavGetDefaultDialogCreationOptions(&dialogOptions);

	require_noerr( status, CantGetNavOptions );

	// Make the window app-wide modal
	dialogOptions.modality = kWindowModalityAppModal;
	dialogOptions.optionFlags != kNavAllowOpenPackages;

	// Create the dialog
	if( bFolder ){
		status = NavCreateChooseFolderDialog(&dialogOptions, NULL, NULL, NULL, &dialog);
	}else{
		status = NavCreateGetFileDialog(&dialogOptions, NULL, NULL, NULL, NULL, NULL, &dialog);
	}

	require_noerr( status, CantCreateDialog );

	// Show it
	status = NavDialogRun(dialog);
	require_noerr( status, CantRunDialog );

	// Get the reply
	status = NavDialogGetReply(dialog, &replyRecord);
	require( ((status == noErr) || (status == userCanceledErr)), CantGetReply );

	// If the user clicked "Cancel", just bail
	if ( status == userCanceledErr ) goto UserCanceled;

	// Get the file
	//TODO: for multiple files - 1 specifies index
	status = AEGetNthPtr(&(replyRecord.selection), 1, typeFSRef, NULL, NULL, &fileAsFSRef, sizeof(FSRef), NULL);
	require_noerr( status, CantExtractFSRef );

	// Convert it to a CFURL
	fileAsCFURLRef = CFURLCreateFromFSRef(NULL, &fileAsFSRef);

	// Cleanup
CantExtractFSRef:
UserCanceled:
	verify_noerr( NavDisposeReply(&replyRecord) );
CantGetReply:
CantRunDialog:
	NavDialogDispose(dialog);
CantCreateDialog:
CantGetNavOptions:
    return fileAsCFURLRef;
}
#endif
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------



// OS specific results here.  "" = cancel or something bad like can't load, can't save, etc...
ofFileDialogResult ofSystemLoadDialog(string windowTitle, bool bFolderSelection){

	ofFileDialogResult results;

	//----------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------       OSX
	//----------------------------------------------------------------------------------------
#ifdef TARGET_OSX
	CFURLRef cfUrl = GetOpenFileFromUser(bFolderSelection);

	CFStringRef cfString = NULL;

	if ( cfUrl != NULL ){
		cfString = CFURLCopyFileSystemPath( cfUrl, kCFURLPOSIXPathStyle );
		CFRelease( cfUrl );

		// copy from a CFString into a local c string (http://www.carbondev.com/site/?page=CStrings+)
		const int kBufferSize = MAXPATHLEN;

		char fileUrl[kBufferSize];
		Boolean bool1 = CFStringGetCString(cfString,fileUrl,kBufferSize,kCFStringEncodingMacRoman);

		//char fileName[kBufferSize];
		//Boolean bool2 = CFStringGetCString(reply.saveFileName,fileName,kBufferSize,kCFStringEncodingMacRoman);

		// append strings together
		CFRelease(cfString);
		results.filePath = fileUrl;
	}

#endif
	//----------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------   windoze
	//----------------------------------------------------------------------------------------
#ifdef TARGET_WIN32

	if (bFolderSelection == false){

        OPENFILENAME ofn;
		char szFileName[MAX_PATH];

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		HWND hwnd = WindowFromDC(wglGetCurrentDC());
		ofn.hwndOwner = hwnd;
#ifdef __MINGW32_VERSION
		ofn.lpstrFilter = "All\0";
		ofn.lpstrFile = szFileName;
#else // VS2010
		ofn.lpstrFilter = LPCWSTR("All\0");
		ofn.lpstrFile = LPWSTR(szFileName);
#endif
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
		ofn.lpstrDefExt = 0;

		if(GetOpenFileName(&ofn)) {
			results.filePath = string(szFileName);
		}

	} else {

		BROWSEINFOW      bi;
		wchar_t         wideCharacterBuffer[MAX_PATH];
		LPITEMIDLIST    pidl;
		LPMALLOC		lpMalloc;

		// Get a pointer to the shell memory allocator
		if(SHGetMalloc(&lpMalloc) != S_OK){
			//TODO: deal with some sort of error here?
		}
		bi.hwndOwner        =   NULL;
		bi.pidlRoot         =   NULL;
		bi.pszDisplayName   =   wideCharacterBuffer;
		bi.lpszTitle        =   L"Select Directory";
		bi.ulFlags          =   BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
		bi.lpfn             =   NULL;
		bi.lParam           =   0;

		if(pidl = SHBrowseForFolderW(&bi)){
			// Copy the path directory to the buffer
			if(SHGetPathFromIDListW(pidl,wideCharacterBuffer)){
				results.filePath = convertWideToNarrow(wideCharacterBuffer);
			}
			lpMalloc->Free(pidl);
		}
		lpMalloc->Release();
	}

	//----------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------   windoze
	//----------------------------------------------------------------------------------------
#endif




	//----------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------   linux
	//----------------------------------------------------------------------------------------
#if defined( TARGET_LINUX ) && defined (OF_USING_GTK)
		if(bFolderSelection) results.filePath = gtkFileDialog(GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,windowTitle);
		else results.filePath = gtkFileDialog(GTK_FILE_CHOOSER_ACTION_OPEN,windowTitle);
#endif
	//----------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------



	if( results.filePath.length() > 0 ){
		results.bSuccess = true;
		results.fileName = ofFilePath::getFileName(results.filePath);
	}

	return results;
}



ofFileDialogResult ofSystemSaveDialog(string defaultName, string messageName){

	ofFileDialogResult results;

	//----------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------       OSX
	//----------------------------------------------------------------------------------------
#ifdef TARGET_OSX

	short fRefNumOut;
	FSRef output_file;
	OSStatus err;

	NavDialogCreationOptions options;
	NavGetDefaultDialogCreationOptions( &options );

	options.optionFlags = kNavNoTypePopup + kNavSupportPackages + kNavAllowOpenPackages;
	options.modality = kWindowModalityAppModal;

	options.optionFlags = kNavDefaultNavDlogOptions;
	options.message = CFStringCreateWithCString(NULL, messageName.c_str(), kCFStringEncodingASCII);;
	options.saveFileName = CFStringCreateWithCString(NULL, defaultName.c_str(), kCFStringEncodingASCII);
	NavDialogRef dialog;

	err = NavCreatePutFileDialog(&options, '.mov', 'Moov', NULL, NULL, &dialog);

	//printf("NavCreatePutFileDialog returned %i\n", err );

	err = NavDialogRun(dialog);
	//printf("NavDialogRun returned %i\n", err );

	NavUserAction action;
	action = NavDialogGetUserAction( dialog );
	//printf("got action %i\n", action);
	if (action == kNavUserActionNone || action == kNavUserActionCancel) {

		return results;
	}

	// get dialog reply
	NavReplyRecord reply;
	err = NavDialogGetReply(dialog, &reply);
	if ( err != noErr )
		return results;

	if ( reply.replacing ) {
		ofLog(OF_LOG_WARNING, "ofSystemSaveDialog: need to replace");
	}

	AEKeyword keyword;
	DescType actual_type;
	Size actual_size;
	FSRef output_dir;
	err = AEGetNthPtr(&(reply.selection), 1, typeFSRef, &keyword, &actual_type,
					  &output_dir, sizeof(output_file), &actual_size);

	//printf("AEGetNthPtr returned %i\n", err );


	CFURLRef cfUrl = CFURLCreateFromFSRef( kCFAllocatorDefault, &output_dir );
	CFStringRef cfString = NULL;
	if ( cfUrl != NULL )
	{
		cfString = CFURLCopyFileSystemPath( cfUrl, kCFURLPOSIXPathStyle );
		CFRelease( cfUrl );
	}

	// copy from a CFString into a local c string (http://www.carbondev.com/site/?page=CStrings+)
	const int kBufferSize = 255;

	char folderURL[kBufferSize];
	Boolean bool1 = CFStringGetCString(cfString,folderURL,kBufferSize,kCFStringEncodingMacRoman);

	char fileName[kBufferSize];
	Boolean bool2 = CFStringGetCString(reply.saveFileName,fileName,kBufferSize,kCFStringEncodingMacRoman);

	// append strings together

	string url1 = folderURL;
	string url2 = fileName;
	string finalURL = url1 + "/" + url2;

	results.filePath = finalURL.c_str();

	//printf("url %s\n", finalURL.c_str());

	// cleanup dialog
	NavDialogDispose(dialog);

#endif
	//----------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------   windoze
	//----------------------------------------------------------------------------------------
#ifdef TARGET_WIN32


	wchar_t fileName[MAX_PATH] = L"";
	char * extension;
	OPENFILENAMEW ofn;
    memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	HWND hwnd = WindowFromDC(wglGetCurrentDC());
	ofn.hwndOwner = hwnd;
	ofn.hInstance = GetModuleHandle(0);
	ofn.nMaxFileTitle = 31;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
	ofn.lpstrDefExt = L"";	// we could do .rxml here?
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
	ofn.lpstrTitle = L"Select Output File";

	if (GetSaveFileNameW(&ofn)){
		results.filePath = convertWideToNarrow(fileName);
	}

#endif
	//----------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------


	//----------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------   linux
	//----------------------------------------------------------------------------------------
#if defined( TARGET_LINUX ) && defined (OF_USING_GTK)

	results.filePath = gtkFileDialog(GTK_FILE_CHOOSER_ACTION_SAVE, messageName,defaultName);

#endif
	//----------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------

	if( results.filePath.length() > 0 ){
		results.bSuccess = true;
		results.fileName = ofFilePath::getFileName(results.filePath);
	}

	return results;
}

#ifdef TARGET_WIN32

// from http://blogs.msdn.com/b/oldnewthing/archive/2005/04/29/412577.aspx
// Raymond Chen 'Building a dialog template at run-time'


#include <windowsx.h>

class DialogTemplate {
public:
 LPCDLGTEMPLATE Template() { return (LPCDLGTEMPLATE)&v[0]; }
 void AlignToDword()
  { if (v.size() % 4) Write(NULL, 4 - (v.size() % 4)); }
 void Write(LPCVOID pvWrite, DWORD cbWrite) {
  v.insert(v.end(), cbWrite, 0);
  if (pvWrite) CopyMemory(&v[v.size() - cbWrite], pvWrite, cbWrite);
 }
 template<typename T> void Write(T t) { Write(&t, sizeof(T)); }
 void WriteString(LPCWSTR psz)
  { Write(psz, (lstrlenW(psz) + 1) * sizeof(WCHAR)); }

void DumpCArray()
{
    printf("static const char textDialogTemplateData[%i] = {", v.size() );
    for ( int i=0; i<v.size(); i++ )
    {
        if ( i%10 == 0 )
            printf("\n   ");

        printf("% 4i, ", int(v[i]) );
    }
    printf("\n};\n");
    printf("static const LPCDLGTEMPLATE textDialogTemplate = (LPCDLGTEMPLATE)textDialogTemplateData;\n");
}

private:
 vector<BYTE> v;
};

// and the output
static const char textDialogTemplateData[164] = {
      1,    0,  255,  255,    0,    0,    0,    0,    0,    0,
      0,    0,  192,    0,  200,    0,    3,    0,   32,    0,
     32,    0,  200,    0,   80,    0,    0,    0,    0,    0,
      0,    0,    8,    0,  144,    1,    0,    1,   84,    0,
     97,    0,  104,    0,  111,    0,  109,    0,   97,    0,
      0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
      0,    0,    0,   80,    7,    0,    7,    0,  186,    0,
     42,    0,  255,  255,  255,  255,  255,  255,  129,    0,
      0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
      0,    0,    1,    0,    3,   80,   35,    0,   59,    0,
     50,    0,   14,    0,    1,    0,    0,    0,  255,  255,
    128,    0,   79,    0,   75,    0,    0,    0,    0,    0,
      0,    0,    0,    0,    0,    0,    0,    0,    1,    0,
      3,   80,  115,    0,   59,    0,   50,    0,   14,    0,
      2,    0,    0,    0,  255,  255,  128,    0,   67,    0,
     97,    0,  110,    0,   99,    0,  101,    0,  108,    0,
      0,    0,    0,    0,
};
static const LPCDLGTEMPLATE textDialogTemplate = (LPCDLGTEMPLATE)textDialogTemplateData;
typedef struct {
    string question;
    string text;
    bool okClicked;
} TextDialogResult;

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
{
    // this is an ugly hack
   static TextDialogResult* results = NULL;

 switch (wm) {
 case WM_INITDIALOG:
    results = (TextDialogResult*)lParam;
    SetDlgItemTextA( hwnd, -1, results->text.c_str() );
    SetWindowTextA( hwnd, results->question.c_str() );
    return TRUE;
 case WM_COMMAND:
    if ( GET_WM_COMMAND_ID(wParam, lParam) == IDOK )
        {
            // fetch item text and display
            char buf[16384];
            GetDlgItemTextA( hwnd, -1, buf, 16384 );
            results->text = buf;
            EndDialog( hwnd, 1 );
        }
        else if ( GET_WM_COMMAND_ID(wParam, lParam) == IDCANCEL )
        {
            EndDialog( hwnd, 0 );
        }
  break;
 }
 return FALSE;
}


#endif


string ofSystemTextBoxDialog(string question, string text){
#if defined( TARGET_LINUX ) && defined (OF_USING_GTK)
	initGTK();
	GtkWidget* dialog = gtk_message_dialog_new (NULL, (GtkDialogFlags) 0, GTK_MESSAGE_QUESTION, (GtkButtonsType) GTK_BUTTONS_OK_CANCEL, question.c_str() );
	GtkWidget* content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	GtkWidget* textbox = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(textbox),text.c_str());
	gtk_container_add (GTK_CONTAINER (content_area), textbox);
	gtk_widget_show_all (dialog);
	if(gtk_dialog_run (GTK_DIALOG (dialog))==GTK_RESPONSE_OK){
		text = gtk_entry_get_text(GTK_ENTRY(textbox));
	}
	startGTK(dialog);
#endif

#ifdef TARGET_OSX
	// create alert dialog
	NSAlert *alert = [[[NSAlert alloc] init] autorelease];
	[alert addButtonWithTitle:@"OK"];
	[alert addButtonWithTitle:@"Cancel"];
	[alert setMessageText:[NSString stringWithCString:question.c_str()
											 encoding:NSUTF8StringEncoding]];
	// create text field
	NSTextField* label = [[NSTextField alloc] initWithFrame:NSRectFromCGRect(CGRectMake(0,0,300,40))];
	[label setStringValue:[NSString stringWithCString:text.c_str()
											 encoding:NSUTF8StringEncoding]];
	// add text field to alert dialog
	[alert setAccessoryView:label];
	NSInteger returnCode = [alert runModal];
	// if OK was clicked, assign value to text
	if ( returnCode == NSAlertFirstButtonReturn )
		text = [[label stringValue] UTF8String];
#endif

#ifdef TARGET_WIN32

    HWND hwnd = WindowFromDC(wglGetCurrentDC());


 BOOL fSuccess = FALSE;
 HDC hdc = GetDC(NULL);
 if (hdc) {
  NONCLIENTMETRICSW ncm = { sizeof(ncm) };
  if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0)) {
      /*
   DialogTemplate tmp;

   // values from DLGITEMTEMPLATE docs
   // http://msdn.microsoft.com/en-us/library/windows/desktop/ms644997(v=vs.85).aspx
   static const DWORD MAGIC_EDIT = 0x0081ffff;
   static const DWORD MAGIC_BUTTON = 0x0080ffff;
   static const DWORD MAGIC_STATIC = 0x0082ffff;

   // Write out the extended dialog template header
   tmp.Write<WORD>(1); // dialog version
   tmp.Write<WORD>(0xFFFF); // extended dialog template
   tmp.Write<DWORD>(0); // help ID
   tmp.Write<DWORD>(0); // extended style
   tmp.Write<DWORD>(WS_CAPTION | WS_SYSMENU | DS_SETFONT | DS_MODALFRAME);
   tmp.Write<WORD>(3); // number of controls
   tmp.Write<WORD>(32); // X
   tmp.Write<WORD>(32); // Y
   tmp.Write<WORD>(200); // width
   tmp.Write<WORD>(80); // height
   tmp.WriteString(L""); // no menu
   tmp.WriteString(L""); // default dialog class
   tmp.WriteString(L""); // title

   // Next comes the font description.
   // See text for discussion of fancy formula.
   if (ncm.lfMessageFont.lfHeight < 0) {
     ncm.lfMessageFont.lfHeight = -MulDiv(ncm.lfMessageFont.lfHeight,
              72, GetDeviceCaps(hdc, LOGPIXELSY));
   }
   tmp.Write<WORD>((WORD)ncm.lfMessageFont.lfHeight); // point
   tmp.Write<WORD>((WORD)ncm.lfMessageFont.lfWeight); // weight
   tmp.Write<BYTE>(ncm.lfMessageFont.lfItalic); // Italic
   tmp.Write<BYTE>(ncm.lfMessageFont.lfCharSet); // CharSet
   tmp.WriteString(ncm.lfMessageFont.lfFaceName);

   // Then come the two controls.  First is the static text.
   tmp.AlignToDword();
   tmp.Write<DWORD>(0); // help id
   tmp.Write<DWORD>(0); // window extended style
   tmp.Write<DWORD>(WS_CHILD | WS_VISIBLE); // style
   tmp.Write<WORD>(7); // x
   tmp.Write<WORD>(7); // y
   tmp.Write<WORD>(200-14); // width
   tmp.Write<WORD>(80-7-14-7-10); // height
   //tmp.Write<WORD>(20);
   tmp.Write<DWORD>(-1); // control ID
   tmp.Write<DWORD>(MAGIC_EDIT); // edit
   //tmp.Write<DWORD>((DWORD)(unsigned char*)"STATIC");
   tmp.WriteString(L""); // text
   tmp.Write<WORD>(0); // no extra data

   // Second control is the OK button.
   tmp.AlignToDword();
   tmp.Write<DWORD>(0); // help id
   tmp.Write<DWORD>(0); // window extended style
   tmp.Write<DWORD>(WS_CHILD | WS_VISIBLE |
                    WS_GROUP | WS_TABSTOP | BS_DEFPUSHBUTTON); // style
   tmp.Write<WORD>(35); // x
   tmp.Write<WORD>(80-7-14); // y
   tmp.Write<WORD>(50); // width
   tmp.Write<WORD>(14); // height
   tmp.Write<DWORD>(IDOK); // control ID
   tmp.Write<DWORD>(MAGIC_BUTTON); // static
   tmp.WriteString(L"OK"); // text
   tmp.Write<WORD>(0); // no extra data


   // Third control is the Cancel button.
   tmp.AlignToDword();
   tmp.Write<DWORD>(0); // help id
   tmp.Write<DWORD>(0); // window extended style
   tmp.Write<DWORD>(WS_CHILD | WS_VISIBLE |
                    WS_GROUP | WS_TABSTOP | BS_DEFPUSHBUTTON); // style
   tmp.Write<WORD>(115); // x
   tmp.Write<WORD>(80-7-14); // y
   tmp.Write<WORD>(50); // width
   tmp.Write<WORD>(14); // height
   tmp.Write<DWORD>(IDCANCEL); // control ID
   tmp.Write<DWORD>(MAGIC_BUTTON); // static
   tmp.WriteString(L"Cancel"); // text
   tmp.Write<WORD>(0); // no extra data

    tmp.DumpCArray();
*/
   // Template is ready - go display it.
   TextDialogResult results;
   results.question= question;
   results.text = text;
   int dialogBoxReturnCode = DialogBoxIndirectParam(GetModuleHandle(0), textDialogTemplate, hwnd, DlgProc, (LPARAM)&results );
   // if ok was clicked
   if ( dialogBoxReturnCode == 1){
       text = results.text;
   }
   //fSuccess = DialogBoxIndirect( GetModuleHandle(0), textDialogTemplate, hwnd, DlgProc) >= 0;
   //ofLog( OF_LOG_NOTICE, "dialog box: %s clicked, string %s", results.okClicked?"OK":"Cancel", results.text.c_str() );
  }
  ReleaseDC(NULL, hdc); // fixed 11 May
 }
/*
    // we need to convert error message to a wide char message.
    // first, figure out the length and allocate a wchar_t at that length + 1 (the +1 is for a terminating character)

    WNDCLASSEX wc;
    MSG Msg;
    const char g_szClassName[] = "myWindowClass";
    //Step 1: Registering the Window Class
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = GetModuleHandle(0);
    wc.lpszClassName = g_szClassName;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
    if(!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return text;
    }
    HWND dialog = CreateWindowEx(        WS_EX_CLIENTEDGE,
        g_szClassName,
        question.c_str(),
        WS_OVERLAPPEDWINDOW | WS_CHILD,
        CW_USEDEFAULT, CW_USEDEFAULT, 240, 140,
        WindowFromDC(wglGetCurrentDC()), NULL, GetModuleHandle(0), NULL);

    if(dialog == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return text;
    }

    HWND hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", text.c_str(),
        WS_CHILD | WS_VISIBLE,
        10, 10, 210, 40, dialog, (HMENU)101, GetModuleHandle(NULL), NULL);


    HWND okButton = CreateWindowEx(WS_EX_CLIENTEDGE, "BUTTON", "Ok",
        WS_CHILD | WS_VISIBLE,
        10, 60, 60, 30, dialog, (HMENU)101, GetModuleHandle(NULL), NULL);

    HWND cancelButton = CreateWindowEx(WS_EX_CLIENTEDGE, "BUTTON", "Cancel",
        WS_CHILD | WS_VISIBLE,
        80, 60, 60, 30, dialog, (HMENU)101, GetModuleHandle(NULL), NULL);
    //EnableWindow(dialog, FALSE);
    ShowWindow(dialog, SW_SHOWNORMAL);
    UpdateWindow(dialog);
    //EnableWindow(WindowFromDC(wglGetCurrentDC()), FALSE);
    while(true)
    {
        WaitForSingleObject(dialog,10);
    }*/
#endif

	return text;
}
