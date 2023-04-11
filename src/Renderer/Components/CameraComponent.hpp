#pragma once

#include <Math/Camera/Camera.hpp>
#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#include "CameraComponent.generated.hpp"

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif

namespace Darius::Graphics
{
	class DClass(Serialize) CameraComponent : public D_ECS_COMP::ComponentBase
	{
		D_H_COMP_BODY(CameraComponent, D_ECS_COMP::ComponentBase, "Rendering/Camera", true);

	public:
		Darius_Graphics_CameraComponent_GENERATED

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _DEBUG

		// Serialization
		virtual void					Serialize(D_SERIALIZATION::Json& j) const override;
		virtual void					Deserialize(D_SERIALIZATION::Json const& j) override;

		// States
		virtual void					Update(float dt) override;
		virtual void					Awake() override;
		virtual void					OnDestroy() override;

		INLINE D_MATH_CAMERA::Camera* operator->() { return &mCamera; }
		//INLINE D_MATH_CAMERA::Camera const* operator->() const { return &mCamera; }

	private:

		DField(Get[const, &, inline])
		D_MATH_CAMERA::Camera			mCamera;

	};
}

File_CameraComponent_GENERATED
