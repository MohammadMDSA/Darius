#pragma once

#include <Utils/DragDropPayload.hpp>

#ifndef D_RESOURCE
#define D_RESOURCE Darius::ResourceManager
#endif // !D_RESOURCE

namespace Darius::ResourceManager
{

#ifdef _D_EDITOR

	struct ResourceDragDropPayloadContent : public D_UTILS::BaseDragDropPayloadContent
	{
		ResourceDragDropPayloadContent() :
			BaseDragDropPayloadContent(BaseDragDropPayloadContent::Type::Resource) { }

		virtual inline bool IsCompatible(D_UTILS::BaseDragDropPayloadContent::Type payloadType, D_CORE::StringId const& type) const override
		{
			return payloadType == D_UTILS::BaseDragDropPayloadContent::Type::Resource && std::strcmp(type.string(), Type.c_str());
		}

		std::string Type;
		D_RESOURCE::ResourceHandle Handle = D_RESOURCE::EmptyResourceHandle;

	};

#endif

}