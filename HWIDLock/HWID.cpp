#include "HWID.h"
#include <fstream>
#include <windows.h>
#include <iostream>
#include <intrin.h>

void HWID::load_hwids()
{
	std::ifstream hwid_file;
	try
	{
		hwid_file.open("hwids.txt");
		std::string str;
		while (getline(hwid_file, str))
		{
			hwids.push_back(str);
		}
	}
	catch (const std::exception &e)
	{
		std::cout << e.what();
	}
}

bool HWID::check_hwid()
{
	for (auto hwid : hwids)
	{
		DWORD disk_serialINT;
		GetVolumeInformationA(nullptr, nullptr, NULL, &disk_serialINT, nullptr, nullptr, nullptr, NULL);
		auto HDDserial = std::to_string(disk_serialINT);
		std::cout << HDDserial << std::endl;
	}
	return false;
}