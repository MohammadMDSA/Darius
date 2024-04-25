#pragma once

#include <Math/Camera/Camera.hpp>
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

		INLINE void						SetFoV(float val) { mCamera.SetFoV(val); }
		INLINE void						SetNearClip(float val) { mCamera.SetZRange(val, mCamera.GetFarClip()); }
		INLINE void						SetFarClip(float val) { mCamera.SetZRange(mCamera.GetNearClip(), val); }
		INLINE void						SetOrthographicSize(float val) { mCamera.SetOrthographicSize(val); }

		// For internal use. DO NOT CALL!
		INLINE void						SetAspectRatio(float val) { mCamera.SetAspectRatio(val); }
		INLINE void						SetInfiniteZ(bool val) { mCamera.SetInfiniteZ(val); }
		INLINE void						SetOrthographic(bool val) { mCamera.SetOrthographic(val); }

	private:
		DField(Get[const, &, inline])
		D_MATH_CAMERA::Camera			mCamera;

	};
}

File_CameraComponent_GENERATED
