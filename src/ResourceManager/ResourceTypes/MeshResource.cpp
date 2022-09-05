#include "Renderer/pch.hpp"
#include "MeshResource.hpp"
#include "ResourceManager/ResourceManager.hpp"

#include <Core/Filesystem/Path.hpp>
#include <Renderer/RenderDeviceManager.hpp>
#include <Renderer/GraphicsCore.hpp>

#include <fbxsdk.h>
#include <fbxsdk/fileio/fbxiosettings.h>

using namespace D_RENDERER_GEOMETRY;
using namespace D_CONTAINERS;

namespace Darius::ResourceManager
{

	void GetFBXUVs(MeshData<MeshResource::VertexType>& meshData, FbxMesh const* mesh);

	bool MeshResource::SuppoertsExtension(std::wstring ext)
	{
		if (ext == L".fbx")
			return true;
		return false;
	}

	void MeshResource::Create(MeshData<VertexType>& data)
	{
		Destroy();
		SetName(GetName());

		Mesh::Draw submesh;
		submesh.IndexCount = (UINT)data.Indices32.size();
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		DVector<D_RENDERER_VERTEX::VertexPositionNormalTexture> vertices(data.Vertices.size());

		for (size_t i = 0; i < data.Vertices.size(); i++)
		{
			auto& ver = data.Vertices[i];
			vertices[i] = D_RENDERER_VERTEX::VertexPositionNormalTexture(ver.mPosition, ver.mNormal, ver.mTexC);
		}

		DVector<std::uint16_t> indices;
		indices.insert(indices.end(), std::begin(data.GetIndices16()), std::end(data.GetIndices16()));

		const UINT vbByteSize = (UINT)vertices.size() * sizeof(D_RENDERER_VERTEX::VertexPositionNormalTexture);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

		mMesh.name = GetName();

		// Create vertex buffer
		mMesh.VertexDataGpu.Create(mMesh.name + L" Vertex Buffer", vertices.size(), sizeof(D_RENDERER_VERTEX::VertexPositionNormalTexture), vertices.data());

		// Create index buffer
		mMesh.IndexDataGpu.Create(mMesh.name + L" Index Buffer", indices.size(), sizeof(std::uint16_t), indices.data());

		mMesh.mDraw.push_back(submesh);

		mMesh.mBoundSp = data.CalcBoundingSphere();
		mMesh.mBoundBox = data.CalcBoundingBox();
	}

	void MeshResource::WriteResourceToFile() const
	{

	}

	void MeshResource::ReadResourceFromFile()
	{
	}

	bool MeshResource::UploadToGpu(D_GRAPHICS::GraphicsContext& context)
	{
		// Create the FBX SDK manager
		FbxManager* lSdkManager = FbxManager::Create();

		// Create an IOSettings object.
		FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
		lSdkManager->SetIOSettings(ios);

		// Create an importer.
		FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

		// Declare the path and filename of the file containing the scene.
		// In this case, we are assuming the file is in the same directory as the executable.
		auto pathStr = GetPath().string();
		const char* lFilename = pathStr.c_str();


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

		// Only dealing with first child
		auto node = lRootNode->GetChild(0);
		auto mesh = node->GetMesh();

		auto controlPoints = mesh->GetControlPoints();

		D_RENDERER_GEOMETRY::MeshData<VertexType> meshData;

		fbxsdk::FbxLayerElementArrayTemplate<fbxsdk::FbxVector2>* uvArr;

		// Add vertext positions
		for (size_t i = 0; i < mesh->GetControlPointsCount(); i++)
		{
			VertexType vert;
			auto controlPoint = controlPoints[i];
			vert.mPosition.x = (float)controlPoint[0];
			vert.mPosition.y = (float)controlPoint[1];
			vert.mPosition.z = (float)controlPoint[2];
			vert.mTexC = XMFLOAT2(0.f, 0.f);
			meshData.Vertices.push_back(vert);
		}

		// Add indices
		auto polyCount = mesh->GetPolygonCount();
		for (size_t polyIdx = 0; polyIdx < mesh->GetPolygonCount(); polyIdx++)
		{
			for (size_t vertIdx = 0; vertIdx < mesh->GetPolygonSize(polyIdx); vertIdx++)
			{
				auto vertexGlobalIdx = mesh->GetPolygonVertex(polyIdx, vertIdx);

				meshData.Indices32.push_back(vertexGlobalIdx);
			}
		}

		//Add UV data
		GetFBXUVs(meshData, mesh);

		// Destroy the SDK manager and all the other objects it was handling.
		lSdkManager->Destroy();

		Create(meshData);
		return true;
	}

	void GetFBXUVs(MeshData<MeshResource::VertexType>& meshData, FbxMesh const* mesh)
	{
		FbxStringList lUVSetNameList;
		mesh->GetUVSetNames(lUVSetNameList);

		const FbxGeometryElementUV* lUVElement = mesh->GetElementUV(lUVSetNameList[0]);

		//index array, where holds the index referenced to the uv data
		const bool lUseIndex = lUVElement->GetReferenceMode() != FbxGeometryElement::eDirect;
		const int lIndexCount = (lUseIndex) ? lUVElement->GetIndexArray().GetCount() : 0;

		//iterating through the data by polygon
		const int lPolyCount = mesh->GetPolygonCount();

		if (lUVElement->GetMappingMode() == FbxGeometryElement::eByControlPoint)
		{
			for (int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex)
			{
				// build the max index array that we need to pass into MakePoly
				const int lPolySize = mesh->GetPolygonSize(lPolyIndex);
				for (int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex)
				{
					FbxVector2 lUVValue;

					//get the index of the current vertex in control points array
					int lPolyVertIndex = mesh->GetPolygonVertex(lPolyIndex, lVertIndex);

					//the UV index depends on the reference mode
					int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyVertIndex) : lPolyVertIndex;

					lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

					meshData.Vertices[lPolyVertIndex].mTexC.x = lUVValue[0];
					meshData.Vertices[lPolyVertIndex].mTexC.y = lUVValue[1];
				}
			}
		}
		else if (lUVElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
		{
			int lPolyIndexCounter = 0;
			for (int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex)
			{
				// build the max index array that we need to pass into MakePoly
				const int lPolySize = mesh->GetPolygonSize(lPolyIndex);
				for (int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex)
				{
					if (lPolyIndexCounter < lIndexCount)
					{
						FbxVector2 lUVValue;

						//the UV index depends on the reference mode
						int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyIndexCounter) : lPolyIndexCounter;

						lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

						//get the index of the current vertex in control points array
						int lPolyVertIndex = meshData.Indices32[lPolyIndexCounter];
						meshData.Vertices[lPolyVertIndex].mTexC.x = lUVValue[0];
						meshData.Vertices[lPolyVertIndex].mTexC.y = lUVValue[1];

						lPolyIndexCounter++;
					}
				}
			}
		}
	}
}