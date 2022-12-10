#include "Renderer/pch.hpp"
#include "FbxLoader.hpp"

#include <Utils/Assert.hpp>


#define FBXSDK_SHARED

#include <fbxsdk.h>
#include <fbxsdk/fileio/fbxiosettings.h>

using namespace fbxsdk;
using namespace D_RESOURCE;
using namespace D_CONTAINERS;

namespace Darius::Renderer::Geometry::ModelLoader
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

        int IndexOffset;
        int TriangleCount;
    };

    // Reads the scene from path and return a pointer to root node of the scene
    // to work with and the sdk manager to get dispose of later.
    bool InitializeFbxScene(D_FILE::Path const& path, FbxNode* rootNode, FbxManager* sdkManager)
    {
        // Create the FBX SDK manager
        sdkManager = FbxManager::Create();

        // Create an IOSettings object.
        FbxIOSettings* ios = FbxIOSettings::Create(sdkManager, IOSROOT);
        sdkManager->SetIOSettings(ios);

        // Declare the path and filename of the file containing the scene.
        // In this case, we are assuming the file is in the same directory as the executable.
        auto pathStr = path.string();
        const char* lFilename = pathStr.c_str();

        // Create an importer.
        FbxImporter* lImporter = FbxImporter::Create(sdkManager, pathStr.c_str());

        // Initialize the importer.
        bool lImportStatus = lImporter->Initialize(lFilename, -1, sdkManager->GetIOSettings());

        if (!lImportStatus) {
            D_LOG_ERROR("Call to FbxImporter::Initialize() failed. Tried to load resource from " << pathStr);
            std::string msg = "Error returned: " + std::string(lImporter->GetStatus().GetErrorString());
            D_LOG_ERROR(msg);
            return false;
        }

        // Create a new scene so it can be populated by the imported file.
        FbxScene* lScene = FbxScene::Create(sdkManager, "modelScene");

        // Import the contents of the file into the scene.
        lImporter->Import(lScene);

        // The file has been imported; we can get rid of the importer.
        lImporter->Destroy();

        // Print the nodes of the scene and their attributes recursively.
        // Note that we are not printing the root node because it should
        // not contain any attributes.
        rootNode = lScene->GetRootNode();
    }

    void TraverseNodes(void* nodeP, std::function<void(void*)> callback);

    bool ReadMeshNode(const void* meshP, MultiPartMeshData<D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned>& result);

    D_CONTAINERS::DVector<D_RESOURCE::ResourceDataInFile> GetResourcesDataFromFile(D_RESOURCE::ResourceType, D_FILE::Path const& path)
    {
        FbxManager* sdkManager;

    }

    bool ReadMeshNode(void* meshP, MultiPartMeshData<D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned>& result)
    {
        auto pMesh = reinterpret_cast<FbxMesh*>(meshP);
        if (!pMesh->GetNode())
            return false;

        pMesh->GenerateNormals();
        pMesh->GenerateTangentsData(0);

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
        D_ASSERT(lTangents);
        D_ASSERT(lUVs);

        for (int i = 0; i < lPolygonVertexCount; i++)
        {
            float* vert = &lVertices[i * VERTEX_STRIDE];
            float* norm = &lNormals[i * NORMAL_STRIDE];
            float* tang = &lTangents[i * TANGENT_STRIDE];
            float* uv = &lUVs[i * UV_STRIDE];
            auto vertex = D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned(vert[0], vert[1], vert[2], norm[0], norm[1], norm[2], tang[0], tang[1], tang[2], tang[3], uv[0], uv[1]);

            result.MeshData.Vertices.push_back(vertex);
        }
        for (int i = 0; i < lPolygonCount * TRIANGLE_VERTEX_COUNT; i++)
        {
            result.MeshData.Indices32.push_back(lIndices[i]);
        }

        delete[] lVertices;
        delete[] lIndices;
        delete[] lNormals;
        delete[] lTangents;
        delete[] lUVs;

        return true;
    }

    void TraverseNodes(void* nodeP, std::function<void(void*)> callback)
    {
        auto node = reinterpret_cast<FbxNode*>(nodeP);
        callback(nodeP);

        for (int i = 0; i < node->GetChildCount(); i++)
        {
            TraverseNodes(node->GetChild(i), callback);
        }
    }

}