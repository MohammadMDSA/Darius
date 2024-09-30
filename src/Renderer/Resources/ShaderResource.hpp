#pragma once

#include <Core/Containers/Vector.hpp>
#include <ResourceManager/Resource.hpp>

#include "ShaderResource.generated.hpp"

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Renderer
{
	class DClass(Serialize, Resource) ShaderResource : public D_RESOURCE::Resource
	{
		GENERATED_BODY();
		D_CH_RESOURCE_BODY(ShaderResource, "Shader", ".hlsl");

	public:


#ifdef _D_EDITOR
		bool										DrawDetails(float params[]);
#endif


		INLINE virtual bool							AreDependenciesDirty() const override { return false; }


	protected:
		ShaderResource(D_CORE::Uuid const& uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, D_RESOURCE::Resource* parent, bool isDefault = false) :
			Resource(uuid, path, name, id, parent, isDefault)
		{}

		virtual bool								WriteResourceToFile(D_SERIALIZATION::Json& j) const override;
		virtual void								ReadResourceFromFile(D_SERIALIZATION::Json const& j, bool& dirtyDisk) override;
		INLINE virtual bool							UploadToGpu() override { return true; }

		virtual void								Unload() override;

	private:
		std::shared_ptr<D_CONTAINERS::DVector<std::byte>> mCode;
	};
}
