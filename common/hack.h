#pragma once

#include <utility>

// This file contains ugliness

namespace std {
	template <typename T1, typename T2>
	std::ostream& operator<<(std::ostream& strm, const std::pair<T1, T2>& pair)	{
		strm << "(" << pair.first << ", " << pair.second << ")";
		return strm;
	}
}
