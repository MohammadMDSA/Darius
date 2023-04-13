#pragma once

#include <Math/Camera/Camera.hpp>
#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#include "CameraComponent.generated.hpp"

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif

namespace Darius::Graphics
{
	class DClass(Serialize[FoV, NearClip, FarClip, OrthographicSize, AspectRatio, bInfiniteZ, bOrthographic]) CameraComponent : public D_ECS_COMP::ComponentBase
	{
		D_H_COMP_BODY(CameraComponent, D_ECS_COMP::ComponentBase, "Rendering/Camera", true);

	public:
		Darius_Graphics_CameraComponent_GENERATED

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _DEBUG

		// States
		virtual void					Update(float dt) override;
		virtual void					Awake() override;
		virtual void					OnDestroy() override;

		// Virtual properties
		INLINE float const&				GetFoV() const { return mCamera.GetFoV(); }
		INLINE float const&				GetNearClip() const { return mCamera.GetNearClip(); }
		INLINE float const&				GetFarClip() const { return mCamera.GetFarClip(); }
		INLINE float const&				GetOrthographicSize() const { return mCamera.GetOrthographicSize(); }
		INLINE float const&				GetAspectRatio() const { return mCamera.GetAspectRatio(); }
		INLINE bool	const&				IsInfiniteZ() const { return mCamera.IsInfiniteZ(); }
		INLINE bool const&				IsOrthographic() const { return mCamera.IsOrthographic(); }

		INLINE void						SetFoV(float const& val) { mCamera.SetFoV(val); }
		INLINE void						SetNearClip(float const& val) { mCamera.SetZRange(val, mCamera.GetFarClip()); }
		INLINE void						SetFarClip(float const& val) { mCamera.SetZRange(mCamera.GetNearClip(), val); }
		INLINE void						SetOrthographicSize(float const& val) { mCamera.SetOrthographicSize(val); }
		INLINE void						SetAspectRatio(float const& val) { mCamera.SetAspectRatio(val); }
		INLINE void						SetInfiniteZ(bool const& val) { mCamera.SetInfiniteZ(val); }
		INLINE void						SetOrthographic(bool const& val) { mCamera.SetOrthographic(val); }

	private:

		DField(Get[const, &, inline])
		D_MATH_CAMERA::Camera			mCamera;

	};
}

File_CameraComponent_GENERATED
