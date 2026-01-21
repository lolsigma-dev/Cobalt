#pragma once
#include "Includes.hpp"

class CVEH
{
public:
	static auto Start() -> bool;
	CVEH() = default;
};
inline auto VEH = std::make_unique<CVEH>();