#include "pch.hpp"
#include "MeshResource.hpp"
#include "ResourceManager.hpp"

#include <Renderer/RenderDeviceManager.hpp>
#include <Renderer/GraphicsCore.hpp>

#include <filesystem>
#include <fbxsdk.h>
#include <fbxsdk/fileio/fbxiosettings.h>

using namespace D_RENDERER_GEOMETRY;
using namespace D_CONTAINERS;

namespace Darius::ResourceManager
{
	void MeshResource::Create(std::wstring name, MeshData<VertexType>& data)
	{
		Destroy();

		this->mName = name;

		Mesh::Draw submesh;
		submesh.IndexCount = (UINT)data.Indices32.size();
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		DVector<D_RENDERER_VERTEX::VertexPositionColor> vertices(data.Vertices.size());

		for (size_t i = 0; i < data.Vertices.size(); i++)
		{
			auto& ver = data.Vertices[i];
			vertices[i] = D_RENDERER_VERTEX::VertexPositionColor(ver.mPosition, (XMFLOAT4)DirectX::Colors::DarkGreen);
		}

		DVector<std::uint16_t> indices;
		indices.insert(indices.end(), std::begin(data.GetIndices16()), std::end(data.GetIndices16()));

		const UINT vbByteSize = (UINT)vertices.size() * sizeof(D_RENDERER_VERTEX::VertexPositionColor);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

		mMesh.name = name;

		D_HR_CHECK(D3DCreateBlob(vbByteSize, &mMesh.mVertexBufferCPU));
		CopyMemory(mMesh.mVertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

		D_HR_CHECK(D3DCreateBlob(ibByteSize, &mMesh.mIndexBufferCPU));
		CopyMemory(mMesh.mIndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

		auto& context = D_GRAPHICS::GraphicsContext::Begin(L"Mesh upload");

		mMesh.mVertexBufferGPU = D_RENDERER_UTILS::CreateDefaultBuffer(D_RENDERER_DEVICE::GetDevice(), context.GetCommandList(), vertices.data(), vbByteSize, mMesh.mVertexBufferUploader);

		mMesh.mIndexBufferGPU = D_RENDERER_UTILS::CreateDefaultBuffer(D_RENDERER_DEVICE::GetDevice(), context.GetCommandList(), indices.data(), ibByteSize, mMesh.mIndexBufferUploader);

		mMesh.mVertexByteStride = sizeof(D_RENDERER_VERTEX::VertexPositionColor);
		mMesh.mVertexBufferByteSize = vbByteSize;
		mMesh.mIndexFormat = DXGI_FORMAT_R16_UINT;
		mMesh.mIndexBufferByteSize = ibByteSize;

		mMesh.mDraw.push_back(submesh);
		context.Finish();

		mMesh.mBoundSp = data.CalcBoundingSphere();

		mLoaded = true;
	}

	bool MeshResource::Save()
	{
		if (mPath.empty())
			return false;
	}

	FbxString GetAttributeTypeName(FbxNodeAttribute::EType type) {
		switch (type) {
		case FbxNodeAttribute::eUnknown: return "unidentified";
		case FbxNodeAttribute::eNull: return "null";
		case FbxNodeAttribute::eMarker: return "marker";
		case FbxNodeAttribute::eSkeleton: return "skeleton";
		case FbxNodeAttribute::eMesh: return "mesh";
		case FbxNodeAttribute::eNurbs: return "nurbs";
		case FbxNodeAttribute::ePatch: return "patch";
		case FbxNodeAttribute::eCamera: return "camera";
		case FbxNodeAttribute::eCameraStereo: return "stereo";
		case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
		case FbxNodeAttribute::eLight: return "light";
		case FbxNodeAttribute::eOpticalReference: return "optical reference";
		case FbxNodeAttribute::eOpticalMarker: return "marker";
		case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
		case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
		case FbxNodeAttribute::eBoundary: return "boundary";
		case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
		case FbxNodeAttribute::eShape: return "shape";
		case FbxNodeAttribute::eLODGroup: return "lodgroup";
		case FbxNodeAttribute::eSubDiv: return "subdiv";
		default: return "unknown";
		}
	}
	/* Tab character ("\t") counter */
	int numTabs = 0;

	void PrintTabs() {
		for (int i = 0; i < numTabs; i++)
			printf("\t");
	}
	/**
	 * Print an attribute.
	 */
	void PrintAttribute(FbxNodeAttribute* pAttribute) {
		if (!pAttribute) return;

		FbxString typeName = GetAttributeTypeName(pAttribute->GetAttributeType());
		FbxString attrName = pAttribute->GetName();
		PrintTabs();
		// Note: to retrieve the character array of a FbxString, use its Buffer() method.
		printf("<attribute type='%s' name='%s'/>\n", typeName.Buffer(), attrName.Buffer());
	}

	/**
	 * Print a node, its attributes, and all its children recursively.
	 */
	void PrintNode(FbxNode* pNode) {
		PrintTabs();
		const char* nodeName = pNode->GetName();
		FbxDouble3 translation = pNode->LclTranslation.Get();
		FbxDouble3 rotation = pNode->LclRotation.Get();
		FbxDouble3 scaling = pNode->LclScaling.Get();

		// Print the contents of the node.
		printf("<node name='%s' translation='(%f, %f, %f)' rotation='(%f, %f, %f)' scaling='(%f, %f, %f)'>\n",
			nodeName,
			translation[0], translation[1], translation[2],
			rotation[0], rotation[1], rotation[2],
			scaling[0], scaling[1], scaling[2]
		);
		numTabs++;

		// Print the node's attributes.
		for (int i = 0; i < pNode->GetNodeAttributeCount(); i++)
			PrintAttribute(pNode->GetNodeAttributeByIndex(i));

		// Recursively print the children.
		for (int j = 0; j < pNode->GetChildCount(); j++)
			PrintNode(pNode->GetChild(j));

		numTabs--;
		PrintTabs();
		printf("</node>\n");
	}


	bool MeshResource::Load()
	{
		if (mPath.empty())
			return false;

		// Create the FBX SDK manager
		FbxManager* lSdkManager = FbxManager::Create();

		// Create an IOSettings object.
		FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
		lSdkManager->SetIOSettings(ios);

		// Create an importer.
		FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

		// Declare the path and filename of the file containing the scene.
		// In this case, we are assuming the file is in the same directory as the executable.
		const char* lFilename = "ff.fbx";

		// Initialize the importer.
		bool lImportStatus = lImporter->Initialize(lFilename, -1, lSdkManager->GetIOSettings());

		if (!lImportStatus) {
			D_LOG_ERROR("Call to FbxImporter::Initialize() failed.");
			std::string msg = "Error returned: " + std::string(lImporter->GetStatus().GetErrorString());
			D_LOG_ERROR(msg);
			return false;
		}

		// Create a new scene so it can be populated by the imported file.
		FbxScene* lScene = FbxScene::Create(lSdkManager, "modelScene");

		// Import the contents of the file into the scene.
		lImporter->Import(lScene);

		// The file has been imported; we can get rid of the importer.
		lImporter->Destroy();



		// Print the nodes of the scene and their attributes recursively.
		// Note that we are not printing the root node because it should
		// not contain any attributes.
		FbxNode* lRootNode = lScene->GetRootNode();
		/*if (lRootNode) {
			for (int i = 0; i < lRootNode->GetChildCount(); i++)
				PrintNode(lRootNode->GetChild(i));
		}*/

		auto node = lRootNode->GetChild(0);
		auto mesh = node->GetMesh();
		
		auto controlPoints = mesh->GetControlPoints();

		D_RENDERER_GEOMETRY::MeshData<VertexType> meshData;

		for (size_t i = 0; i < mesh->GetControlPointsCount(); i++)
		{
			VertexType vert;
			auto controlPoint = controlPoints[i];
			vert.mPosition.x = (float)controlPoint[0];
			vert.mPosition.y = (float)controlPoint[1];
			vert.mPosition.z = (float)controlPoint[2];
			meshData.Vertices.push_back(vert);
		}

		auto po = mesh->GetPolygonCount();
		for (size_t polyIdx = 0; polyIdx < mesh->GetPolygonCount(); polyIdx++)
		{
			for (size_t vertIdx = 0; vertIdx < mesh->GetPolygonSize(polyIdx); vertIdx++)
			{
				meshData.Indices32.push_back(mesh->GetPolygonVertex(polyIdx, vertIdx));
			}
		}

		// Destroy the SDK manager and all the other objects it was handling.
		lSdkManager->Destroy();

		auto path = std::filesystem::path(mPath);
		auto filename = path.filename().wstring();
		Create(filename.substr(0, filename.length() - path.extension().string().length()), meshData);

		return true;
	}
}