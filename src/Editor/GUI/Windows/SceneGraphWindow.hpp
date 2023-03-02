#pragma once

#include "Window.hpp"

#include <Core/Containers/Vector.hpp>
#include <Scene/GameObject.hpp>

using namespace D_CONTAINERS;
using namespace D_SCENE;

namespace Darius::Editor::Gui::Windows
{
	class SceneGraphWindow : public Window
	{
		D_CH_EDITOR_WINDOW_BODY(SceneGraphWindow, "Scene Graph");

	public:
		SceneGraphWindow(D_SERIALIZATION::Json const& config);
		~SceneGraphWindow() = default;

		SceneGraphWindow(SceneGraphWindow const& other) = delete;

		virtual void Render(D_GRAPHICS::GraphicsContext& context) override;
		virtual void Update(float dt) override;
		virtual void DrawGUI() override;

	private:

		void DrawObject(GameObject* go, GameObject* selectedObj);
	};

}
