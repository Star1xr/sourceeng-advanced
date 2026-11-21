#pragma once
#ifndef MANGOHUD_TIME_COMPAT_H
#define MANGOHUD_TIME_COMPAT_H

#include <cstddef>
#include <ctime>
#include <string>
#include <vector>

/* NDK r10e's gnustl <iomanip> has no std::put_time, so provide the same
 * shape on top of the C strftime that bionic always exposes. */
namespace mh
{

inline std::string put_time(const std::tm* tm, const char* fmt)
{
	if (!tm || !fmt || fmt[0] == '\0')
		return std::string();

	std::vector<char> buffer(256);
	for (;;)
	{
		std::size_t len = std::strftime(buffer.data(), buffer.size(), fmt, tm);
		if (len > 0)
			return std::string(buffer.data(), len);
		if (buffer.size() >= 65536)
			return std::string();
		buffer.resize(buffer.size() * 2);
	}
}

} // namespace mh

#endif /* MANGOHUD_TIME_COMPAT_H */
