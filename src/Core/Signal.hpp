#pragma once

#include <boost/signals2.hpp>

#ifndef D_CORE
#define D_CORE Darius::Core
#endif // !D_UTILS

namespace Darius::Core
{

	template<typename T>
	using Signal = boost::signals2::signal<T>;

}