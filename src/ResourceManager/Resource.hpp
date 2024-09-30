#pragma once

#include <Core/StringId.hpp>
#include <Core/Memory/Memory.hpp>
#include <Core/Containers/Map.hpp>
#include <Core/Containers/Set.hpp>
#include <Core/Containers/Vector.hpp>
#include <Core/Exceptions/Exception.hpp>
#include <Core/Filesystem/FileUtils.hpp>
#include <Core/Filesystem/Path.hpp>
#include <Core/RefCounting/Counted.hpp>
#include <Core/RefCounting/Ref.hpp>
#include <Core/Serialization/Json.hpp>
#include <Core/Signal.hpp>
#include <Core/Uuid.hpp>
#include <Utils/Common.hpp>
#include <Utils/Assert.hpp>
#include <Utils/Detailed.hpp>

#include <rttr/registration_friend.h>
#include <rttr/rttr_enable.h>

#include <atomic>

#include "Resource.generated.hpp"

#ifndef D_RESOURCE
#define D_RESOURCE Darius::ResourceManager
#endif // !D_RESOURCE

#define D_T_RESOURCE_ID UINT16

#define D_CH_RESOURCE_ABSTRACT_BODY(T) \
public:\
static INLINE std::string ClassName() { return D_NAMEOF(T); } \

// TODO: Better resource allocation
#define D_CH_RESOURCE_BODY(T, ResT, ...) \
D_CH_RESOURCE_ABSTRACT_BODY(T) \
public: \
	class T##Factory : public D_RESOURCE::Resource::ResourceFactory \
	{ \
	public: \
		virtual std::shared_ptr<D_RESOURCE::Resource> Create(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, D_RESOURCE::Resource* parent, bool isDefault) const; \
	}; \
public: \
	friend class Factory; \
	virtual INLINE rttr::type GetResourceReflectionType() const override { return rttr::type::get<T>(); } \
	static INLINE D_RESOURCE::ResourceType GetResourceType() { return D_RESOURCE::Resource::GetResourceTypeFromName(*T::NameId); } \
	INLINE D_RESOURCE::ResourceType GetType() const override { return D_RESOURCE::Resource::GetResourceTypeFromName(*T::NameId); } \
	virtual INLINE D_CORE::StringId GetResourceTypeName() const override { return *T::NameId; } \
	static INLINE D_CORE::StringId GetResourceTypeNameStatic() { return *T::NameId; } \
	static void Register() \
	{ \
		if(rttr::type::get<T>() == rttr::type::get<D_RESOURCE::Resource>()) \
		{ \
			Resource::ResourceTypeMap[0] = ""_SId; \
			Resource::ResourceTypeMapR[""_SId] = 0; \
			Resource::ResourceFactories[0] = nullptr; \
		}\
		T::NameId = std::make_unique<D_CORE::StringId>(D_CORE::StringId(ResT)); \
		D_ASSERT_M(!D_RESOURCE::Resource::GetResourceTypeFromName(*NameId), "Resource " #T " is already registered."); \
		auto resType = D_RESOURCE::Resource::RegisterResourceTypeName<T, T::T##Factory>(*NameId); \
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
	private: \
	static std::unique_ptr<D_CORE::StringId> NameId;

// TODO: Better resource allocation
#define D_CH_RESOURCE_DEF(T) \
std::unique_ptr<D_CORE::StringId> T::NameId; \
std::shared_ptr<D_RESOURCE::Resource> T::T##Factory::Create(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, D_RESOURCE::Resource* parent, bool isDefault) const { return std::shared_ptr<D_RESOURCE::Resource>(new T(uuid, path, name, id, parent, isDefault)); } \

#if _D_EDITOR

#define D_H_RESOURCE_DRAG_DROP_DESTINATION(resourceType, handleSetter) \
{\
	if(ImGui::BeginDragDropTarget()) \
    { \
        ImGuiPayload const* imPayload = ImGui::GetDragDropPayload(); \
        D_UTILS::BaseDragDropPayloadContent const* payload = (D_UTILS::BaseDragDropPayloadContent const*)(imPayload->Data); \
		if(payload && payload->PayloadType != D_UTILS::BaseDragDropPayloadContent::Type::Invalid && payload->IsCompatible(Darius::Utils::BaseDragDropPayloadContent::Type::Resource, resourceType::GetResourceTypeNameStatic())) \
		{ \
			if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload(D_PAYLOAD_TYPE_RESOURCE)) \
			{ \
				handleSetter(static_cast<resourceType*>(D_RESOURCE::GetRawResourceSync(reinterpret_cast<D_RESOURCE::ResourceDragDropPayloadContent const*>(imPayload->Data)->Handle))); \
			} \
		} \
		ImGui::EndDragDropTarget(); \
	} \
}


#define D_H_RESOURCE_SELECTION_DRAW(resourceType, prop, placeHolder, handleFunction, ...) \
{ \
	static char search[100]; \
	static std::wstring searchStr; \
	static D_CONTAINERS::DVector<D_RESOURCE::ResourcePreview> previews; \
	static D_CONTAINERS::DVector<D_RESOURCE::ResourcePreview> filteredPreviews; \
    resourceType* currentResource = prop.Get(); \
     \
    std::string cuurrentResourceName; \
    if(prop.IsValid()) \
    { \
        auto nameW = prop->GetName(); \
        cuurrentResourceName = WSTR2STR(nameW); \
    } \
    else \
        cuurrentResourceName = placeHolder; \
    auto availableSpace = ImGui::GetContentRegionAvail(); \
    auto selectorWidth = 20.f; \
    if(ImGui::Button(cuurrentResourceName.c_str(), ImVec2(availableSpace.x - 2 * selectorWidth - 10.f, 0)) && prop.IsValid()) \
		RequestPathChange(prop->GetPath().parent_path(), prop->GetHandle(), ImGui::IsKeyDown(ImGuiKey_LeftCtrl)); \
    D_H_RESOURCE_DRAG_DROP_DESTINATION(resourceType, handleFunction) \
	ImGui::SameLine(availableSpace.x - selectorWidth); \
	std::string popupName = (std::string(placeHolder" Res ") + std::string(__VA_ARGS__)); \
	static bool gainSearchFocus; \
	gainSearchFocus = false; \
	if (ImGui::Button((std::string(ICON_FA_ELLIPSIS_VERTICAL) + std::string("##" placeHolder) + std::string(__VA_ARGS__)).c_str(), ImVec2(selectorWidth, 0))) \
	{ \
		ImGui::OpenPopup(popupName.c_str()); \
		for(int i = 0; i < 100; i++) search[i] = 0; \
		searchStr = L""; \
		gainSearchFocus = true; \
		previews = D_RESOURCE::GetResourcePreviews(resourceType::GetResourceType()); \
		filteredPreviews = previews; \
	} \
	\
	if (ImGui::BeginPopup(popupName.c_str())) \
	{ \
		int idx = 0; \
		if(gainSearchFocus) ImGui::SetKeyboardFocusHere(); \
		if(ImGui::InputText((std::string("##Search"placeHolder) + std::string(__VA_ARGS__)).c_str(), search, 100)) \
		{ \
			std::string searchTmp(search); \
			searchStr = STR2WSTR(std::string(searchTmp)); \
			boost::algorithm::to_lower(searchStr); \
			filteredPreviews.clear(); \
			if(!searchStr.empty()) \
				for (auto const& prev : previews) \
				{ \
					if(boost::algorithm::to_lower_copy(prev.Name).starts_with(searchStr)) \
						filteredPreviews.push_back(prev); \
				} \
			else \
				filteredPreviews = previews; \
		} \
		bool noneSelected = currentResource == nullptr; \
		if(ImGui::Selectable("<None>", &noneSelected)) \
		{ \
			handleFunction(nullptr); \
			valueChanged = true; \
		} \
		\
		ImGuiListClipper clipper; \
		clipper.Begin((int)filteredPreviews.size()); \
		while(clipper.Step()) \
		{ \
			for (int idx = clipper.DisplayStart; idx < clipper.DisplayEnd; idx++) \
			{ \
				auto const& prev = filteredPreviews[idx]; \
				bool selected = currentResource && prev.Handle.Id == currentResource->GetId() && prev.Handle.Type == currentResource->GetType(); \
				\
				auto Name = WSTR2STR(prev.Name); \
				ImGui::PushID((Name + std::to_string(idx)).c_str()); \
				if (ImGui::Selectable(Name.c_str(), &selected)) \
				{ \
					handleFunction(static_cast<resourceType*>(D_RESOURCE::GetRawResourceSync(prev.Handle))); \
					valueChanged = true; \
				} \
				ImGui::PopID(); \
				\
			} \
		} \
		\
		ImGui::EndPopup(); \
	} \
}
#define D_H_RESOURCE_SELECTION_DRAW_SHORT(type, name, label) D_H_RESOURCE_SELECTION_DRAW(type, m##name, label, Set##name)
#define D_H_RESOURCE_SELECTION_DRAW_DEF(type, name) D_H_RESOURCE_SELECTION_DRAW_SHORT(type, name, "Select " #name)


#endif // _D_EDITOR


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
		ResourceDataInFile							Parent =
		{
			.Name = "",
			.Type = 0
		};
		D_CONTAINERS::DVector<ResourceDataInFile>	Resources;
	};

	struct ResourceHandle
	{
		ResourceType			Type = 0;
		DResourceId				Id = 0;

		INLINE bool IsValid() const { return Type != 0; }
		INLINE bool operator== (ResourceHandle const& other) const { return other.Id == Id && other.Type == Type; }
	};

	constexpr ResourceHandle EmptyResourceHandle = {0, 0};

	struct SubResourceConstructionData
	{
		ResourceDataInFile Parent =
		{
			.Name = "",
			.Type = 0
		};
		D_CONTAINERS::DVector<ResourceDataInFile> SubResources;
	};

#ifdef _D_EDITOR
	struct ResourcePreview
	{
		std::wstring			Name;
		D_FILE::Path			Path;
		ResourceHandle			Handle;
	};
#endif

	D_CORE::StringId ResourceTypeToString(ResourceType type);
	ResourceType StringToResourceType(D_CORE::StringId type);

	enum class ResourceGpuUpdateResult
	{
		Success,
		AlreadyClean,
		DirtyDependency
	};

	class DClass(Serialize) Resource : public D_CORE::Counted, public Detailed
	{
		GENERATED_BODY();

	public:

		enum class GPUDirtyState : uint8_t
		{
			Clean,
			Uploading,
			Dirty
		};

		class ResourceFactory
		{
		public:
			ResourceFactory() = default;
			~ResourceFactory() = default;

			virtual std::shared_ptr<Resource> Create(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, DResourceId id, D_RESOURCE::Resource* parent, bool isDefault) const = 0;
		};
	public:

		virtual ~Resource() { };

		void Destroy()
		{
			mVersion++;
		}

#if _D_EDITOR
		INLINE ResourcePreview		GetPreview() const
		{
			return {mName, mPath, { GetType(), mId }};
		}
#endif

		virtual ResourceType		GetType() const = 0;

		virtual INLINE rttr::type	GetResourceReflectionType() const { return rttr::type::get<Resource>(); }
		virtual INLINE D_CORE::StringId	GetResourceTypeName() const = 0;

		INLINE bool					IsSelfDirtyGPU() const { return mDirtyGPU.load() != GPUDirtyState::Clean; }
		INLINE bool					IsDirtyGPU() const { return IsSelfDirtyGPU() || AreDependenciesDirty(); }
		INLINE GPUDirtyState		GetGpuState() const { return mDirtyGPU.load(); }
		INLINE void					SetUploading() { D_ASSERT(GetGpuState() == GPUDirtyState::Dirty); mDirtyGPU.store(GPUDirtyState::Uploading); }
		INLINE void					SetUploadComplete() { D_ASSERT(GetGpuState() == GPUDirtyState::Uploading); mDirtyGPU.store(GPUDirtyState::Clean); }
		virtual bool				AreDependenciesDirty() const = 0;
		INLINE bool					IsLocked() const { return mLocked.load(); }
		INLINE void					SetLocked(bool value) { mLocked.store(value); }
		// This calling this is discauraged and might pull the resource in an undefined state
		INLINE void					_SetLoaded(bool loaded) { mLoaded.store(loaded); }

		INLINE operator ResourceHandle() const { return GetHandle(); }
		INLINE ResourceHandle		GetHandle() const { return {GetType(), mId}; }
#if _D_EDITOR
		INLINE operator ResourcePreview() const { return GetPreview(); }
		INLINE virtual std::string	GetDetailedName() const override
		{
			std::string name = WSTR2STR(GetName());
			return name;
		}

		INLINE virtual Detailed*	GetDetailedParent() const override { return mParent; }
#endif
		INLINE D_FILE::Path const&	GetPath() const { return mPath; }
		INLINE virtual bool			IsLoaded() const { return mLoaded.load(); }
		INLINE unsigned int			GetVersion() const { return mVersion; }
		INLINE bool					IsDirtyDisk() const { return mDirtyDisk; }
		INLINE DResourceId			GetId() const { return mId; }
		INLINE D_CORE::Uuid const&	GetUuid() const { return mUuid; }
		INLINE bool					IsDefault() const { return mDefault; }
		INLINE std::wstring const&	GetName() const { return mName; }

		INLINE void					SetName(std::wstring const& name) { mName = name; }

	public:

#if _D_EDITOR
		static D_CORE::Signal<void(D_FILE::Path const&, ResourceHandle const&, bool selected)> RequestPathChange;
#endif // _D_EDITOR


	private:

		DField()
		D_FILE::Path				mPath;

		std::atomic_bool			mLoaded;

		DField()
		unsigned int				mVersion;

		DField()
		bool						mDirtyDisk;

		std::atomic<GPUDirtyState>	mDirtyGPU;

		DField()
		const DResourceId			mId;

		DField()
		const D_CORE::Uuid			mUuid;

		DField()
		const bool					mDefault;

		Resource* const				mParent;

		DField()
		std::wstring				mName;

		// For saving / loading / manipulation purposes
		std::atomic_bool			mLocked;


	public:
		ResourceGpuUpdateResult		UpdateGPU();

#ifdef _D_EDITOR
		// The result of this method is obsolete
		virtual bool				DrawDetails(float params[]) = 0;
		virtual bool				IsEditableInDetailsWindow() const;
#endif // _D_EDITOR

#pragma region Registeration
		static INLINE ResourceType	GetResourceTypeFromName(D_CORE::StringId name) { return ResourceTypeMapR.contains(name) ? ResourceTypeMapR[name] : 0; }
		static INLINE D_CORE::StringId	GetResourceName(ResourceType type) { return ResourceTypeMap.at(type); }
		static INLINE ResourceFactory* GetFactoryForResourceType(ResourceType type) { return ResourceFactories.contains(type) ? ResourceFactories[type] : nullptr; }
		static INLINE void			RegisterResourceExtension(std::string ext, ResourceType type) { D_ASSERT_M(!ResourceExtensionMap.contains(ext), "Only one resource type per extension is allowed"); ResourceExtensionMap[ext] = type; }
		static INLINE std::optional<ResourceType>	GetResourceTypeByExtension(std::string ext) { auto find = ResourceExtensionMap.find(ext); if(find == ResourceExtensionMap.end()) return std::nullopt; return find->second; }
		static INLINE void			RegisterConstructionValidation(ResourceType type, std::function<SubResourceConstructionData(ResourceType, D_FILE::Path const&)> func) { ConstructValidationMap[type] = func; }
		static INLINE SubResourceConstructionData	CanConstructTypeFromPath(ResourceType type, D_FILE::Path const& path) { return ConstructValidationMap.contains(type) ? ConstructValidationMap[type](type, path) : SubResourceConstructionData(); }
#pragma endregion

		template<class R, class FAC>
		static ResourceType			RegisterResourceTypeName(D_CORE::StringId const& name)
		{
			// Checking if T is a resource type
			using conv = std::is_convertible<R*, Resource*>;
			D_STATIC_ASSERT(conv::value);
			using convFac = std::is_convertible<FAC*, ResourceFactory*>;
			D_STATIC_ASSERT(convFac::value);

			// TODO: Better allocation
			ResourceType type = (ResourceType)(ResourceTypeMap.size() + 1);
			ResourceTypeMap.insert({type, name});
			ResourceTypeMapR[name] = type;
			ResourceFactories.insert({type, new FAC});

			return type;
		}

		INLINE void					MakeDiskDirty() { D_ASSERT_M(!mLocked, "Resource cannot be manipulated while locked."); if(!IsDefault()) mDirtyDisk = true; }
		INLINE void					MakeGpuDirty() { if(!IsDefault()) mDirtyGPU.store(GPUDirtyState::Dirty); }
		INLINE void					MakeDiskClean() { mDirtyDisk = false; }
		INLINE void					MakeGpuClean() { mDirtyGPU.store(GPUDirtyState::Clean); }

		friend class DResourceManager;
		friend class ResourceLoader;

		template<class T>
		friend class D_CORE::Ref;

	protected:
		Resource(D_CORE::Uuid const& uuid, std::wstring const& path, std::wstring const& name, DResourceId id, Resource * parent, bool isDefault) :
			mLoaded(isDefault),
			mPath(path),
			mName(name),
			mVersion(1),
			mId(id),
			mDefault(isDefault),
			mDirtyDisk(false),
			mDirtyGPU(GPUDirtyState::Dirty),
			mUuid(uuid),
			mLocked(false),
			mParent(parent)
		{ }

		// Serialization methods get a json param as an input/output to write variation properties of the resource
		// in case the main resource file is a third-party format whose resources are being read with extra 
		// parameters provided with the json param. If you have your custom resource and need to write it's properties
		// to a file and it's likely that you are loading seperate resources from seperate files of this resource without
		// extra parameters, it is strongly suggested that you avoid writing/reading resource properties on/from the json
		// param.
		virtual bool				WriteResourceToFile(D_SERIALIZATION::Json & j) const = 0;
		virtual void				ReadResourceFromFile(D_SERIALIZATION::Json const& j, bool& dirtyDisk) = 0;

		virtual bool				UploadToGpu() = 0;

		// Unload and Evict need implementation for every resource
		virtual void				EvictFromGpu() { }
		virtual void				Unload() = 0;

		// Don't trigger change signal inside this. It'll break the whole thing!
		virtual void				OnChange() { }
		void						SignalChange();
		virtual bool				Release() override;

		D_CH_SIGNAL(Change, void(Resource*));

		static void					AddTypeContainer(ResourceType type);

		static SubResourceConstructionData CanConstructFrom(ResourceType type, D_FILE::Path const& path);

		static D_CONTAINERS::DUnorderedMap<ResourceType, D_CORE::StringId> ResourceTypeMap;
		static D_CONTAINERS::DUnorderedMap<D_CORE::StringId, ResourceType> ResourceTypeMapR;
		static D_CONTAINERS::DUnorderedMap<ResourceType, ResourceFactory*> ResourceFactories;
		static D_CONTAINERS::DUnorderedMap<std::string, ResourceType> ResourceExtensionMap;
		static D_CONTAINERS::DUnorderedMap<ResourceType, std::function<SubResourceConstructionData(ResourceType type, D_FILE::Path const&)>> ConstructValidationMap;
	};

#if _D_EDITOR
	INLINE bool Resource::IsEditableInDetailsWindow() const
	{
		if(IsDefault())
			return false;
		return true;
	}
#endif

}

File_Resource_GENERATED