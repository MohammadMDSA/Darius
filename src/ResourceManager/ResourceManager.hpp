#pragma once

#include "Resource.hpp"
#include "ResourceLoader.hpp"
#include "ResourceRef.hpp"

#include <Core/Ref.hpp>
#include <Core/Uuid.hpp>
#include <Core/Containers/Vector.hpp>
#include <Core/Exceptions/Exception.hpp>

#include <boost/functional/hash.hpp>

#include <optional>

#ifndef D_RESOURCE
#define D_RESOURCE Darius::ResourceManager
#endif // !D_RESOURCE

namespace Darius::ResourceManager
{
	class DResourceManager;
	class ResourceLoader;

	void					Initialize(D_SERIALIZATION::Json const& settings);
	void					Shutdown();
#ifdef _D_EDITOR
	bool					OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options);
#endif


	void					UpdateGPUResources();

	DResourceManager*		GetManager();

	Resource*				_GetRawResource(D_CORE::Uuid const& uuid, bool load = false);
	Resource*				_GetRawResource(ResourceHandle handle, bool load = false);
	void					SaveAll();

#ifdef _D_EDITOR
	void					GetAllResources(D_CONTAINERS::DVector<Resource*>& resources);
	void					GetAllResourcePaths(D_CONTAINERS::DVector<D_FILE::Path>& paths);
#endif // _D_EDITOR

#pragma region GetResource
	// Resource retreival stuff
	template<class T>
	ResourceRef<T> GetResource(D_CORE::Uuid const& uuid, std::optional<D_CORE::CountedOwner> ownerData)
	{
		// Checking if T is a resource type
		using conv = std::is_convertible<T*, Resource*>;
		D_STATIC_ASSERT(conv::value);

		return ResourceRef(dynamic_cast<T*>(_GetRawResource(uuid, true)), ownerData);
	}

	// Untyped version
	INLINE Resource* GetUncountedResource(ResourceHandle handle, std::optional<D_CORE::CountedOwner> ownerData = std::nullopt)
	{
		// Requested None resource so we return nothing
		if (handle.Type == 0)
			return nullptr;

		return _GetRawResource(handle, true);
	}

	template<class T>
	ResourceRef<T> GetResource(ResourceHandle handle, std::optional<D_CORE::CountedOwner> ownerData = std::nullopt)
	{
		// Checking if T is a resource type
		using conv = std::is_convertible<T*, Resource*>;
		D_STATIC_ASSERT(conv::value);

		// Requested None resource so we return nothing
		if (handle.Type == 0)
			return ResourceRef<T>(nullptr);

		// Requested resource type must be compatible with T
		if (handle.Type != T::GetResourceType())
			throw D_EXCEPTION::Exception("Requested type and handle type are not compatible");

		return ResourceRef(dynamic_cast<T*>(_GetRawResource(handle, true)), ownerData);
	}

	template<class T>
	ResourceRef<T> GetResource(ResourceHandle handle, void* owner, std::wstring const& ownerName, rttr::type const& ownerType, std::function<void()> changeCallback = nullptr)
	{
		return GetResource<T>(handle, std::optional<D_CORE::CountedOwner> { D_CORE::CountedOwner { ownerName, ownerType, owner, 0, changeCallback } } );
	}

	ResourceHandle GetResourceHandle(D_CORE::Uuid const& uuid);

#pragma endregion

	D_CONTAINERS::DVector<ResourcePreview> GetResourcePreviews(ResourceType type);
	
	class DResourceManager : NonCopyable
	{
	public:
		DResourceManager();
		~DResourceManager();

		// Retreival
		Resource*					GetRawResource(ResourceHandle handle);
		Resource*					GetRawResource(D_CORE::Uuid const& uuid);

		// Save resouce
		void						SaveAllResources();

		D_CONTAINERS::DVector<ResourcePreview>	GetResourcePreviews(ResourceType type);

		template<class T>
		INLINE ResourceHandle		CreateResource(D_CORE::Uuid const& uuid, std::wstring const& path, std::wstring const& name, bool isDefault = false) {
			if (D_H_ENSURE_DIR(path))
				throw D_EXCEPTION::Exception(("A file with the same name already exists: " + WSTR2STR(path)).c_str());
			return CreateResource<T>(uuid, path, name, isDefault, false);
		}

		template<class T>
		INLINE ResourceHandle		CreateResource(std::wstring const& path, std::wstring const& name, bool isDefault = false) {
			return CreateResource<T>(D_CORE::GenerateUuid(), path, name, isDefault);
		}

		INLINE ResourceHandle		CreateResource(ResourceType type, std::wstring const& path, std::wstring const& name)
		{
			if (D_H_ENSURE_DIR(path))
				throw D_EXCEPTION::Exception(("A file with the same name already exists: " + WSTR2STR(path)).c_str());
			return CreateResource(type, D_CORE::GenerateUuid(), path, name, false, false);
		}

		void						UpdateGPUResources();

#ifdef _D_EDITOR
		void						GetAllResources(D_CONTAINERS::DVector<Resource*>& resources) const;
		void						GetAllResourcePaths(D_CONTAINERS::DVector<D_FILE::Path>& paths) const;
#endif // _D_EDITOR

	private:
		friend class ResourceLoader;
		friend class Resource;

		ResourceHandle				CreateResource(ResourceType type, D_CORE::Uuid const& uuid, std::wstring const& path, std::wstring const& name, bool isDefault, bool fromFile);

		template<class T>
		INLINE ResourceHandle		CreateResource(D_CORE::Uuid const& uuid, std::wstring const& path, std::wstring const& name, bool isDefault, bool fromFile)
		{
			// Checking if T is a resource type
			using conv = std::is_convertible<T*, Resource*>;
			D_STATIC_ASSERT(conv::value);

			return CreateResource(T::GetResourceType(), uuid, path, name, isDefault, fromFile);
		}

		void						UpdateMaps(std::shared_ptr<Resource> resuorce);

		INLINE DResourceId			GetNewId() { return ++mLastId; }

		D_CONTAINERS::DUnorderedMap<ResourceType, D_CONTAINERS::DUnorderedMap<DResourceId, std::shared_ptr<Resource>>>	mResourceMap;
		D_CONTAINERS::DUnorderedMap<D_CORE::Uuid, Resource*, D_CORE::UuidHasher>			mUuidMap;
		D_CONTAINERS::DUnorderedMap<std::wstring, D_CONTAINERS::DVector<ResourceHandle>>	mPathMap;
		DResourceId																			mLastId = 0;
	};

}