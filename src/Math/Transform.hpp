#pragma once 
#include "Math.hpp"

namespace Darius
{
	namespace Math
	{
		class AffineTransform : Matrix
		{
			/// <summary>
			/// Returns Scale of transform. If possible, use `Decompose` method instead.
			/// </summary>
			/// <returns></returns>
			Vector3 GetScale();

			/// <summary>
			/// Returns Rotation of transform. 
			/// </summary>
			/// <returns></returns>
			Quaternion GetRotation();

			/// <summary>
			/// Returns Position of transform. If possible, use `Decompose` method instead.
			/// </summary>
			/// <returns></returns>
			Vector3 GetPosition();
		};

		using Transform = AffineTransform;
	}
}