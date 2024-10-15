#include "Renderer/pch.hpp"
#include "ShaderMaterialResource.hpp"

#include <ResourceManager/ResourceManager.hpp>
#include <Utils/Assert.hpp>

#if _D_EDITOR
#include <imgui.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>
#include <ResourceManager/ResourceDragDropPayload.hpp>

#endif

namespace Darius::Renderer
{
	D_CH_RESOURCE_DEF(ShaderMaterialResource);

	ShaderMaterialResource::~ShaderMaterialResource()
	{
		mShaderCompileConnection.disconnect();
	}

	ShaderMaterialResource::ConstantBufferData::ConstantBufferData(size_t size) :
		mSize(size)
	{
		D_ASSERT(size % 16 == 0);
		mData = DMemAlloc(size);
		std::memset(mData, 0, size);
	}

	ShaderMaterialResource::ConstantBufferData::~ConstantBufferData()
	{
		DMemFree(mData);
	}

	ShaderMaterialResource::ShaderMaterialResource(D_CORE::Uuid const& uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, D_RESOURCE::Resource* parent, bool isDefault) :
		GenericMaterialResource(uuid, path, name, id, parent, isDefault)
	{}

	void ShaderMaterialResource::SetShader(ShaderResource* shader)
	{
		if (mShader == shader)
			return;

		mShaderCompileConnection.disconnect();

		mShader = shader;

		if (mShader.IsValid())
		{
			mShaderCompileConnection = mShader->SubscribeOnCompiled([&](ShaderResource* shader, std::shared_ptr<ShaderConstantParameters> preConstantParams) { OnShaderChanged(shader, preConstantParams); });

			if (!mShader->IsLoaded())
			{
				D_RESOURCE_LOADER::LoadResourceAsync(mShader.Get(), nullptr);
			}
			else
			{
				OnShaderChanged(mShader.Get(), nullptr);
			}
		}

		if (!IsLocked())
			MakeDiskDirty();

		SignalChange();
	}

	bool ShaderMaterialResource::WriteResourceToFile(D_SERIALIZATION::Json& data) const
	{
		using namespace D_SERIALIZATION;
		Json j;

		SerializeTextures(j["Textures"]);
		SerializeSamplers(j["Samplers"]);

		if (!mShader.IsNull())
			j["Shader"] = D_CORE::ToString(mShader->GetUuid());

		auto& constants = j["Constants"];
		SerializeConstants(constants);

		std::ofstream os(GetPath());
		os << j;
		os.close();

		return true;
	}

	void ShaderMaterialResource::ReadResourceFromFile(D_SERIALIZATION::Json const& data, bool& dirtyDisk)
	{
		using namespace D_SERIALIZATION;
		Json j;
		std::ifstream is(GetPath());
		is >> j;
		is.close();

		if (j.contains("Textures"))
			DeserializeTextures(j["Textures"]);

		if (j.contains("Samplers"))
			DeserializeSamplers(j["Samplers"]);

		if (j.contains("Shader"))
		{
			auto shader = D_RESOURCE::GetResourceSync<ShaderResource>(D_CORE::FromString(j["Shader"]));
			SetShader(shader.Get());

		}

		if (j.contains("Constants"))
			mSerializedConstants = j.at("Constants");
	}

	void ShaderMaterialResource::SerializeConstants(D_SERIALIZATION::Json& constants) const
	{
		if (mShader.IsNull() || !mShader->IsLoaded() || !mConstantBufferMemory)
			return;

		auto paramsMap = mShader->GetConstantParameters();
		paramsMap->Serialize(constants, { mConstantBufferMemory->mData, mConstantBufferMemory->mSize });
	}

	void ShaderMaterialResource::DeserializeConstants(D_SERIALIZATION::Json const& constants)
	{
		if (mShader.IsNull() || !mConstantBufferMemory)
			return;

		auto paramsMap = mShader->GetConstantParameters();
		if (!paramsMap)
			return;

		paramsMap->Deserialize(constants, { mConstantBufferMemory->mData, mConstantBufferMemory->mSize });
	}

	bool ShaderMaterialResource::IsPrepared() const
	{
		return IsLoaded() && (!mShader.IsNull() || mShader->IsLoaded());
	}

#if _D_EDITOR
	bool ShaderMaterialResource::DrawDetails(float params[])
	{

		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE("ResourceData");

		// Shader selection
		{
			D_H_DETAILS_DRAW_PROPERTY("Shader");
			D_H_RESOURCE_SELECTION_DRAW(ShaderResource, mShader, "Select Shader", SetShader);

		}

		D_H_DETAILS_DRAW_END_TABLE();

		if (!mShader.IsNull() && mShader->IsLoaded())
		{
			auto constantParams = mShader->GetConstantParameters();
			if (constantParams)
			{
				auto constantsValueChanged = constantParams->DrawDetails(mConstantBufferMemory->GetShaderConstantPropertyBuffer());
				if (constantsValueChanged)
				{
					valueChanged = true;
					constantParams->Serialize(mSerializedConstants, mConstantBufferMemory->GetShaderConstantPropertyBuffer());
					MakeDiskDirty();
					MakeGpuDirty();
				}
			}
		}

		return valueChanged;
	}
#endif // _D_EDITOR


	void ShaderMaterialResource::OnShaderChanged(ShaderResource* shader, std::shared_ptr< ShaderConstantParameters> preConstantParams)
	{
		if (!mShader.IsValid())
		{
			mConstantBufferMemory = nullptr;

			return;
		}

		auto constantBufferDataSize = mShader->GetConstantParamsBufferSize();

		if (constantBufferDataSize == 0)
			return;

		// Creating constant buffer cpu memory
		auto newBufferMemory = std::make_unique<ConstantBufferData>(constantBufferDataSize);

		auto newParameters = shader->GetConstantParameters();

		if (preConstantParams)
		{
			// Copying params data to new memory
			preConstantParams->CopyTo(newParameters.get(), { mConstantBufferMemory->mData, mConstantBufferMemory->mSize }, { newBufferMemory->mData, newBufferMemory->mSize });
		}
		else
		{
			newParameters->Deserialize(mSerializedConstants, { newBufferMemory->mData, newBufferMemory->mSize });
		}

		mConstantBufferMemory = std::move(newBufferMemory);

		MakeGpuDirty();
	}

}
