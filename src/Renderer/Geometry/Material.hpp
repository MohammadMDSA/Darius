#include <Math/VectorMath.hpp>
struct Material
{
	// Unique material name.
	std::wstring				Name;
	
	// Index into constant buffer corresponding to this material.
	int							MatCBIndex;
};