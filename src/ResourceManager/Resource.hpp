#pragma once

#include <Core/Counted.hpp>

#include <Utils/Common.hpp>

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
		Mesh
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
		Resource(DResourceId id) :
			mLoaded(false),
			mPath(L""),
			mName(L""),
			mVersion(1),
			mId(id)
		{}
		
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

		D_CH_RW_FIELD_ACC(std::wstring, Path, protected);
		D_CH_RW_FIELD_ACC(std::wstring, Name, protected);
		D_CH_R_FIELD_CONST_ACC(DResourceId, Id, protected);

		D_CH_R_FIELD_ACC(bool, Loaded, protected);
		D_CH_R_FIELD_ACC(UINT, Version, protected);

		friend class Darius::ResourceManager::DResourceManager;
	};

}
