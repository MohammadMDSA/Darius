#define BOOST_TEST_MODULE MathTests
#define BOOST_TEST_DYN_LINK

#include <Core/Serialization/TypeSerializer.hpp>
#include <Math/Matrix4.hpp>

#include <rttr/registration.h>

#include <boost/test/included/unit_test.hpp>

using namespace Darius::Math;

BOOST_AUTO_TEST_SUITE(MathReflection)

BOOST_AUTO_TEST_CASE(Vec3GetRef)
{
	auto f = (Vector3::Up);
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
	auto f = (Vector4::Up);

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

BOOST_AUTO_TEST_CASE(QuatGetRef)
{
	auto f = (Quaternion::Identity);

	Quaternion v(1.f, 2.f, 3.f, 4.f);
	rttr::type t = rttr::type::get<Quaternion>();

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
	auto f = (Vector3::Up);
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
	auto f = (Vector4::Up);
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

BOOST_AUTO_TEST_CASE(Mat3Serialization)
{
	auto f = (Matrix3::Identity);
	Matrix3 v({ 1.f, 2.f, 3.f }, { 4.f, 5.f, 6.f }, { 7.f, 8.f, 9.f });

	D_SERIALIZATION::Json j;

	D_SERIALIZATION::Serialize(v, j);

	Matrix3 des;
	D_SERIALIZATION::Deserialize(des, j);

	bool xRes = des.GetX() == Vector3(1.f, 2.f, 3.f);
	bool yRes = des.GetY() == Vector3(4.f, 5.f, 6.f);
	bool zRes = des.GetZ() == Vector3(7.f, 8.f, 9.f);
	BOOST_TEST(xRes);
	BOOST_TEST(yRes);
	BOOST_TEST(zRes);
}

BOOST_AUTO_TEST_CASE(Mat4Serialization)
{
	auto f = (Matrix4::Identity);
	Matrix4 v(
		{ 1.f, 2.f, 3.f, 4.f },
		{ 5.f, 6.f, 7.f, 8.f }, 
		{ 9.f, 10.f, 11.f, 12.f },
		{ 13.f, 14.f, 15.f, 16.f });

	D_SERIALIZATION::Json j;

	D_SERIALIZATION::Serialize(v, j);

	Matrix4 des;
	D_SERIALIZATION::Deserialize(des, j);

	bool xRes = des.GetX() == Vector4(1.f, 2.f, 3.f, 4.f);
	bool yRes = des.GetY() == Vector4(5.f, 6.f, 7.f, 8.f);
	bool zRes = des.GetZ() == Vector4(9.f, 10.f, 11.f, 12.f);
	bool wRes = des.GetW() == Vector4(13.f, 14.f, 15.f, 16.f);
	BOOST_TEST(xRes);
	BOOST_TEST(yRes);
	BOOST_TEST(zRes);
	BOOST_TEST(wRes);

	auto str = j.dump();
}

BOOST_AUTO_TEST_CASE(QuatSerialization)
{
	auto f = (Quaternion::Identity);
	Quaternion v(1.f, 2.f, 3.f, 4.f);

	D_SERIALIZATION::Json j;

	D_SERIALIZATION::Serialize(v, j);

	Quaternion des;
	D_SERIALIZATION::Deserialize(des, j);

	BOOST_TEST(des.GetX() == 1.f);
	BOOST_TEST(des.GetY() == 2.f);
	BOOST_TEST(des.GetZ() == 3.f);
	BOOST_TEST(des.GetW() == 4.f);
}

BOOST_AUTO_TEST_CASE(ScalarSerialization)
{
	auto f = (Vector3::Up);
	Scalar s = 5.2f;
	D_SERIALIZATION::Json j;

	D_SERIALIZATION::Serialize(s, j);

	Scalar dest;
	D_SERIALIZATION::Deserialize(dest, j);

	BOOST_TEST(dest == 5.2f);
}


BOOST_AUTO_TEST_SUITE_END()