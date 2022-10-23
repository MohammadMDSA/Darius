#include "ResourceManager/pch.hpp"
#include "SkeletalMeshResource.hpp"

#include <fbxsdk.h>
#include <fbxsdk/fileio/fbxiosettings.h>

namespace Darius::ResourceManager
{

	D_CH_RESOURCE_DEF(SkeletalMeshResource);

	void SkeletalMeshResource::Create(MultiPartMeshData<VertexType>& data, D_RENDERER_GEOMETRY::Mesh::SceneGraphNode* skeleton)
	{
		Destroy();
		SetName(GetName());

		auto totalVertices = 0u;
		auto totalIndices = 0u;
		for (auto meshData : data.meshParts)
		{
			totalVertices += meshData.Vertices.size();
			totalIndices += meshData.Indices32.size();
		}

		DVector<D_GRAPHICS_VERTEX::VertexPositionNormalTextureSkinned> vertices(totalVertices);
		DVector<std::uint16_t> indices;

		mMesh.mNumTotalIndices = totalIndices;
		mMesh.mNumTotalVertices = totalVertices;

		auto vertexIndex = 0;
		auto indexIndex = 0;
		for (auto meshData : data.meshParts)
		{
			// Creating submesh
			Mesh::Draw submesh;
			submesh.IndexCount = meshData.Indices32.size();
			submesh.StartIndexLocation = indexIndex;
			submesh.BaseVertexLocation = vertexIndex;

			// Adding vertices
			for (size_t i = 0; i < meshData.Vertices.size(); i++)
			{
				auto& ver = meshData.Vertices[i];
				vertices[vertexIndex] = D_GRAPHICS_VERTEX::VertexPositionNormalTextureSkinned(ver.mPosition, ver.mNormal, ver.mTexC, ver.mBlendIndices, ver.mBlendWeights);
				vertexIndex++;
			}

			// Adding indices
			for (auto index : meshData.GetIndices16())
			{
				indices.push_back(index + submesh.BaseVertexLocation);
			}
			indexIndex += submesh.IndexCount;

			// Updating bounding sphear
			mMesh.mBoundSp = mMesh.mBoundSp.Union(meshData.CalcBoundingSphere());
			mMesh.mBoundBox = mMesh.mBoundBox.Union(meshData.CalcBoundingBox());

			mMesh.mDraw.push_back(submesh);
		}

		mMesh.name = GetName();

		// Create vertex buffer
		mMesh.VertexDataGpu.Create(mMesh.name + L" Vertex Buffer", vertices.size(), sizeof(D_GRAPHICS_VERTEX::VertexPositionNormalTextureSkinned), vertices.data());

		// Create index buffer
		mMesh.IndexDataGpu.Create(mMesh.name + L" Index Buffer", indices.size(), sizeof(std::uint16_t), indices.data());
	}

	bool SkeletalMeshResource::CanConstructFrom(Path const& path)
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
		auto pathStr = path.string();
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

		FbxNode* node = 0;
		for (int i = 0; i < lRootNode->GetChildCount(); i++)
		{
			auto attr = lRootNode->GetChild(i)->GetNodeAttribute();
			if (attr->GetAttributeType() == FbxNodeAttribute::eMesh)
			{
				node = lRootNode->GetChild(i);
			}
		}

		if (!node)
			return false;

		auto mesh = node->GetMesh();

		auto deformerCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
		return deformerCount != 0;
	}

	bool SkeletalMeshResource::UploadToGpu(D_GRAPHICS::GraphicsContext& context)
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

		// Searching for mesh just in first level
		FbxNode* node = 0;
		for (int i = 0; i < lRootNode->GetChildCount(); i++)
		{
			auto attr = lRootNode->GetChild(i)->GetNodeAttribute();
			if (attr->GetAttributeType() == FbxNodeAttribute::eMesh)
			{
				node = lRootNode->GetChild(i);
			}
		}

		if (!node)
			return true;

		auto mesh = node->GetMesh();

		MultiPartMeshData<VertexType> meshDataVec;

		fbxsdk::FbxLayerElementArrayTemplate<fbxsdk::FbxVector2>* uvArr;

		DVector<DUnorderedMap<int, int>> indexMapper(mesh->GetPolygonCount());

		// Add polygons
		GetFBXPolygons(meshDataVec, mesh, indexMapper);

		//Add UV data
		GetFBXUVs(meshDataVec, mesh, indexMapper);

		// Add Normal data
		GetFBXNormalss(meshDataVec, mesh, indexMapper);

		// Add SkinData
		GetFBXSkin(meshDataVec, mesh, indexMapper);

		// Destroy the SDK manager and all the other objects it was handling.
		lSdkManager->Destroy();


		Create(meshDataVec, nullptr);
		return true;
	}

	void SkeletalMeshResource::GetFBXSkin(MultiPartMeshData<VertexType>& meshDataVec, void const* meshP, DVector<DUnorderedMap<int, int>>& indexMapper)
	{

		auto mesh = (FbxMesh const*)meshP;

		if (mesh->GetDeformerCount(FbxDeformer::eSkin) == 0)
			return;

		auto deformer = (FbxSkin*)mesh->GetDeformer(0, FbxDeformer::eSkin);

		auto clusterCount = deformer->GetClusterCount();

		// Finding skeleton root
		FbxSkeleton* skeletonRoot = 0;
		{
			for (int i = 0; i < clusterCount; i++)
			{
				auto cluster = deformer->GetCluster(i);
				if (!cluster->GetLink())
					continue;
				skeletonRoot = cluster->GetLink()->GetSkeleton();
			}

			if (!skeletonRoot)
				return;

			while (!skeletonRoot->IsSkeletonRoot())
				skeletonRoot = skeletonRoot->GetNode()->GetParent()->GetSkeleton();
		}

		// Creating skeleton hierarchy
		DVector<Mesh::SceneGraphNode> skeletonHierarchy;
		DMap<void const*, int> skeletonIndexMap;
		{
			Mesh::SceneGraphNode sceneGraphNode;
			sceneGraphNode.matrixIdx = 0;
			sceneGraphNode.hasSibling = 0;
			sceneGraphNode.staleMatrix = true;
			sceneGraphNode.skeletonRoot = true;
			skeletonHierarchy.push_back(sceneGraphNode);
			skeletonIndexMap.insert({ skeletonRoot, 0 });
			AddSkeletonChildren(skeletonRoot, skeletonHierarchy, skeletonIndexMap);
		}

		// Assigning blend weights and indices
		{
			meshDataVec.jointWeight.resize(mesh->GetControlPointsCount());
			for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++)
			{
				const auto cluster = deformer->GetCluster(clusterIndex);
				
				if (!cluster->GetLink())
					continue;

				auto controlPointIndices = cluster->GetControlPointIndices();
				auto controlPointWeights = cluster->GetControlPointWeights();
				for (size_t clusterItem = 0; clusterItem < cluster->GetControlPointIndicesCount(); clusterItem++)
				{
					auto controlPointIndex = controlPointIndices[clusterItem];
					auto controlPointWeight = controlPointWeights[clusterItem];

					auto jointSkeleton = cluster->GetLink()->GetSkeleton();
					auto jointIndex = skeletonIndexMap[jointSkeleton];

					meshDataVec.jointWeight[controlPointIndex].push_back({ jointIndex, controlPointWeight });
				}
			}
		}


		for (size_t i = 0; i < meshDataVec.jointWeight.size(); i++)
		{
			D_LOG_INFO("Count weights: " << meshDataVec.jointWeight[i].size());
		}
	}

	void SkeletalMeshResource::AddSkeletonChildren(void const* skeletonNode, DVector<Mesh::SceneGraphNode>& skeletonData, DMap<void const*, int>& skeletonIndexMap)
	{
		auto skeleton = (FbxSkeleton*)skeletonNode;
		auto node = skeleton->GetNode();

		// Setting translation
		auto fbxTrans = node->GeometricTranslation.Get();
		auto fbxSclae = node->GeometricScaling.Get();
		auto fbxRot = node->GeometricRotation.Get();

		auto& currentSceneGraphNode = skeletonData[skeletonData.size() - 1];
		currentSceneGraphNode.xform.SetW({ (float)fbxTrans.mData[0], (float)fbxTrans.mData[1], (float)fbxTrans.mData[2], 1.f });
		currentSceneGraphNode.scale = { (float)fbxSclae.mData[0], (float)fbxSclae.mData[1], (float)fbxSclae.mData[2] };
		currentSceneGraphNode.rotation = { (float)fbxRot.mData[0], (float)fbxRot.mData[1], (float)fbxRot.mData[2], 1.f };

		// TODO: What if children are not skeleton?
		currentSceneGraphNode.hasChildren = node->GetChildCount() > 0;

		currentSceneGraphNode.name = node->GetInitialName();
		
		for (int i = 0; i < node->GetChildCount(); i++)
		{
			auto child = node->GetChild(i);

			// TODO: What if a there is a non-skeleton in hierarchy?
			/*if (!child->GetSkeleton())
				continue;*/

			Mesh::SceneGraphNode sceneGraphNode;
			ZeroMemory(&sceneGraphNode, sizeof(Mesh::SceneGraphNode));

			if (i != node->GetChildCount() - 1)
				sceneGraphNode.hasSibling = true;

			if (node->GetChild(i)->GetChildCount() > 0)
				sceneGraphNode.hasChildren = true;

			sceneGraphNode.staleMatrix = true;
			sceneGraphNode.matrixIdx = skeletonData.size();
			skeletonData.push_back(sceneGraphNode);
			AddSkeletonChildren(node->GetChild(i)->GetSkeleton(), skeletonData, skeletonIndexMap);
		}
	}

}