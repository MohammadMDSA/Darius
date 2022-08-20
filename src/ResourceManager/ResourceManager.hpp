#pragma once

#include "Resource.hpp"

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
		BoxMesh,
		CylinderMesh,
		GeosphereMesh,
		GridMesh,
		QuadMesh,
		SphereMesh,
		DefaultMaterial
	};

	extern const DMap<std::string, ResourceType>		ResourceTypeMap;

	void					Initialize();
	void					Shutdown();

	void					UpdateGPUResources(D_GRAPHICS::GraphicsContext& context);

	DResourceManager*		GetManager();
	Resource*				_GetRawResource(ResourceHandle handle, bool load = false);
	void					SaveAll();

	ResourceHandle			GetDefaultResource(DefaultResource type);

#ifdef _D_EDITOR
	void					GetAllResources(DVector<Resource*>& resources);
#endif // _D_EDITOR


	// Resource retreival stuff
	template<class T>
	Ref<T> GetResource(ResourceHandle handle, std::optional<CountedOwner> ownerData = std::nullopt)
	{
		// Checking if T is a resource type
		using conv = std::is_convertible<T*, Resource*>;
		D_STATIC_ASSERT(conv::value);

		// Requested None resource so we return nothing
		if (handle.Type == ResourceType::None)
			return Ref<T>();

		// Requested resource type must be compatible with T
		if (handle.Type != ResourceTypeMap.at(T::GetTypeName()))
			throw D_EXCEPTION::Exception("Requested type and handle type are not compatible");
			
		return Ref(dynamic_cast<T*>(_GetRawResource(handle, true)), ownerData);
	}

	template<class T>
	Ref<T> GetResource(ResourceHandle handle, void* owner, std::wstring const& ownerName, std::string const& ownerType)
	{
		return GetResource<T>(handle, std::optional<CountedOwner>{CountedOwner{ ownerName, ownerType, owner, 0 }});
	}

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

		ResourceHandle				CreateMaterial(std::wstring const& path);
		ResourceHandle				CreateMesh(std::wstring const& path);

		void						UpdateGPUResources(D_GRAPHICS::GraphicsContext& context);

		ResourceHandle				GetDefaultResource(DefaultResource type);

#ifdef _D_EDITOR
		void						GetAllResources(DVector<Resource*>& resources);
#endif // _D_EDITOR
	private:
		friend class ResourceLoader;

		ResourceHandle				CreateMaterial(std::wstring const& path, bool isDefault, bool fromFile);
		ResourceHandle				CreateMesh(std::wstring const& path, bool isDefault, bool fromFile);

		void						UpdateMaps(Resource* resuorce);

		void LoadDefaultResources();
		INLINE DResourceId GetNewId() { return ++mLastId; }

		DMap<ResourceType, DMap<DResourceId, Resource*>>	mResourceMap;
		DMap<Uuid, Resource*, boost::hash<Uuid>>			mUuidMap;
		DMap<std::wstring, Resource*>						mPathMap;
		DMap<DefaultResource, ResourceHandle>				mDefaultResourceMap;

		DResourceId											mLastId = 0;
	};

}