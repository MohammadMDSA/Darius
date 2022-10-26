#include "ResourceManager/pch.hpp"
#include "SkeletalMeshResource.hpp"

#include <fbxsdk.h>
#include <fbxsdk/fileio/fbxiosettings.h>
#include <imgui.h>

namespace Darius::ResourceManager
{

	D_CH_RESOURCE_DEF(SkeletalMeshResource);

	void SkeletalMeshResource::Create(MultiPartMeshData<VertexType>& data)
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
				vertices[vertexIndex] = D_GRAPHICS_VERTEX::VertexPositionNormalTextureSkinned(ver.mPosition, Vector3(ver.mNormal).Normalize(), ver.mTexC, ver.mBlendIndices, ver.mBlendWeights);
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

		mMesh.Name = GetName();

		// Create vertex buffer
		mMesh.VertexDataGpu.Create(mMesh.Name + L" Vertex Buffer", vertices.size(), sizeof(D_GRAPHICS_VERTEX::VertexPositionNormalTextureSkinned), vertices.data());

		// Create index buffer
		mMesh.IndexDataGpu.Create(mMesh.Name + L" Index Buffer", indices.size(), sizeof(std::uint16_t), indices.data());

		mJointCount = mSkeleton.size();
		mSkeletonRoot = mJointCount > 0 ? &mSkeleton.front() : nullptr;
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
				break;
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
				break;
			}
		}

		if (!node)
			return true;

		auto mesh = node->GetMesh();

		MultiPartMeshData<VertexType> meshDataVec;

		fbxsdk::FbxLayerElementArrayTemplate<fbxsdk::FbxVector2>* uvArr;

		// List of polygon in which mapping global index of vertex to polygon index of it
		// If controlpoint mapping, there will be only poly and all key-values will be the same
		DVector<DUnorderedMap<int, int>> indexMapper(mesh->GetPolygonCount());

		// Add polygons
		GetFBXPolygons(meshDataVec, mesh, indexMapper);

		//Add UV data
		GetFBXUVs(meshDataVec, mesh, indexMapper);

		// Add Normal data
		GetFBXNormals(meshDataVec, mesh, indexMapper);

		// Add SkinData
		GetFBXSkin(meshDataVec, mesh, mSkeleton, indexMapper);

		// Update vertex position with cache poses
		ReadFBXCacheVertexPositions(meshDataVec, mesh, indexMapper);

		// Destroy the SDK manager and all the other objects it was handling.
		lSdkManager->Destroy();


		Create(meshDataVec);
		return true;
	}

	void SkeletalMeshResource::GetFBXSkin(MultiPartMeshData<VertexType>& meshDataVec, void const* meshP, DList<Mesh::SkeletonJoint>& skeletonHierarchy, DVector<DUnorderedMap<int, int>>& indexMapper)
	{
		auto mesh = (FbxMesh const*)meshP;
		if (mesh->GetDeformerCount(FbxDeformer::eSkin) == 0)
			return;

		auto deformer = (FbxSkin*)mesh->GetDeformer(0, FbxDeformer::eSkin);

		auto clusterCount = deformer->GetClusterCount();

		// Finding skeleton root
		FbxSkeleton* SkeletonRoot = 0;
		{
			for (int i = 0; i < clusterCount; i++)
			{
				auto cluster = deformer->GetCluster(i);
				if (!cluster->GetLink())
					continue;
				SkeletonRoot = cluster->GetLink()->GetSkeleton();
			}

			if (!SkeletonRoot)
				return;

			while (!SkeletonRoot->IsSkeletonRoot())
				SkeletonRoot = SkeletonRoot->GetNode()->GetParent()->GetSkeleton();
		}

		// Creating skeleton hierarchy
		DMap<void const*, int> skeletonIndexMap;
		{
			Mesh::SkeletonJoint sceneGraphNode;
			sceneGraphNode.MatrixIdx = 0;
			sceneGraphNode.StaleMatrix = true;
			sceneGraphNode.SkeletonRoot = true;
			skeletonHierarchy.push_back(sceneGraphNode);
			skeletonIndexMap.insert({ SkeletonRoot, 0 });
			SkeletonRoot->GetNode()->SetGeometricTranslation(FbxNode::eDestinationPivot, { 0.f, 0.f, 0.f, 1.f });
			AddSkeletonChildren(SkeletonRoot, skeletonHierarchy, skeletonIndexMap);
		}

		// Assigning blend weights and indices
		VertexBlendWeightData skinData;
		{
			skinData.jointWeight.resize(mesh->GetControlPointsCount());
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

					skinData.jointWeight[controlPointIndex].push_back({ jointIndex, controlPointWeight });
				}
			}
		}

		AddJointWeightToVertices(meshDataVec, skinData, indexMapper);
	}

	void SkeletalMeshResource::AddSkeletonChildren(void const* skeletonNode, DList<Mesh::SkeletonJoint>& skeletonData, DMap<void const*, int>& skeletonIndexMap)
	{
		auto skeleton = (FbxSkeleton*)skeletonNode;
		auto node = skeleton->GetNode();

		auto index = skeletonData.size() - 1;

		auto& currentSceneGraphNode = skeletonData.back();

		// Setting translation
		auto transform = node->EvaluateLocalTransform();
		FbxDouble* fbxTrans;
		if (currentSceneGraphNode.SkeletonRoot)
			fbxTrans = FbxDouble3(0, 0, 0).Buffer();
		else
			fbxTrans = node->EvaluateLocalTranslation().Buffer();

		auto fbxSclae = node->EvaluateLocalScaling();
		auto fbxRot = transform.GetQ();
		//FbxQuaternion()
		currentSceneGraphNode.Xform.SetW({ (float)fbxTrans[0], (float)fbxTrans[1], (float)fbxTrans[2], 1.f });
		currentSceneGraphNode.Scale = { (float)fbxSclae.mData[0], (float)fbxSclae.mData[1], (float)fbxSclae.mData[2] };
		currentSceneGraphNode.Rotation = Quaternion((float)fbxRot.mData[0], (float)fbxRot.mData[1], (float)fbxRot.mData[2], (float)fbxRot.mData[3]);

		currentSceneGraphNode.Name = node->GetInitialName();

		// Compute ibm
		{
			float matData[16];
			auto& fbxMat = node->EvaluateGlobalTransform();
			if (currentSceneGraphNode.SkeletonRoot)
				fbxMat.SetT({ 0, 0, 0, 1 });
			for (int row = 0; row < 4; row++)
				for (int col = 0; col < 4; col++)
					matData[row * 4 + col] = fbxMat.Get(row, col);

			currentSceneGraphNode.IBM = Matrix4(matData).Inverse();
		}

		skeletonIndexMap.insert({ skeleton, (int)currentSceneGraphNode.MatrixIdx });

		for (int i = 0; i < node->GetChildCount(); i++)
		{
			auto child = node->GetChild(i);

			if (!child->GetSkeleton())
				continue;

			Mesh::SkeletonJoint sceneGraphNode;
			ZeroMemory(&sceneGraphNode, sizeof(Mesh::SkeletonJoint));
			sceneGraphNode.StaleMatrix = true;
			sceneGraphNode.MatrixIdx = skeletonData.size();
			skeletonData.push_back(sceneGraphNode);
			currentSceneGraphNode.Children.push_back(&skeletonData.back());
			AddSkeletonChildren(node->GetChild(i)->GetSkeleton(), skeletonData, skeletonIndexMap);
		}
	}

	void SkeletalMeshResource::AddJointWeightToVertices(
		MultiPartMeshData<VertexType>& meshDataVec,
		VertexBlendWeightData& skinData,
		DVector<DUnorderedMap<int, int>> const& indexMapper
	)
	{
		// Not by control points
		if (meshDataVec.meshParts.size() != 1)
		{
			for (int polyIdx = 0; polyIdx < indexMapper.size(); polyIdx++)
			{
				for (auto [vertGlobalIndex, vertSubmeshIndex] : indexMapper[polyIdx])
				{
					// Which vertex to put data in
					auto& targetVertex = meshDataVec.meshParts[polyIdx].Vertices[vertSubmeshIndex];

					// Blend data to put in vector
					// Taking 4 data with most significant blend values
					auto& vertBlendData = skinData.jointWeight[vertGlobalIndex];

					AddBlendDataToVertex(targetVertex, vertBlendData);
				}
			}
		}
		// By control point
		else
		{
			for (int vertIndex = 0; vertIndex < meshDataVec.meshParts[0].Vertices.size(); vertIndex++)
			{
				auto& targetvertex = meshDataVec.meshParts[0].Vertices[vertIndex];
				auto& vertBlendData = skinData.jointWeight[vertIndex];
				AddBlendDataToVertex(targetvertex, vertBlendData);
			}
		}
	}

	void SkeletalMeshResource::AddBlendDataToVertex(MeshResource::VertexType& vertex, DVector<std::pair<int, float>>& blendData)
	{
		std::sort(blendData.begin(), blendData.end(),
			[](std::pair<int, float> const& a, std::pair<int, float> const& b)
			{
				return b.second < a.second;
			});

		// Adding blend data to vertex
		for (int i = 0; i < 4; i++)
		{
			if (i >= blendData.size())
			{
				((float*)&vertex.mBlendWeights)[i] = 0.f;
				((int*)&vertex.mBlendIndices)[i] = 0;
			}
			else
			{
				((float*)&vertex.mBlendWeights)[i] = blendData[i].second;
				((int*)&vertex.mBlendIndices)[i] = blendData[i].first;
			}
		}

		auto weightVec = Vector4(vertex.mBlendWeights);
		weightVec = weightVec / Dot(weightVec, Vector4(1.f, 1.f, 1.f, 1.f));
		vertex.mBlendWeights = weightVec;
	}

	void SkeletalMeshResource::ReadFBXCacheVertexPositions(MultiPartMeshData<VertexType>& meshDataVec, void const* meshP, DVector<DUnorderedMap<int, int>>& indexMapper)
	{
		auto mesh = (FbxMesh*)meshP;

		auto a1 = mesh->GetDeformerCount(FbxDeformer::eVertexCache);
		auto a2 = mesh->GetDeformerCount(FbxDeformer::eSkin);
		auto a3 = mesh->GetDeformerCount(FbxDeformer::eUnknown);
		auto a4 = mesh->GetDeformerCount(FbxDeformer::eBlendShape);
		auto def = mesh->GetDeformer(FbxDeformer::eUnknown);

		// Find vertex pos deformer
		FbxVertexCacheDeformer* deformer = nullptr;
		for (int i = 0; i < mesh->GetDeformerCount(FbxDeformer::eVertexCache); i++)
		{
			auto iterDeformer = static_cast<FbxVertexCacheDeformer*>(mesh->GetDeformer(i, FbxDeformer::eVertexCache));
			if (iterDeformer->Type.Get() == FbxVertexCacheDeformer::ePositions)
			{
				deformer = iterDeformer;
				break;
			}
		}

		// Abort if doesn't exist
		if (!deformer)
			return;

		// Reading vertex pos from cache
		auto cache = deformer->GetCache();
		int channelIndex = cache->GetChannelIndex(deformer->Channel.Get());
		float* buffer;
		int vertexCount = mesh->GetControlPointsCount();

		// If there is cache for every vertex component xyz
		{
			unsigned int length = 0;
			cache->Read(nullptr, length, FBXSDK_TIME_ZERO, channelIndex);

			if (length != vertexCount * 3)
				return;
		}

		unsigned int bufferSize = 0;
		auto readSucceed = cache->Read(&buffer, bufferSize, FBXSDK_TIME_ZERO, channelIndex);

		if (!readSucceed)
			return;

		unsigned int readBufferIndex = 0;
		// Assigning vertices their initial positions
		{
			// Vertex by control point or polygon?
			if (meshDataVec.meshParts.size() == 1)
			{
				auto& vertArray = meshDataVec.meshParts[0].Vertices;
				while (readBufferIndex < 3 * vertexCount)
				{
					auto vertGlobalIdx = readBufferIndex / 3;

					vertArray[vertGlobalIdx].mPosition.x = buffer[readBufferIndex]; readBufferIndex++;
					vertArray[vertGlobalIdx].mPosition.y = buffer[readBufferIndex]; readBufferIndex++;
					vertArray[vertGlobalIdx].mPosition.z = buffer[readBufferIndex]; readBufferIndex++;
				}
			}
			// Indirect
			else
			{
				// Map global vertex id to index in poly
				DVector<std::pair<int, int>> vertexGlobalIndexMapper(vertexCount);
				for (int polyIdx = 0; polyIdx < indexMapper.size(); polyIdx++)
				{
					for (auto [vertGlobalIdx, vertPolyIdx] : indexMapper[polyIdx])
					{
						vertexGlobalIndexMapper[vertGlobalIdx] = { polyIdx, vertPolyIdx };
					}
				}

				// Assiging values
				while (readBufferIndex < 3 * vertexCount)
				{
					auto vertGlobalIdx = readBufferIndex / 3;
					auto const& dic = vertexGlobalIndexMapper[vertGlobalIdx];
					auto& vert = meshDataVec.meshParts[dic.first].Vertices[dic.second];

					vert.mPosition.x = buffer[readBufferIndex]; readBufferIndex++;
					vert.mPosition.y = buffer[readBufferIndex]; readBufferIndex++;
					vert.mPosition.z = buffer[readBufferIndex]; readBufferIndex++;
				}
			}
		}
	}


#ifdef _D_EDITOR
	void DrawJoint(const Mesh::SkeletonJoint* joint)
	{
		ImGuiTreeNodeFlags flag = joint->Children.size() ? 0 : ImGuiTreeNodeFlags_Leaf;
		if (ImGui::TreeNodeEx(joint->Name.c_str(), flag))
		{

			for (auto childNode : joint->Children)
			{
				DrawJoint(childNode);
			}

			ImGui::TreePop();
		}
	}

	bool SkeletalMeshResource::DrawDetails(float params[])
	{
		DrawJoint(mSkeletonRoot);

		return false;
	}
#endif

}