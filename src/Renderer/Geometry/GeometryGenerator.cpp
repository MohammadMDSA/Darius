#include "Renderer/pch.hpp"

#include "GeometryGenerator.hpp"

namespace Darius::Renderer::Geometry::GeometryGenerator
{
	Vertex MidPoint(const Vertex& v0, const Vertex& v1)
	{
		XMVECTOR p0 = XMLoadFloat3(&v0.mPosition);
		XMVECTOR p1 = XMLoadFloat3(&v1.mPosition);

		XMVECTOR n0 = XMLoadFloat3(&v0.mNormal);
		XMVECTOR n1 = XMLoadFloat3(&v1.mNormal);

		XMVECTOR tan0 = XMLoadFloat3(&v0.mTangent);
		XMVECTOR tan1 = XMLoadFloat3(&v1.mTangent);

		XMVECTOR tex0 = XMLoadFloat2(&v0.mTexC);
		XMVECTOR tex1 = XMLoadFloat2(&v1.mTexC);

		// Compute the midpoints of all the attributes.  Vectors need to be normalized
		// since linear interpolating can make them not unit length.  
		XMVECTOR pos = 0.5f * (p0 + p1);
		XMVECTOR normal = XMVector3Normalize(0.5f * (n0 + n1));
		XMVECTOR tangent = XMVector3Normalize(0.5f * (tan0 + tan1));
		XMVECTOR tex = 0.5f * (tex0 + tex1);

		Vertex v;
		XMStoreFloat3(&v.mPosition, pos);
		XMStoreFloat3(&v.mNormal, normal);
		XMStoreFloat3(&v.mTangent, tangent);
		XMStoreFloat2(&v.mTexC, tex);

		return v;
	}

	void Subdivide(MeshData<Vertex>& meshData)
	{
		// Save a copy of the input geometry.
		MeshData<Vertex> inputCopy = meshData;

		meshData.mVertices.resize(0);
		meshData.mIndices32.resize(0);

		//       v1
	//       *
	//      / \
	//     /   \
	//  m0*-----*m1
	//   / \   / \
	//  /   \ /   \
	// *-----*-----*
	// v0    m2     v2

		uint32 numTris = (uint32)inputCopy.mIndices32.size() / 3;
		for (uint32 i = 0; i < numTris; ++i)
		{
			Vertex v0 = inputCopy.mVertices[inputCopy.mIndices32[i * 3 + 0]];
			Vertex v1 = inputCopy.mVertices[inputCopy.mIndices32[i * 3 + 1]];
			Vertex v2 = inputCopy.mVertices[inputCopy.mIndices32[i * 3 + 2]];

			//
			// Generate the midpoints.
			//

			Vertex m0 = MidPoint(v0, v1);
			Vertex m1 = MidPoint(v1, v2);
			Vertex m2 = MidPoint(v0, v2);

			//
			// Add new geometry.
			//

			meshData.mVertices.push_back(v0); // 0
			meshData.mVertices.push_back(v1); // 1
			meshData.mVertices.push_back(v2); // 2
			meshData.mVertices.push_back(m0); // 3
			meshData.mVertices.push_back(m1); // 4
			meshData.mVertices.push_back(m2); // 5

			meshData.mIndices32.push_back(i * 6 + 0);
			meshData.mIndices32.push_back(i * 6 + 3);
			meshData.mIndices32.push_back(i * 6 + 5);

			meshData.mIndices32.push_back(i * 6 + 3);
			meshData.mIndices32.push_back(i * 6 + 4);
			meshData.mIndices32.push_back(i * 6 + 5);

			meshData.mIndices32.push_back(i * 6 + 5);
			meshData.mIndices32.push_back(i * 6 + 4);
			meshData.mIndices32.push_back(i * 6 + 2);

			meshData.mIndices32.push_back(i * 6 + 3);
			meshData.mIndices32.push_back(i * 6 + 1);
			meshData.mIndices32.push_back(i * 6 + 4);
		}
	}

	MeshData<Vertex> CreateBox(float width, float height, float depth, uint32 numSubdivisions)
	{
		MeshData<Vertex> meshData;

		//
		// Create the vertices.
		//

		Vertex v[24];

		float w2 = 0.5f * width;
		float h2 = 0.5f * height;
		float d2 = 0.5f * depth;

		// Fill in the front face vertex data.
		v[0] = Vertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		v[1] = Vertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		v[2] = Vertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
		v[3] = Vertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

		// Fill in the back face vertex data.
		v[4] = Vertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
		v[5] = Vertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		v[6] = Vertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		v[7] = Vertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

		// Fill in the top face vertex data.
		v[8] = Vertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		v[9] = Vertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		v[10] = Vertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
		v[11] = Vertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

		// Fill in the bottom face vertex data.
		v[12] = Vertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
		v[13] = Vertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		v[14] = Vertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		v[15] = Vertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

		// Fill in the left face vertex data.
		v[16] = Vertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
		v[17] = Vertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
		v[18] = Vertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
		v[19] = Vertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

		// Fill in the right face vertex data.
		v[20] = Vertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
		v[21] = Vertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
		v[22] = Vertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
		v[23] = Vertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

		meshData.mVertices.assign(&v[0], &v[24]);

		//
		// Create the indices.
		//

		uint32 i[36];

		// Fill in the front face index data
		i[0] = 0; i[1] = 1; i[2] = 2;
		i[3] = 0; i[4] = 2; i[5] = 3;

		// Fill in the back face index data
		i[6] = 4; i[7] = 5; i[8] = 6;
		i[9] = 4; i[10] = 6; i[11] = 7;

		// Fill in the top face index data
		i[12] = 8; i[13] = 9; i[14] = 10;
		i[15] = 8; i[16] = 10; i[17] = 11;

		// Fill in the bottom face index data
		i[18] = 12; i[19] = 13; i[20] = 14;
		i[21] = 12; i[22] = 14; i[23] = 15;

		// Fill in the left face index data
		i[24] = 16; i[25] = 17; i[26] = 18;
		i[27] = 16; i[28] = 18; i[29] = 19;

		// Fill in the right face index data
		i[30] = 20; i[31] = 21; i[32] = 22;
		i[33] = 20; i[34] = 22; i[35] = 23;

		meshData.mIndices32.assign(&i[0], &i[36]);

		// Put a cap on the number of subdivisions.
		numSubdivisions = std::min<uint32>(numSubdivisions, 6u);

		for (uint32 i = 0; i < numSubdivisions; ++i)
			Subdivide(meshData);

		return meshData;
	}

	MeshData<Vertex> CreateSphere(float radius, uint32 sliceCount, uint32 stackCount)
	{
		MeshData<Vertex> meshData;

		//
		// Compute the vertices stating at the top pole and moving down the stacks.
		//

		// Poles: note that there will be texture coordinate distortion as there is
		// not a unique point on the texture map to assign to the pole when mapping
		// a rectangular texture onto a sphere.
		Vertex topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		Vertex bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

		meshData.mVertices.push_back(topVertex);

		float phiStep = XM_PI / stackCount;
		float thetaStep = 2.0f * XM_PI / sliceCount;

		// Compute vertices for each stack ring (do not count the poles as rings).
		for (uint32 i = 1; i <= stackCount - 1; ++i)
		{
			float phi = i * phiStep;

			// Vertices of ring.
			for (uint32 j = 0; j <= sliceCount; ++j)
			{
				float theta = j * thetaStep;

				Vertex v;

				// spherical to cartesian
				v.mPosition.x = radius * sinf(phi) * cosf(theta);
				v.mPosition.y = radius * cosf(phi);
				v.mPosition.z = radius * sinf(phi) * sinf(theta);

				// Partial derivative of P with respect to theta
				v.mTangent.x = -radius * sinf(phi) * sinf(theta);
				v.mTangent.y = 0.0f;
				v.mTangent.z = +radius * sinf(phi) * cosf(theta);

				XMVECTOR T = XMLoadFloat3(&v.mTangent);
				XMStoreFloat3(&v.mTangent, XMVector3Normalize(T));

				XMVECTOR p = XMLoadFloat3(&v.mPosition);
				XMStoreFloat3(&v.mNormal, XMVector3Normalize(p));

				v.mTexC.x = theta / XM_2PI;
				v.mTexC.y = phi / XM_PI;

				meshData.mVertices.push_back(v);
			}
		}

		meshData.mVertices.push_back(bottomVertex);

		//
		// Compute indices for top stack.  The top stack was written first to the vertex buffer
		// and connects the top pole to the first ring.
		//

		for (uint32 i = 1; i <= sliceCount; ++i)
		{
			meshData.mIndices32.push_back(0);
			meshData.mIndices32.push_back(i + 1);
			meshData.mIndices32.push_back(i);
		}

		//
		// Compute indices for inner stacks (not connected to poles).
		//

		// Offset the indices to the index of the first vertex in the first ring.
		// This is just skipping the top pole vertex.
		uint32 baseIndex = 1;
		uint32 ringVertexCount = sliceCount + 1;
		for (uint32 i = 0; i < stackCount - 2; ++i)
		{
			for (uint32 j = 0; j < sliceCount; ++j)
			{
				meshData.mIndices32.push_back(baseIndex + i * ringVertexCount + j);
				meshData.mIndices32.push_back(baseIndex + i * ringVertexCount + j + 1);
				meshData.mIndices32.push_back(baseIndex + (i + 1) * ringVertexCount + j);

				meshData.mIndices32.push_back(baseIndex + (i + 1) * ringVertexCount + j);
				meshData.mIndices32.push_back(baseIndex + i * ringVertexCount + j + 1);
				meshData.mIndices32.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
			}
		}

		//
		// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
		// and connects the bottom pole to the bottom ring.
		//

		// South pole vertex was added last.
		uint32 southPoleIndex = (uint32)meshData.mVertices.size() - 1;

		// Offset the indices to the index of the first vertex in the last ring.
		baseIndex = southPoleIndex - ringVertexCount;

		for (uint32 i = 0; i < sliceCount; ++i)
		{
			meshData.mIndices32.push_back(southPoleIndex);
			meshData.mIndices32.push_back(baseIndex + i);
			meshData.mIndices32.push_back(baseIndex + i + 1);
		}

		return meshData;
	}

	MeshData<Vertex> CreateGeosphere(float radius, uint32 numSubdivisions)
	{
		MeshData<Vertex> meshData;

		// Put a cap on the number of subdivisions.
		numSubdivisions = std::min<uint32>(numSubdivisions, 6u);

		// Approximate a sphere by tessellating an icosahedron.

		const float X = 0.525731f;
		const float Z = 0.850651f;

		XMFLOAT3 pos[12] =
		{
			XMFLOAT3(-X, 0.0f, Z),  XMFLOAT3(X, 0.0f, Z),
			XMFLOAT3(-X, 0.0f, -Z), XMFLOAT3(X, 0.0f, -Z),
			XMFLOAT3(0.0f, Z, X),   XMFLOAT3(0.0f, Z, -X),
			XMFLOAT3(0.0f, -Z, X),  XMFLOAT3(0.0f, -Z, -X),
			XMFLOAT3(Z, X, 0.0f),   XMFLOAT3(-Z, X, 0.0f),
			XMFLOAT3(Z, -X, 0.0f),  XMFLOAT3(-Z, -X, 0.0f)
		};

		uint32 k[60] =
		{
			1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,
			1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,
			3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
			10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7
		};

		meshData.mVertices.resize(12);
		meshData.mIndices32.assign(&k[0], &k[60]);

		for (uint32 i = 0; i < 12; ++i)
			meshData.mVertices[i].mPosition = pos[i];

		for (uint32 i = 0; i < numSubdivisions; ++i)
			Subdivide(meshData);

		// Project vertices onto sphere and scale.
		for (uint32 i = 0; i < meshData.mVertices.size(); ++i)
		{
			// Project onto unit sphere.
			XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&meshData.mVertices[i].mPosition));

			// Project onto sphere.
			XMVECTOR p = radius * n;

			XMStoreFloat3(&meshData.mVertices[i].mPosition, p);
			XMStoreFloat3(&meshData.mVertices[i].mNormal, n);

			// Derive texture coordinates from spherical coordinates.
			float theta = atan2f(meshData.mVertices[i].mPosition.z, meshData.mVertices[i].mPosition.x);

			// Put in [0, 2pi].
			if (theta < 0.0f)
				theta += XM_2PI;

			float phi = acosf(meshData.mVertices[i].mPosition.y / radius);

			meshData.mVertices[i].mTexC.x = theta / XM_2PI;
			meshData.mVertices[i].mTexC.y = phi / XM_PI;

			// Partial derivative of P with respect to theta
			meshData.mVertices[i].mTangent.x = -radius * sinf(phi) * sinf(theta);
			meshData.mVertices[i].mTangent.y = 0.0f;
			meshData.mVertices[i].mTangent.z = +radius * sinf(phi) * cosf(theta);

			XMVECTOR T = XMLoadFloat3(&meshData.mVertices[i].mTangent);
			XMStoreFloat3(&meshData.mVertices[i].mTangent, XMVector3Normalize(T));
		}

		return meshData;
	}

	void BuildCylinderTopCap(float bottomRadius, float topRadius, float height,
		uint32 sliceCount, uint32 stackCount, MeshData<Vertex>& meshData)
	{
		uint32 baseIndex = (uint32)meshData.mVertices.size();

		float y = 0.5f * height;
		float dTheta = 2.0f * XM_PI / sliceCount;

		// Duplicate cap ring vertices because the texture coordinates and normals differ.
		for (uint32 i = 0; i <= sliceCount; ++i)
		{
			float x = topRadius * cosf(i * dTheta);
			float z = topRadius * sinf(i * dTheta);

			// Scale down by the height to try and make top cap texture coord area
			// proportional to base.
			float u = x / height + 0.5f;
			float v = z / height + 0.5f;

			meshData.mVertices.push_back(Vertex(x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
		}

		// Cap center vertex.
		meshData.mVertices.push_back(Vertex(0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

		// Index of center vertex.
		uint32 centerIndex = (uint32)meshData.mVertices.size() - 1;

		for (uint32 i = 0; i < sliceCount; ++i)
		{
			meshData.mIndices32.push_back(centerIndex);
			meshData.mIndices32.push_back(baseIndex + i + 1);
			meshData.mIndices32.push_back(baseIndex + i);
		}
	}

	void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height,
		uint32 sliceCount, uint32 stackCount, MeshData<Vertex>& meshData)
	{
		// 
		// Build bottom cap.
		//

		uint32 baseIndex = (uint32)meshData.mVertices.size();
		float y = -0.5f * height;

		// vertices of ring
		float dTheta = 2.0f * XM_PI / sliceCount;
		for (uint32 i = 0; i <= sliceCount; ++i)
		{
			float x = bottomRadius * cosf(i * dTheta);
			float z = bottomRadius * sinf(i * dTheta);

			// Scale down by the height to try and make top cap texture coord area
			// proportional to base.
			float u = x / height + 0.5f;
			float v = z / height + 0.5f;

			meshData.mVertices.push_back(Vertex(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
		}

		// Cap center vertex.
		meshData.mVertices.push_back(Vertex(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

		// Cache the index of center vertex.
		uint32 centerIndex = (uint32)meshData.mVertices.size() - 1;

		for (uint32 i = 0; i < sliceCount; ++i)
		{
			meshData.mIndices32.push_back(centerIndex);
			meshData.mIndices32.push_back(baseIndex + i);
			meshData.mIndices32.push_back(baseIndex + i + 1);
		}
	}

	MeshData<Vertex> CreateCylinder(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount)
	{
		MeshData<Vertex> meshData;

		//
		// Build Stacks.
		// 

		float stackHeight = height / stackCount;

		// Amount to increment radius as we move up each stack level from bottom to top.
		float radiusStep = (topRadius - bottomRadius) / stackCount;

		uint32 ringCount = stackCount + 1;

		// Compute vertices for each stack ring starting at the bottom and moving up.
		for (uint32 i = 0; i < ringCount; ++i)
		{
			float y = -0.5f * height + i * stackHeight;
			float r = bottomRadius + i * radiusStep;

			// vertices of ring
			float dTheta = 2.0f * XM_PI / sliceCount;
			for (uint32 j = 0; j <= sliceCount; ++j)
			{
				Vertex vertex;

				float c = cosf(j * dTheta);
				float s = sinf(j * dTheta);

				vertex.mPosition = XMFLOAT3(r * c, y, r * s);

				vertex.mTexC.x = (float)j / sliceCount;
				vertex.mTexC.y = 1.0f - (float)i / stackCount;

				// Cylinder can be parameterized as follows, where we introduce v
				// parameter that goes in the same direction as the v tex-coord
				// so that the bitangent goes in the same direction as the v tex-coord.
				//   Let r0 be the bottom radius and let r1 be the top radius.
				//   y(v) = h - hv for v in [0,1].
				//   r(v) = r1 + (r0-r1)v
				//
				//   x(t, v) = r(v)*cos(t)
				//   y(t, v) = h - hv
				//   z(t, v) = r(v)*sin(t)
				// 
				//  dx/dt = -r(v)*sin(t)
				//  dy/dt = 0
				//  dz/dt = +r(v)*cos(t)
				//
				//  dx/dv = (r0-r1)*cos(t)
				//  dy/dv = -h
				//  dz/dv = (r0-r1)*sin(t)

				// This is unit length.
				vertex.mTangent = XMFLOAT3(-s, 0.0f, c);

				float dr = bottomRadius - topRadius;
				XMFLOAT3 bitangent(dr * c, -height, dr * s);

				XMVECTOR T = XMLoadFloat3(&vertex.mTangent);
				XMVECTOR B = XMLoadFloat3(&bitangent);
				XMVECTOR N = XMVector3Normalize(XMVector3Cross(T, B));
				XMStoreFloat3(&vertex.mNormal, N);

				meshData.mVertices.push_back(vertex);
			}
		}

		// Add one because we duplicate the first and last vertex per ring
		// since the texture coordinates are different.
		uint32 ringVertexCount = sliceCount + 1;

		// Compute indices for each stack.
		for (uint32 i = 0; i < stackCount; ++i)
		{
			for (uint32 j = 0; j < sliceCount; ++j)
			{
				meshData.mIndices32.push_back(i * ringVertexCount + j);
				meshData.mIndices32.push_back((i + 1) * ringVertexCount + j);
				meshData.mIndices32.push_back((i + 1) * ringVertexCount + j + 1);

				meshData.mIndices32.push_back(i * ringVertexCount + j);
				meshData.mIndices32.push_back((i + 1) * ringVertexCount + j + 1);
				meshData.mIndices32.push_back(i * ringVertexCount + j + 1);
			}
		}

		BuildCylinderTopCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);
		BuildCylinderBottomCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);

		return meshData;
	}


	MeshData<Vertex> CreateGrid(float width, float depth, uint32 m, uint32 n)
	{
		MeshData<Vertex> meshData;

		uint32 vertexCount = m * n;
		uint32 faceCount = (m - 1) * (n - 1) * 2;

		//
		// Create the vertices.
		//

		float halfWidth = 0.5f * width;
		float halfDepth = 0.5f * depth;

		float dx = width / (n - 1);
		float dz = depth / (m - 1);

		float du = 1.0f / (n - 1);
		float dv = 1.0f / (m - 1);

		meshData.mVertices.resize(vertexCount);
		for (uint32 i = 0; i < m; ++i)
		{
			float z = halfDepth - i * dz;
			for (uint32 j = 0; j < n; ++j)
			{
				float x = -halfWidth + j * dx;

				meshData.mVertices[i * n + j].mPosition = XMFLOAT3(x, 0.0f, z);
				meshData.mVertices[i * n + j].mNormal = XMFLOAT3(0.0f, 1.0f, 0.0f);
				meshData.mVertices[i * n + j].mTangent = XMFLOAT3(1.0f, 0.0f, 0.0f);

				// Stretch texture over grid.
				meshData.mVertices[i * n + j].mTexC.x = j * du;
				meshData.mVertices[i * n + j].mTexC.y = i * dv;
			}
		}

		//
		// Create the indices.
		//

		meshData.mIndices32.resize(faceCount * 3); // 3 indices per face

		// Iterate over each quad and compute indices.
		uint32 k = 0;
		for (uint32 i = 0; i < m - 1; ++i)
		{
			for (uint32 j = 0; j < n - 1; ++j)
			{
				meshData.mIndices32[k] = i * n + j;
				meshData.mIndices32[k + 1] = i * n + j + 1;
				meshData.mIndices32[k + 2] = (i + 1) * n + j;

				meshData.mIndices32[k + 3] = (i + 1) * n + j;
				meshData.mIndices32[k + 4] = i * n + j + 1;
				meshData.mIndices32[k + 5] = (i + 1) * n + j + 1;

				k += 6; // next quad
			}
		}

		return meshData;
	}


	MeshData<Vertex> CreateQuad(float x, float y, float w, float h, float depth)
	{
		MeshData<Vertex> meshData;

		meshData.mVertices.resize(4);
		meshData.mIndices32.resize(6);

		// Position coordinates specified in NDC space.
		meshData.mVertices[0] = Vertex(
			x, y - h, depth,
			0.0f, 0.0f, -1.0f,
			1.0f, 0.0f, 0.0f,
			0.0f, 1.0f);

		meshData.mVertices[1] = Vertex(
			x, y, depth,
			0.0f, 0.0f, -1.0f,
			1.0f, 0.0f, 0.0f,
			0.0f, 0.0f);

		meshData.mVertices[2] = Vertex(
			x + w, y, depth,
			0.0f, 0.0f, -1.0f,
			1.0f, 0.0f, 0.0f,
			1.0f, 0.0f);

		meshData.mVertices[3] = Vertex(
			x + w, y - h, depth,
			0.0f, 0.0f, -1.0f,
			1.0f, 0.0f, 0.0f,
			1.0f, 1.0f);

		meshData.mIndices32[0] = 0;
		meshData.mIndices32[1] = 1;
		meshData.mIndices32[2] = 2;

		meshData.mIndices32[3] = 0;
		meshData.mIndices32[4] = 2;
		meshData.mIndices32[5] = 3;

		return meshData;
	}

}