#include "../ads.h"
#include "../AdsAbstract.h"
class AdsPubcenter : public AdsAbstract
{
	public:
		AdsPubcenter();
		~AdsPubcenter();
		virtual void setKey(gads_Parameter *params);
		virtual void enableTesting();
		virtual void loadAd(gads_Parameter *params);
		virtual void showAd(gads_Parameter *params);
		virtual void hideAd(const char* type);
		virtual void setAlignment(const char *hor, const char *verr);
		virtual void setX(int x);
		virtual void setY(int y);
		virtual int getX();
		virtual int getY();
		virtual int getWidth();
		virtual int getHeight();
	private:
		std::string key;
		std::map<std::string, std::pair<int, int>> adSizes;
		std::map<std::string, std::string> testUnits;
		bool test = false;
};