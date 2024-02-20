// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows 10 or later.
#define _WIN32_WINNT 0x0A00	// Change this to the appropriate value to target other versions of Windows.
#endif

// STL
#include <algorithm>
#include <chrono>
#include <format>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>
#include <optional>
#include <ranges>

// 3RD PARTY
#include <libpq-fe.h>