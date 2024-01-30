#pragma once

#include "Utils/Common.hpp"
#include "Vector.hpp"

#include <rttr/rttr_enable.h>

#include "Rotator.generated.hpp"

namespace Darius::Math
{
	class Quaternion;

	class DClass(Serialize) Rotator
	{
		GENERATED_BODY();
	public:
		using ElementType = Vector3::ElementType;

		INLINE Rotator() : mPitchYawRoll() { }
		explicit INLINE Rotator(float val) : mPitchYawRoll(val) { }
		explicit INLINE Rotator(float pitch, float yaw, float roll) : mPitchYawRoll(pitch, yaw, roll) { }
		explicit Rotator(Quaternion const& quat);
		explicit Rotator(Vector3 const& pitchYawRoll) : mPitchYawRoll(pitchYawRoll) { }

		// Binary Operators
		INLINE Rotator operator + (Rotator const& rhs) const { return Rotator(mPitchYawRoll + rhs.mPitchYawRoll); }
		INLINE Rotator operator - (Rotator const& rhs) const { return Rotator(mPitchYawRoll - rhs.mPitchYawRoll); }
		INLINE Rotator operator * (float scale) const { return Rotator(mPitchYawRoll * scale); }
		INLINE Rotator& operator *= (float scale) { mPitchYawRoll *= scale; return *this; }

		// Binary comparison operators
		INLINE bool operator == (Rotator const& other) const { return DirectX::XMVector3Equal(mPitchYawRoll, other.mPitchYawRoll); }
		INLINE bool operator != (Rotator const& other) const { return !(*this == other); }

		// Assignment operators
		INLINE Rotator& operator += (Rotator const& other) { mPitchYawRoll += other.mPitchYawRoll; return *this; }
		INLINE Rotator& operator -= (Rotator const& other) { mPitchYawRoll -= other.mPitchYawRoll; return *this; }

		INLINE bool IsNearZero(float tolerance) const { return mPitchYawRoll.IsNearZero(tolerance); }
		INLINE bool IsZero() const { return mPitchYawRoll.IsZero(); }
		INLINE bool Equls(Rotator const& other) const { return *this == other; }

		INLINE Rotator& Add(float deltaPitch, float deltaYaw, float deltaRoll) { return *this += Rotator(deltaPitch, deltaYaw, deltaRoll); }
		Rotator GetInverse() const;

		// Returns Pitch, Yaw, Roll
		INLINE Vector3 GetVector() const { return mPitchYawRoll; }
		// Returns Roll, Pitch, Yaw
		INLINE Vector3 Euler() const { return Vector3(mPitchYawRoll.GetZ(), mPitchYawRoll.GetZ(), mPitchYawRoll.GetY()); }
		void GetQuaternion(Quaternion & outQuat) const;

		Vector3 RotateVector(Vector3 const& vec) const;
		Vector3 UnrotateVector(Vector3 const& vec) const;
		Rotator Clamp() const;
		Rotator GetNormalized() const;
		Rotator GetDenormalized() const;
		void Normalize();

		INLINE float GetPitch() const { return mPitchYawRoll.GetX(); }
		INLINE float GetYaw() const { return mPitchYawRoll.GetY(); }
		INLINE float GetRoll() const { return mPitchYawRoll.GetZ(); }
		INLINE void SetPitch(float pitch) { mPitchYawRoll.SetX(pitch); }
		INLINE void SetYaw(float yaw) { mPitchYawRoll.SetY(yaw); }
		INLINE void SetRoll(float roll) { mPitchYawRoll.SetZ(roll); }

	public:
		// Statics
		static float ClampAxis(float angle);

		static float NormalizeAxis(float angle);

		// Accepts Roll, Pitch, Yaw
		INLINE static Rotator MakeFromEuler(Vector3 const& vec) { return Rotator(vec.GetY(), vec.GetZ(), vec.GetX()); }

		static const Rotator ZeroRotator;

	private:

		RTTR_REGISTRATION_FRIEND


		DField(Serialize, NotAnimate)
		Vector3					mPitchYawRoll;
	};
}