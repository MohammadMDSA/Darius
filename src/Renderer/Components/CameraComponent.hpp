#pragma once

#include "Renderer/Resources/TextureResource.hpp"

#include <Math/Camera/Camera.hpp>
#include <ResourceManager/ResourceRef.hpp>
#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#include "CameraComponent.generated.hpp"

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Renderer
{
	class DClass(Serialize[FoV, NearClip, FarClip, OrthographicSize, bInfiniteZ, bOrthographic]) CameraComponent : public D_ECS_COMP::ComponentBase
	{
		GENERATED_BODY();
		D_H_COMP_BODY(CameraComponent, D_ECS_COMP::ComponentBase, "Rendering/Camera", true);

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _DEBUG

		// States
		virtual void					Update(float dt) override;
		virtual void					Awake() override;
		virtual void					OnDestroy() override;

		// Virtual properties
		INLINE float					GetFoV() const { return mCamera.GetFoV(); }
		INLINE float					GetNearClip() const { return mCamera.GetNearClip(); }
		INLINE float					GetFarClip() const { return mCamera.GetFarClip(); }
		INLINE float					GetOrthographicSize() const { return mCamera.GetOrthographicSize(); }
		INLINE float					GetAspectRatio() const { return mCamera.GetAspectRatio(); }
		INLINE bool						IsInfiniteZ() const { return mCamera.IsInfiniteZ(); }
		INLINE bool						IsOrthographic() const { return mCamera.IsOrthographic(); }

		void							SetFoV(float val);
		void							SetNearClip(float val);
		void							SetFarClip(float val);
		void							SetOrthographicSize(float val);

		// For internal use. DO NOT CALL!
		void							SetAspectRatio(float val);
		void							SetInfiniteZ(bool val);
		void							SetOrthographic(bool val);

		INLINE TextureResource*			GetSkyboxSpecularTexture() const { return mSkyboxSpecular.Get(); }
		INLINE TextureResource*			GetSkyboxDiffuseTexture() const { return mSkyboxDiffuse.Get(); }
		void							SetSkyboxSpecularTexture(TextureResource* texture);
		void							SetSkyboxDiffuseTexture(TextureResource* texture);

		INLINE D_MATH_CAMERA::Camera const& GetCameraMath() const { return mCamera; }

	private:
		DField()
		D_MATH_CAMERA::Camera			mCamera;

		DField(Serialize)
		D_RESOURCE::ResourceRef<TextureResource> mSkyboxSpecular;

		DField(Serialize)
		D_RESOURCE::ResourceRef<TextureResource> mSkyboxDiffuse;

	};
}

File_CameraComponent_GENERATED
