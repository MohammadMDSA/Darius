#pragma once

#include <Utils/DragDropPayload.hpp>

#ifndef D_SCENE
#define D_SCENE Darius::Scene
#endif // !D_SCENE

namespace Darius::Scene
{

#ifdef _D_EDITOR

#define D_PAYLOAD_TYPE_GAMEOBJECT "$PAYLOAD$$GAMEOBJECT$"

	struct GameObjectDragDropPayloadContent : public D_UTILS::BaseDragDropPayloadContent
	{

		GameObjectDragDropPayloadContent() :
			BaseDragDropPayloadContent(BaseDragDropPayloadContent::Type::GameObject) { }

		virtual inline bool IsCompatible(D_UTILS::BaseDragDropPayloadContent::Type payloadType, std::string const& type) const override
		{
			return payloadType == D_UTILS::BaseDragDropPayloadContent::Type::GameObject;
		}

		D_SCENE::GameObject* GameObject = nullptr;
	};

#endif

}