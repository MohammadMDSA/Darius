#include "Transform.hpp"

using namespace Darius::Math;

Vector3 Darius::Math::AffineTransform::GetScale()
{
	Vector3 scale;
	Quaternion rot;
	Vector3 pos;

	Decompose(scale, rot, pos);
	return scale;
}

Quaternion Darius::Math::AffineTransform::GetRotation()
{
	Vector3 scale;
	Quaternion rot;
	Vector3 pos;

	Decompose(scale, rot, pos);
	return rot;
}

Vector3 Darius::Math::AffineTransform::GetPosition()
{
	Vector3 scale;
	Quaternion rot;
	Vector3 pos;

	Decompose(scale, rot, pos);
	return pos;
}