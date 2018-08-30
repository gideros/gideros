#ifndef ADDONS_H
#define ADDONS_H

#include <string>
#include <set>
#include <vector>
#include <map>

class Addon {
private:
	std::string path;
public:
	std::string name;
	std::string title;
	std::string gapp;
	std::vector<std::string> exts;
	Addon(std::string path) { this->path=path; };
	std::string getGApp() { return path+"/"+gapp; };
};

class AddonsManager
{

public:
    static std::vector<Addon> loadAddons(bool refresh);
    static void launch(std::string name, std::string env);
    static std::string addonForExtension(std::string ext);
private:
    static std::vector<Addon> addons;
};

#endif
