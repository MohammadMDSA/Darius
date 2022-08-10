#pragma once

#include "Resource.hpp"

#include <Core/Ref.hpp>
#include <Core/Containers/Vector.hpp>
#include <Core/Exceptions/Exception.hpp>

#include <optional>

#ifndef D_RESOURCE
#define D_RESOURCE Darius::ResourceManager
#endif // !D_RESOURCE

using namespace D_CORE;

namespace Darius::ResourceManager
{

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

	ResourceHandle			LoadResource(std::wstring path);
	ResourceHandle			CreateResource(std::wstring path, ResourceType type);

	Resource* _GetRawResource(ResourceHandle handle);

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

		return Ref(dynamic_cast<T*>(_GetRawResource(handle)), ownerData);
	}

	template<class T>
	Ref<T> GetResource(ResourceHandle handle, void* owner, std::wstring const& ownerName, std::string const& ownerType)
	{
		return GetResource<T>(handle, std::optional<CountedOwner>{CountedOwner{ ownerName, ownerType, owner, 0 }});
	}

	D_CONTAINERS::DVector<ResourcePreview> GetResourcePreviews(ResourceType type);
	
	ResourceHandle GetDefaultResource(DefaultResource type);

	class DResourceManager : NonCopyable
	{
	public:
		DResourceManager();
		~DResourceManager();

		Resource*					GetRawResource(ResourceHandle handle);
		ResourceHandle				LoadResource(std::wstring path);

		DVector<ResourcePreview>	GetResourcePreviews(ResourceType type);

		ResourceHandle				CreateMaterial(std::wstring path);
		ResourceHandle				CreateMesh(std::wstring path);

		ResourceHandle GetDefaultResource(DefaultResource type);
	private:

		ResourceHandle				CreateMaterial(std::wstring path, bool isDefault);
		ResourceHandle				CreateMesh(std::wstring path, bool isDefault);

		void LoadDefaultResources();
		INLINE void AddMeshResource(Resource* res);
		INLINE void AddMaterialResource(Resource* res);
		INLINE DResourceId GetNewId() { return ++mLastId; }

		DMap<ResourceType, DMap<DResourceId, Resource*>>	mResourceMap;
		DMap<std::wstring, ResourceHandle>					mPathMap;
		DMap<DefaultResource, ResourceHandle>				mDefaultResourceMap;

		DResourceId											mLastId = 0;
	};

	INLINE void DResourceManager::AddMeshResource(Resource* res)
	{
		mResourceMap.at(ResourceType::Mesh).insert({ res->GetId(), res });
	}

	INLINE void DResourceManager::AddMaterialResource(Resource* res)
	{
		mResourceMap.at(ResourceType::Material).insert({ res->GetId(), res });
	}

}