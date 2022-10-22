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

// TODO: Better resource allocation
#define D_CH_RESOURCE_BODY(T, ResT, ...) \
public: \
	class T##Factory : public Resource::ResourceFactory \
	{ \
	public: \
		virtual std::shared_ptr<Resource> Create(Uuid uuid, std::wstring const& path, DResourceId id, bool isDefault) const; \
	}; \
public: \
	friend class Factory; \
	static INLINE ResourceType GetResourceType() { return Resource::GetResourceType(ResT); } \
	INLINE ResourceType GetType() const override { return Resource::GetResourceType(ResT); } \
	static void Register() \
	{ \
		D_ASSERT_M(!Resource::GetResourceType(ResT), "Resource " #T " is already registered."); \
		auto resType = Resource::RegisterResourceTypeName<T, T::T##Factory>(ResT); \
		\
		std::string supportedExtensions[] = { __VA_ARGS__ }; \
		for (std::string const& ext : supportedExtensions) \
		{ \
			if(ext == "") continue; \
			Resource::RegisterResourceExtension(ext, resType); \
		} \
	} \
	\

#define D_CH_RESOURCE_DEF(T) \
std::shared_ptr<Resource> T::T##Factory::Create(Uuid uuid, std::wstring const& path, DResourceId id, bool isDefault) const { return std::shared_ptr<Resource>(new T(uuid, path, id, isDefault)); }

using namespace D_CORE;
using namespace D_FILE;

namespace Darius::ResourceManager
{
	class DResourceManager;
	class ResourceLoader;

	typedef D_T_RESOURCE_ID DResourceId;

	typedef uint16_t ResourceType;

	struct ResourceHandle
	{
		ResourceType			Type = 0;
		DResourceId				Id = 0;

		INLINE bool IsValid() { return Type != 0; }
	};

	constexpr ResourceHandle EmptyResourceHandle = { 0, 0 };

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

		class ResourceFactory
		{
		public:
			ResourceFactory() = default;
			~ResourceFactory() = default;

			virtual std::shared_ptr<Resource> Create(Uuid uuid, std::wstring const& path, DResourceId id, bool isDefault) const = 0;
		};

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

#ifdef _D_EDITOR
		virtual bool				DrawDetails(float params[]) = 0;
#endif // _D_EDITOR

		static INLINE ResourceType	GetResourceType(std::string name) { return ResourceTypeMapR.contains(name) ? ResourceTypeMapR[name] : 0; }
		static INLINE std::string	GetResourceName(ResourceType type) { return ResourceTypeMap[type]; }
		static INLINE ResourceFactory* GetFactoryForResourceType(ResourceType type) { return ResourceFactories.contains(type) ? ResourceFactories[type] : nullptr; }
		static INLINE void			RegisterResourceExtension(std::string ext, ResourceType type) { ResourceExtensionMap[ext] = type; }
		static INLINE ResourceType	GetResourceTypeByExtension(std::string ext) { return ResourceExtensionMap.contains(ext) ? ResourceExtensionMap[ext] : 0; }

		template<class R, class FAC>
		static ResourceType			RegisterResourceTypeName(std::string name)
		{
			// Checking if T is a resource type
			using conv = std::is_convertible<R*, Resource*>;
			D_STATIC_ASSERT(conv::value);
			using convFac = std::is_convertible<FAC*, ResourceFactory*>;
			D_STATIC_ASSERT(convFac::value);

			// TODO: Better allocation
			ResourceType type = (ResourceType)(ResourceTypeMap.size() + 1);
			ResourceTypeMap[type] = name;
			ResourceTypeMapR[name] = type;
			ResourceFactories.insert({ type, new FAC });

			return type;
		}

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

		static DUnorderedMap<ResourceType, std::string> ResourceTypeMap;
		static DUnorderedMap<std::string, ResourceType> ResourceTypeMapR;
		static DUnorderedMap<ResourceType, ResourceFactory*> ResourceFactories;
		static DUnorderedMap<std::string, ResourceType> ResourceExtensionMap;
	};

}
