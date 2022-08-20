#include "pch.hpp"
#include "Resource.hpp"

namespace Darius::ResourceManager
{
	
	void Resource::UpdateGPU(D_GRAPHICS::GraphicsContext& context)
	{
		// Is gpu already up to date
		if (!mDirtyGPU)
			return;

		UploadToGpu(context);
		mDirtyGPU = false;
	}

}
