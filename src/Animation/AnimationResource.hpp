#pragma once

#include "AnimationCommon.hpp"

#include <Core/Containers/Map.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#include "AnimationResource.generated.hpp"

#ifndef D_ANIMATION
#define D_ANIMATION Darius::Animation
#endif // !D_ANIMATION

namespace Darius::Animation
{
	struct DStruct(Serialize) ComponentAnimationData
	{
		GENERATED_BODY();

	public:

		DField(Serialize)
		D_CORE::StringId							ComponentName = ""_Comp;

		DField(Serialize)
		Sequence									AnimationSequence;
	};

	class DClass(Serialize, Resource) AnimationResource : public D_RESOURCE::Resource
	{
		D_CH_RESOURCE_BODY(AnimationResource, "Animation", ".anim");
		GENERATED_BODY();

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override { return false; }
#endif // _D_EDITOR

		virtual void					WriteResourceToFile(D_SERIALIZATION::Json& json) const override;
		virtual void					ReadResourceFromFile(D_SERIALIZATION::Json const& json, bool& dirtyDisk) override;
		virtual INLINE bool				UploadToGpu() override { return true; };
		virtual INLINE void				Unload() override;

		INLINE virtual bool				AreDependenciesDirty() const override { return false; }

		void							CreateSkeletalAnimation(Sequence const& seq, D_CONTAINERS::DUnorderedMap<D_CORE::StringId, int> const& jointNameIndexMap);
		void							SetFrameRate(float frameRate);

		INLINE Sequence const&			GetSkeletalAnimationSequence() const { return mSkeletalAnimationSequence; }
		INLINE D_CONTAINERS::DVector<ComponentAnimationData> const& GetComponentAnimationData() const { return mComponentAnimation; }
		INLINE D_CONTAINERS::DVector<ComponentAnimationData>& GetComponentAnimationData() { return mComponentAnimation; }
		INLINE D_CONTAINERS::DUnorderedMap<D_CORE::StringId, int> GetSkeletonNameIndexMap() const { return mSkeletonNameIndexMap; }

		INLINE bool						IsSkeletalAnimation() const { return mSkeletalAnimation; }
		NODISCARD float					GetStartTime() const;
		NODISCARD float					GetEndTime() const;
		NODISCARD INLINE UINT			GetFramesPerSecond() const { return mFramesPerSecond; }
		NODISCARD INLINE float			GetDuration() const { return GetEndTime() - GetStartTime(); }
		NODISCARD INLINE UINT			GetFirstFrame() const { return (UINT)(GetStartTime() * mFramesPerSecond); }
		NODISCARD INLINE UINT			GetLastFrame() const { return (UINT)(GetEndTime() * mFramesPerSecond); }


		static D_RESOURCE::SubResourceConstructionData CanConstructFrom(D_RESOURCE::ResourceType type, D_FILE::Path const& path);

	protected:
		AnimationResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, D_RESOURCE::Resource* parent, bool isDefault = false);

		virtual void					ReadNativeAnimationFromFile(D_SERIALIZATION::Json const& json);
		
		DField()
		Sequence								mSkeletalAnimationSequence;
		
		DField(Serialize)
		D_CONTAINERS::DVector<ComponentAnimationData> mComponentAnimation;

		DField(Serialize)
		UINT									mFramesPerSecond;

		DField()
		D_CONTAINERS::DUnorderedMap<D_CORE::StringId, int> mSkeletonNameIndexMap;

		DField()
		bool									mSkeletalAnimation;

	};
}

File_AnimationResource_GENERATED