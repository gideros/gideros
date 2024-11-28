#include <platform.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <map>

static std::map<std::string,std::string> cursorMap={
		{ "arrow", "initial" },
		{ "upArrow", "n-resize" },
		{ "cross", "crosshair" },
		{ "wait", "wait" },
		{ "IBeam", "text" },
		{ "sizeVer", "ns-resize" },
		{ "sizeHor", "ew-resize" },
		{ "sizeBDiag", "nwse-resize" },
		{ "sizeFDiag", "nesw-resize" },
		{ "sizeAll", "all-scroll" }, //Doesn't really fit, but best so far'
		{ "blank", "none" },
		{ "splitV", "row-resize" },
		{ "splitH", "col-resize" },
		{ "pointingHand", "pointer" },
		{ "forbidden", "not-allowed" },
		{ "whatsThis", "help" },
		{ "busy", "progress" },
		{ "openHand", "grab" },
		{ "closedHand", "grabbing" },
		{ "dragCopy", "copy" },
		{ "dragMove", "move" },
		{ "dragLink", "alias" },
};

std::vector<std::string> getDeviceInfo()
{
  std::vector<std::string> result;
        
  result.push_back("Web");
	char *userAgentC=(char *) EM_ASM_PTR({
	 return stringToNewUTF8(navigator.userAgent);
	});
  std::string userAgent=userAgentC;
  free(userAgentC);
  result.push_back(userAgent);
                                
 return result;
}

std::string getAppId(){
	return "";
}

void getSafeDisplayArea(int &x,int &y,int &w,int &h)
{
}

void setKeepAwake(bool awake)
{
}

bool setKeyboardVisibility(bool visible){
	return false;
}

bool setTextInput(int type,const char *buffer,int selstart,int selend,const char *label,const char *actionLabel, const char *hintText, const char *context)
{
	return false;
}

int setClipboard(std::string data,std::string mimeType, int luaFunc) {
	if ((mimeType=="")||(mimeType=="text/plain")) {
		EM_ASM_({
			if (window.navigator.clipboard && window.navigator.clipboard.writeText) {
				window.navigator.clipboard.writeText(UTF8ToString($1)).then(
					function() { Module._gapplication_clipboardCallback($0,1,0,0) }
				).catch(
					function() { Module._gapplication_clipboardCallback($0,-1,0,0) }
				);
			}
			else
				Module._gapplication_clipboardCallback($0,-1,"","");
		},luaFunc,data.c_str());
		return 0;
	}
	return -1;
}

int getClipboard(std::string &data,std::string &mimeType, int luaFunc) {
	if ((mimeType=="")||(mimeType=="text/plain")) {
		EM_ASM_({
			if (window.navigator.clipboard && window.navigator.clipboard.readText) {
				window.navigator.clipboard.readText().then(function (clipText) {
						var cClip=stringToNewUTF8(clipText);
						var cMime=stringToNewUTF8("text/plain");
						Module._gapplication_clipboardCallback($0,1, cClip, cMime);
						_free(cClip);
						_free(cMime);
					}
				).catch(
					err => Module._gapplication_clipboardCallback($0,-1,0,0)
				);
			}
			else
				Module._gapplication_clipboardCallback($0,-1,0,0);
		},luaFunc);
		return 0;
	}
	return -1;
}

extern int s_KeyboardModifiers;
int getKeyboardModifiers() {
	return s_KeyboardModifiers;
}

void vibrate(int ms)
{
	EM_ASM_({
		window.navigator.vibrate($0);
	},ms);
}

void setWindowSize(int width, int height){
	//TODO
}

void setFullScreen(bool fullScreen){
	emscripten_request_fullscreen("canvas",true);
}

std::string getDeviceName(){
	return ""; //TODO
}

std::string getLocale()
{
	return ""; //TODO
}

std::string getLanguage()
{
	char *lang=(char *) EM_ASM_PTR({
	 return stringToNewUTF8(Module.gplatformLanguage());
	});
	std::string sLang=lang;
	free(lang);
	return sLang;
}

void openUrl(const char* url)
{
	EM_ASM_({
		window.open(UTF8ToString($0));
	},url);
}

bool canOpenUrl(const char *url)
{
    return true;
}

extern const char *currentUrl;
std::vector<gapplication_Variant> g_getsetProperty(bool set, const char* what, std::vector<gapplication_Variant> &args)
{
	std::vector<gapplication_Variant> rets;
	gapplication_Variant r;
	if (!set) {
		if (strcmp(what, "screenSize") == 0)
		{
			int width,height;
			emscripten_get_screen_size(&width,&height);
			double ratio=emscripten_get_device_pixel_ratio();
			r.type=gapplication_Variant::DOUBLE;
			r.d=ratio*width;
			rets.push_back(r);
			r.d=ratio*height;
			rets.push_back(r);
			/*------------------------------------------------------------------*/
        }else if (strcmp(what, "windowTitle") == 0)
        {
            r.type=gapplication_Variant::STRING;
            r.s=emscripten_get_window_title();
            rets.push_back(r);
            /*------------------------------------------------------------------*/
		} else if (!strcmp(what,"currentUrl"))
		{
			r.type=gapplication_Variant::STRING;
			r.s=currentUrl;
			rets.push_back(r);
		}
	}
	else {
		if (!strcmp(what,"cursor")) {
			std::string mapped=cursorMap[args[0].s];
			if (mapped.empty()) mapped="initial";
			EM_ASM_({
				document.getElementById("canvas").style.cursor=UTF8ToString($0);
			},mapped.c_str());
        }else if (strcmp(what, "windowTitle") == 0)
        {
            if (args.size()>=1)
                emscripten_set_window_title(args[0].s.c_str());
            /*------------------------------------------------------------------*/
		}
	}
	return rets;
}

static int s_fps = 60;

extern "C" {

int g_getFps()
{
    return s_fps;
}

void g_setFps(int fps)
{
    s_fps = fps;
}


}


void g_exit()
{
    exit(0);
}
