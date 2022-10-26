#pragma once

#include "ResourceTypes/Resource.hpp"

#include <Core/Ref.hpp>
#include <Core/Uuid.hpp>
#include <Core/Containers/Vector.hpp>
#include <Core/Exceptions/Exception.hpp>

#include <boost/functional/hash.hpp>

#include <optional>

#ifndef D_RESOURCE
#define D_RESOURCE Darius::ResourceManager
#endif // !D_RESOURCE

using namespace D_CORE;

namespace Darius::ResourceManager
{
	class DResourceManager;
	class ResourceLoader;

	enum class DefaultResource
	{
		// Meshes
		BoxMesh,
		CylinderMesh,
		GeosphereMesh,
		GridMesh,
		QuadMesh,
		SphereMesh,
		LowPolySphereMesh,
		LineMesh,

		// Materials
		Material,

		// Textures
		Texture2DMagenta,
		Texture2DBlackOpaque,
		Texture2DBlackTransparent,
		Texture2DWhiteOpaque,
		Texture2DWhiteTransparent,
		Texture2DNormalMap,
	};

	void					Initialize();
	void					Shutdown();

	void					UpdateGPUResources(D_GRAPHICS::GraphicsContext& context);

	DResourceManager*		GetManager();

	Resource*				_GetRawResource(Uuid uuid, bool load = false);
	Resource*				_GetRawResource(ResourceHandle handle, bool load = false);
	void					SaveAll();

	ResourceHandle			GetDefaultResource(DefaultResource type);

#ifdef _D_EDITOR
	void					GetAllResources(DVector<Resource*>& resources);
#endif // _D_EDITOR

#pragma region GetResource
	// Resource retreival stuff
	template<class T>
	Ref<T> GetResource(Uuid uuid, std::optional<CountedOwner> ownerData = std::nullopt)
	{
		// Checking if T is a resource type
		using conv = std::is_convertible<T*, Resource*>;
		D_STATIC_ASSERT(conv::value);

		return Ref(dynamic_cast<T*>(_GetRawResource(uuid, true)), ownerData);
	}

	template<class T>
	Ref<T> GetResource(ResourceHandle handle, std::optional<CountedOwner> ownerData = std::nullopt)
	{
		// Checking if T is a resource type
		using conv = std::is_convertible<T*, Resource*>;
		D_STATIC_ASSERT(conv::value);

		// Requested None resource so we return nothing
		if (handle.Type == 0)
			return Ref<T>();

		// Requested resource type must be compatible with T
		if (handle.Type != T::GetResourceType())
			throw D_EXCEPTION::Exception("Requested type and handle type are not compatible");

		return Ref(dynamic_cast<T*>(_GetRawResource(handle, true)), ownerData);
	}

	template<class T>
	Ref<T> GetResource(ResourceHandle handle, void* owner, std::wstring const& ownerName, std::string const& ownerType)
	{
		return GetResource<T>(handle, std::optional<CountedOwner>{CountedOwner{ ownerName, ownerType, owner, 0 }});
	}

	ResourceHandle GetResourceHandle(Uuid uuid);

#pragma endregion

	D_CONTAINERS::DVector<ResourcePreview> GetResourcePreviews(ResourceType type);
	
	class DResourceManager : NonCopyable
	{
	public:
		DResourceManager();
		~DResourceManager();

		// Retreival
		Resource*					GetRawResource(ResourceHandle handle);
		Resource*					GetRawResource(Uuid uuid);

		// Save resouce
		void						SaveAllResources();

		DVector<ResourcePreview>	GetResourcePreviews(ResourceType type);

		ResourceHandle				CreateMaterial(std::wstring const& dirpath);

		void						UpdateGPUResources(D_GRAPHICS::GraphicsContext& context);

		ResourceHandle				GetDefaultResource(DefaultResource type);

#ifdef _D_EDITOR
		void						GetAllResources(DVector<Resource*>& resources);
#endif // _D_EDITOR

		void						LoadDefaultResources();
	private:
		friend class ResourceLoader;
		friend class Resource;

		ResourceHandle				CreateResource(ResourceType type, Uuid uuid, std::wstring const& path, bool isDefault, bool fromFile)
		{
			
			auto factory = Resource::GetFactoryForResourceType(type);
			if (!factory)
				return EmptyResourceHandle;

			auto res = factory->Create(uuid, path, GetNewId(), isDefault);

			res->mLoaded = !fromFile;

			// Add the handle to path and resource maps
			ResourceHandle handle = { type , res->GetId()};
			UpdateMaps(res);

			return handle;
		}

		template<class T>
		INLINE ResourceHandle		CreateResource(Uuid uuid, std::wstring const& path, bool isDefault, bool fromFile)
		{
			// Checking if T is a resource type
			using conv = std::is_convertible<T*, Resource*>;
			D_STATIC_ASSERT(conv::value);

			return CreateResource(T::GetResourceType(), uuid, path, isDefault, fromFile);
		}

		void						UpdateMaps(std::shared_ptr<Resource> resuorce);

		INLINE DResourceId			GetNewId() { return ++mLastId; }

		DUnorderedMap<ResourceType, DUnorderedMap<DResourceId, std::shared_ptr<Resource>>>	mResourceMap;
		DUnorderedMap<Uuid, Resource*, UuidHasher>					mUuidMap;
		DUnorderedMap<std::wstring, Resource*>						mPathMap;
		DUnorderedMap<DefaultResource, ResourceHandle>				mDefaultResourceMap;

		DResourceId											mLastId = 0;
	};

}