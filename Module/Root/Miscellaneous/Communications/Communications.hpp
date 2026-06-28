#pragma once
#include <string>
#include <cstdint>
#include "Engine.hpp"
#include <Execution/Execution.hpp>

class CCommunications {
public:
	static void Initialize();
};

inline auto Communication = std::make_unique<CCommunications>();
