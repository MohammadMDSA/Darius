#include <cassert>
#include <stdexcept>
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>

#define D_ASSERT(cond)			BOOST_ASSERT(cond)
#define D_ASSERT_M(cond, msg)	BOOST_ASSERT_MSG(cond, msg)
#define D_VERIFY(cond)			BOOST_VERIFY(cond)
#define D_VERIFY_M(cond, msg)	BOOST_VERIFY_MSG(cond, msg)
#define D_STATIC_ASSERT(cond)	BOOST_STATIC_ASSERT(cond)
#define D_STATIC_ASSERT_M(cond, msg) BOOST_STATIC_ASSERT_MSG(cond, msg)
