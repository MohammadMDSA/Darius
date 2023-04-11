#define BOOST_TEST_MODULE MathTests
#define BOOST_TEST_DYN_LINK

#include <Core/Serialization/TypeSerializer.hpp>
#include <Math/Matrix4.hpp>
#include <Math/Camera/Camera.hpp>

#include <rttr/registration.h>

#include <boost/test/included/unit_test.hpp>

using namespace D_MATH;
using namespace D_MATH_CAMERA;

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

	bool xRes = des.GetX().Equals({ 1.f, 2.f, 3.f });
	bool yRes = des.GetY().Equals({ 4.f, 5.f, 6.f });
	bool zRes = des.GetZ().Equals({ 7.f, 8.f, 9.f });
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

	bool xRes = des.GetX().Equals({ 1.f, 2.f, 3.f, 4.f });
	bool yRes = des.GetY().Equals({ 5.f, 6.f, 7.f, 8.f });
	bool zRes = des.GetZ().Equals({ 9.f, 10.f, 11.f, 12.f });
	bool wRes = des.GetW().Equals({ 13.f, 14.f, 15.f, 16.f });
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

BOOST_AUTO_TEST_CASE(CameraRef)
{
	D_MATH_CAMERA::Camera cam;

	cam.SetFOV(DirectX::XM_PI / 3);
	cam.SetZRange(0.001f, 10000.f);
	cam.SetPosition(Vector3(2.f, 2.f, 2.f));
	cam.SetLookDirection(Vector3(-2.f), Vector3::Up);
	cam.SetOrthographicSize(15.f);
	cam.SetOrthographic(true);
	cam.ReverseZ(false);
	cam.SetPosition({ 1.f, 2.f, 3.f });
	cam.SetRotation({ 4.f, 5.f, 6.f, 7.f });
	cam.SetAspectRatio(0.6666f);
	auto referenceRot = cam.GetRotation();

	rttr::type t = rttr::type::get<D_MATH_CAMERA::Camera>();

	#define	TEST_PROP(name, type, testValue) \
	{ \
		auto prop = t.get_property(name); \
		BOOST_TEST(prop.is_valid()); \
		auto value = prop.get_value(cam); \
		BOOST_TEST(value.is_valid()); \
		BOOST_TEST(value.is_type<type>()); \
		BOOST_TEST(value.get_value<type>() == testValue); \
	}

	TEST_PROP("VerticalFOV", float, DirectX::XM_PI / 3);
	TEST_PROP("AspectRatio", float, 0.6666f);
	TEST_PROP("NearClip", float, 0.001f);
	TEST_PROP("FarClip", float, 10000.f);
	TEST_PROP("OrthographicSize", float, 15.f);
	TEST_PROP("ReverseZ", bool, false);
	TEST_PROP("InfiniteZ", bool, false);
	TEST_PROP("Orthographic", bool, true);

	{
		auto prop = t.get_property("Position");
		BOOST_TEST(prop.is_valid());
		auto value = prop.get_value(cam);
		BOOST_TEST(value.is_valid());
		BOOST_TEST(value.is_type<Vector3>());
		BOOST_TEST(value.get_value<Vector3>().Equals({ 1.f, 2.f, 3.f }));
	}

	{
		auto prop = t.get_property("Rotation");
		BOOST_TEST(prop.is_valid());
		auto value = prop.get_value(cam);
		BOOST_TEST(value.is_valid());
		BOOST_TEST(value.is_type<Quaternion>());
		BOOST_TEST(value.get_value<Quaternion>().Equals(referenceRot));
	}

}

BOOST_AUTO_TEST_CASE(CameraSerialization)
{
	D_MATH_CAMERA::Camera cam;

	cam.SetFOV(DirectX::XM_PI / 3);
	cam.SetZRange(0.001f, 10000.f);
	cam.SetOrthographicSize(15.f);
	cam.SetOrthographic(true);
	cam.ReverseZ(false);
	cam.SetPosition({ 1.f, 2.f, 3.f });
	cam.SetRotation({ 4.f, 5.f, 6.f, 7.f });
	cam.SetAspectRatio(0.6666f);
	auto referenceRot = cam.GetRotation();

	rttr::type t = rttr::type::get<D_MATH_CAMERA::Camera>();

	D_SERIALIZATION::Json j;

	D_SERIALIZATION::Serialize(cam, j);

	auto dump = j.dump();

	D_MATH_CAMERA::Camera dest;

	D_SERIALIZATION::Deserialize(dest, j);

	BOOST_TEST(cam.GetFOV() == dest.GetFOV());
	BOOST_TEST(cam.GetNearClip() == dest.GetNearClip());
	BOOST_TEST(cam.GetFarClip() == dest.GetFarClip());
	BOOST_TEST(cam.GetOrthographicSize() == dest.GetOrthographicSize());
	BOOST_TEST(cam.IsOrthographic() == dest.IsOrthographic());
	BOOST_TEST(cam.IsReverseZ() == dest.IsReverseZ());
	BOOST_TEST(cam.IsInfiniteZ() == dest.IsInfiniteZ());
	BOOST_TEST(cam.GetAspectRatio() == dest.GetAspectRatio());
	BOOST_TEST(cam.GetPosition().Equals(dest.GetPosition()));
	BOOST_TEST(cam.GetRotation().Equals(dest.GetRotation()));
}

BOOST_AUTO_TEST_SUITE_END()