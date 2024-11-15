#pragma once

#include <Core/RefCounting/Counted.hpp>
#include <Core/StringId.hpp>
#include <Core/Memory/Allocators/Allocator.hpp>
#include <Core/Memory/Memory.hpp>

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

#define D_H_RENDER_PASS_FACTORY(Type) \
public: \
class Factory : public Darius::Renderer::RenderPassFactory \
{ \
public: \
	INLINE virtual RenderPass* CreateRenderPass() override { return DMemNew(Type); } \
};\
static INLINE D_CORE::StringId GetPassNameStatic() { return D_CORE::StringId(#Type); } \
friend class Type::Factory;

namespace Darius::Renderer
{

	class RenderPassFactory
	{
	public:
		virtual RenderPass* CreateRenderPass() = 0;
	};

	class RenderPass : public D_CORE::Counted
	{

	private:
		INLINE virtual bool Release() override {}

	};
}