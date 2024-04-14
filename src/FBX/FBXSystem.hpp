#pragma once

#include <Core/Serialization/Json.hpp>

#ifndef D_FBX
#define D_FBX Darius::Fbx
#endif // !D_FBX

namespace Darius::Fbx
{
	void Initialize(D_SERIALIZATION::Json const& settings);
	void Shutdown();
}