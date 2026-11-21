#pragma once
#ifndef MANGOHUD_STRING_COMPAT_H
#define MANGOHUD_STRING_COMPAT_H

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <string>

/* NDK r10e's gnustl is configured without _GLIBCXX_USE_C99, so <string>
 * does not declare std::stoi/stoll/stof or std::to_string. Bionic still
 * provides the underlying C functions, so wrap them here instead of
 * forcing the libstdc++ configuration macro. */

namespace mh
{

inline long long stoll(const std::string& str, std::size_t* idx = nullptr, int base = 10)
{
	char* end = nullptr;
	long long value = ::strtoll(str.c_str(), &end, base);
	if (idx)
		*idx = static_cast<std::size_t>(end - str.c_str());
	return value;
}

inline int stoi(const std::string& str, std::size_t* idx = nullptr, int base = 10)
{
	return static_cast<int>(mh::stoll(str, idx, base));
}

inline unsigned long stoul(const std::string& str, std::size_t* idx = nullptr, int base = 10)
{
	char* end = nullptr;
	unsigned long value = ::strtoul(str.c_str(), &end, base);
	if (idx)
		*idx = static_cast<std::size_t>(end - str.c_str());
	return value;
}

inline float stof(const std::string& str, std::size_t* idx = nullptr)
{
	char* end = nullptr;
	float value = ::strtof(str.c_str(), &end);
	if (idx)
		*idx = static_cast<std::size_t>(end - str.c_str());
	return value;
}

inline std::string to_string(int value)
{
	char buffer[32];
	std::snprintf(buffer, sizeof(buffer), "%d", value);
	return buffer;
}

inline std::string to_string(unsigned value)
{
	char buffer[32];
	std::snprintf(buffer, sizeof(buffer), "%u", value);
	return buffer;
}

inline std::string to_string(long value)
{
	char buffer[32];
	std::snprintf(buffer, sizeof(buffer), "%ld", value);
	return buffer;
}

inline std::string to_string(unsigned long value)
{
	char buffer[32];
	std::snprintf(buffer, sizeof(buffer), "%lu", value);
	return buffer;
}

inline std::string to_string(long long value)
{
	char buffer[32];
	std::snprintf(buffer, sizeof(buffer), "%lld", value);
	return buffer;
}

inline std::string to_string(unsigned long long value)
{
	char buffer[32];
	std::snprintf(buffer, sizeof(buffer), "%llu", value);
	return buffer;
}

} // namespace mh

#endif /* MANGOHUD_STRING_COMPAT_H */
