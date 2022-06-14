#include <cassert>
#include <stdexcept>

#ifdef _DEBUG
#define D_Assert(cond)		assert(cond); _assume(cond)
#else
#define D_Assert(cond)		_assume(cond)
#endif

#define D_Error(msg, ...)	throw std::runtime_error(msg)

#define D_Verify(cond)		if (!(cond)) { D_Error("Condition failed: " #cond); } _assume(cond)