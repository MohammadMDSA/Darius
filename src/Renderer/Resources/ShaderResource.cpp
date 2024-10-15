#include "Renderer/pch.hpp"
#include "ShaderResource.hpp"

#include <Core/Filesystem/FileUtils.hpp>
#include <Core/Serialization/TypeSerializer.hpp>
#include <Graphics/GraphicsCore.hpp>
#include <Graphics/GraphicsUtils/Shader/ShaderFactory.hpp>

#include <sstream>

#if _D_EDITOR
#include <imgui.h>
#endif

#include "ShaderResource.sgenerated.hpp"

namespace Darius::Renderer
{
	D_CH_RESOURCE_DEF(ShaderResource);

#pragma region Constant Property


	template<typename T>
	class ShaderConstantPropertyGeneric : public ShaderConstantProperty
	{
	public:
		INLINE ShaderConstantPropertyGeneric(D_CORE::StringId const& parameterName, size_t offset) :
			ShaderConstantProperty(parameterName, offset)
		{
		}

		INLINE virtual size_t		GetSizeInConstantBuffer() const override { return sizeof(T); }

		INLINE virtual void* GetValuePtr(void* buffer) const override { return GetDataPtr(buffer); }

	protected:
		INLINE T const* GetDataPtr(void const* buffer) const { return reinterpret_cast<T const*>(reinterpret_cast<std::byte const*>(buffer) + GetBufferOffset()); }
		INLINE T* GetDataPtr(void* buffer) const { return reinterpret_cast<T*>(reinterpret_cast<std::byte*>(buffer) + GetBufferOffset()); }
	};

	template<typename T>
	class IntShaderConstantPropertyGeneric : public ShaderConstantPropertyGeneric<T>
	{
	public:
		INLINE IntShaderConstantPropertyGeneric(D_CORE::StringId const& parameterName, size_t offset) :
			ShaderConstantPropertyGeneric<T>(parameterName, offset) {}

		INLINE virtual bool			Deserialize(D_SERIALIZATION::Json const& json, void* buffer) const override
		{
			if (!json.is_number_integer())
			{
				*ShaderConstantPropertyGeneric<T>::GetDataPtr(buffer) = 0;
				return false;
			}

			*ShaderConstantPropertyGeneric<T>::GetDataPtr(buffer) = json.get<T>();
			return true;
		}

		INLINE virtual void			Serialize(D_SERIALIZATION::Json& json, void const* buffer) const override
		{
			json = *ShaderConstantPropertyGeneric<T>::GetDataPtr(buffer);
		}

	};

	template<typename T>
	class UintShaderConstantPropertyGeneric : public ShaderConstantPropertyGeneric<T>
	{
	public:
		INLINE UintShaderConstantPropertyGeneric(D_CORE::StringId const& parameterName, size_t offset) :
			ShaderConstantPropertyGeneric<T>(parameterName, offset) {}

		INLINE virtual bool			Deserialize(D_SERIALIZATION::Json const& json, void* buffer) const override
		{
			if (!json.is_number_unsigned())
			{
				*ShaderConstantPropertyGeneric<T>::GetDataPtr(buffer) = 0u;
				return false;
			}

			*ShaderConstantPropertyGeneric<T>::GetDataPtr(buffer) = json.get<T>();
			return true;
		}

		INLINE virtual void			Serialize(D_SERIALIZATION::Json& json, void const* buffer) const override
		{
			json = *ShaderConstantPropertyGeneric<T>::GetDataPtr(buffer);
		}

	};

	template<typename T>
	class FloatingPointShaderConstantPropertyGeneric : public ShaderConstantPropertyGeneric<T>
	{
	public:
		INLINE FloatingPointShaderConstantPropertyGeneric(D_CORE::StringId const& parameterName, size_t offset) :
			ShaderConstantPropertyGeneric<T>(parameterName, offset) {}

		INLINE virtual bool			Deserialize(D_SERIALIZATION::Json const& json, void* buffer) const override
		{
			if (!json.is_number_float())
			{
				*ShaderConstantPropertyGeneric<T>::GetDataPtr(buffer) = 0;
				return false;
			}

			*ShaderConstantPropertyGeneric<T>::GetDataPtr(buffer) = json.get<T>();
			return true;
		}

		INLINE virtual void			Serialize(D_SERIALIZATION::Json& json, void const* buffer) const override
		{
			json = *ShaderConstantPropertyGeneric<T>::GetDataPtr(buffer);
		}

	};

	template<typename T>
	class VectorShaderConstantPropertyGeneric : public ShaderConstantPropertyGeneric<T>
	{
	public:
		INLINE VectorShaderConstantPropertyGeneric(D_CORE::StringId const& parameterName, size_t offset) :
			ShaderConstantPropertyGeneric<T>(parameterName, offset) {}

		INLINE virtual bool			Deserialize(D_SERIALIZATION::Json const& json, void* buffer) const override
		{
			T value;
			D_SERIALIZATION::Deserialize(value, json);
			*ShaderConstantPropertyGeneric<T>::GetDataPtr(buffer) = value;
			return true;
		}

		INLINE virtual void			Serialize(D_SERIALIZATION::Json& json, void const* buffer) const override
		{
			T value = *ShaderConstantPropertyGeneric<T>::GetDataPtr(buffer);
			D_SERIALIZATION::Serialize(value, json);
		}
	};

	class BoolShaderConstantProperty : public ShaderConstantPropertyGeneric<bool>
	{
	public:
		INLINE BoolShaderConstantProperty(D_CORE::StringId const& parameterName, size_t offset) :
			ShaderConstantPropertyGeneric<bool>(parameterName, offset) {}

		INLINE virtual bool			Deserialize(D_SERIALIZATION::Json const& json, void* buffer) const override
		{
			if (!json.is_boolean())
				return false;

			*GetDataPtr(buffer) = json.get<bool>();
			return true;
		}

		INLINE virtual void			Serialize(D_SERIALIZATION::Json& json, void const* buffer) const override
		{
			json = *GetDataPtr(buffer);
		}

		INLINE virtual std::string	GetParameterTypeName() const override { return "Bool"; }

#if _D_EDITOR
		bool DrawDetails(void* buffer) override
		{
			return ImGui::Checkbox(GetParameterName().string(), GetDataPtr(buffer));
		}

#endif // _D_EDITOR
	};

	class FloatShaderConstantProperty : public FloatingPointShaderConstantPropertyGeneric<float>
	{
	public:
		INLINE FloatShaderConstantProperty(D_CORE::StringId const& parameterName, size_t offset) :
			FloatingPointShaderConstantPropertyGeneric<float>(parameterName, offset) {}

		INLINE virtual std::string	GetParameterTypeName() const override { return "Float"; }

#if _D_EDITOR
		bool DrawDetails(void* buffer) override
		{
			return ImGui::DragFloat(GetParameterName().string(), GetDataPtr(buffer));
		}

#endif // _D_EDITOR
	};

	class DoubleShaderConstantProperty : public FloatingPointShaderConstantPropertyGeneric<double>
	{
	public:
		INLINE DoubleShaderConstantProperty(D_CORE::StringId const& parameterName, size_t offset) :
			FloatingPointShaderConstantPropertyGeneric<double>(parameterName, offset) {}

		INLINE virtual std::string	GetParameterTypeName() const override { return "Double"; }

#if _D_EDITOR
		bool DrawDetails(void* buffer) override
		{
			float value = (float)*GetDataPtr(buffer);
			bool changed = ImGui::DragFloat(GetParameterName().string(), &value);
			*GetDataPtr(buffer) = value;
			return changed;
		}

#endif // _D_EDITOR
	};

	class IntShaderConstantProperty : public IntShaderConstantPropertyGeneric<int>
	{
	public:
		INLINE IntShaderConstantProperty(D_CORE::StringId const& parameterName, size_t offset) :
			IntShaderConstantPropertyGeneric<int>(parameterName, offset) {}

		INLINE virtual std::string	GetParameterTypeName() const override { return "Int"; }

#if _D_EDITOR
		bool DrawDetails(void* buffer) override
		{
			return ImGui::DragScalar(GetParameterName().string(), ImGuiDataType_S32, GetDataPtr(buffer));
		}

#endif // _D_EDITOR
	};

	class Int16ShaderConstantProperty : public IntShaderConstantPropertyGeneric<int16_t>
	{
	public:
		INLINE Int16ShaderConstantProperty(D_CORE::StringId const& parameterName, size_t offset) :
			IntShaderConstantPropertyGeneric<int16_t>(parameterName, offset) {}

		INLINE virtual std::string	GetParameterTypeName() const override { return "Int16"; }

#if _D_EDITOR
		bool DrawDetails(void* buffer) override
		{
			return ImGui::DragScalar(GetParameterName().string(), ImGuiDataType_S16, GetDataPtr(buffer));
		}

#endif // _D_EDITOR
	};

	class Int64ShaderConstantProperty : public IntShaderConstantPropertyGeneric<int64_t>
	{
	public:
		INLINE Int64ShaderConstantProperty(D_CORE::StringId const& parameterName, size_t offset) :
			IntShaderConstantPropertyGeneric<int64_t>(parameterName, offset) {}

		INLINE virtual std::string	GetParameterTypeName() const override { return "Int64"; }

#if _D_EDITOR
		bool DrawDetails(void* buffer) override
		{
			return ImGui::DragScalar(GetParameterName().string(), ImGuiDataType_S64, GetDataPtr(buffer));
		}

#endif // _D_EDITOR
	};

	class Uint8ShaderConstantProperty : public UintShaderConstantPropertyGeneric<uint8_t>
	{
	public:
		INLINE Uint8ShaderConstantProperty(D_CORE::StringId const& parameterName, size_t offset) :
			UintShaderConstantPropertyGeneric<uint8_t>(parameterName, offset) {}

		INLINE virtual std::string	GetParameterTypeName() const override { return "Uint8"; }

#if _D_EDITOR
		bool DrawDetails(void* buffer) override
		{
			return ImGui::DragScalar(GetParameterName().string(), ImGuiDataType_U8, GetDataPtr(buffer));
		}

#endif // _D_EDITOR
	};

	class Uint16ShaderConstantProperty : public UintShaderConstantPropertyGeneric<uint16_t>
	{
	public:
		INLINE Uint16ShaderConstantProperty(D_CORE::StringId const& parameterName, size_t offset) :
			UintShaderConstantPropertyGeneric<uint16_t>(parameterName, offset) {}

		INLINE virtual std::string	GetParameterTypeName() const override { return "Uint16"; }

#if _D_EDITOR
		bool DrawDetails(void* buffer) override
		{
			return ImGui::DragScalar(GetParameterName().string(), ImGuiDataType_U16, GetDataPtr(buffer));
		}

#endif // _D_EDITOR
	};

	class UintShaderConstantProperty : public UintShaderConstantPropertyGeneric<uint32_t>
	{
	public:
		INLINE UintShaderConstantProperty(D_CORE::StringId const& parameterName, size_t offset) :
			UintShaderConstantPropertyGeneric<uint32_t>(parameterName, offset) {}

		INLINE virtual std::string	GetParameterTypeName() const override { return "Uint"; }

#if _D_EDITOR
		bool DrawDetails(void* buffer) override
		{
			return ImGui::DragScalar(GetParameterName().string(), ImGuiDataType_U32, GetDataPtr(buffer));
		}

#endif // _D_EDITOR
	};

	class Uint64ShaderConstantProperty : public UintShaderConstantPropertyGeneric<uint64_t>
	{
	public:
		INLINE Uint64ShaderConstantProperty(D_CORE::StringId const& parameterName, size_t offset) :
			UintShaderConstantPropertyGeneric<uint64_t>(parameterName, offset) {}

		INLINE virtual std::string	GetParameterTypeName() const override { return "Uint64"; }

#if _D_EDITOR
		bool DrawDetails(void* buffer) override
		{
			return ImGui::DragScalar(GetParameterName().string(), ImGuiDataType_U64, GetDataPtr(buffer));
		}

#endif // _D_EDITOR
	};

	class Vec2ShaderConstantProperty : public VectorShaderConstantPropertyGeneric<D_MATH::Vector2>
	{
	public:
		INLINE Vec2ShaderConstantProperty(D_CORE::StringId const& parameterName, size_t offset) :
			VectorShaderConstantPropertyGeneric<D_MATH::Vector2>(parameterName, offset) {}

		INLINE virtual std::string	GetParameterTypeName() const override { return "Vec2"; }

#if _D_EDITOR
		bool DrawDetails(void* buffer) override
		{
			return D_MATH::DrawDetails(*GetDataPtr(buffer));
		}

#endif // _D_EDITOR
	};

	class Vec3ShaderConstantProperty : public VectorShaderConstantPropertyGeneric<D_MATH::Vector3>
	{
	public:
		INLINE Vec3ShaderConstantProperty(D_CORE::StringId const& parameterName, size_t offset) :
			VectorShaderConstantPropertyGeneric<D_MATH::Vector3>(parameterName, offset) {}

		INLINE virtual std::string	GetParameterTypeName() const override { return "Vec3"; }
		INLINE virtual size_t		GetSizeInConstantBuffer() const override { return 3 * sizeof(D_MATH::Vector3::ElementType); }

#if _D_EDITOR
		bool DrawDetails(void* buffer) override
		{
			return D_MATH::DrawDetails(*GetDataPtr(buffer));
		}

#endif // _D_EDITOR
	};

	class Vec4ShaderConstantProperty : public VectorShaderConstantPropertyGeneric<D_MATH::Vector4>
	{
	public:
		INLINE Vec4ShaderConstantProperty(D_CORE::StringId const& parameterName, size_t offset) :
			VectorShaderConstantPropertyGeneric<D_MATH::Vector4>(parameterName, offset) {}

		INLINE virtual std::string	GetParameterTypeName() const override { return "Vec4"; }

#if _D_EDITOR
		bool DrawDetails(void* buffer) override
		{
			return D_MATH::DrawDetails(*GetDataPtr(buffer));
		}

#endif // _D_EDITOR
	};

	std::shared_ptr<ShaderConstantProperty> CreatePropertyFromVariableData(ID3D12ShaderReflectionVariable* variable)
	{
		auto typeData = variable->GetType();

		D3D12_SHADER_VARIABLE_DESC variableDesc;
		variable->GetDesc(&variableDesc);

		D3D12_SHADER_TYPE_DESC typeDesc;
		typeData->GetDesc(&typeDesc);

		if (typeDesc.Class > D3D_SVC_VECTOR)
			return nullptr;

		ShaderConstantProperty* property = nullptr;

		if (typeDesc.Class == D3D_SVC_SCALAR)
		{
			switch (typeDesc.Type)
			{
			case D3D_SVT_BOOL:
				property = new BoolShaderConstantProperty(D_CORE::StringId(variableDesc.Name), (size_t)variableDesc.StartOffset);
				break;
			case D3D_SVT_INT:
				property = new IntShaderConstantProperty(D_CORE::StringId(variableDesc.Name), (size_t)variableDesc.StartOffset);
				break;
			case D3D_SVT_FLOAT:
				property = new FloatShaderConstantProperty(D_CORE::StringId(variableDesc.Name), (size_t)variableDesc.StartOffset);
				break;
			case D3D_SVT_UINT:
				property = new UintShaderConstantProperty(D_CORE::StringId(variableDesc.Name), (size_t)variableDesc.StartOffset);
				break;
			case D3D_SVT_UINT8:
				property = new Uint8ShaderConstantProperty(D_CORE::StringId(variableDesc.Name), (size_t)variableDesc.StartOffset);
				break;
			case D3D_SVT_DOUBLE:
				property = new DoubleShaderConstantProperty(D_CORE::StringId(variableDesc.Name), (size_t)variableDesc.StartOffset);
				break;
			case D3D_SVT_INT16:
				property = new Int16ShaderConstantProperty(D_CORE::StringId(variableDesc.Name), (size_t)variableDesc.StartOffset);
				break;
			case D3D_SVT_UINT16:
				property = new Uint16ShaderConstantProperty(D_CORE::StringId(variableDesc.Name), (size_t)variableDesc.StartOffset);
				break;
			case D3D_SVT_INT64:
				property = new Int64ShaderConstantProperty(D_CORE::StringId(variableDesc.Name), (size_t)variableDesc.StartOffset);
				break;
			case D3D_SVT_UINT64:
				property = new Uint64ShaderConstantProperty(D_CORE::StringId(variableDesc.Name), (size_t)variableDesc.StartOffset);
				break;
			default:
				return nullptr;
			}
		}
		else if (typeDesc.Class = D3D_SVC_VECTOR)
		{
			switch (typeDesc.Type)
			{

			case D3D_SVT_FLOAT:
				if (typeDesc.Columns == 2)
					property = new Vec2ShaderConstantProperty(D_CORE::StringId(variableDesc.Name), (size_t)variableDesc.StartOffset);

				else if (typeDesc.Columns == 3)
					property = new Vec3ShaderConstantProperty(D_CORE::StringId(variableDesc.Name), (size_t)variableDesc.StartOffset);

				else if (typeDesc.Columns == 4)
					property = new Vec4ShaderConstantProperty(D_CORE::StringId(variableDesc.Name), (size_t)variableDesc.StartOffset);
				break;
			default:
				return nullptr;
			}
		}

		D_ASSERT(property);
		size_t propertySizeInBuffer = property->GetSizeInConstantBuffer();
		D_VERIFY(propertySizeInBuffer <= variableDesc.Size);

		return std::shared_ptr<ShaderConstantProperty>(property);
	}


#pragma endregion Constant Property

	void ShaderConstantParameters::Serialize(D_SERIALIZATION::Json& constantsJson, ShaderConstantPropertyBuffer const& buffer) const
	{
		for (auto const& [name, param] : mPropertyMap)
		{
			D_ASSERT(buffer.BufferSize >= param->GetBufferOffset() + param->GetSizeInConstantBuffer());

			auto& token = constantsJson[name.string()];
			param->Serialize(token, buffer.BufferMemory);
		}
	}

	void ShaderConstantParameters::Deserialize(D_SERIALIZATION::Json const& constantsJson, ShaderConstantPropertyBuffer const& buffer) const
	{
		for (auto& [name, param] : mPropertyMap)
		{
			D_ASSERT(buffer.BufferSize >= param->GetBufferOffset() + param->GetSizeInConstantBuffer());

			auto key = name.string();
			if (!constantsJson.contains(key))
				continue;

			param->Deserialize(constantsJson.at(key), buffer.BufferMemory);
		}
	}

	void ShaderConstantParameters::CopyTo(ShaderConstantParameters* destParam, ShaderConstantPropertyBuffer const& srcBuffer, ShaderConstantPropertyBuffer const& destBuffer) const
	{
		// Copying previous prop values to new ones if the type is the same
		for (auto& [name, destParam] : destParam->mPropertyMap)
		{
			auto search = mPropertyMap.find(name);
			if (search == mPropertyMap.end())
				continue;

			auto foundSrcParam = search->second.get();
			D_ASSERT(srcBuffer.BufferSize >= foundSrcParam->GetSizeInConstantBuffer() + foundSrcParam->GetBufferOffset());

			// If the type has changed, don't copy
			if (typeid(*destParam.get()) != typeid(*foundSrcParam))
				continue;

			D_ASSERT(destBuffer.BufferSize >= destParam->GetSizeInConstantBuffer() + destParam->GetBufferOffset());

			foundSrcParam->CopyTo(destParam.get(), srcBuffer.BufferMemory, destBuffer.BufferMemory);
		}
	}

#if _D_EDITOR
	bool ShaderConstantParameters::DrawDetails(ShaderConstantPropertyBuffer const& buffer)
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE("Constants");

		if (mPropertyMap.size() > 0)
		{
			D_H_DETAILS_DRAW_PROPERTY("Constants:");

			for (auto& [name, prop] : mPropertyMap)
			{
				D_ASSERT(buffer.BufferSize >= prop->GetBufferOffset() + prop->GetSizeInConstantBuffer());

				D_H_DETAILS_DRAW_PROPERTY(name.string());
				valueChanged |= prop->DrawDetails(buffer.BufferMemory);
			}
		}
		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	}
#endif // _D_EDITOR

	bool ShaderConstantParameters::AddProperty(std::shared_ptr<ShaderConstantProperty> prop)
	{
		D_ASSERT(prop);

		auto name = prop->GetParameterName();
		if (mPropertyMap.contains(name))
			return false;

		mPropertyMap.emplace(name, prop);
	}


	bool ShaderResource::WriteResourceToFile(D_SERIALIZATION::Json& json) const
	{
		return D_FILE::WriteFileHelper(GetPath(), mCode);
	}

	void ShaderResource::ReadResourceFromFile(D_SERIALIZATION::Json const& json, bool& dirtyDisk)
	{
		dirtyDisk = false;
		mCode = D_FILE::ReadFileSync(GetPath());

		CompileShader(false);

	}

	bool ShaderResource::CompileShader(bool force)
	{
		if (!D_VERIFY(mCode != nullptr))
			return false;

		D_GRAPHICS_SHADERS::ShaderCompileConfig config
		{
			.EntryPoint = "__main__"_SId,
			.Path = GetPath(),
			.ForceDisableOptimization = true
		};

		mCompileMessage = "";
		auto templateShader = D_GRAPHICS::GetShaderFactory()->CompilePixelShader(config, force, mCode);

		if (!templateShader)
			return false;

		mShaderReflectionData = templateShader->GetReflectionData();
		mCompileMessage = templateShader->GetCompileLog();

		mConstantParamsBufferSize = 0;
		if (mShaderReflectionData)
		{
			auto constantBuffer = mShaderReflectionData->GetConstantBufferByName(GetConstantBufferName().c_str());
			if (constantBuffer)
			{
				D3D12_SHADER_BUFFER_DESC constantBufferDesc;
				constantBuffer->GetDesc(&constantBufferDesc);
				mConstantParamsBufferSize = constantBufferDesc.Size;
			}
		}

		auto preConstantParams = mConstantParams;

		ConstructConstantParams();

		OnShaderCompiled(this, preConstantParams);
		return true;
	}

	void ShaderResource::Unload()
	{
		mCode = nullptr;
	}

	D_CORE::SignalConnection ShaderResource::SubscribeOnCompiled(std::function<ShaderCompileSignalCallback> callback)
	{
		D_ASSERT(callback);
		return OnShaderCompiled.connect(callback);
	}

	void ShaderResource::ConstructConstantParams()
	{
		auto preConstantParams = mConstantParams;
		mConstantParams = nullptr;

		auto reflectionData = GetReflectionData();
		auto constantBufferData = reflectionData->GetConstantBufferByName(GetConstantBufferName().c_str());
		if (!constantBufferData)
			return;

		mConstantParams = std::make_shared<ShaderConstantParameters>();

		// Preparing buffer desc
		D3D12_SHADER_BUFFER_DESC bufferDesc;
		constantBufferData->GetDesc(&bufferDesc);

		for (UINT i = 0u; i < bufferDesc.Variables; i++)
		{
			auto prop = CreatePropertyFromVariableData(constantBufferData->GetVariableByIndex(i));

			if (!prop)
				continue;

			mConstantParams->AddProperty(prop);
		}

	}

#if _D_EDITOR

	bool ShaderResource::ReloadAndRecompile()
	{
		mCode = D_FILE::ReadFileSync(GetPath());

		return CompileShader(true);
	}

	std::string GetBindTypeName(D3D_SHADER_INPUT_TYPE type)
	{
		switch (type)
		{
		case D3D_SIT_CBUFFER:
			return "Constant Buffer";
		case D3D_SIT_TBUFFER:
			return "Texture Buffer";
		case D3D_SIT_TEXTURE:
			return "Texture";
		case D3D_SIT_SAMPLER:
			return "Sampler";
		case D3D_SIT_UAV_RWTYPED:
			return "RW Type Buffer";
		case D3D_SIT_STRUCTURED:
			return "Structured Buffer";
		case D3D_SIT_UAV_RWSTRUCTURED:
			return "RW Structured Buffer";
		case D3D_SIT_BYTEADDRESS:
			return "Byte Address Buffer";
		case D3D_SIT_UAV_RWBYTEADDRESS:
			return "RW Byte Address Buffer";
		case D3D_SIT_UAV_APPEND_STRUCTURED:
			return "Append Structured Buffer";
		case D3D_SIT_UAV_CONSUME_STRUCTURED:
			return "Consume Structured Buffer";
		case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
			return "RW Structured Buffer Counted";
		case D3D_SIT_RTACCELERATIONSTRUCTURE:
			return "Accelleration Structure Buffer";
		case D3D_SIT_UAV_FEEDBACKTEXTURE:
			return "Feedback Texture";

		default:
			return "";
		}
	}

	bool ShaderResource::DrawDetails(float params[])
	{

		bool valueChanged = false;

		if (ImGui::Button("Open", ImVec2(-1.f, 0.f)))
		{
			ShellExecuteW(0, 0, GetPath().wstring().c_str(), 0, 0, SW_SHOW);
		}

		if (ImGui::Button("Recompile", ImVec2(-1.f, 0.f)))
		{
			valueChanged |= ReloadAndRecompile();
		}

		if (mCompileMessage != "")
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 0.f, 1.f));
			ImGui::Text(mCompileMessage.c_str());
			ImGui::PopStyleColor();
		}

		if (mShaderReflectionData)
		{
			D3D12_SHADER_DESC shaderDesc;
			mShaderReflectionData->GetDesc(&shaderDesc);

			D_H_DETAILS_DRAW_BEGIN_TABLE();

			// Constant Buffers
			{
				D_H_DETAILS_DRAW_PROPERTY("Constant Buffers Count");
				ImGui::Text(std::to_string(shaderDesc.ConstantBuffers).c_str());
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
				{
					ImGui::BeginTooltip();

					for (UINT i = 0u; i < shaderDesc.ConstantBuffers; i++)
					{
						auto buffer = mShaderReflectionData->GetConstantBufferByIndex(i);
						D3D12_SHADER_BUFFER_DESC bufferDesc;
						buffer->GetDesc(&bufferDesc);
						std::stringstream ss;
						ss << i << ": ";
						ss << bufferDesc.Name;
						ss << " (" << bufferDesc.Size << "bytes, ";
						ss << bufferDesc.Variables << " variables)";
						ImGui::Text(ss.str().c_str());
					}

					ImGui::EndTooltip();
				}
			}

			// Bound Resources
			{
				D_H_DETAILS_DRAW_PROPERTY("Bound Resources Count");
				ImGui::Text(std::to_string(shaderDesc.BoundResources).c_str());
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
				{
					ImGui::BeginTooltip();

					for (UINT i = 0u; i < shaderDesc.BoundResources; i++)
					{
						D3D12_SHADER_INPUT_BIND_DESC bindDesc;
						auto buffer = mShaderReflectionData->GetResourceBindingDesc(i, &bindDesc);
						std::stringstream ss;
						ss << bindDesc.BindPoint << ": ";
						ss << bindDesc.Name << "[" << bindDesc.BindCount << "]";
						ss << " (" << GetBindTypeName(bindDesc.Type) << ")";
						ImGui::Text(ss.str().c_str());
					}

					ImGui::EndTooltip();
				}
			}

			D_H_DETAILS_DRAW_END_TABLE();
		}


		return valueChanged;
	}

#endif
}