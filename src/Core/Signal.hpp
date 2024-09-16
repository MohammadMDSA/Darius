#pragma once

#include <boost/signals2.hpp>

#include <mutex>
#include <functional>

#ifndef D_CORE
#define D_CORE Darius::Core
#endif // !D_UTILS

namespace Darius::Core
{

    template<typename Signature,
        typename Combiner=boost::signals2::optional_last_value<typename boost::function_traits<Signature>::result_type>,
        typename Group=int,
        typename GroupCompare=std::less<Group>,
        typename SlotFunction=std::function<Signature>,
        typename ExtendedSlotFunction=typename boost::signals2::detail::extended_signature<boost::function_traits<Signature>::arity, Signature>::function_type,
        typename Mutex=std::mutex>
	using Signal = boost::signals2::signal<Signature, Combiner, Group, GroupCompare, SlotFunction, ExtendedSlotFunction, Mutex>;

    using SignalConnection = boost::signals2::connection;
    using SignalScopedConnection = boost::signals2::scoped_connection;

}

#define D_H_SIGNAL_DEFINITION(name, ...) \
D_CORE::Signal<__VA_ARGS__> name##Signal; \
D_CORE::SignalConnection SubscribeOn##name(std::function<__VA_ARGS__> callback) { return name##Signal.connect(callback); }

#define D_H_SIGNAL_DECL(name, ...) \
D_CORE::SignalConnection SubscribeOn##name(std::function<__VA_ARGS__> callback);

#define D_CH_SIGNAL(name, ...) \
private: \
D_CORE::Signal<__VA_ARGS__> m##name##Signal; \
public: \
INLINE Darius::Core::SignalConnection SubscribeOn##name(std::function<__VA_ARGS__> callback) { return m##name##Signal.connect(callback); }