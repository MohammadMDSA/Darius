#pragma once

#include <boost/signals2.hpp>

namespace Darius
{
	namespace Engine
	{
		namespace Core
		{
			template<typename T>
			using Signal = boost::signals2::signal<T>;
		}
	}
}