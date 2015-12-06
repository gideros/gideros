#include "../ads.h"
#include "../AdsAbstract.h"
class AdsPubcenter : public AdsAbstract
{
public:
	AdsPubcenter();
	~AdsPubcenter();
	void setKey(gads_Parameter *params);
	void enableTesting();
	void loadAd(gads_Parameter *params);
	void showAd(gads_Parameter *params);
	void hideAd(const char* type);
	void setAlignment(const char *hor, const char *verr);
	void setX(int x);
	void setY(int y);
	int getX();
	int getY();
	int getWidth();
	int getHeight();
private:
	std::string key;
	std::string curSize = "250x250";
	std::map<std::string, std::pair<int, int>> adSizes;
	std::map<std::string, std::string> testUnits;
	bool test = false;
};