#include <cassert>
#include <stdexcept>

#define D_ASSERT(expr)			assert(expr)
#define D_ASSERT_M(expr, msg)	assert((expr)&&(msg))
#define D_STATIC_ASSERT(...)	static_assert(__VA_ARGS__, #__VA_ARGS__)
#define D_STATIC_ASSERT_M(...)	static_assert(__VA_ARGS__)
