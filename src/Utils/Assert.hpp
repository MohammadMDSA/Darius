#include <cassert>
#include <stdexcept>

#define D_ASSUME(expr)			__assume(expr)
#define D_PLATFORM_BREAK()		(__nop(), __debugbreak())
#define D_LIKELY(expr)			(!!(expr))

// Assert
#if _DEBUG
#define D_ASSERT_IMPL(expr)			assert(expr)
#define D_ASSERT_M_IMPL(expr, msg)	assert((expr)&&(msg))
#else
#define D_ASSERT(expr)				D_ASSUME(expr)
#define D_ASSERT_M(expr, msg)		D_ASSUME(expr)
#endif

#define D_ASSERT(expr)			D_ASSERT_IMPL(expr)
#define D_ASSERT_M(expr, msg)	D_ASSERT_M_IMPL(expr, msg)

// Verify
#if _DEBUG
#define D_VERIFY_IMPL(expr) \
		(D_LIKELY(!!(expr)) || []() { D_PLATFORM_BREAK(); D_ASSERT(false); return false; } ())
#else
#define D_VERIFY_IMPL(expr)			D_LIKELY(!!(expr))
#endif // _DEBUG

#define D_VERIFY(expr)			D_VERIFY_IMPL(expr)

// Static Assert
#define D_STATIC_ASSERT(...)	static_assert(__VA_ARGS__, #__VA_ARGS__)
#define D_STATIC_ASSERT_M(...)	static_assert(__VA_ARGS__)