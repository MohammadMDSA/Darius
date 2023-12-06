#pragma once

#include "AnimationTypes.hpp"

#include <Core/Containers/Map.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/Resource.hpp>

#include "AnimationResource.generated.hpp"

#ifndef D_ANIMATION
#define D_ANIMATION Darius::Animation
#endif // !D_ANIMATION

namespace Darius::Animation
{
	class DClass(Serialize, Resource) AnimationResource : public D_RESOURCE::Resource
	{
		D_CH_RESOURCE_BODY(AnimationResource, "Animation", ".fbx");
		GENERATED_BODY();

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override { return false; }
#endif // _D_EDITOR

		virtual INLINE void				WriteResourceToFile(D_SERIALIZATION::Json& json) const override { throw D_EXCEPTION::UnsupportedException(); };
		virtual void					ReadResourceFromFile(D_SERIALIZATION::Json const& json) override;
		virtual INLINE bool				UploadToGpu() override { return true; };
		virtual INLINE void				Unload() override { EvictFromGpu(); }

		INLINE virtual bool				AreDependenciesDirty() const override { return false; }

		INLINE AnimationLayer			GetAnimationData() const { return mAnimationData; }
		INLINE D_CONTAINERS::DVector<AnimationCurve> GetCurvesData() const { return mCurvesData; }
		INLINE D_CONTAINERS::DVector<Keyframe> GetKeyframes() const { return mKeyframes; }
		INLINE D_CONTAINERS::DUnorderedMap<std::string, int> GetSkeletonNameIndexMap() const { return mSkeletonNameIndexMap; }

		static D_CONTAINERS::DVector<D_RESOURCE::ResourceDataInFile> CanConstructFrom(D_RESOURCE::ResourceType type, D_FILE::Path const& path);

	protected:
		AnimationResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, bool isDefault = false) :
			Resource(uuid, path, name, id, isDefault),
			mAnimationData() {}

		
		DField()
		AnimationLayer							mAnimationData;
		
		DField()
		D_CONTAINERS::DVector<AnimationCurve>	mCurvesData;
		
		DField()
		D_CONTAINERS::DVector<Keyframe>			mKeyframes;

		DField()
		D_CONTAINERS::DUnorderedMap<std::string, int> mSkeletonNameIndexMap;

	private:
		bool						GetPropertyData(int jointIndex, void* propP, void* currentLayerP, AnimationCurve& animCurve, const char* channelName);
	};
}

File_AnimationResource_GENERATED