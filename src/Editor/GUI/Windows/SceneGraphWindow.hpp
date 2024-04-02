#pragma once

#include "Window.hpp"

#include <Core/Containers/Vector.hpp>
#include <Scene/GameObject.hpp>

namespace Darius::Editor::Gui::Windows
{
	class SceneGraphWindow : public Window
	{
		D_CH_EDITOR_WINDOW_BODY_RAW(SceneGraphWindow, "Scene Graph");

	public:
		SceneGraphWindow(D_SERIALIZATION::Json& config);
		~SceneGraphWindow() = default;

		SceneGraphWindow(SceneGraphWindow const& other) = delete;

		virtual void Update(float dt) override;
		virtual void DrawGUI() override;

	private:

		void DrawObject(D_SCENE::GameObject* go, D_SCENE::GameObject* selectedObj, bool& abort);
	};

}
