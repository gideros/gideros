#include <ogl.h>
#include <color.h>
#include <string.h>
#include <vector>
#include <string>
#include <platform.h>

static void drawIP(const char* ip, int size, int xx, int yy)
{
	static const char* chardot =	" "
							" "
							" "
							" "
							".";

	static const char* char0 =	"..."
							". ."
							". ."
							". ."
							"...";

	static const char* char1 =	"."
							"."
							"."
							"."
							".";

	static const char* char2 =	"..."
							"  ."
							"..."
							".  "
							"...";

	static const char* char3 =	"..."
							"  ."
							"..."
							"  ."
							"...";

	static const char* char4 =	".  "
							". ."
							". ."
							"..."
							"  .";

	static const char* char5 =	"..."
							".  "
							"..."
							"  ."
							"...";

	static const char* char6 =	"..."
							".  "
							"..."
							". ."
							"...";

	static const char* char7 =	"..."
							"  ."
							"  ."
							"  ."
							"  .";

	static const char* char8 =	"..."
							". ."
							"..."
							". ."
							"...";

	static const char* char9 =	"..."
							". ."
							"..."
							"  ."
							"...";

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
        "... ... . .     ... .  "
        "  . . . . . .   . . . ."
        "... . . . . .   . . . ."
        ".   . . . ...   . . ..."
        "... ... .   . . ...   .";

	static const char* chars[] = {char0, char1, char2, char3, char4, char5, char6, char7, char8, char9};

	glPushColor();
	glSetColor(0, 0, 0, 1);

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
		else if (ip[i] == 'I')
			chr = localip;
		else if (ip[i] == 'L')
			chr = loading;
		else if (ip[i] == 'V')
			chr = version;
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

    drawIP("V", 4, 2, 2);
    drawIP("I", 4, 2, 2+3+7);

	int x = 6;
	int y = 2+3+7+7;

	for (std::size_t i = 0; i < ips.size(); ++i)
	{
		if (ips[i] == "0.0.0.0")
			continue;
        drawIP(ips[i].c_str(), 4, x, y);
		y = y + 7;
	}
}

