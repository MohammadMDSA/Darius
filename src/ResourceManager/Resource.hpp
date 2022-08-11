#pragma once

#include <Core/Counted.hpp>
#include <Renderer/CommandContext.hpp>
#include <Utils/Common.hpp>

#include <filesystem>

#ifndef D_RESOURCE
#define D_RESOURCE Darius::ResourceManager
#endif // !D_RESOURCE

#define D_T_RESOURCE_ID UINT16

#define D_CH_RESOUCE_BODY(T, ResT) D_CH_TYPE_NAME_GETTER(T)

namespace Darius::ResourceManager
{
	class DResourceManager;

	typedef D_T_RESOURCE_ID DResourceId;

	enum class ResourceType
	{
		None,
		Mesh,
		Material
	};

	struct ResourceHandle
	{
		ResourceType			Type;
		DResourceId				Id;

	};

	struct ResourcePreview
	{
		std::wstring			Name;
		std::wstring			Path;
		ResourceHandle			Handle;
	};

	class Resource : public D_CORE::Counted
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

		INLINE operator ResourceHandle const() { return { GetType(), mId }; };
		INLINE operator ResourcePreview const() { return GetPreview(); }

		D_CH_RW_FIELD_ACC(std::filesystem::path, Path, protected);
		D_CH_RW_FIELD_ACC(std::wstring, Name, protected);
		D_CH_R_FIELD_CONST_ACC(DResourceId, Id, protected);
		D_CH_R_FIELD_CONST_ACC(bool, Default, protected);

		D_CH_R_FIELD_ACC(bool, Loaded, protected);
		D_CH_R_FIELD_ACC(UINT, Version, protected);
		D_CH_R_FIELD_ACC(bool, DirtyDisk, protected);
		D_CH_R_FIELD_ACC(bool, DirtyGPU, protected);

	public:
		virtual bool				Save() = 0;
		virtual bool				Load() = 0;
		virtual void				UpdateGPU(D_GRAPHICS::GraphicsContext& context) = 0;
		virtual bool				SuppoertsExtension(std::wstring ext) = 0;

		friend class DResourceManager;

	protected:
		Resource(std::wstring path, DResourceId id, bool isDefault) :
			mLoaded(false),
			mPath(path),
			mName(L""),
			mVersion(1),
			mId(id),
			mDefault(isDefault),
			mDirtyDisk(false),
			mDirtyGPU(true)
		{}

	};

}
