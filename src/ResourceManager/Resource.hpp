#pragma once

#include <Core/Filesystem/Path.hpp>
#include <Core/Filesystem/FileUtils.hpp>
#include <Core/Counted.hpp>
#include <Core/Containers/Set.hpp>
#include <Core/Uuid.hpp>
#include <Core/Exceptions/Exception.hpp>
#include <Core/Serialization/Json.hpp>
#include <Utils/Common.hpp>
#include <Utils/Assert.hpp>
#include <Utils/Detailed.hpp>

#ifndef D_RESOURCE
#define D_RESOURCE Darius::ResourceManager
#endif // !D_RESOURCE

#define D_CH_RESOURCE_RW_FIELD_ACC(type, name, access) \
public: \
INLINE void Set##name(type const& val) { m##name = val; MakeDiskDirty(); MakeGpuDirty(); } \
D_CH_R_FIELD_ACC(type, name, access)

#define D_CH_RESOURCE_RW_FIELD(type, name) D_CH_RESOURCE_RW_FIELD_ACC(type, name, private)

#define D_T_RESOURCE_ID UINT16

// TODO: Better resource allocation
#define D_CH_RESOURCE_BODY(T, ResT, ...) \
public: \
	class T##Factory : public D_RESOURCE::Resource::ResourceFactory \
	{ \
	public: \
		virtual std::shared_ptr<D_RESOURCE::Resource> Create(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, bool isDefault) const; \
	}; \
public: \
	friend class Factory; \
	static INLINE D_RESOURCE::ResourceType GetResourceType() { return D_RESOURCE::Resource::GetResourceTypeFromName(ResT); } \
	INLINE D_RESOURCE::ResourceType GetType() const override { return D_RESOURCE::Resource::GetResourceTypeFromName(ResT); } \
	static INLINE std::string ClassName() { return D_NAMEOF(T); } \
	static void Register() \
	{ \
		D_ASSERT_M(!D_RESOURCE::Resource::GetResourceTypeFromName(ResT), "Resource " #T " is already registered."); \
		auto resType = D_RESOURCE::Resource::RegisterResourceTypeName<T, T::T##Factory>(ResT); \
		D_RESOURCE::Resource::AddTypeContainer(resType); \
		RegisterConstructionValidation(resType, CanConstructFrom); \
		\
		std::string supportedExtensions[] = { __VA_ARGS__ }; \
		for (std::string const& ext : supportedExtensions) \
		{ \
			if(ext == "") continue; \
			Resource::RegisterResourceExtension(ext, resType); \
		} \
	} \
	\
// TODO: Better resource allocation
#define D_CH_RESOURCE_DEF(T) \
std::shared_ptr<D_RESOURCE::Resource> T::T##Factory::Create(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, bool isDefault) const { return std::shared_ptr<D_RESOURCE::Resource>(new T(uuid, path, name, id, isDefault)); }

namespace Darius::ResourceManager
{
	class DResourceManager;
	class ResourceLoader;

	typedef D_T_RESOURCE_ID DResourceId;

	typedef uint16_t ResourceType;

	struct ResourceDataInFile
	{
		std::string					Name;
		ResourceType				Type;
		D_CORE::Uuid				Uuid;
	};

	struct ResourceFileMeta
	{
		std::wstring								FileName;
		D_CONTAINERS::DVector<ResourceDataInFile>	Resources;
	};

	struct ResourceHandle
	{
		ResourceType			Type = 0;
		DResourceId				Id = 0;

		INLINE bool IsValid() const { return Type != 0; }
		INLINE bool operator== (ResourceHandle const& other) const { return other.Id == Id && other.Type == Type; }
	};

	constexpr ResourceHandle EmptyResourceHandle = { 0, 0 };

	struct ResourcePreview
	{
		std::wstring			Name;
		std::wstring			Path;
		ResourceHandle			Handle;
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

			virtual std::shared_ptr<Resource> Create(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, DResourceId id, bool isDefault) const = 0;
		};

	public:

		void Destroy()
		{
			mLoaded = false;
			mVersion++;
		}

		INLINE ResourcePreview		GetPreview() const
		{
			return { mName, mPath, { GetType(), mId } };
		}

		virtual ResourceType		GetType() const = 0;

		INLINE operator ResourceHandle const() { return { GetType(), mId }; }
		INLINE operator ResourcePreview const() { return GetPreview(); }

		D_CH_RW_FIELD(std::wstring, Name);
		D_CH_R_FIELD(D_FILE::Path, Path);
		D_CH_R_FIELD_CONST(DResourceId, Id);
		D_CH_R_FIELD_CONST(D_CORE::Uuid, Uuid);
		D_CH_R_FIELD_CONST(bool, Default);

		D_CH_R_FIELD(bool, Loaded);
		D_CH_R_FIELD(UINT, Version);
		D_CH_R_FIELD(bool, DirtyDisk);
		D_CH_R_FIELD(bool, DirtyGPU);

	public:
		void						UpdateGPU();

#ifdef _D_EDITOR
		virtual bool				DrawDetails(float params[]) = 0;
#endif // _D_EDITOR

#pragma region Registeration
		static INLINE ResourceType	GetResourceTypeFromName(std::string name) { return ResourceTypeMapR.contains(name) ? ResourceTypeMapR[name] : 0; }
		static INLINE std::string	GetResourceName(ResourceType type) { return ResourceTypeMap[type]; }
		static INLINE ResourceFactory* GetFactoryForResourceType(ResourceType type) { return ResourceFactories.contains(type) ? ResourceFactories[type] : nullptr; }
		static INLINE void			RegisterResourceExtension(std::string ext, ResourceType type) { auto& key = ResourceExtensionMap[ext]; key.insert(type); }
		static INLINE D_CONTAINERS::DSet<ResourceType>	GetResourceTypeByExtension(std::string ext) { return ResourceExtensionMap.contains(ext) ? ResourceExtensionMap[ext] : D_CONTAINERS::DSet<ResourceType>(); }
		static INLINE void			RegisterConstructionValidation(ResourceType type, std::function<D_CONTAINERS::DVector<ResourceDataInFile>(ResourceType, D_FILE::Path const&)> func) { ConstructValidationMap[type] = func; }
		static INLINE D_CONTAINERS::DVector<ResourceDataInFile>	CanConstructTypeFromPath(ResourceType type, D_FILE::Path const& path) { return ConstructValidationMap.contains(type) ? ConstructValidationMap[type](type, path) : D_CONTAINERS::DVector<ResourceDataInFile>(); }
#pragma endregion

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

		INLINE void					MakeDiskDirty() { mDirtyDisk = true; }
		INLINE void					MakeGpuDirty() { mDirtyGPU = true; }
		INLINE void					MakeDiskClean() { mDirtyDisk = false; }
		INLINE void					MakeGpuClean() { mDirtyGPU = false; }

		friend class DResourceManager;
		friend class ResourceLoader;

	protected:
		Resource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, DResourceId id, bool isDefault) :
			mLoaded(false),
			mPath(path),
			mName(name),
			mVersion(1),
			mId(id),
			mDefault(isDefault),
			mDirtyDisk(false),
			mDirtyGPU(true),
			mUuid(uuid)
		{
		}

		// Serialization methods get a json param as an input/output to write variation properties of the resource
		// in case the main resource file is a third-party format whose resources are being read with extra 
		// parameters provided with the json param. If you have your custom resource and need to write it's properties
		// to a file and it's likely that you are loading seperate resources from seperate files of this resource without
		// extra parameters, it is strongly suggested that you avoid writing/reading resource properties on/from the json
		// param.
		virtual void				WriteResourceToFile(D_SERIALIZATION::Json& j) const = 0;
		virtual void				ReadResourceFromFile(D_SERIALIZATION::Json const& j) = 0;

		virtual bool				UploadToGpu() = 0;

		// Unload and Evict need implementation for every resource
		virtual void				EvictFromGpu() {}
		virtual void				Unload() = 0;
		static void					AddTypeContainer(ResourceType type);

		static D_CONTAINERS::DVector<ResourceDataInFile> CanConstructFrom(ResourceType type, D_FILE::Path const& path);

		static D_CONTAINERS::DUnorderedMap<ResourceType, std::string> ResourceTypeMap;
		static D_CONTAINERS::DUnorderedMap<std::string, ResourceType> ResourceTypeMapR;
		static D_CONTAINERS::DUnorderedMap<ResourceType, ResourceFactory*> ResourceFactories;
		static D_CONTAINERS::DUnorderedMap<std::string, D_CONTAINERS::DSet<ResourceType>> ResourceExtensionMap;
		static D_CONTAINERS::DUnorderedMap<ResourceType, std::function<D_CONTAINERS::DVector<ResourceDataInFile>(ResourceType type, D_FILE::Path const&)>> ConstructValidationMap;
	};

}
