#pragma once 
#include "Includes.hpp"
#include <fstream>
#include <filesystem>

namespace Logger
{
	extern std::ofstream logFile;
	extern std::filesystem::path dllDir;

	void printf(const char* fmt, ...);

	void Init();
}