#include "Renderer/pch.hpp"
#include "FrameGraph.hpp"

#include "RenderPass.hpp"

#include <Core/Filesystem/Path.hpp>
#include <Core/Filesystem/FileUtils.hpp>
#include <Core/Memory/Memory.hpp>
#include <Core/Memory/Allocators/LinearAllocator.hpp>
#include <Core/Memory/Allocators/StackAllocator.hpp>
#include <Core/Memory/Allocators/MallocAllocator.hpp>
#include <Core/Serialization/Json.hpp>

using namespace D_CORE;
using namespace D_SERIALIZATION;

namespace Darius::Renderer
{
	static FrameGraphResourceType			StringToResourceType(std::string const& type)
	{

		if (type == "Texture")
			return FrameGraphResourceType::Texture;

		if (type == "Attachment")
			return FrameGraphResourceType::Attachment;

		if (type == "Buffer")
			return FrameGraphResourceType::Buffer;

		if (type == "Reference")
			return FrameGraphResourceType::Reference;

		if (type == "ShadingRate")
			return FrameGraphResourceType::ShadingRate;

		D_ASSERT_NOENTRY();
		return FrameGraphResourceType::Invalid;
	}

	DXGI_FORMAT StringToDXGIFormat(std::string const& fmt)
	{

		if (fmt == "DXGI_FORMAT_R32G32B32A32_TYPELESS")
			return DXGI_FORMAT_R32G32B32A32_TYPELESS;
		else if (fmt == "DXGI_FORMAT_R32G32B32A32_FLOAT")
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		else if (fmt == "DXGI_FORMAT_R32G32B32A32_UINT")
			return DXGI_FORMAT_R32G32B32A32_UINT;
		else if (fmt == "DXGI_FORMAT_R32G32B32A32_SINT")
			return DXGI_FORMAT_R32G32B32A32_SINT;
		else if (fmt == "DXGI_FORMAT_R32G32B32_TYPELESS")
			return DXGI_FORMAT_R32G32B32_TYPELESS;
		else if (fmt == "DXGI_FORMAT_R32G32B32_FLOAT")
			return DXGI_FORMAT_R32G32B32_FLOAT;
		else if (fmt == "DXGI_FORMAT_R32G32B32_UINT")
			return DXGI_FORMAT_R32G32B32_UINT;
		else if (fmt == "DXGI_FORMAT_R32G32B32_SINT")
			return DXGI_FORMAT_R32G32B32_SINT;
		else if (fmt == "DXGI_FORMAT_R16G16B16A16_TYPELESS")
			return DXGI_FORMAT_R16G16B16A16_TYPELESS;
		else if (fmt == "DXGI_FORMAT_R16G16B16A16_FLOAT")
			return DXGI_FORMAT_R16G16B16A16_FLOAT;
		else if (fmt == "DXGI_FORMAT_R16G16B16A16_UNORM")
			return DXGI_FORMAT_R16G16B16A16_UNORM;
		else if (fmt == "DXGI_FORMAT_R16G16B16A16_UINT")
			return DXGI_FORMAT_R16G16B16A16_UINT;
		else if (fmt == "DXGI_FORMAT_R16G16B16A16_SNORM")
			return DXGI_FORMAT_R16G16B16A16_SNORM;
		else if (fmt == "DXGI_FORMAT_R16G16B16A16_SINT")
			return DXGI_FORMAT_R16G16B16A16_SINT;
		else if (fmt == "DXGI_FORMAT_R32G32_TYPELESS")
			return DXGI_FORMAT_R32G32_TYPELESS;
		else if (fmt == "DXGI_FORMAT_R32G32_FLOAT")
			return DXGI_FORMAT_R32G32_FLOAT;
		else if (fmt == "DXGI_FORMAT_R32G32_UINT")
			return DXGI_FORMAT_R32G32_UINT;
		else if (fmt == "DXGI_FORMAT_R32G32_SINT")
			return DXGI_FORMAT_R32G32_SINT;
		else if (fmt == "DXGI_FORMAT_R32G8X24_TYPELESS")
			return DXGI_FORMAT_R32G8X24_TYPELESS;
		else if (fmt == "DXGI_FORMAT_D32_FLOAT_S8X24_UINT")
			return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		else if (fmt == "DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS")
			return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		else if (fmt == "DXGI_FORMAT_X32_TYPELESS_G8X24_UINT")
			return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
		else if (fmt == "DXGI_FORMAT_R10G10B10A2_TYPELESS")
			return DXGI_FORMAT_R10G10B10A2_TYPELESS;
		else if (fmt == "DXGI_FORMAT_R10G10B10A2_UNORM")
			return DXGI_FORMAT_R10G10B10A2_UNORM;
		else if (fmt == "DXGI_FORMAT_R10G10B10A2_UINT")
			return DXGI_FORMAT_R10G10B10A2_UINT;
		else if (fmt == "DXGI_FORMAT_R11G11B10_FLOAT")
			return DXGI_FORMAT_R11G11B10_FLOAT;
		else if (fmt == "DXGI_FORMAT_R8G8B8A8_TYPELESS")
			return DXGI_FORMAT_R8G8B8A8_TYPELESS;
		else if (fmt == "DXGI_FORMAT_R8G8B8A8_UNORM")
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		else if (fmt == "DXGI_FORMAT_R8G8B8A8_UNORM_SRGB")
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		else if (fmt == "DXGI_FORMAT_R8G8B8A8_UINT")
			return DXGI_FORMAT_R8G8B8A8_UINT;
		else if (fmt == "DXGI_FORMAT_R8G8B8A8_SNORM")
			return DXGI_FORMAT_R8G8B8A8_SNORM;
		else if (fmt == "DXGI_FORMAT_R8G8B8A8_SINT")
			return DXGI_FORMAT_R8G8B8A8_SINT;
		else if (fmt == "DXGI_FORMAT_R16G16_TYPELESS")
			return DXGI_FORMAT_R16G16_TYPELESS;
		else if (fmt == "DXGI_FORMAT_R16G16_FLOAT")
			return DXGI_FORMAT_R16G16_FLOAT;
		else if (fmt == "DXGI_FORMAT_R16G16_UNORM")
			return DXGI_FORMAT_R16G16_UNORM;
		else if (fmt == "DXGI_FORMAT_R16G16_UINT")
			return DXGI_FORMAT_R16G16_UINT;
		else if (fmt == "DXGI_FORMAT_R16G16_SNORM")
			return DXGI_FORMAT_R16G16_SNORM;
		else if (fmt == "DXGI_FORMAT_R16G16_SINT")
			return DXGI_FORMAT_R16G16_SINT;
		else if (fmt == "DXGI_FORMAT_R32_TYPELESS")
			return DXGI_FORMAT_R32_TYPELESS;
		else if (fmt == "DXGI_FORMAT_D32_FLOAT")
			return DXGI_FORMAT_D32_FLOAT;
		else if (fmt == "DXGI_FORMAT_R32_FLOAT")
			return DXGI_FORMAT_R32_FLOAT;
		else if (fmt == "DXGI_FORMAT_R32_UINT")
			return DXGI_FORMAT_R32_UINT;
		else if (fmt == "DXGI_FORMAT_R32_SINT")
			return DXGI_FORMAT_R32_SINT;
		else if (fmt == "DXGI_FORMAT_R24G8_TYPELESS")
			return DXGI_FORMAT_R24G8_TYPELESS;
		else if (fmt == "DXGI_FORMAT_D24_UNORM_S8_UINT")
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
		else if (fmt == "DXGI_FORMAT_R24_UNORM_X8_TYPELESS")
			return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		else if (fmt == "DXGI_FORMAT_X24_TYPELESS_G8_UINT")
			return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
		else if (fmt == "DXGI_FORMAT_R8G8_TYPELESS")
			return DXGI_FORMAT_R8G8_TYPELESS;
		else if (fmt == "DXGI_FORMAT_R8G8_UNORM")
			return DXGI_FORMAT_R8G8_UNORM;
		else if (fmt == "DXGI_FORMAT_R8G8_UINT")
			return DXGI_FORMAT_R8G8_UINT;
		else if (fmt == "DXGI_FORMAT_R8G8_SNORM")
			return DXGI_FORMAT_R8G8_SNORM;
		else if (fmt == "DXGI_FORMAT_R8G8_SINT")
			return DXGI_FORMAT_R8G8_SINT;
		else if (fmt == "DXGI_FORMAT_R16_TYPELESS")
			return DXGI_FORMAT_R16_TYPELESS;
		else if (fmt == "DXGI_FORMAT_R16_FLOAT")
			return DXGI_FORMAT_R16_FLOAT;
		else if (fmt == "DXGI_FORMAT_D16_UNORM")
			return DXGI_FORMAT_D16_UNORM;
		else if (fmt == "DXGI_FORMAT_R16_UNORM")
			return DXGI_FORMAT_R16_UNORM;
		else if (fmt == "DXGI_FORMAT_R16_UINT")
			return DXGI_FORMAT_R16_UINT;
		else if (fmt == "DXGI_FORMAT_R16_SNORM")
			return DXGI_FORMAT_R16_SNORM;
		else if (fmt == "DXGI_FORMAT_R16_SINT")
			return DXGI_FORMAT_R16_SINT;
		else if (fmt == "DXGI_FORMAT_R8_TYPELESS")
			return DXGI_FORMAT_R8_TYPELESS;
		else if (fmt == "DXGI_FORMAT_R8_UNORM")
			return DXGI_FORMAT_R8_UNORM;
		else if (fmt == "DXGI_FORMAT_R8_UINT")
			return DXGI_FORMAT_R8_UINT;
		else if (fmt == "DXGI_FORMAT_R8_SNORM")
			return DXGI_FORMAT_R8_SNORM;
		else if (fmt == "DXGI_FORMAT_R8_SINT")
			return DXGI_FORMAT_R8_SINT;
		else if (fmt == "DXGI_FORMAT_A8_UNORM")
			return DXGI_FORMAT_A8_UNORM;
		else if (fmt == "DXGI_FORMAT_R1_UNORM")
			return DXGI_FORMAT_R1_UNORM;
		else if (fmt == "DXGI_FORMAT_R9G9B9E5_SHAREDEXP")
			return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
		else if (fmt == "DXGI_FORMAT_R8G8_B8G8_UNORM")
			return DXGI_FORMAT_R8G8_B8G8_UNORM;
		else if (fmt == "DXGI_FORMAT_G8R8_G8B8_UNORM")
			return DXGI_FORMAT_G8R8_G8B8_UNORM;
		else if (fmt == "DXGI_FORMAT_BC1_TYPELESS")
			return DXGI_FORMAT_BC1_TYPELESS;
		else if (fmt == "DXGI_FORMAT_BC1_UNORM")
			return DXGI_FORMAT_BC1_UNORM;
		else if (fmt == "DXGI_FORMAT_BC1_UNORM_SRGB")
			return DXGI_FORMAT_BC1_UNORM_SRGB;
		else if (fmt == "DXGI_FORMAT_BC2_TYPELESS")
			return DXGI_FORMAT_BC2_TYPELESS;
		else if (fmt == "DXGI_FORMAT_BC2_UNORM")
			return DXGI_FORMAT_BC2_UNORM;
		else if (fmt == "DXGI_FORMAT_BC2_UNORM_SRGB")
			return DXGI_FORMAT_BC2_UNORM_SRGB;
		else if (fmt == "DXGI_FORMAT_BC3_TYPELESS")
			return DXGI_FORMAT_BC3_TYPELESS;
		else if (fmt == "DXGI_FORMAT_BC3_UNORM")
			return DXGI_FORMAT_BC3_UNORM;
		else if (fmt == "DXGI_FORMAT_BC3_UNORM_SRGB")
			return DXGI_FORMAT_BC3_UNORM_SRGB;
		else if (fmt == "DXGI_FORMAT_BC4_TYPELESS")
			return DXGI_FORMAT_BC4_TYPELESS;
		else if (fmt == "DXGI_FORMAT_BC4_UNORM")
			return DXGI_FORMAT_BC4_UNORM;
		else if (fmt == "DXGI_FORMAT_BC4_SNORM")
			return DXGI_FORMAT_BC4_SNORM;
		else if (fmt == "DXGI_FORMAT_BC5_TYPELESS")
			return DXGI_FORMAT_BC5_TYPELESS;
		else if (fmt == "DXGI_FORMAT_BC5_UNORM")
			return DXGI_FORMAT_BC5_UNORM;
		else if (fmt == "DXGI_FORMAT_BC5_SNORM")
			return DXGI_FORMAT_BC5_SNORM;
		else if (fmt == "DXGI_FORMAT_B5G6R5_UNORM")
			return DXGI_FORMAT_B5G6R5_UNORM;
		else if (fmt == "DXGI_FORMAT_B5G5R5A1_UNORM")
			return DXGI_FORMAT_B5G5R5A1_UNORM;
		else if (fmt == "DXGI_FORMAT_B8G8R8A8_UNORM")
			return DXGI_FORMAT_B8G8R8A8_UNORM;
		else if (fmt == "DXGI_FORMAT_B8G8R8X8_UNORM")
			return DXGI_FORMAT_B8G8R8X8_UNORM;
		else if (fmt == "DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM")
			return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
		else if (fmt == "DXGI_FORMAT_B8G8R8A8_TYPELESS")
			return DXGI_FORMAT_B8G8R8A8_TYPELESS;
		else if (fmt == "DXGI_FORMAT_B8G8R8A8_UNORM_SRGB")
			return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		else if (fmt == "DXGI_FORMAT_B8G8R8X8_TYPELESS")
			return DXGI_FORMAT_B8G8R8X8_TYPELESS;
		else if (fmt == "DXGI_FORMAT_B8G8R8X8_UNORM_SRGB")
			return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
		else if (fmt == "DXGI_FORMAT_BC6H_TYPELESS")
			return DXGI_FORMAT_BC6H_TYPELESS;
		else if (fmt == "DXGI_FORMAT_BC6H_UF16")
			return DXGI_FORMAT_BC6H_UF16;
		else if (fmt == "DXGI_FORMAT_BC6H_SF16")
			return DXGI_FORMAT_BC6H_SF16;
		else if (fmt == "DXGI_FORMAT_BC7_TYPELESS")
			return DXGI_FORMAT_BC7_TYPELESS;
		else if (fmt == "DXGI_FORMAT_BC7_UNORM")
			return DXGI_FORMAT_BC7_UNORM;
		else if (fmt == "DXGI_FORMAT_BC7_UNORM_SRGB")
			return DXGI_FORMAT_BC7_UNORM_SRGB;
		else if (fmt == "DXGI_FORMAT_AYUV")
			return DXGI_FORMAT_AYUV;
		else if (fmt == "DXGI_FORMAT_Y410")
			return DXGI_FORMAT_Y410;
		else if (fmt == "DXGI_FORMAT_Y416")
			return DXGI_FORMAT_Y416;
		else if (fmt == "DXGI_FORMAT_NV12")
			return DXGI_FORMAT_NV12;
		else if (fmt == "DXGI_FORMAT_P010")
			return DXGI_FORMAT_P010;
		else if (fmt == "DXGI_FORMAT_P016")
			return DXGI_FORMAT_P016;
		else if (fmt == "DXGI_FORMAT_420_OPAQUE")
			return DXGI_FORMAT_420_OPAQUE;
		else if (fmt == "DXGI_FORMAT_YUY2")
			return DXGI_FORMAT_YUY2;
		else if (fmt == "DXGI_FORMAT_Y210")
			return DXGI_FORMAT_Y210;
		else if (fmt == "DXGI_FORMAT_Y216")
			return DXGI_FORMAT_Y216;
		else if (fmt == "DXGI_FORMAT_NV11")
			return DXGI_FORMAT_NV11;
		else if (fmt == "DXGI_FORMAT_AI44")
			return DXGI_FORMAT_AI44;
		else if (fmt == "DXGI_FORMAT_IA44")
			return DXGI_FORMAT_IA44;
		else if (fmt == "DXGI_FORMAT_P8")
			return DXGI_FORMAT_P8;
		else if (fmt == "DXGI_FORMAT_A8P8")
			return DXGI_FORMAT_A8P8;
		else if (fmt == "DXGI_FORMAT_B4G4R4A4_UNORM")
			return DXGI_FORMAT_B4G4R4A4_UNORM;
		else if (fmt == "DXGI_FORMAT_P208")
			return DXGI_FORMAT_P208;
		else if (fmt == "DXGI_FORMAT_V208")
			return DXGI_FORMAT_V208;
		else if (fmt == "DXGI_FORMAT_V408")
			return DXGI_FORMAT_V408;
		else if (fmt == "DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE")
			return DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE;
		else if (fmt == "DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE")
			return DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE;
		else if (fmt == "DXGI_FORMAT_A4B4G4R4_UNORM")
			return DXGI_FORMAT_A4B4G4R4_UNORM;
		else if (fmt == "DXGI_FORMAT_FORCE_UINT")
			return DXGI_FORMAT_FORCE_UINT;
		else
			return DXGI_FORMAT_UNKNOWN;
	}

	FramGraphTextureOperation StringToFramGraphTextureOperation(std::string const& op)
	{
		if (op == "Clear")
			return FramGraphTextureOperation::Clear;

		if (op == "Load")
			return FramGraphTextureOperation::Load;

		D_ASSERT_NOENTRY();
		return FramGraphTextureOperation::DontCare;
	}

	// Frame Graph

	void FrameGraph::Initialize(FrameGraphBuilder* builder, std::shared_ptr<RenderPassManager> passManager)
	{
		mAllocator = std::unique_ptr<D_MEMORY::MallocAllocator>(new D_MEMORY::MallocAllocator());

		mLocalAllocator->Configure(1024 * 1024);

		mBuilder = builder;

		mNodes.reserve(FrameGraphBuilder::MaxNodesCount);
		mAllNodes.reserve(FrameGraphBuilder::MaxNodesCount);

		mRenderPassManager = passManager;
	}

	void FrameGraph::Shutdown()
	{
		for (uint32_t i = 0; i < mAllNodes.size(); i++)
		{
			auto handle = mAllNodes[i];
			auto node = mBuilder->AccessNode(handle);

			mRenderPassManager->DeleteRenderPass(node->RenderPass);
			// TODO: Destroy frame buffer

			node->Inputs.clear();
			node->Outputs.clear();
			node->Edges.clear();
		}

		mAllNodes.clear();
		mNodes.clear();

		mLocalAllocator->Reset();
	}

	void FrameGraph::Parse(D_FILE::Path const& path, D_MEMORY::StackAllocator* tempAllocator)
	{
		D_SERIALIZATION::Json graphData;

		if (!D_FILE::ReadJsonFile(path, graphData))
		{
			D_ASSERT_NOENTRY();
			return;
		}

		size_t currentAllocatorMarker = tempAllocator->GetMarker();

		mName = D_CORE::StringId(graphData.value("Name", "").c_str());

		Json const& passes = graphData["Passes"];
		for (size_t i = 0; i < passes.size(); i++)
		{
			Json const& pass = passes[i];

			Json const& passInputs = pass["Inputs"];
			Json const& passOutputs = pass["Outputs"];

			FrameGraphNodeCreation nodeCreation;
			nodeCreation.Input.reserve(passInputs.size());
			nodeCreation.Output.reserve(passOutputs.size());

			for (size_t ii = 0; ii < passInputs.size(); ii++)
			{
				Json const& passInput = passInputs[ii];

				FrameGraphResourceInputCreation inputCreation;

				std::string inputType = passInput.value("Type", "");
				D_ASSERT(!inputType.empty());

				std::string inputName = passInput.value("Name", "");
				D_ASSERT(!inputName.empty());

				inputCreation.Type = StringToResourceType(inputType);
				inputCreation.ResourceInfo.External = false;

				inputCreation.Name = D_CORE::StringId::FromString(inputName);

				nodeCreation.Input.push_back(inputCreation);
			}

			for (size_t oi = 0; oi < passOutputs.size(); oi++)
			{
				Json const& passOutput = passOutputs[oi];

				FrameGraphResourceOutputCreation outputCreation;

				std::string outputType = passOutput.value("Type", "");
				D_ASSERT(!outputType.empty());

				std::string outputName = passOutput.value("Name", "");
				D_ASSERT(!outputName.empty());

				outputCreation.Type = StringToResourceType(outputType);
				outputCreation.Name = D_CORE::StringId::FromString(outputName);

				switch (outputCreation.Type)
				{
				case FrameGraphResourceType::Attachment:
				case FrameGraphResourceType::Texture:
				{
					std::string format = passOutput.value("Format", "");
					D_ASSERT(!format.empty());

					outputCreation.ResourceInfo.Texture.Format = StringToDXGIFormat(format);

					std::string loadOp = passOutput.value("Op", "");
					D_ASSERT(!loadOp.empty());

					outputCreation.ResourceInfo.Texture.LoadOp = StringToFramGraphTextureOperation(loadOp);

					Json const& resolution = passOutput["Resolution"];
					outputCreation.ResourceInfo.Texture.Width = resolution[0];
					outputCreation.ResourceInfo.Texture.Height = resolution[1];
					outputCreation.ResourceInfo.Texture.Depth = resolution[2];
				}
				break;

				case FrameGraphResourceType::Buffer:
				{
					D_ASSERT_M(false, "Buffers are still not supported");
				}
				break;

				default:
					break;
				}

				nodeCreation.Output.push_back(outputCreation);
			}

			std::string passName = pass.value("Name", "");
			D_ASSERT(!passName.empty());

			bool enabled = pass.value("Enabled", true);

			nodeCreation.Name = StringId::FromString(passName);
			nodeCreation.Enabled = enabled;

			FrameGraphNodeHandle nodeHandle = mBuilder->CreateNode(nodeCreation);
			mNodes.push_back(nodeHandle);
		}

		tempAllocator->FreeMarker(currentAllocatorMarker);
	}

	static void ComputeEdges(FrameGraph* frameGraph, FrameGraphNode* node, uint32_t nodeIndex)
	{
		for (uint32_t r = 0; r < node->Inputs.size(); r++)
		{
			FrameGraphResource* resource = frameGraph->AccessResource(node->Inputs[r]);
			
			FrameGraphResource* outputResource = frameGraph->GetResource(resource->Name);
			if (outputResource == nullptr && !resource->ResouceInfo.External)
			{
				D_ASSERT(false, "Requested resource is not produced by any node and is not external.");
				continue;
			}

			D_ASSERT(resource);

			resource->Producer = outputResource->Producer;
			resource->ResouceInfo = outputResource->ResouceInfo;
			resource->OutputHandle = outputResource->OutputHandle;

			FrameGraphNode* parentNode = frameGraph->AccessNode(resource->Producer);

			parentNode->Edges.push_back(frameGraph->mNodes[nodeIndex]);
		}
	}

	static void CreateFrameBuffer(FrameGraph* frameGraph, FrameGraphNode* node)
	{
		D_ASSERT_NOENTRY();
	}

	static void CreateRenderPass(FrameGraph* frameGraph, FrameGraphNode* node)
	{
		FrameGraphRenderPass
	}
}