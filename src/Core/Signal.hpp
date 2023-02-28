#pragma once

#include <boost/signals2.hpp>

#include <functional>

#ifndef D_CORE
#define D_CORE Darius::Core
#endif // !D_UTILS

namespace Darius::Core
{

	template<typename T>
	using Signal = boost::signals2::signal<T>;
	using SignalConnection = boost::signals2::connection;

}

#define D_H_SIGNAL_DEFINITION(name, ...) \
D_CORE::Signal<__VA_ARGS__> name##Signal; \
SignalConnection SubscribeOn##name(std::function<__VA_ARGS__> callback) { return name##Signal.connect(callback); }

#define D_H_SIGNAL_DECL(name, ...) \
SignalConnection SubscribeOn##name(std::function<__VA_ARGS__> callback);

#define D_CH_SIGNAL(name, ...) \
private: \
D_CORE::Signal<__VA_ARGS__> name##Signal; \
public: \
INLINE SignalConnection SubscribeOn##name(std::function<__VA_ARGS__> callback) { return name##Signal.connect(callback); }