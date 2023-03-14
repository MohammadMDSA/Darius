#pragma once

#include "AnimationTypes.hpp"

#include <Core/Containers/Map.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/Resource.hpp>

#ifndef D_ANIMATION
#define D_ANIMATION Darius::Animation
#endif // !D_ANIMATION

namespace Darius::Animation
{
	class AnimationResource : public D_RESOURCE::Resource
	{
		D_CH_RESOURCE_BODY(AnimationResource, "Animation", ".fbx");

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override { return false; }
#endif // _D_EDITOR

		virtual INLINE void				WriteResourceToFile(D_SERIALIZATION::Json& json) const override { throw D_EXCEPTION::UnsupportedException(); };
		virtual void					ReadResourceFromFile(D_SERIALIZATION::Json const& json) override;
		virtual INLINE bool				UploadToGpu(void* context) override { return true; };
		virtual INLINE void				Unload() override { EvictFromGpu(); }

		D_CONTAINERS::DUnorderedMap<std::string, int> const& GetSkeletonNameIndexMap() const { return mSkeletonNameIndexMap; }

		static D_CONTAINERS::DVector<D_RESOURCE::ResourceDataInFile> CanConstructFrom(D_RESOURCE::ResourceType type, D_FILE::Path const& path);

		D_CH_R_FIELD_ACC(AnimationLayer, AnimationData, protected)
		D_CH_R_FIELD_ACC(D_CONTAINERS::DVector<AnimationCurve>, CurvesData, protected)
		D_CH_R_FIELD_ACC(D_CONTAINERS::DVector<Keyframe>, Keyframes, protected)

	protected:
		AnimationResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, bool isDefault = false) :
			Resource(uuid, path, name, id, isDefault),
			mAnimationData() {}

		D_CONTAINERS::DUnorderedMap<std::string, int> mSkeletonNameIndexMap;

	private:
		bool						GetPropertyData(int jointIndex, void* propP, void* currentLayerP, AnimationCurve& animCurve, const char* channelName);

	};
}
