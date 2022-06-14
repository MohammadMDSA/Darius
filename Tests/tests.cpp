#define BOOST_TEST_MODULE MainTest
#include <boost/test/included/unit_test.hpp>

int add(int i, int j) { return i + j; }

BOOST_AUTO_TEST_CASE(your_test_case) {
    std::vector<int> a{ 1, 2 };
    std::vector<int> b{ 1, 2 };
    BOOST_TEST(a == b);
}