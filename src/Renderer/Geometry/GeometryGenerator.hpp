#pragma once

#include "MeshData.hpp"

#include "Renderer/VertexTypes.hpp"

#ifndef D_RENDERER_GEOMETRY_GENERATOR
#define D_RENDERER_GEOMETRY_GENERATOR Darius::Renderer::Geometry::GeometryGenerator
#endif

namespace
{
	using Vertex = D_RENDERER_VERTEX::VertexPositionNormalTangentTextureSkinned;
	using uint32 = std::uint32_t;
}

namespace Darius::Renderer::Geometry::GeometryGenerator
{

	MeshData<Vertex> CreateBox(float width, float height, float depth, uint32 numSubdivisions);
	MeshData<Vertex> CreateSphere(float radius, uint32 sliceCount, uint32 stackCount);
	MeshData<Vertex> CreateGeosphere(float radius, uint32 numSubdivisions);
	MeshData<Vertex> CreateCylinder(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount);
	MeshData<Vertex> CreateGrid(float width, float depth, uint32 m, uint32 n);
	MeshData<Vertex> CreateGridQuadPatch(float width, float depth, uint32 m, uint32 n);
	MeshData<Vertex> CreateQuad(float x, float y, float w, float h, float depth);
	MeshData<Vertex> CreateLine(float x1, float y1, float z1, float x2, float y2, float z2);

}
