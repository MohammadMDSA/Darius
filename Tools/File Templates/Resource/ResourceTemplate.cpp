#include "pch.hpp"
#include "%%CLASS_NAME%%.hpp"

#include <Utils/Common.hpp>

#include <imgui.h>

namespace %%NAMESPACE%%
{

	D_CH_RESOURCE_DEF(%%CLASS_NAME%%);

	void %%CLASS_NAME%%::WriteResourceToFile() const
	{
		
	}

	void %%CLASS_NAME%%::ReadResourceFromFile()
	{
		
	}

	bool %%CLASS_NAME%%::UploadToGpu(D_GRAPHICS::GraphicsContext& context)
	{
		return true;
	}

	bool %%CLASS_NAME%%::DrawDetails(float params[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();

			D_H_DETAILS_DRAW_PROPERTY("Property Name");
			float dummy;
			valueChanged |= ImGui::InputFloat("##dummy", &dummy);

		D_H_DETAILS_DRAW_END_TABLE();

		return false;
	}
}