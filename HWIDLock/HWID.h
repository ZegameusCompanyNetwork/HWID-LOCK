#pragma once
#include <string>
#include <tuple>
#include <vector>
#include <list>

class HWID
{
	std::vector<std::string> hwids;
public:
	void load_hwids();
	bool check_hwid();
};
