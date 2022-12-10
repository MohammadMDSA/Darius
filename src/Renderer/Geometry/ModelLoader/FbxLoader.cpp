#include "Renderer/pch.hpp"
#include "FbxLoader.hpp"

#include "Renderer/Resources/SkeletalMeshResource.hpp"

#include <Utils/Assert.hpp>


#define FBXSDK_SHARED

#include <fbxsdk.h>
#include <fbxsdk/fileio/fbxiosettings.h>

using namespace fbxsdk;
using namespace D_RESOURCE;
using namespace D_CONTAINERS;

namespace Darius::Renderer::Geometry::ModelLoader::Fbx
{
    const int TRIANGLE_VERTEX_COUNT = 3;

    // Four floats for every position.
    const int VERTEX_STRIDE = 4;
    // Three floats for every normal.
    const int NORMAL_STRIDE = 3;
    // Two floats for every UV.
    const int UV_STRIDE = 2;
    // Four floats for every Tangent.
    const int TANGENT_STRIDE = 4;

    // For every material, record the offsets in every VBO and triangle counts
    struct SubMesh
    {
        SubMesh() : IndexOffset(0), TriangleCount(0) {}

		UINT IndexOffset;
		UINT TriangleCount;
    };

	void TraverseNodes(FbxNode* nodeP, std::function<void(FbxNode*)> callback);
	bool ReadMeshNode(FbxMesh* pMesh, MultiPartMeshData<D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned>& result, DUnorderedMap<int, DVector<int>>& controlPointIndexToVertexIndexMap);
	bool ReadMeshSkin(FbxMesh* pMesh, MultiPartMeshData<D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned>& meshData, DList<D_RENDERER_GEOMETRY::Mesh::SkeletonJoint>& skeleton, DUnorderedMap<int, DVector<int>> const& controlPointIndexToVertexIndexMap);
	void AddSkeletonChildren(FbxSkeleton const* skeletonNode, DList<Mesh::SkeletonJoint>& skeletonData, DMap<FbxSkeleton const*, int>& skeletonIndexMap);
	void ReadFBXCacheVertexPositions(MultiPartMeshData<D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned>& meshDataVec, FbxMesh const* mesh, DUnorderedMap<int, DVector<int>> const& controlPointIndexToVertexIndexMap);
	void AddJointWeightToVertices(VertexBlendWeightData const& skinData, MultiPartMeshData<D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned>& meshData, DUnorderedMap<int, DVector<int>> const& controlPointIndexToVertexIndexMap);

#pragma region Common
	// Reads the scene from path and return a pointer to root node of the scene
	// to work with and the sdk manager to get dispose of later.
	bool InitializeFbxScene(D_FILE::Path const& path, FbxNode** rootNode, FbxManager** sdkManager)
	{
		// Create the FBX SDK manager
		*sdkManager = FbxManager::Create();

		// Create an IOSettings object.
		FbxIOSettings* ios = FbxIOSettings::Create(*sdkManager, IOSROOT);
		(*sdkManager)->SetIOSettings(ios);

		// Declare the path and filename of the file containing the scene.
		// In this case, we are assuming the file is in the same directory as the executable.
		auto pathStr = path.string();
		const char* lFilename = pathStr.c_str();

		// Create an importer.
		FbxImporter* lImporter = FbxImporter::Create(*sdkManager, pathStr.c_str());

		// Initialize the importer.
		bool lImportStatus = lImporter->Initialize(lFilename, -1, (*sdkManager)->GetIOSettings());

		if (!lImportStatus) {
			D_LOG_ERROR("Call to FbxImporter::Initialize() failed. Tried to load resource from " << pathStr);
			std::string msg = "Error returned: " + std::string(lImporter->GetStatus().GetErrorString());
			D_LOG_ERROR(msg);
			return false;
		}

		// Create a new scene so it can be populated by the imported file.
		FbxScene* lScene = FbxScene::Create(*sdkManager, "modelScene");

		// Import the contents of the file into the scene.
		lImporter->Import(lScene);

		// The file has been imported; we can get rid of the importer.
		lImporter->Destroy();

		// Print the nodes of the scene and their attributes recursively.
		// Note that we are not printing the root node because it should
		// not contain any attributes.
		*rootNode = lScene->GetRootNode();
	}

	void TraverseNodes(FbxNode* node, std::function<void(FbxNode*)> callback)
	{
		callback(node);

		for (int i = 0; i < node->GetChildCount(); i++)
		{
			TraverseNodes(node->GetChild(i), callback);
		}
	}

	INLINE Matrix4 GetMat4(FbxAMatrix const& mat)
	{
		float matData[16];
		for (int row = 0; row < 4; row++)
			for (int col = 0; col < 4; col++)
				matData[row * 4 + col] = mat.Get(row, col);
		return Matrix4(matData);
	}

	// Get the geometry offset to a node. It is never inherited by the children.
	FbxAMatrix GetGeometry(FbxNode* pNode)
	{
		const FbxVector4 lT = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
		const FbxVector4 lR = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
		const FbxVector4 lS = pNode->GetGeometricScaling(FbxNode::eSourcePivot);

		return FbxAMatrix(lT, lR, lS);
	}

	// Get the matrix of the given pose
	FbxAMatrix GetPoseMatrix(FbxPose* pPose, int pNodeIndex)
	{
		FbxAMatrix lPoseMatrix;
		FbxMatrix lMatrix = pPose->GetMatrix(pNodeIndex);

		memcpy((double*)lPoseMatrix, (double*)lMatrix, sizeof(lMatrix.mData));

		return lPoseMatrix;
	}

	// Get the global position of the node for the current pose.
	// If the specified node is not part of the pose or no pose is specified, get its
	// global position at the current time.
	FbxAMatrix GetGlobalPosition(FbxNode* pNode, const FbxTime& pTime, FbxPose* pPose = nullptr, FbxAMatrix* pParentGlobalPosition = nullptr)
	{
		FbxAMatrix lGlobalPosition;
		bool        lPositionFound = false;

		if (pPose)
		{
			int lNodeIndex = pPose->Find(pNode);

			if (lNodeIndex > -1)
			{
				// The bind pose is always a global matrix.
				// If we have a rest pose, we need to check if it is
				// stored in global or local space.
				if (pPose->IsBindPose() || !pPose->IsLocalMatrix(lNodeIndex))
				{
					lGlobalPosition = GetPoseMatrix(pPose, lNodeIndex);
				}
				else
				{
					// We have a local matrix, we need to convert it to
					// a global space matrix.
					FbxAMatrix lParentGlobalPosition;

					if (pParentGlobalPosition)
					{
						lParentGlobalPosition = *pParentGlobalPosition;
					}
					else
					{
						if (pNode->GetParent())
						{
							lParentGlobalPosition = GetGlobalPosition(pNode->GetParent(), pTime, pPose);
						}
					}

					FbxAMatrix lLocalPosition = GetPoseMatrix(pPose, lNodeIndex);
					lGlobalPosition = lParentGlobalPosition * lLocalPosition;
				}

				lPositionFound = true;
			}
		}

		if (!lPositionFound)
		{
			// There is no pose entry for that node, get the current global position instead.

			// Ideally this would use parent global position and local position to compute the global position.
			// Unfortunately the equation 
			//    lGlobalPosition = pParentGlobalPosition * lLocalPosition
			// does not hold when inheritance type is other than "Parent" (RSrs).
			// To compute the parent rotation and scaling is tricky in the RrSs and Rrs cases.
			lGlobalPosition = pNode->EvaluateGlobalTransform(pTime);
		}

		return lGlobalPosition;
	}

#pragma endregion Common

#pragma region Mesh

	D_CONTAINERS::DVector<D_RESOURCE::ResourceDataInFile> GetMeshResourcesDataFromFile(D_RESOURCE::ResourceType type, D_FILE::Path const& path)
    {
		FbxManager* sdkManager = nullptr;
		FbxNode* rootNode = nullptr;

		InitializeFbxScene(path, &rootNode, &sdkManager);

		DVector<ResourceDataInFile> results;

		bool isSkeletal = D_GRAPHICS::SkeletalMeshResource::GetResourceType() == type;

		TraverseNodes(rootNode, [&results, type, isSkeletal](FbxNode* node)
			{
				auto attr = node->GetNodeAttribute();
				if (attr && attr->GetAttributeType() == FbxNodeAttribute::eMesh)
				{
					auto mesh = node->GetMesh();
					auto hasSkin = mesh->GetDeformerCount(FbxDeformer::eSkin) > 0;
					if ((hasSkin && isSkeletal) || (!hasSkin && !isSkeletal))
					{
						ResourceDataInFile data;
						data.Name = node->GetName();
						data.Type = type;
						results.push_back(data);
					}
				}
			});

		sdkManager->Destroy();

		return results;
	}

	bool ReadMeshByName(D_FILE::Path const& path, std::wstring const& meshName, MultiPartMeshData<D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned>& result, DUnorderedMap<int, DVector<int>>& controlPointIndexToVertexIndexMap, FbxMesh** mesh, FbxManager** sdkManager)
	{
		FbxNode* rootNode = nullptr;

		InitializeFbxScene(path, &rootNode, sdkManager);

		// Searching for a mesh node with our resource name
		FbxNode* targetNode = 0;
		TraverseNodes(rootNode, [&](FbxNode* node)
			{
				auto attr = node->GetNodeAttribute();
				auto nodeName = std::string(node->GetName());
				if (attr && attr->GetAttributeType() == FbxNodeAttribute::eMesh && WSTR_STR(nodeName) == meshName)
				{
					targetNode = node;
				}
			});

		if (!targetNode)
			return false;

		*mesh = targetNode->GetMesh();
		ReadMeshNode(*mesh, result, controlPointIndexToVertexIndexMap);
	}

	bool ReadMeshByName(D_FILE::Path const& path, std::wstring const& meshName, MultiPartMeshData<D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned>& result)
	{
		FbxMesh* mesh;
		FbxManager* sdkManager;

		DUnorderedMap<int, DVector<int>> controlPointIndexToVertexIndexMap;
		auto res = ReadMeshByName(path, meshName, result, controlPointIndexToVertexIndexMap, &mesh, &sdkManager);


		sdkManager->Destroy();
		return res;
	}

	bool ReadMeshNode(FbxMesh* pMesh, MultiPartMeshData<D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned>& result, DUnorderedMap<int, DVector<int>>& controlPointIndexToVertexIndexMap)
	{
        if (!pMesh->GetNode())
            return false;

        pMesh->GenerateNormals();
		auto f = pMesh->GenerateTangentsData();

        FbxArray<SubMesh*> subMeshes;
        bool mHasNormal;
        bool mHasUV;
        bool mHasTangent;
        bool mAllByControlPoint = true; // Save data in VBO by control point or by polygon vertex.

        const int lPolygonCount = pMesh->GetPolygonCount();

        // Count the polygon count of each material
        FbxLayerElementArrayTemplate<int>* lMaterialIndice = NULL;
        FbxGeometryElement::EMappingMode lMaterialMappingMode = FbxGeometryElement::eNone;
        if (pMesh->GetElementMaterial())
        {
            lMaterialIndice = &pMesh->GetElementMaterial()->GetIndexArray();
            lMaterialMappingMode = pMesh->GetElementMaterial()->GetMappingMode();
            if (lMaterialIndice && lMaterialMappingMode == FbxGeometryElement::eByPolygon)
            {
                FBX_ASSERT(lMaterialIndice->GetCount() == lPolygonCount);
                if (lMaterialIndice->GetCount() == lPolygonCount)
                {
                    // Count the faces of each material
                    for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex)
                    {
                        const int lMaterialIndex = lMaterialIndice->GetAt(lPolygonIndex);
                        if (subMeshes.GetCount() < lMaterialIndex + 1)
                        {
                            subMeshes.Resize(lMaterialIndex + 1);
                        }
                        if (subMeshes[lMaterialIndex] == NULL)
                        {
                            subMeshes[lMaterialIndex] = new SubMesh;
                        }
                        subMeshes[lMaterialIndex]->TriangleCount += 1;
                    }

                    // Make sure we have no "holes" (NULL) in the subMeshes table. This can happen
                    // if, in the loop above, we resized the subMeshes by more than one slot.
                    for (int i = 0; i < subMeshes.GetCount(); i++)
                    {
                        if (subMeshes[i] == NULL)
                            subMeshes[i] = new SubMesh;
                    }

                    // Record the offset (how many vertex)
                    const int lMaterialCount = subMeshes.GetCount();
                    int lOffset = 0;
                    for (int lIndex = 0; lIndex < lMaterialCount; ++lIndex)
                    {
                        subMeshes[lIndex]->IndexOffset = lOffset;
                        lOffset += subMeshes[lIndex]->TriangleCount * 3;
                        // This will be used as counter in the following procedures, reset to zero
                        subMeshes[lIndex]->TriangleCount = 0;
                    }
                    FBX_ASSERT(lOffset == lPolygonCount * 3);
                }
            }
        }

        // All faces will use the same material.
        if (subMeshes.GetCount() == 0)
        {
            subMeshes.Resize(1);
            subMeshes[0] = new SubMesh();
        }

        // Congregate all the data of a mesh to be cached in VBOs.
        // If normal or UV is by polygon vertex, record all vertex attributes by polygon vertex.
        mHasNormal = pMesh->GetElementNormalCount() > 0;
        mHasUV = pMesh->GetElementUVCount() > 0;
        mHasTangent = pMesh->GetElementTangentCount() > 0;
        FbxGeometryElement::EMappingMode lNormalMappingMode = FbxGeometryElement::eNone;
        FbxGeometryElement::EMappingMode lUVMappingMode = FbxGeometryElement::eNone;
        FbxGeometryElement::EMappingMode lTangentMappingMode = FbxGeometryElement::eNone;
        if (mHasNormal)
        {
            lNormalMappingMode = pMesh->GetElementNormal(0)->GetMappingMode();
            if (lNormalMappingMode == FbxGeometryElement::eNone)
            {
                mHasNormal = false;
            }
            if (mHasNormal && lNormalMappingMode != FbxGeometryElement::eByControlPoint)
            {
                mAllByControlPoint = false;
            }
        }
        if (mHasUV)
        {
            lUVMappingMode = pMesh->GetElementUV(0)->GetMappingMode();
            if (lUVMappingMode == FbxGeometryElement::eNone)
            {
                mHasUV = false;
            }
            if (mHasUV && lUVMappingMode != FbxGeometryElement::eByControlPoint)
            {
                mAllByControlPoint = false;
            }
        }
        if (mHasTangent)
        {
            lTangentMappingMode = pMesh->GetElementTangent(0)->GetMappingMode();
            if (lTangentMappingMode == FbxGeometryElement::eNone)
            {
                mHasTangent = false;
            }
            if (mHasTangent && lTangentMappingMode != FbxGeometryElement::eByControlPoint)
            {
                mAllByControlPoint = false;
            }
        }

        // Allocate the array memory, by control point or by polygon vertex.
        int lPolygonVertexCount = pMesh->GetControlPointsCount();
        if (!mAllByControlPoint)
        {
            lPolygonVertexCount = lPolygonCount * TRIANGLE_VERTEX_COUNT;
        }

        // Vertices
        float* lVertices = new float[lPolygonVertexCount * VERTEX_STRIDE];

        // Indices
        unsigned int* lIndices = new unsigned int[lPolygonCount * TRIANGLE_VERTEX_COUNT];

        // Normals
        float* lNormals = NULL;
        if (mHasNormal)
        {
            lNormals = new float[lPolygonVertexCount * NORMAL_STRIDE];
        }

        // UVs
        float* lUVs = NULL;
        FbxStringList lUVNames;
        pMesh->GetUVSetNames(lUVNames);
        const char* lUVName = NULL;
        if (mHasUV && lUVNames.GetCount())
        {
            lUVs = new float[lPolygonVertexCount * UV_STRIDE];
            lUVName = lUVNames[0];
        }

        // Tangents
        float* lTangents = NULL;
        if (mHasTangent)
        {
            lTangents = new float[lPolygonVertexCount * TANGENT_STRIDE];
        }

        // Populate the array with vertex attribute, if by control point.
        const FbxVector4* lControlPoints = pMesh->GetControlPoints();
        FbxVector4 lCurrentVertex;
        FbxVector4 lCurrentNormal;
        FbxVector2 lCurrentUV;
        FbxVector4 lCurrentTangent;

        if (mAllByControlPoint)
        {
            const FbxGeometryElementNormal* lNormalElement = NULL;
            const FbxGeometryElementUV* lUVElement = NULL;
            const FbxGeometryElementTangent* lTangentElement = NULL;
            if (mHasNormal)
            {
                lNormalElement = pMesh->GetElementNormal(0);
            }
            if (mHasUV)
            {
                lUVElement = pMesh->GetElementUV(0);
            }
            if (mHasTangent)
            {
                lTangentElement = pMesh->GetElementTangent(0);
            }
            for (int lIndex = 0; lIndex < lPolygonVertexCount; ++lIndex)
            {
                // Save the vertex position.
                lCurrentVertex = lControlPoints[lIndex];
                lVertices[lIndex * VERTEX_STRIDE] = static_cast<float>(lCurrentVertex[0]);
                lVertices[lIndex * VERTEX_STRIDE + 1] = static_cast<float>(lCurrentVertex[1]);
                lVertices[lIndex * VERTEX_STRIDE + 2] = static_cast<float>(lCurrentVertex[2]);
                lVertices[lIndex * VERTEX_STRIDE + 3] = 1;

				controlPointIndexToVertexIndexMap.insert({ lIndex, { lIndex } });

                // Save the normal.
                if (mHasNormal)
                {
                    int lNormalIndex = lIndex;
                    if (lNormalElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                    {
                        lNormalIndex = lNormalElement->GetIndexArray().GetAt(lIndex);
                    }
                    lCurrentNormal = lNormalElement->GetDirectArray().GetAt(lNormalIndex);
                    lNormals[lIndex * NORMAL_STRIDE] = static_cast<float>(lCurrentNormal[0]);
                    lNormals[lIndex * NORMAL_STRIDE + 1] = static_cast<float>(lCurrentNormal[1]);
                    lNormals[lIndex * NORMAL_STRIDE + 2] = static_cast<float>(lCurrentNormal[2]);
                }

                // Save the UV.
                if (mHasUV)
                {
                    int lUVIndex = lIndex;
                    if (lUVElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                    {
                        lUVIndex = lUVElement->GetIndexArray().GetAt(lIndex);
                    }
                    lCurrentUV = lUVElement->GetDirectArray().GetAt(lUVIndex);
                    lUVs[lIndex * UV_STRIDE] = static_cast<float>(lCurrentUV[0]);
                    lUVs[lIndex * UV_STRIDE + 1] = static_cast<float>(lCurrentUV[1]);
                }

                // Save the tangent
                if (mHasTangent)
                {
                    int lTantentIndex = lIndex;
                    if (lTangentElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
                    {
                        lTantentIndex = lTangentElement->GetIndexArray().GetAt(lIndex);
                    }
                    lCurrentTangent = lTangentElement->GetDirectArray().GetAt(lTantentIndex);
                    lTangents[lIndex * TANGENT_STRIDE] = static_cast<float>(lCurrentTangent[0]);
                    lTangents[lIndex * TANGENT_STRIDE + 1] = static_cast<float>(lCurrentTangent[1]);
                    lTangents[lIndex * TANGENT_STRIDE + 2] = static_cast<float>(lCurrentTangent[2]);
                    lTangents[lIndex * TANGENT_STRIDE + 3] = static_cast<float>(lCurrentTangent[3]);
                }
            }

        }

        int lVertexCount = 0;
        for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex)
        {
            // The material for current face.
            int lMaterialIndex = 0;
            if (lMaterialIndice && lMaterialMappingMode == FbxGeometryElement::eByPolygon)
            {
                lMaterialIndex = lMaterialIndice->GetAt(lPolygonIndex);
            }

            // Where should I save the vertex attribute index, according to the material
            const int lIndexOffset = subMeshes[lMaterialIndex]->IndexOffset +
                subMeshes[lMaterialIndex]->TriangleCount * 3;
            for (int lVerticeIndex = 0; lVerticeIndex < TRIANGLE_VERTEX_COUNT; ++lVerticeIndex)
            {
                const int lControlPointIndex = pMesh->GetPolygonVertex(lPolygonIndex, lVerticeIndex);
                // If the lControlPointIndex is -1, we probably have a corrupted mesh data. At this point,
                // it is not guaranteed that the cache will work as expected.
                if (lControlPointIndex >= 0)
                {
                    if (mAllByControlPoint)
                    {
                        lIndices[lIndexOffset + lVerticeIndex] = static_cast<unsigned int>(lControlPointIndex);
                    }
                    // Populate the array with vertex attribute, if by polygon vertex.
                    else
                    {
                        lIndices[lIndexOffset + lVerticeIndex] = static_cast<unsigned int>(lVertexCount);

                        lCurrentVertex = lControlPoints[lControlPointIndex];
                        lVertices[lVertexCount * VERTEX_STRIDE] = static_cast<float>(lCurrentVertex[0]);
                        lVertices[lVertexCount * VERTEX_STRIDE + 1] = static_cast<float>(lCurrentVertex[1]);
                        lVertices[lVertexCount * VERTEX_STRIDE + 2] = static_cast<float>(lCurrentVertex[2]);
                        lVertices[lVertexCount * VERTEX_STRIDE + 3] = 1;

						// Add index map
						{
							if (!controlPointIndexToVertexIndexMap.contains(lControlPointIndex))
								controlPointIndexToVertexIndexMap.insert({ lControlPointIndex, {} });
							controlPointIndexToVertexIndexMap[lControlPointIndex].push_back(lVertexCount);
						}

                        if (mHasNormal)
                        {
                            pMesh->GetPolygonVertexNormal(lPolygonIndex, lVerticeIndex, lCurrentNormal);
                            lNormals[lVertexCount * NORMAL_STRIDE] = static_cast<float>(lCurrentNormal[0]);
                            lNormals[lVertexCount * NORMAL_STRIDE + 1] = static_cast<float>(lCurrentNormal[1]);
                            lNormals[lVertexCount * NORMAL_STRIDE + 2] = static_cast<float>(lCurrentNormal[2]);
                        }

                        if (mHasUV)
                        {
                            bool lUnmappedUV;
                            pMesh->GetPolygonVertexUV(lPolygonIndex, lVerticeIndex, lUVName, lCurrentUV, lUnmappedUV);
                            lUVs[lVertexCount * UV_STRIDE] = static_cast<float>(lCurrentUV[0]);
                            lUVs[lVertexCount * UV_STRIDE + 1] = static_cast<float>(lCurrentUV[1]);
                        }
                    }
                }
                ++lVertexCount;
            }
            subMeshes[lMaterialIndex]->TriangleCount += 1;
        }

        result.SubMeshes.resize(subMeshes.Size());
        for (int i = 0; i < subMeshes.GetCount(); i++)
        {
            result.SubMeshes[i].IndexOffset = subMeshes[i]->IndexOffset;
            result.SubMeshes[i].IndexCount = subMeshes[i]->TriangleCount * TRIANGLE_VERTEX_COUNT;
            delete subMeshes[i];
        }

        subMeshes.Clear();

        D_ASSERT(lNormals);

		float empty[] = { 0.f, 1.f, 0.f, 1.f };

        for (int i = 0; i < lPolygonVertexCount; i++)
        {
            float* vert = &lVertices[i * VERTEX_STRIDE];
            float* norm = &lNormals[i * NORMAL_STRIDE];
			
			float* tang;
			if (mHasTangent)
				tang = &lTangents[i * TANGENT_STRIDE];
			else
				tang = empty;

			float* uv;
			if (mHasUV)
				uv = &lUVs[i * UV_STRIDE];
			else
				uv = empty;

			auto vertex = D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned(vert[0], vert[1], vert[2], norm[0], norm[1], norm[2], tang[0], tang[1], tang[2], tang[3], uv[0], 1 - uv[1]);

            result.MeshData.Vertices.push_back(vertex);
        }
        for (int i = 0; i < lPolygonCount * TRIANGLE_VERTEX_COUNT; i++)
        {
            result.MeshData.Indices32.push_back(lIndices[i]);
        }

		delete[] lVertices;
		delete[] lIndices;
		delete[] lNormals;
		if (mHasTangent)
			delete[] lTangents;
		if (mHasUV)
			delete[] lUVs;

		return true;
	}

#pragma endregion Mesh

#pragma region Skinned Mesh

	bool ReadMeshByName(D_FILE::Path const& path, std::wstring const& meshName, MultiPartMeshData<D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned>& result, DList<D_RENDERER_GEOMETRY::Mesh::SkeletonJoint>& skeleton)
	{
		DUnorderedMap<int, DVector<int>> controlPointIndexToVertexIndexMap;

		// Read mesh data
		FbxMesh* mesh;
		FbxManager* sdkManager;

		if (!ReadMeshByName(path, meshName, result, controlPointIndexToVertexIndexMap, &mesh, &sdkManager))
		{
			sdkManager->Destroy();
			return false;
		}

		// Read skeleton data
		if (ReadMeshSkin(mesh, result, skeleton, controlPointIndexToVertexIndexMap))
			ReadFBXCacheVertexPositions(result, mesh, controlPointIndexToVertexIndexMap);


		sdkManager->Destroy();
		return true;

	}

	void ComputeClusterDeformation(FbxAMatrix& pGlobalPosition,
		FbxMesh const* pMesh,
		FbxCluster* pCluster,
		FbxAMatrix& pVertexTransformMatrix,
		FbxTime pTime,
		FbxPose* pPose)
	{
		FbxCluster::ELinkMode lClusterMode = pCluster->GetLinkMode();

		FbxAMatrix lReferenceGlobalInitPosition;
		FbxAMatrix lReferenceGlobalCurrentPosition;
		FbxAMatrix lAssociateGlobalInitPosition;
		FbxAMatrix lAssociateGlobalCurrentPosition;
		FbxAMatrix lClusterGlobalInitPosition;
		FbxAMatrix lClusterGlobalCurrentPosition;

		FbxAMatrix lReferenceGeometry;
		FbxAMatrix lAssociateGeometry;
		FbxAMatrix lClusterGeometry;

		FbxAMatrix lClusterRelativeInitPosition;
		FbxAMatrix lClusterRelativeCurrentPositionInverse;

		if (lClusterMode == FbxCluster::eAdditive && pCluster->GetAssociateModel())
		{
			pCluster->GetTransformAssociateModelMatrix(lAssociateGlobalInitPosition);
			// Geometric transform of the model
			lAssociateGeometry = GetGeometry(pCluster->GetAssociateModel());
			lAssociateGlobalInitPosition *= lAssociateGeometry;
			lAssociateGlobalCurrentPosition = GetGlobalPosition(pCluster->GetAssociateModel(), pTime, pPose);

			pCluster->GetTransformMatrix(lReferenceGlobalInitPosition);
			// Multiply lReferenceGlobalInitPosition by Geometric Transformation
			lReferenceGeometry = GetGeometry(pMesh->GetNode());
			lReferenceGlobalInitPosition *= lReferenceGeometry;
			lReferenceGlobalCurrentPosition = pGlobalPosition;

			// Get the link initial global position and the link current global position.
			pCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
			// Multiply lClusterGlobalInitPosition by Geometric Transformation
			lClusterGeometry = GetGeometry(pCluster->GetLink());
			lClusterGlobalInitPosition *= lClusterGeometry;
			lClusterGlobalCurrentPosition = GetGlobalPosition(pCluster->GetLink(), pTime, pPose);

			// Compute the shift of the link relative to the reference.
			//ModelM-1 * AssoM * AssoGX-1 * LinkGX * LinkM-1*ModelM
			pVertexTransformMatrix = lReferenceGlobalInitPosition.Inverse() * lAssociateGlobalInitPosition * lAssociateGlobalCurrentPosition.Inverse() *
				lClusterGlobalCurrentPosition * lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;
		}
		else
		{
			pCluster->GetTransformMatrix(lReferenceGlobalInitPosition);
			lReferenceGlobalCurrentPosition = pGlobalPosition;
			// Multiply lReferenceGlobalInitPosition by Geometric Transformation
			lReferenceGeometry = GetGeometry(pMesh->GetNode());
			lReferenceGlobalInitPosition *= lReferenceGeometry;

			// Get the link initial global position and the link current global position.
			pCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
			lClusterGlobalCurrentPosition = GetGlobalPosition(pCluster->GetLink(), pTime, pPose);

			// Compute the initial position of the link relative to the reference.
			lClusterRelativeInitPosition = lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;

			// Compute the current position of the link relative to the reference.
			lClusterRelativeCurrentPositionInverse = lReferenceGlobalCurrentPosition.Inverse() * lClusterGlobalCurrentPosition;

			// Compute the shift of the link relative to the reference.
			pVertexTransformMatrix = lClusterRelativeCurrentPositionInverse * lClusterRelativeInitPosition;
		}
	}

	bool ReadMeshSkin(FbxMesh* mesh, MultiPartMeshData<D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned>& meshData, DList<D_RENDERER_GEOMETRY::Mesh::SkeletonJoint>& skeletonHierarchy, DUnorderedMap<int, DVector<int>> const& controlPointIndexToVertexIndexMap)
	{
		if (mesh->GetDeformerCount(FbxDeformer::eSkin) == 0)
			return false;

		auto deformer = (FbxSkin*)mesh->GetDeformer(0, FbxDeformer::eSkin);
		auto clusterCount = deformer->GetClusterCount();

		// Finding skeleton root
		if (clusterCount <= 0)
        return true;
		FbxSkeleton* skeletonRoot = nullptr;
		{
			// Finding first eligible skeleton joint
			for (int i = 0; i < clusterCount; i++)
			{
				auto cluster = deformer->GetCluster(i);
				if (!cluster->GetLink())
					continue;
				skeletonRoot = cluster->GetLink()->GetSkeleton();
				break;
			}

			if (!skeletonRoot)
				return false;

			while (!skeletonRoot->IsSkeletonRoot())
				skeletonRoot = skeletonRoot->GetNode()->GetParent()->GetSkeleton();
		}

		// Creating skeleton hierarchy
		DMap<FbxSkeleton const*, int> skeletonIndexMap;
		{
			Mesh::SkeletonJoint sceneGraphNode;
			sceneGraphNode.MatrixIdx = 0;
			sceneGraphNode.StaleMatrix = true;
			sceneGraphNode.SkeletonRoot = true;
			skeletonHierarchy.push_back(sceneGraphNode);
			skeletonIndexMap.insert({ skeletonRoot, 0 });
			AddSkeletonChildren(skeletonRoot, skeletonHierarchy, skeletonIndexMap);
    }

		// Assigning blend weights and indices
		VertexBlendWeightData skinData;
		{
			skinData.resize(mesh->GetControlPointsCount());

			// For each joint (cluster)
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

					auto glob = FbxAMatrix();
					glob.SetIdentity();
					FbxAMatrix lVertexTransformMatrix;
					ComputeClusterDeformation(glob, mesh, cluster, lVertexTransformMatrix, FBXSDK_TIME_ZERO, nullptr);

					skinData[controlPointIndex].push_back({ jointIndex, { controlPointWeight, GetMat4(lVertexTransformMatrix)}
						});

				}
			}
		}

		AddJointWeightToVertices(skinData, meshData, controlPointIndexToVertexIndexMap);
	}

	void AddBlendDataToVertex(D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned& vertex, DVector<std::pair<int, std::pair<float, D_MATH::Matrix4>>>& blendData)
	{
		std::sort(blendData.begin(), blendData.end(),
			[](std::pair<int, std::pair<float, D_MATH::Matrix4>> const& a, std::pair<int, std::pair<float, D_MATH::Matrix4>> const& b)
			{
				return b.second.first < a.second.first;
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
				((float*)&vertex.mBlendWeights)[i] = blendData[i].second.first;
				((int*)&vertex.mBlendIndices)[i] = blendData[i].first;
			}
		}

		auto weightVec = Vector4(vertex.mBlendWeights);
		weightVec = weightVec / Dot(weightVec, Vector4(1.f, 1.f, 1.f, 1.f));
		vertex.mBlendWeights = weightVec;

		Vector3 pos(kZero);
		auto weights = (float*)&vertex.mBlendWeights;
		for (int i = 0; i < 4; i++)
		{
			if (i < blendData.size())
				pos += Matrix4(weights[i] * blendData[i].second.second) * vertex.mPosition;
		}

		vertex.mPosition = (DirectX::XMFLOAT3)pos;
	}

	void AddJointWeightToVertices(VertexBlendWeightData const& skinData, MultiPartMeshData<D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned>& meshData, DUnorderedMap<int, DVector<int>> const& controlPointIndexToVertexIndexMap)
	{
		for (int controlPointIndex = 0; controlPointIndex < skinData.size(); controlPointIndex++)
		{
			auto controlPointBlendData = skinData[controlPointIndex];
			for (auto const& vertexIndex : controlPointIndexToVertexIndexMap.at(controlPointIndex))
			{
				AddBlendDataToVertex(meshData.MeshData.Vertices[vertexIndex], controlPointBlendData);
			}
		}
	}

	void AddSkeletonChildren(FbxSkeleton const* skeleton, DList<Mesh::SkeletonJoint>& skeletonData, DMap<FbxSkeleton const*, int>& skeletonIndexMap)
	{
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
		auto fbxRot = node->LclRotation.Get();

		currentSceneGraphNode.Xform.SetW({ (float)fbxTrans[0], (float)fbxTrans[1], (float)fbxTrans[2], 1.f });
		currentSceneGraphNode.Scale = { (float)fbxSclae.mData[0], (float)fbxSclae.mData[1], (float)fbxSclae.mData[2] };
		currentSceneGraphNode.Rotation = XMFLOAT3((float)fbxRot.mData[0], (float)fbxRot.mData[1], (float)fbxRot.mData[2]);

		currentSceneGraphNode.Name = node->GetInitialName();

		// Compute ibm
		{
			float matData[16];
			auto& fbxMat = node->EvaluateGlobalTransform();
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

	void ReadFBXCacheVertexPositions(MultiPartMeshData<D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned>& meshDataVec, FbxMesh const* mesh, DUnorderedMap<int, DVector<int>> const& controlPointIndexToVertexIndexMap)
	{

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
			while (readBufferIndex < 3 * vertexCount)
			{
				auto controlPointIdx = readBufferIndex / 3;

				for (auto vertexIndex : controlPointIndexToVertexIndexMap.at(controlPointIdx))
				{
					auto& vertex = meshDataVec.MeshData.Vertices[vertexIndex];
					vertex.mPosition.x = buffer[readBufferIndex]; readBufferIndex++;
					vertex.mPosition.y = buffer[readBufferIndex]; readBufferIndex++;
					vertex.mPosition.z = buffer[readBufferIndex]; readBufferIndex++;

				}
			}

		}

	}

#pragma endregion Skinned Mesh
}