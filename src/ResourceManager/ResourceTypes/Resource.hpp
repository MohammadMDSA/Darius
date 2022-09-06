#pragma once

#include <Core/Filesystem/Path.hpp>
#include <Core/Counted.hpp>
#include <Core/Uuid.hpp>
#include <Core/Exceptions/Exception.hpp>
#include <Core/Serialization/Json.hpp>
#include <Renderer/CommandContext.hpp>
#include <Utils/Common.hpp>
#include <Utils/Detailed.hpp>

#ifndef D_RESOURCE
#define D_RESOURCE Darius::ResourceManager
#endif // !D_RESOURCE

#define D_T_RESOURCE_ID UINT16

#define D_CH_RESOURCE_BODY(T, ResT) D_CH_TYPE_NAME_GETTER(T)\
public: \
static INLINE ResourceType GetResourceType() { return ResT; } \
INLINE ResourceType GetType() const override { return ResT; }

using namespace D_CORE;
using namespace D_FILE;

namespace Darius::ResourceManager
{
	class DResourceManager;
	class ResourceLoader;

	typedef D_T_RESOURCE_ID DResourceId;

	enum class ResourceType
	{
		None,
		Mesh,
		Batch,
		Material,
		Texture2D
	};

	D_H_SERIALIZE_ENUM(ResourceType, {
		{ ResourceType::None, "None" },
		{ ResourceType::Mesh, "Mesh" },
		{ ResourceType::Batch, "Batch" },
		{ ResourceType::Material, "Material" },
		{ ResourceType::Texture2D, "Texture2D" },
		})

	struct ResourceHandle
	{
		ResourceType			Type = ResourceType::None;
		DResourceId				Id = 0;

		INLINE bool IsValid() { return Type != ResourceType::None; }
	};

	constexpr ResourceHandle EmptyHandle = { ResourceType::None, 0 };

	struct ResourcePreview
	{
		std::wstring			Name;
		std::wstring			Path;
		ResourceHandle			Handle;
	};

	struct ResourceMeta
	{
		Path					Path;
		std::wstring			Name;
		ResourceType			Type;
		Uuid					Uuid;
	};

	std::string ResourceTypeToString(ResourceType type);
	ResourceType StringToResourceType(std::string type);

	class Resource : public D_CORE::Counted, public Detailed
	{
	public:
		
		void Destroy()
		{
			mLoaded = false;
			mVersion++;
		}

		INLINE ResourcePreview GetPreview() const
		{
			return { mName, mPath, { GetType(), mId } };
		}

		virtual ResourceType		GetType() const = 0;

		INLINE operator ResourceHandle const() { return { GetType(), mId }; }
		INLINE operator ResourcePreview const() { return GetPreview(); }
		INLINE operator ResourceMeta const() { return { mPath, mName, GetType(), mUuid }; }

		D_CH_RW_FIELD(std::wstring, Name);
		D_CH_R_FIELD(D_FILE::Path, Path);
		D_CH_R_FIELD_CONST(DResourceId, Id);
		D_CH_R_FIELD_CONST(Uuid, Uuid);
		D_CH_R_FIELD_CONST(bool, Default);

		D_CH_R_FIELD(bool, Loaded);
		D_CH_R_FIELD(UINT, Version);
		D_CH_R_FIELD(bool, DirtyDisk);
		D_CH_R_FIELD(bool, DirtyGPU);

	public:
		void						UpdateGPU(D_GRAPHICS::GraphicsContext& context);
		virtual bool				SuppoertsExtension(std::wstring ext) = 0;

#ifdef _D_EDITOR
		virtual bool				DrawDetails(float params[]) = 0;
#endif // _D_EDITOR


		friend class DResourceManager;
		friend class ResourceLoader;

	protected:
		Resource(Uuid uuid, std::wstring const& path, DResourceId id, bool isDefault) :
			mLoaded(false),
			mPath(path),
			mName(L""),
			mVersion(1),
			mId(id),
			mDefault(isDefault),
			mDirtyDisk(false),
			mDirtyGPU(true),
			mUuid(uuid)
		{
			// Processing name
			auto ext = mPath.extension().wstring();
			auto filename = mPath.filename().wstring();
			mName = filename.substr(0, filename.size() - ext.size());
		}
		
		INLINE void					MakeDiskDirty() { mDirtyDisk = true; }
		INLINE void					MakeGpuDirty() { mDirtyGPU = true; }

		virtual void				WriteResourceToFile() const = 0;
		virtual void				ReadResourceFromFile() = 0;
		virtual bool				UploadToGpu(D_GRAPHICS::GraphicsContext& context) = 0;
	};
}
