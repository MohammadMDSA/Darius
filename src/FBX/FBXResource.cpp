#include "pch.hpp"
#include "FBXResource.hpp"

#include "FbxLoader.hpp"

#include "FBXResource.sgenerated.hpp"

#if _D_EDITOR
#include <imgui.h>
#endif // _D_EDITOR



namespace Darius::Fbx
{

	D_CH_RESOURCE_DEF(FBXResource);

	D_RESOURCE::SubResourceConstructionData FBXResource::CanConstructFrom(D_RESOURCE::ResourceType type, D_FILE::Path const& path)
	{
		auto subs = GetResourcesDataFromFile(path);
		auto name = WSTR2STR(D_FILE::GetFileName(path));
		D_RESOURCE::ResourceDataInFile parent = D_RESOURCE::ResourceDataInFile
			{
				.Name = name,
				.Type = FBXResource::GetResourceType()
			};
		D_RESOURCE::SubResourceConstructionData result
		{
			.Parent = parent,
			.SubResources = subs
		};
		return result;
	}

	void FBXResource::ReadResourceFromFile(D_SERIALIZATION::Json const&, bool& dirtyDisk)
	{
		LoadSubResources(GetPath(), this);
	}

#ifdef _D_EDITOR
	bool FBXResource::DrawDetails(float params[])
	{
		if (mResourceDataInFile.size() == 0)
			mResourceDataInFile = GetResourcesDataFromFile(GetPath());

		if (ImGui::Button("Reimport", ImVec2(-1, 0)))
		{
			LoadSubResources(GetPath(), this);
		}

		if (ImGui::BeginTable("##ResourcesInFbxData", 3))
		{
			ImGui::TableSetupColumn(" ");
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("Type");

			ImGui::TableHeadersRow();

			static auto flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable;

			ImGuiListClipper clipper;
			clipper.Begin((int)mResourceDataInFile.size());

			while (clipper.Step())
			{
				for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
				{
					auto item = mResourceDataInFile.at(row);

					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					ImGui::Text(std::to_string(row + 1).c_str());

					ImGui::TableSetColumnIndex(1);
					ImGui::Text(item.Name.c_str());

					ImGui::TableSetColumnIndex(2);
					ImGui::Text(D_RESOURCE::ResourceTypeToString(item.Type).string());
				}
			}

			ImGui::EndTable();
		}

		return false;
	}
#endif // _D_EDITOR

	void FBXResource::Unload()
	{
		EvictFromGpu();
	}
}
