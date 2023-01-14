#pragma once

#ifndef D_UTILS
#define D_UTILS Darius::Utils
#endif

namespace Darius::Utils
{

#define	D_PAYLOAD_TYPE_RESOURCE "$PAYLOAD$$RESOURCE$"

	struct BaseDragDropPayloadContent
	{
		virtual bool IsCompatible(std::string const& type) const = 0;
	};

	struct ResourceDragDropPayloadContent : public BaseDragDropPayloadContent
	{
		std::string Type;
		D_RESOURCE::ResourceHandle Handle;

		virtual inline bool IsCompatible(std::string const& type) const
		{
			return type == Type;
		}
	};

	struct DDragDropPayload
	{
		enum class Type
		{
			Resource
		};

		Type PayloadType;

		ResourceDragDropPayloadContent ResourcePayload;

		inline bool IsCompatible(Type type, std::string dataType) const
		{
			if (PayloadType != type)
				return false;
			switch (type)
			{
			case Type::Resource:
				return ResourcePayload.IsCompatible(dataType);
			default:
				return false;
			}
		}
	};

}
