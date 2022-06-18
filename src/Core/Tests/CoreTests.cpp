#define BOOST_TEST_MODULE CoreTests
#define BOOST_TEST_DYN_LINK

#include <boost/test/included/unit_test.hpp>
#include <Memory.hpp>

using namespace Darius::Core::Memory;

inline void test(std::size_t alignment)
{
    {
        void* p = Darius::Core::Memory::aligned_alloc(alignment, alignment + 1);
        BOOST_TEST(p);
        BOOST_TEST(Darius::Core::Memory::is_aligned(p, alignment));
        std::memset(p, 0, alignment);
        Darius::Core::Memory::aligned_free(p);
    }
    {
        void* p = Darius::Core::Memory::aligned_alloc(alignment, 1);
        BOOST_TEST(p);
        BOOST_TEST(Darius::Core::Memory::is_aligned(p, alignment));
        std::memset(p, 0, 1);
        Darius::Core::Memory::aligned_free(p);
    }
    {
        void* p = Darius::Core::Memory::aligned_alloc(alignment, 0);
        Darius::Core::Memory::aligned_free(p);
    }
}

BOOST_AUTO_TEST_SUITE(MemoryAlloc)

BOOST_AUTO_TEST_CASE(Alloc1)
{
    test(1);
}

BOOST_AUTO_TEST_CASE(Alloc2)
{
    test(2);
}

BOOST_AUTO_TEST_CASE(Alloc4)
{
    test(4);
}

BOOST_AUTO_TEST_CASE(Alloc8)
{
    test(8);
}

BOOST_AUTO_TEST_CASE(Alloc16)
{
    test(16);
}

BOOST_AUTO_TEST_CASE(Alloc32)
{
    test(32);
}

BOOST_AUTO_TEST_CASE(Alloc64)
{
    test(64);
}

BOOST_AUTO_TEST_CASE(Alloc128)
{
    test(128);
}

BOOST_AUTO_TEST_SUITE_END()