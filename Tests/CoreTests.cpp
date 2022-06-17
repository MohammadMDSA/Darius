#define BOOST_TEST_MODULE CoreTests
#define BOOST_TEST_DYN_LINK

#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(MemoryTests)

BOOST_AUTO_TEST_CASE(sample)
{
    std::vector<int> a{ 1, 2 };
    std::vector<int> b{ 1, 2 };
    BOOST_TEST(a == b);
}

BOOST_AUTO_TEST_SUITE_END()