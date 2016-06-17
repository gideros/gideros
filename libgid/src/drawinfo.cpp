#include <ogl.h>
#include <color.h>
#include <string.h>
#include <vector>
#include <string>
#include <platform.h>
#include <sstream>
#include "gideros.h"

float infoColor_[3];

static void drawIP(const char* ip, int size, int xx, int yy)
{
    static const char* chardot = " "
                                 " "
                                 " "
                                 " "
                                 ".";

    static const char* charcolon = " "
                                   "."
                                   " "
                                   "."
                                   " ";

    static const char* char0 = " .. "
                               ".  ."
                               ".  ."
                               ".  ."
                               " .. ";

    static const char* char1 = " . "
                               ".. "
                               " . "
                               " . "
                               "...";

    static const char* char2 = " .. "
                               ".  ."
                               "  . "
                               " .  "
                               "....";

    static const char* char3 = "... "
                               "   ."
                               " .. "
                               "   ."
                               "... ";

    static const char* char4 = "  .."
                               " . ."
                               ".  ."
                               "...."
                               "   .";

    static const char* char5 = "...."
                               ".   "
                               "...."
                               "   ."
                               "... ";

    static const char* char6 = " .. "
                               ".   "
                               "... "
                               ".  ."
                               " .. ";

    static const char* char7 = "...."
                               "   ."
                               "  . "
                               "  . "
                               "  . ";

    static const char* char8 = " .. "
                               ".  ."
                               " .. "
                               ".  ."
                               " .. ";

	static const char* char9 = " .. "
							   ".  ."
		                       " ..."
		                       "   ."
		                       " .. ";

	static const char* chara = " .. "
		                       ".  ."
		                       "...."
		                       ".  ."
		                       ".  .";

	static const char* charb = "... "
		                       ".  ."
		                       "... "
		                       ".  ."
		                       "... ";

	static const char* charc = " ..."
		                       ".   "
		                       ".   "
		                       ".   "
		                       " ...";

	static const char* chard = "... "
		                       ".  ."
		                       ".  ."
		                       ".  ."
		                       "... ";

	static const char* chare = "...."
		                       ".   "
		                       "... "
		                       ".   "
		                       "....";

	static const char* charf = "...."
		                       ".   "
		                       "... "
		                       ".   "
		                       ".   ";

	static const char* charPercent = ".   ."
                                     "   . "
                                     "  .  "
                                     " .   "
                                     ".   .";

    static const char* charX = ".  ."
                               ".  ."
                               " .. "
                               ".  ."
                               ".  .";

    static const char* charSpace = " "
                                   " "
                                   " "
                                   " "
                                   " ";

	static const char* loading =
        ".    ..   ..  ...  . .  .  ..."
        ".   .  . .  . .  . . .. . .   "
        ".   .  . .... .  . . . .. . .."
        ".   .  . .  . .  . . .  . .  ."
        "...  ..  .  . ...  . .  .  ...";


	static const char* localip =
        ".    ..   ...  ..  .     . ...   . .  . ...  ..   "
        ".   .  . .    .  . .     . .  .  . .. . .   .  . ."
        ".   .  . .    .... .     . ...   . . .. ... .  .  "
        ".   .  . .    .  . .     . .     . .  . .   .  . ."
        "...  ..   ... .  . ...   . .     . .  . .    ..   ";

    static const char* resolution =
        "...  ...  ...  ..  .   .  . ..... .  ..  .  .  "
        ".  . .   .    .  . .   .  .   .   . .  . .. . ."
        "...  ...  ..  .  . .   .  .   .   . .  . . ..  "
        ". .  .      . .  . .   .  .   .   . .  . .  . ."
        ".  . ... ...   ..  ...  ..    .   .  ..  .  .  ";

    static const char* hardware =
        ".  .  ..  ...  ...  . . .  ..  ...  ...  "
        ".  . .  . .  . .  . . . . .  . .  . .   ."
        ".... .... ...  .  . . . . .... ...  ...  "
        ".  . .  . . .  .  .  . .  .  . . .  .   ."
        ".  . .  . .  . ...   . .  .  . .  . ...  ";

    static const char* zoom =
        "....  ..   ..  .   .  "
        "   . .  . .  . .. .. ."
        " ..  .  . .  . . . .  "
        ".    .  . .  . .   . ."
        "....  ..   ..  .   .  ";

	static const char* chars[] = {char0, char1, char2, char3, char4, char5, char6, char7, char8, char9, chara, charb, charc, chard, chare, charf };

	glPushColor();

    glSetColor(infoColor_[0], infoColor_[1], infoColor_[2], 1);

	float v[8];

	int len = strlen(ip);
	for (int i = 0; i < len; ++i)
	{
		const char* chr=charSpace;
        if (ip[i] == '.')
			chr = chardot;
        else if (ip[i] == ':')
			chr = charcolon;
        else if(ip[i] == ' ')
            chr = charSpace;
        else if(ip[i] == 'X')
            chr = charX;
        else if(ip[i] == '%')
            chr = charPercent;
        else if (ip[i] == 'I')
			chr = localip;
        else if (ip[i] == 'L')
			chr = loading;
        else if (ip[i] == 'R')
            chr = resolution;
        else if (ip[i] == 'H')
            chr = hardware;
        else if (ip[i] == 'Z')
            chr = zoom;
		else if ((ip[i]>='a')&&(ip[i]<='f'))
			chr = chars[ip[i] - 'a'+10];
		else if ((ip[i] >= '0') && (ip[i] <= '9'))
			chr = chars[ip[i] - '0'];

		int height = 5;
		int width = strlen(chr) / height;

		for (int y = 0; y < height; ++y)
			for (int x = 0; x < width; ++x)
			{
				if (chr[x + y * width] == '.')
				{
					//glBegin(GL_QUADS);
					//glVertex2i((x + xx)     * size, (y + yy)     * size);
					//glVertex2i((x + xx + 1) * size, (y + yy)     * size);
					//glVertex2i((x + xx + 1) * size, (y + yy + 1) * size);
					//glVertex2i((x + xx)     * size, (y + yy + 1) * size);
					//glEnd();

					v[0] = (x + xx)     * size; v[1] = (y + yy)     * size;
					v[2] = (x + xx + 1) * size; v[3] = (y + yy)     * size;
					v[4] = (x + xx)     * size; v[5] = (y + yy + 1) * size;
					v[6] = (x + xx + 1) * size; v[7] = (y + yy + 1) * size;

					ShaderProgram::stdBasic->setData(ShaderProgram::DataVertex,ShaderProgram::DFLOAT,2, v,4, true, NULL);
					ShaderProgram::stdBasic->drawArrays(ShaderProgram::TriangleStrip, 0, 4);
				}
			}

		xx = xx + width + 1;
	}


	glPopColor();
}

static std::vector<std::string> ips;

void refreshLocalIPs()
{
	ips = getLocalIPs();

	for (int i = (int)ips.size() - 1; i >= 0; --i)
		if (ips[i].find_first_not_of("0123456789abcdef.:") != std::string::npos)
			ips.erase(ips.begin() + i);
}

void drawInfo()
{
	static int frame = 0;

#ifdef __ANDROID__	
	if (frame++ == 0)
#else
	if ((frame++ % 60) == 0)
#endif
		refreshLocalIPs();

    // set background color of opengl canvas to black and clear the buffer to render again
	ShaderEngine::Engine->clearColor(1,1,1,1);

    drawIP(GIDEROS_VERSION, 3, 2, 2);
    drawIP("I", 3, 2, 2+7+3+7+3+7+3);

	int x = 6;
    int y = 2+7+3+7+3+7+1;

	for (std::size_t i = 0; i < ips.size(); ++i)
	{
		if (ips[i] == "0.0.0.0")
			continue;
        drawIP(ips[i].c_str(), 4, x, y);
		y = y + 7;
	}
}

void drawInfoResolution(int width, int height, int scale, int lWidth, int lHeight, bool drawRunning, float canvasColor[3], float infoColor[3]){
    static int frame = 0;

    #ifdef __ANDROID__
        if (frame++ == 0)
    #else
        if ((frame++ % 60) == 0)
    #endif

    refreshLocalIPs();

    infoColor_[0] = infoColor[0];
    infoColor_[1] = infoColor[1];
    infoColor_[2] = infoColor[2];

    if(!drawRunning){
    	ShaderEngine::Engine->clearColor(canvasColor[0], canvasColor[1], canvasColor[2], 1);
    }

    int y = 1;
    drawIP(GIDEROS_VERSION, 2, 1, y);

    y = y + 8;
    drawIP("I", 2, 1, y);

    y = y + 8;
    for (std::size_t i = 0; i < ips.size(); ++i)
    {
        if (ips[i] == "0.0.0.0")
            continue;

        drawIP(ips[i].c_str(), 2, 4, y);

        y = y + 8;
    }

    // draw the hardware label and value on non running canvas
    drawIP("H", 2, 1, y);

    std::ostringstream oWidth;
    std::ostringstream oHeight;
    oWidth << width;
    oHeight << height;
    std::string hardware = oWidth.str() + " X " + oHeight.str();

    y = y + 8;
    drawIP(hardware.c_str(), 2, 4, y);

    // draw the resolution label and value on non running canvas
    if(drawRunning){
        y = y + 8;
        drawIP("R", 2, 1, y);

        std::ostringstream logWidth;
        std::ostringstream logHeight;
        logWidth << lWidth;
        logHeight << lHeight;
        std::string resolution = logWidth.str() + " X " + logHeight.str();

        y = y + 8;
        drawIP(resolution.c_str(), 2, 4, y);
    }

    // draw zoom label and value passed as parameters
    y = y + 8;
    drawIP("Z", 2, 2, y);

    std::ostringstream oScale;
    oScale << scale;
    std::string zoom = oScale.str() + "%";

    y = y + 8;
    drawIP(zoom.c_str(), 2, 4, y);
}
