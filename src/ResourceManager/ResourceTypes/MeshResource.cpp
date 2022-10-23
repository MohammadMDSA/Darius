#include "ResourceManager/pch.hpp"
#include "MeshResource.hpp"
#include "ResourceManager/ResourceManager.hpp"

#include <Core/Filesystem/Path.hpp>
#include <Core/Containers/Set.hpp>
#include <Renderer/RenderDeviceManager.hpp>
#include <Renderer/GraphicsCore.hpp>

#include <fbxsdk.h>
#include <fbxsdk/fileio/fbxiosettings.h>

using namespace D_RENDERER_GEOMETRY;
using namespace D_CONTAINERS;

namespace Darius::ResourceManager
{

	void MeshResource::GetFBXPolygons(MultiPartMeshData<VertexType>& meshDataVec, void const* meshP, DVector<DUnorderedMap<int, int>>& indexMapper)
	{
	
		auto mesh = (FbxMesh const*)meshP;
		auto polyCount = mesh->GetPolygonCount();

		auto controlPoints = mesh->GetControlPoints();

		auto mappinMode = mesh->GetElementNormal()->GetMappingMode();

		if (mappinMode == FbxGeometryElement::eByControlPoint)
		{
			D_RENDERER_GEOMETRY::MeshData<VertexType> submeshData;
			for (size_t i = 0; i < mesh->GetControlPointsCount(); i++)
			{
				VertexType vert;
				auto controlPoint = controlPoints[i];
				vert.mPosition.x = (float)controlPoint[0];
				vert.mPosition.y = (float)controlPoint[1];
				vert.mPosition.z = (float)controlPoint[2];
				vert.mNormal = { 0.f, 0.f, 0.f };
				vert.mTangent = { 0.f, 0.f, 0.f };
				submeshData.Vertices.push_back(vert);
			}

			for (size_t polyIdx = 0; polyIdx < polyCount; polyIdx++)
			{
				for (size_t vertIdx = 0; vertIdx < mesh->GetPolygonSize(polyIdx); vertIdx++)
				{
					// Add new mapped index
					auto vertexGlobalIdx = mesh->GetPolygonVertex(polyIdx, vertIdx);
					indexMapper[polyIdx][vertexGlobalIdx] = vertIdx;
					submeshData.Indices32.push_back(vertexGlobalIdx);
				}
			}
			meshDataVec.meshParts.push_back(submeshData);
		}
		else
		{
			for (size_t polyIdx = 0; polyIdx < polyCount; polyIdx++)
			{
				// A submesh for each polygon
				D_RENDERER_GEOMETRY::MeshData<VertexType> submeshData;

				for (size_t vertIdx = 0; vertIdx < mesh->GetPolygonSize(polyIdx); vertIdx++)
				{
					// Add new mapped index
					auto vertexGlobalIdx = mesh->GetPolygonVertex(polyIdx, vertIdx);
					if (!indexMapper[polyIdx].contains(vertexGlobalIdx))
					{
						VertexType vert;
						auto controlPoint = controlPoints[vertexGlobalIdx];
						vert.mPosition.x = (float)controlPoint[0];
						vert.mPosition.y = (float)controlPoint[1];
						vert.mPosition.z = (float)controlPoint[2];
						vert.mNormal = { 0.f, 0.f, 0.f };
						vert.mTangent = { 0.f, 0.f, 0.f };
						submeshData.Vertices.push_back(vert);

						indexMapper[polyIdx][vertexGlobalIdx] = submeshData.Vertices.size() - 1;
					}

					submeshData.Indices32.push_back(indexMapper[polyIdx][vertexGlobalIdx]);
				}

				meshDataVec.meshParts.push_back(submeshData);
			}
		}
	}

	void MeshResource::GetFBXNormals(MultiPartMeshData<VertexType>& meshDataVec, void const* meshP, DVector<DUnorderedMap<int, int>>& indexMapper)
	{
		auto mesh = (FbxMesh const*)meshP;

		//get the normal element
		FbxGeometryElementNormal const* lNormalElement = mesh->GetElementNormal();
		if (lNormalElement)
		{
			//mapping mode is by control points. The mesh should be smooth and soft.
			//we can get normals by retrieving each control point
			if (lNormalElement->GetMappingMode() == FbxGeometryElement::eByControlPoint)
			{
				
				//Let's get normals of each vertex, since the mapping mode of normal element is by control point
				for (int lVertexIndex = 0; lVertexIndex < mesh->GetControlPointsCount(); lVertexIndex++)
				{
					int lNormalIndex = 0;
					//reference mode is direct, the normal index is same as vertex index.
					//get normals by the index of control vertex
					if (lNormalElement->GetReferenceMode() == FbxGeometryElement::eDirect)
						lNormalIndex = lVertexIndex;

					//reference mode is index-to-direct, get normals by the index-to-direct
					if (lNormalElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
						lNormalIndex = lNormalElement->GetIndexArray().GetAt(lVertexIndex);

					//Got normals of each vertex.
					FbxVector4 lNormal = lNormalElement->GetDirectArray().GetAt(lNormalIndex);
					//add your custom code here, to output normals or get them into a list, such as KArrayTemplate<FbxVector4>

					auto& vert = meshDataVec.meshParts[0].Vertices[lVertexIndex];
					vert.mNormal.x = lNormal.mData[0];
					vert.mNormal.y = lNormal.mData[1];
					vert.mNormal.z = lNormal.mData[2];
				}//end for lVertexIndex
			}//end eByControlPoint
			//mapping mode is by polygon-vertex.
			//we can get normals by retrieving polygon-vertex.
			else if (lNormalElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
			{
				int lIndexByPolygonVertex = 0;
				//Let's get normals of each polygon, since the mapping mode of normal element is by polygon-vertex.
				for (int lPolygonIndex = 0; lPolygonIndex < mesh->GetPolygonCount(); lPolygonIndex++)
				{
					auto& polyMeshData = meshDataVec.meshParts[lPolygonIndex];
					//get polygon size, you know how many vertices in current polygon.
					int lPolygonSize = mesh->GetPolygonSize(lPolygonIndex);
					//retrieve each vertex of current polygon.
					for (int i = 0; i < lPolygonSize; i++)
					{
						int lNormalIndex = 0;
						//reference mode is direct, the normal index is same as lIndexByPolygonVertex.
						if (lNormalElement->GetReferenceMode() == FbxGeometryElement::eDirect)
							lNormalIndex = lIndexByPolygonVertex;

						//reference mode is index-to-direct, get normals by the index-to-direct
						if (lNormalElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
							lNormalIndex = lNormalElement->GetIndexArray().GetAt(lIndexByPolygonVertex);

						//Got normals of each polygon-vertex.
						FbxVector4 lNormal = lNormalElement->GetDirectArray().GetAt(lNormalIndex);
						
						auto& vert = polyMeshData.Vertices[polyMeshData.Indices32[i]];
						vert.mNormal.x = lNormal.mData[0];
						vert.mNormal.y = lNormal.mData[1];
						vert.mNormal.z = lNormal.mData[2];

						lIndexByPolygonVertex++;
					}//end for i //lPolygonSize
				}//end for lPolygonIndex //PolygonCount

			}//end eByPolygonVertex
		}
	}

	void MeshResource::GetFBXUVs(MultiPartMeshData<VertexType>& meshDataVec, void const* meshP, DVector<DUnorderedMap<int, int>>& indexMapper)
	{
		auto mesh = (FbxMesh const*)meshP;

		FbxStringList lUVSetNameList;
		mesh->GetUVSetNames(lUVSetNameList);

		if (lUVSetNameList.GetCount() == 0)
			return;

		const FbxGeometryElementUV* lUVElement = mesh->GetElementUV(lUVSetNameList[0]);

		//index array, where holds the index referenced to the uv data
		const bool lUseIndex = lUVElement->GetReferenceMode() != FbxGeometryElement::eDirect;
		const int lIndexCount = (lUseIndex) ? lUVElement->GetIndexArray().GetCount() : 0;

		//iterating through the data by polygon
		if (lUVElement->GetMappingMode() == FbxGeometryElement::eByControlPoint)
		{
			for (int lPolyIndex = 0; lPolyIndex < mesh->GetPolygonCount(); ++lPolyIndex)
			{
				auto& polyMeshData = meshDataVec.meshParts[lPolyIndex];
				// build the max index array that we need to pass into MakePoly
				for (int lVertIndex = 0; lVertIndex < mesh->GetPolygonSize(lPolyIndex); ++lVertIndex)
				{
					FbxVector2 lUVValue;

					int lPolyVertIndex = mesh->GetPolygonVertex(lPolyIndex, lVertIndex);

					//the UV index depends on the reference mode
					int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyVertIndex) : lPolyVertIndex;

					lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

					auto& vert = polyMeshData.Vertices[indexMapper[lPolyIndex][polyMeshData.Indices32[lVertIndex]]];

					vert.mTexC.x = lUVValue[0];
					vert.mTexC.y = lUVValue[1];
				}
			}
		}
		else if (lUVElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
		{
			int lPolyIndexCounter = 0;
			for (int lPolyIndex = 0; lPolyIndex < mesh->GetPolygonCount(); ++lPolyIndex)
			{
				auto& polyMeshData = meshDataVec.meshParts[lPolyIndex];

				// build the max index array that we need to pass into MakePoly
				for (int lVertIndex = 0; lVertIndex < mesh->GetPolygonSize(lPolyIndex); ++lVertIndex)
				{
					if (lPolyIndexCounter < lIndexCount)
					{
						FbxVector2 lUVValue;

						//the UV index depends on the reference mode
						int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyIndexCounter) : lPolyIndexCounter;

						lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

						//get the index of the current vertex in control points array
						auto& vert = polyMeshData.Vertices[polyMeshData.Indices32[lVertIndex]];
						vert.mTexC.x = lUVValue[0];
						vert.mTexC.y = 1 - lUVValue[1];

						lPolyIndexCounter++;
					}
				}
			}
		}
	}
}