#include <ogl.h>
#include <color.h>
#include <string.h>
#include <vector>
#include <string>
#include <platform.h>
#include <sstream>

static void drawIP(const char* ip, int size, int xx, int yy)
{
    static const char* chardot = " "
                                 " "
                                 " "
                                 " "
                                 ".";

    static const char* char0 = "..."
                               ". ."
                               ". ."
                               ". ."
                               "...";

    static const char* char1 = "."
                               "."
                               "."
                               "."
                               ".";

    static const char* char2 = "..."
                               "  ."
                               "..."
                               ".  "
                               "...";

    static const char* char3 = "..."
                               "  ."
                               "..."
                               "  ."
                               "...";

    static const char* char4 = ".  "
                               ". ."
                               ". ."
                               "..."
                               "  .";

    static const char* char5 = "..."
                               ".  "
                               "..."
                               "  ."
                               "...";

    static const char* char6 = "..."
                               ".  "
                               "..."
                               ". ."
                               "...";

    static const char* char7 = "..."
                               "  ."
                               "  ."
                               "  ."
                               "  .";

    static const char* char8 = "..."
                               ". ."
                               "..."
                               ". ."
                               "...";

    static const char* char9 = "..."
                               ". ."
                               "..."
                               "  ."
                               "...";

    static const char* charx = "   "
                               "   "
                               ". ."
                               " . "
                               ". .";

	static const char* loading =
		".   ... ... ..  . ... ...      "
		".   . . . . . . . . . .        "
		".   . . ... . . . . . . .      "
		".   . . . . . . . . . . .      "
		"... ... . . ..  . . . ... . . .";


	static const char* localip =
		".   ... ... ... .     . ...   . ... ... ...  "
		".   . . .   . . .     . . .   . . . .   . . ."
		".   . . .   ... .     . ...   . . . ..  . .  "
		".   . . .   . . .     . .     . . . .   . . ."
		"... ... ... . . ...   . .     . . . .   ...  ";

	static const char* ipinfo =
		". ...   . ... ... ...  "
		". . .   . . . .   . .  "
		". ...   . . . ..  . . ."
		". .     . . . .   . .  "
		". .     . . . .   ... .";

	static const char* version =
        "... ... . .     . ..."
        "  . . . . . .   . . ."
        "... . . . . .   . . ."
        ".   . . . ...   . . ."
        "... ... .   . . . ...";

    static const char* resolution =
        "... ... ... ... .   . . ... . ... ...  "
        ". . .   .   . . .   . .  .  . . . . . ."
        "..  ... ... . . .   . .  .  . . . . .  "
        ". . .     . . . .   . .  .  . . . . . ."
        ". . ... ... ... ... ...  .  . ... . .  ";

	static const char* chars[] = {char0, char1, char2, char3, char4, char5, char6, char7, char8, char9};

	glPushColor();

    // set info text color to white
    glSetColor(1, 1, 1, 1);

	oglDisable(GL_TEXTURE_2D);

	oglEnableClientState(GL_VERTEX_ARRAY);

	float v[8];
	glVertexPointer(2, GL_FLOAT, 0, v);

	int len = strlen(ip);
	for (int i = 0; i < len; ++i)
	{
		const char* chr;
        if (ip[i] == '.')
			chr = chardot;
        else if(ip[i] == 'x')
            chr = charx;
        else if (ip[i] == 'I')
        {
            // set color of labels to gray
            glSetColor(0.5, 0.5, 0.5, 1);

			chr = localip;
        }
        else if (ip[i] == 'L')
			chr = loading;
		else if (ip[i] == 'V')
        {
            // set color of labels to gray
            glSetColor(0.5, 0.5, 0.5, 1);

			chr = version;
        }

        // if param is R, draw resolution label in gray color
        else if (ip[i] == 'R')
        {
            glSetColor(0.5, 0.5, 0.5, 1);
            chr = resolution;
        }
		else
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

					oglDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				}
			}

		xx = xx + width + 1;
	}

	oglDisableClientState(GL_VERTEX_ARRAY);

	glPopColor();
}

static std::vector<std::string> ips;

void refreshLocalIPs()
{
	ips = getLocalIPs();

	for (int i = (int)ips.size() - 1; i >= 0; --i)
		if (ips[i].find_first_not_of("0123456789.") != std::string::npos)
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
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    drawIP("V", 3, 2, 2);
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

void drawInfoResolution(int width, int height)
{
    static int frame = 0;

#ifdef __ANDROID__
    if (frame++ == 0)
#else
    if ((frame++ % 60) == 0)
#endif
        refreshLocalIPs();

    // set background color of opengl canvas to black and clear the buffer to render again
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    drawIP("V", 3, 2, 2);

    // draw the resolution label and value on non running canvas
    drawIP("R", 3, 2, 2+7+3);

    std::ostringstream oWidth;
    std::ostringstream oHeight;

    oWidth << width;
    oHeight << height;

    std::string resolution = oWidth.str() + "x" + oHeight.str();
    drawIP(resolution.c_str(), 4, 6, 2+7+3+3);

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

