#pragma once

#include "GenericMaterialResource.hpp"
#include "Renderer/Rasterization/Renderer.hpp"
#include "Renderer/RendererCommon.hpp"
#include "TextureResource.hpp"

#include <Graphics/GraphicsUtils/Buffers/GpuBuffer.hpp>
#include <Graphics/GraphicsUtils/Buffers/UploadBuffer.hpp>
#include <ResourceManager/Resource.hpp>
#include <Core/RefCounting/Ref.hpp>

#include "MaterialResource.generated.hpp"

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Renderer
{
	class DResourceManager;

	class DClass(Serialize, Resource) MaterialResource : public GenericMaterialResource
	{
		GENERATED_BODY();
		D_CH_RESOURCE_BODY(MaterialResource, "Material", ".mat")
		
	public:

		enum TextureType : uint32_t
		{
			kBaseColor,
			kMetallic,
			kRoughness,
			kAmbientOcclusion,
			kEmissive,
			kNormal,
			kWorldDisplacement,

			kNumTextures
		};

		INLINE D_RENDERER::MaterialConstants* ModifyMaterialData() { MakeDiskDirty(); MakeGpuDirty(); return &mMaterial; }
		INLINE const D_RENDERER::MaterialConstants* GetMaterialData() const { return &mMaterial; }
		INLINE uint16_t						GetPsoFlags() const
		{
			return mPsoFlags
#ifdef _D_EDITOR
				| D_RENDERER_RAST::GetForcedPsoFlags()
#endif
				;
		}

		// Two sided
		INLINE bool							IsTwoSided() const { return mPsoFlags & RenderItem::TwoSided; }
		void								SetTwoSided(bool value);

		INLINE float						GetAlphaCutout() const { return mCutout; }
		void								SetAlphaCutout(float value);

		INLINE float						GetOpacity() const { return mMaterial.Opacity; }
		void								SetOpacity(float value);

		INLINE float						GetSpecular() const { return mMaterial.Specular; }
		void								SetSpecular(float value);

		// Displacement
		INLINE bool							HasDisplacement() const { return (mMaterial.TextureStatusMask & (1 << kWorldDisplacement)) != 0; }
		INLINE float						GetDisplacementAmount() const { return mMaterial.DisplacementAmount; }
		void								SetDisplacementAmount(float value);

		// Emissive
		INLINE D_MATH::Color				GetEmissiveColor() const { return D_MATH::Color(mMaterial.Emissive); }
		void								SetEmissiveColor(D_MATH::Color const& value);
		INLINE bool							HasEmissiveTexture() const { return mMaterial.TextureStatusMask & (1 << kEmissive); }

		// Roughness
		INLINE float						GetRoughness() const { return mMaterial.Roughness; }
		void								SetRoughness(float value);
		INLINE bool							HasRoughnessTexture() const { return mMaterial.TextureStatusMask & (1 << kRoughness); }

		// Metallic
		INLINE float						GetMetallic() const { return mMaterial.Metallic; }
		void								SetMetallic(float value);
		INLINE bool							HasMetallicTexture() const { return mMaterial.TextureStatusMask & (1 << kMetallic); }

		// Albedo
		INLINE D_MATH::Color				GetAlbedoColor() const { return D_MATH::Color(D_MATH::Vector4(mMaterial.DifuseAlbedo)); }
		void								SetAlbedoColor(D_MATH::Color const& value);
		INLINE bool							HasAlbedoTexture() const { return mMaterial.TextureStatusMask & (1 << kBaseColor); }

		virtual void						SetTexture(TextureResource* texture, uint32_t textureIndex) override;

		INLINE virtual size_t				GetConstantsBufferSize() const { return sizeof(mMaterial); }
		INLINE virtual void const*			GetConstantsBufferData() const override { return &mMaterial; }

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]);
#endif // _D_EDITOR

		// Stand alone texture setters declarations
#define TextureSetter(type) \
		void Set##type##Texture(TextureResource* texture);
			
		TextureSetter(BaseColor);
		TextureSetter(Metallic);
		TextureSetter(Roughness);
		TextureSetter(AmbientOcclusion);
		TextureSetter(Emissive);
		TextureSetter(Normal);
		TextureSetter(WorldDisplacement);
#undef TextureSetter

	protected:

		virtual bool						WriteResourceToFile(D_SERIALIZATION::Json& j) const override ;
		virtual void						ReadResourceFromFile(D_SERIALIZATION::Json const& j, bool& dirtyDisk) override;
		virtual TextureResource*			GetFallbackTexture(uint32_t textureIndex) const override;
		virtual void						OnTextureChanged(uint32_t index) override;

	private:
		friend class DResourceManager;

		MaterialResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, D_RESOURCE::Resource* parent, bool isDefault = false);
		
		D_RENDERER::MaterialConstants				mMaterial;

		uint16_t									mPsoFlags;

		float										mCutout;

	};
}

File_MaterialResource_GENERATED