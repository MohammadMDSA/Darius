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

	extern const DMap<std::string, ResourceType>		ResourceTypeMap;

	void					Initialize();
	void					Shutdown();

	ResourceHandle			LoadResource(std::wstring path);

	Resource* _GetRawResource(ResourceHandle handle);

	template<class T>
	Ref<T>					GetResource(ResourceHandle handle, std::optional<CountedOwner> ownerData = std::nullopt)
	{
		using conv = std::is_convertible<T*, Resource*>;
		D_STATIC_ASSERT(conv::value);
		if (handle.Type != ResourceTypeMap.at(T::GetTypeName()))
			throw D_EXCEPTION::Exception("Requested type and handle type are not compatible");

		auto ref = Ref(dynamic_cast<T*>(_GetRawResource(handle)), ownerData);
		return ref;
	}

	template<class T>
	Ref<T>					GetResource(ResourceHandle handle, void* owner, std::wstring const& ownerName, std::string const& ownerType)
	{
		return GetResource<T>(handle, std::optional<CountedOwner>{CountedOwner{ ownerName, ownerType, owner, 0 }});
	}

	D_CONTAINERS::DVector<ResourcePreview> GetResourcePreviews(ResourceType type);

	class DResourceManager : NonCopyable
	{
	public:
		DResourceManager();
		~DResourceManager();

		Resource* GetRawResource(ResourceHandle handle);

		DVector<ResourcePreview> GetResourcePreviews(ResourceType type);
	private:
		void LoadDefaultResources();
		INLINE DResourceId GetNewId() { return ++mLastId; }

		DMap<ResourceType, DMap<DResourceId, Resource*>>	mResourceMap;

		DResourceId											mLastId = 0;
	};

}