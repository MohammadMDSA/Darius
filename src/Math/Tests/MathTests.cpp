#define BOOST_TEST_MODULE MathTests
#define BOOST_TEST_DYN_LINK

#include <Core/Serialization/TypeSerializer.hpp>
#include <Math/Vector.hpp>

#include <rttr/registration.h>

#include <boost/test/included/unit_test.hpp>

using namespace Darius::Math;

BOOST_AUTO_TEST_SUITE(MathReflection)

BOOST_AUTO_TEST_CASE(Vec3GetRef)
{
    Vector3 v(1.f, 2.f, 3.f);
    rttr::type t = rttr::type::get<Vector3>();

    auto prop = t.get_property("X");
    BOOST_TEST(prop.is_valid());
    auto value = prop.get_value(v);
    BOOST_TEST(value.is_valid());
    BOOST_TEST(value.is_type<float>());
    BOOST_TEST(value.get_value<float>() == 1.f);

    prop = t.get_property("Y");
    BOOST_TEST(prop.is_valid());
    value = prop.get_value(v);
    BOOST_TEST(value.is_valid());
    BOOST_TEST(value.is_type<float>());
    BOOST_TEST(value.get_value<float>() == 2.f);

    prop = t.get_property("Z");
    BOOST_TEST(prop.is_valid());
    value = prop.get_value(v);
    BOOST_TEST(value.is_valid());
    BOOST_TEST(value.is_type<float>());
    BOOST_TEST(value.get_value<float>() == 3.f);

}

BOOST_AUTO_TEST_CASE(Vec4GetRef)
{
    Vector4 v(1.f, 2.f, 3.f, 4.f);
    rttr::type t = rttr::type::get<Vector4>();

    auto prop = t.get_property("X");
    BOOST_TEST(prop.is_valid());
    auto value = prop.get_value(v);
    BOOST_TEST(value.is_valid());
    BOOST_TEST(value.is_type<float>());
    BOOST_TEST(value.get_value<float>() == 1.f);

    prop = t.get_property("Y");
    BOOST_TEST(prop.is_valid());
    value = prop.get_value(v);
    BOOST_TEST(value.is_valid());
    BOOST_TEST(value.is_type<float>());
    BOOST_TEST(value.get_value<float>() == 2.f);

    prop = t.get_property("Z");
    BOOST_TEST(prop.is_valid());
    value = prop.get_value(v);
    BOOST_TEST(value.is_valid());
    BOOST_TEST(value.is_type<float>());
    BOOST_TEST(value.get_value<float>() == 3.f);

    prop = t.get_property("W");
    BOOST_TEST(prop.is_valid());
    value = prop.get_value(v);
    BOOST_TEST(value.is_valid());
    BOOST_TEST(value.is_type<float>());
    BOOST_TEST(value.get_value<float>() == 4.f);

}

BOOST_AUTO_TEST_CASE(Vec3Serialization)
{
    Vector3 v(1.f, 2.f, 3.f);

    D_SERIALIZATION::Json j;

    D_SERIALIZATION::Serialize(v, j);

    Vector3 des;
    D_SERIALIZATION::Deserialize(des, j);

    BOOST_TEST(des.GetX() == 1.f);
    BOOST_TEST(des.GetY() == 2.f);
    BOOST_TEST(des.GetZ() == 3.f);
}

BOOST_AUTO_TEST_CASE(Vec4Serialization)
{
    Vector4 v(1.f, 2.f, 3.f, 4.f);

    D_SERIALIZATION::Json j;

    D_SERIALIZATION::Serialize(v, j);

    Vector4 des;
    D_SERIALIZATION::Deserialize(des, j);

    BOOST_TEST(des.GetX() == 1.f);
    BOOST_TEST(des.GetY() == 2.f);
    BOOST_TEST(des.GetZ() == 3.f);
    BOOST_TEST(des.GetW() == 4.f);
}

BOOST_AUTO_TEST_SUITE_END()