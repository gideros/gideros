#include <string>
using namespace Windows::System::UserProfile;
using namespace Windows::Globalization;
std::string getLocale(){
	std::wstring data = GlobalizationPreferences::Languages->GetAt(0)->Data();
	return std::string(data.begin(), data.end());
}

std::string getLanguage()
{
	std::wstring data = GlobalizationPreferences::Languages->GetAt(0)->Data();
	std::string s(data.begin(), data.end());
	return s.substr(0,2);
}
