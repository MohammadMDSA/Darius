#pragma once

#include <Math/Camera/Camera.hpp>
#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif

namespace Darius::Graphics
{
	class CameraComponent : public D_ECS_COMP::ComponentBase
	{
		D_H_COMP_BODY(CameraComponent, D_ECS_COMP::ComponentBase, "Rendering/Camera", true);

	public:

#ifdef _DEBUG
		virtual bool					DrawDetails(float params[]) override;
#endif // _DEBUG

		// Serialization
		virtual void					Serialize(Json& j) const override;
		virtual void					Deserialize(Json const& j) override;

		// States
		virtual void					Awake() override { }
		virtual void					OnDestroy() override {  }

		INLINE D_MATH_CAMERA::Camera* operator->() { return &mCamera; }
		//INLINE D_MATH_CAMERA::Camera const* operator->() const { return &mCamera; }

		D_CH_R_FIELD(D_MATH_CAMERA::Camera, Camera);

	};
}