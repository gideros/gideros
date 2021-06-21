#include "../ads.h"
#include "../AdsAbstract.h"
#include <collection.h>
using namespace Windows::Foundation;

ref class AdVungleState {
public:
	property bool loaded;
	property bool toshow;
internal:
	std::string type;
};

ref class AdsVungleListener;

class AdsVungle : public AdsAbstract
{
	public:
		AdsVungle();
		~AdsVungle();
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
		Platform::Collections::UnorderedMap<Platform::String^, AdVungleState^>^ state;
		void PlayAd(Platform::String^ stype, AdVungleState^ adstate);
	private:
		AdsVungleListener^ listeners;
		VungleSDK::UI::VungleAdControl^ banner;
		VungleSDK::VungleAd^ sdkInstance;
};

ref class AdsVungleListener {
internal:
	AdsVungle* adv;
public:
	void OnInitCompleted(Platform::Object^ sender, VungleSDK::ConfigEventArgs^ args);
	// Event handler called when e->AdPlayable is changed
	void OnOnAdPlayableChanged(Platform::Object^ sender, VungleSDK::AdPlayableEventArgs^ args);
	// Event Handler called before playing an ad
	void OnAdStart(Platform::Object^ sender, VungleSDK::AdEventArgs^ e);
	void OnVideoView(Platform::Object^ sender, VungleSDK::AdViewEventArgs^ e);
	// Event handler called when the user leaves ad and control is return to the hosting app
	void OnAdEnd(Platform::Object^ sender, VungleSDK::AdEndEventArgs^ e);
	// Diagnostics
	void Diagnostic(Platform::Object^ sender, VungleSDK::DiagnosticLogEvent^ e);
};
