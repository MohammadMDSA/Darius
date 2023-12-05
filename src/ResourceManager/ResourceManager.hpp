#pragma once

#include "Resource.hpp"
#include "ResourceLoader.hpp"
#include "ResourceRef.hpp"

#include <Core/Uuid.hpp>
#include <Core/Containers/Vector.hpp>
#include <COre/Containers/Map.hpp>
#include <Core/Exceptions/Exception.hpp>

#include <boost/functional/hash.hpp>

#include <concurrent_unordered_map.h>

#include <optional>

#ifndef D_RESOURCE
#define D_RESOURCE Darius::ResourceManager
#endif // !D_RESOURCE

namespace Darius::ResourceManager
{
	class DResourceManager;
	class ResourceLoader;

	template<class RESOURCE> using ResourceLoadedTypedResourceCalllback = std::function<void(D_RESOURCE::ResourceRef<RESOURCE> resource)>;

	void					Initialize(D_SERIALIZATION::Json const& settings);
	void					Shutdown();
#ifdef _D_EDITOR
	bool					OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options);
#endif


	void					UpdateGPUResources();

	DResourceManager* GetManager();

	/// <summary>
	/// Gets a resource by its UUID
	/// </summary>
	/// <param name="uuid">UUID of the resource</param>
	/// <param name="syncLoad">Whether it should be loaded or not. If true, loading will happen synchronously</param>
	Resource*				GetRawResourceSync(D_CORE::Uuid const& uuid, bool syncLoad = false);

	/// <summary>
	/// Gets a resource by its handle
	/// </summary>
	/// <param name="handle">handle of the resource</param>
	/// <param name="syncLoad">Whether it should be loaded or not. If true, loading will happen synchronously</param>
	Resource*				GetRawResourceSync(ResourceHandle handle, bool syncLoad = false);

	void					GetRawResourceAsync(ResourceHandle handle, ResourceLoadedResourceCalllback callback);
	void					GetRawResourceAsync(D_CORE::Uuid const& uuid, ResourceLoadedResourceCalllback callback);

	void					SaveAll();

#ifdef _D_EDITOR
	void					GetAllResources(D_CONTAINERS::DVector<Resource*>& resources);
	void					GetAllResourcePaths(D_CONTAINERS::DVector<D_FILE::Path>& paths);
#endif // _D_EDITOR

#pragma region GetResource
	// Resource retreival stuff
	template<class T>
	ResourceRef<T> GetResourceSync(D_CORE::Uuid const& uuid)
	{
		// Checking if T is a resource type
		using conv = std::is_convertible<T*, Resource*>;
		D_STATIC_ASSERT(conv::value);

		return ResourceRef(dynamic_cast<T*>(GetRawResourceSync(uuid, true)));
	}

	template<class T>
	void GetResourceAsync(ResourceHandle handle, ResourceLoadedTypedResourceCalllback<T> callback)
	{
		// Checking if T is a resource type
		using conv = std::is_convertible<T*, Resource*>;
		D_STATIC_ASSERT(conv::value);

		// Requested None resource so we return nothing
		if (handle.Type == 0)
			return;

		// Requested resource type must be compatible with T
		if (handle.Type != T::GetResourceType())
			throw D_EXCEPTION::Exception("Requested type and handle type are not compatible");

		GetRawResourceAsync(handle, [callback](auto resource)
			{
				if (callback)
				{
					callback(ResourceRef(static_cast<T*>(resource)));
				}
			});
	}

	template<class T>
	ResourceRef<T> GetResourceSync(ResourceHandle handle)
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

		return ResourceRef(dynamic_cast<T*>(GetRawResourceSync(handle)));
	}

	ResourceHandle GetResourceHandle(D_CORE::Uuid const& uuid);

#pragma endregion

#if _D_EDITOR
	D_CONTAINERS::DVector<ResourcePreview> GetResourcePreviews(ResourceType type);
	ResourcePreview					GetResourcePreview(ResourceHandle const& handle);
#endif

	class DResourceManager : NonCopyable
	{
	public:
		DResourceManager();
		~DResourceManager();

		// Save resouce
		void						SaveAllResources();

#if _D_EDITOR
		D_CONTAINERS::DVector<ResourcePreview>	GetResourcePreviews(ResourceType type);
		ResourcePreview				GetResourcePreview(ResourceHandle const& handle);
#endif

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
		friend Resource* GetRawResourceSync(ResourceHandle handle, bool syncLoad);
		friend Resource* GetRawResourceSync(D_CORE::Uuid const& uuid, bool syncLoad);
		friend void GetRawResourceAsync(ResourceHandle handle, ResourceLoadedResourceCalllback callback);
		friend void GetRawResourceAsync(D_CORE::Uuid const& uuid, ResourceLoadedResourceCalllback callback);
		friend ResourceHandle GetResourceHandle(D_CORE::Uuid const& uuid);
		friend class Resource;
		friend class ResourceLoader;
		friend struct AsyncResourceLoadingFromPathTask;


		// Retreival
		Resource* GetRawResource(ResourceHandle handle);
		Resource* GetRawResource(D_CORE::Uuid const& uuid);

		ResourceHandle				CreateResource(ResourceType type, D_CORE::Uuid const& uuid, std::wstring const& path, std::wstring const& name, bool isDefault, bool fromFile);

		template<class T>
		INLINE ResourceHandle		CreateResource(D_CORE::Uuid const& uuid, std::wstring const& path, std::wstring const& name, bool isDefault, bool fromFile)
		{
			// Checking if T is a resource type
			using conv = std::is_convertible<T*, Resource*>;
			D_STATIC_ASSERT(conv::value);

			return CreateResource(T::GetResourceType(), uuid, path, name, isDefault, fromFile);
		}

		Resource* GetRawResourceSafe(ResourceHandle handle);

		void						UpdateMaps(std::shared_ptr<Resource> resuorce);

		INLINE DResourceId			GetNewId() { return ++mLastId; }
		INLINE D_CONTAINERS::DVector<ResourceHandle> const& GetHandleFromPath(std::wstring const& path) const { if (mPathMap.find(path) == mPathMap.end()) throw std::exception("Path Not Found"); return mPathMap.at(path); }
		INLINE bool					TryGetHandleFromPath(std::wstring const& path, _OUT_ D_CONTAINERS::DVector<ResourceHandle>const** result) const
		{
			if (mPathMap.find(path) == mPathMap.end())
			{
				result = nullptr;
				return false;
			}
			*result = &mPathMap.at(path);
			return true;
		}

		D_CONTAINERS::DConcurrentUnorderedMap<ResourceType, D_CONTAINERS::DUnorderedMap<DResourceId, std::shared_ptr<Resource>>>	mResourceMap;
		D_CONTAINERS::DConcurrentUnorderedMap<D_CORE::Uuid, Resource*, D_CORE::UuidHasher>			mUuidMap;
		D_CONTAINERS::DConcurrentUnorderedMap<std::wstring, D_CONTAINERS::DVector<ResourceHandle>>	mPathMap;
		D_CONTAINERS::DConcurrentVector<ResourceRef<Resource>> mDefaultResourcesSet;
		DResourceId								mLastId = 0;
	};

}