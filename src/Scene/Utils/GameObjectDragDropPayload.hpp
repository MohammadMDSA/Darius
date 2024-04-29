#pragma once

#include <Utils/DragDropPayload.hpp>

#ifndef D_SCENE
#define D_SCENE Darius::Scene
#endif // !D_SCENE

namespace Darius::Scene
{

#ifdef _D_EDITOR

#define D_PAYLOAD_TYPE_GAMEOBJECT "$PAYLOAD$$GAMEOBJECT$"
#define D_PAYLOAD_TYPE_COMPONENT "$PAYLOAD$$COMPONENT$"
#define D_PAYLOAD_GAMEOBJECT_HIERARCHY_TYPE "HIERARCHY"

	struct GameObjectDragDropPayloadContent : public D_UTILS::BaseDragDropPayloadContent
	{

		GameObjectDragDropPayloadContent() :
			BaseDragDropPayloadContent(BaseDragDropPayloadContent::Type::GameObject) { }

		virtual inline bool IsCompatible(D_UTILS::BaseDragDropPayloadContent::Type payloadType, std::string const& type) const override
		{

			if (payloadType == D_UTILS::BaseDragDropPayloadContent::Type::GameObject && type == D_PAYLOAD_TYPE_GAMEOBJECT)
				return true;

			if (payloadType == D_UTILS::BaseDragDropPayloadContent::Type::Component && GameObjectRef && GameObjectRef->IsValid())
				return GameObjectRef->HasComponent(type);

			return false;
		}

		D_SCENE::GameObject* GameObjectRef = nullptr;
	};

	//struct ComponentDragDropPayloadContent : public D_UTILS::BaseDragDropPayloadContent
	//{
	//	ComponentDragDropPayloadContent() :
	//		BaseDragDropPayloadContent(BaseDragDropPayloadContent::Type::Component) { }

	//	virtual inline bool IsCompatible(D_UTILS::BaseDragDropPayloadContent::Type payloadType, std::string const& type) const override
	//	{
	//		
	//	}

	//	std::string ComponentName;
	//};

#endif

}