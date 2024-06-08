#pragma once

#include "PhysicsScene.hpp"

#include <Math/VectorMath.hpp>
#include <ResourceManager/Resource.hpp>
#include <Renderer/Geometry/Mesh.hpp>

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

namespace Darius::Job
{
	class CancellationToken;
}

namespace Darius::Physics
{
	struct ConvexMeshData;
	struct TriangleMeshData;

	enum class MeshType
	{
		ConvexMesh,
		TriangleMesh
	};

	struct BaseMeshData : public D_CORE::Counted
	{
		BaseMeshData(MeshType type, D_CORE::Uuid const& uuid) :
			Type(type),
			Uuid(uuid) {}

		BaseMeshData(BaseMeshData const& other) :
			Type(other.Type),
			Uuid(other.Uuid) {}

		D_CORE::Uuid const			Uuid;

		MeshType const				Type;

#if _D_EDITOR
		D_RENDERER_GEOMETRY::Mesh	Mesh;
#endif

		virtual bool		Release() override;
	};

	template<typename T, typename CHILD>
	struct MeshData : public BaseMeshData
	{
		MeshData(T* pxMesh, MeshType type, D_CORE::Uuid const& uuid) :
			BaseMeshData(type, uuid),
			PxMesh(pxMesh)
		{}

		MeshData(MeshData const& other) :
			BaseMeshData(other),
			PxMesh(other.PxMesh) {}

		T* const					PxMesh;
	};

	struct MeshDataHandle : public D_CORE::Ref<BaseMeshData>
	{
		MeshDataHandle(ConvexMeshData* meshData) :
			Ref<BaseMeshData>(reinterpret_cast<BaseMeshData*>(meshData)),
			ConvexMesh(meshData),
			Type(MeshType::ConvexMesh) {}

		MeshDataHandle(TriangleMeshData* meshData) :
			Ref<BaseMeshData>(reinterpret_cast<BaseMeshData*>(meshData)),
			TriangleMesh(meshData),
			Type(MeshType::TriangleMesh) {}

		MeshDataHandle() :
			Ref<BaseMeshData>(nullptr),
			ConvexMesh(nullptr),
			Type(MeshType::ConvexMesh) {}

		INLINE virtual bool		IsValid() const override { return _p != nullptr; }

		union
		{
			ConvexMeshData* 	ConvexMesh;
			TriangleMeshData*	TriangleMesh;
			void*				_p = nullptr;
		};

		MeshType				Type;
	};

	const MeshDataHandle InvalidMeshDataHandle;

	struct ConvexMeshData : public MeshData<physx::PxConvexMesh, ConvexMeshData>
	{
		ConvexMeshData(physx::PxConvexMesh* pxMesh, D_CORE::Uuid const& uuid) :
			MeshData(pxMesh, MeshType::ConvexMesh, uuid) {}
	};

	struct TriangleMeshData : public MeshData<physx::PxTriangleMesh, TriangleMeshData>
	{
		TriangleMeshData(physx::PxTriangleMesh* pxMesh, D_CORE::Uuid const& uuid) :
			MeshData(pxMesh, MeshType::TriangleMesh, uuid) {}
	};

	typedef std::function<void(MeshDataHandle handle)> MeshCreationCallback;

	void							Initialize(D_SERIALIZATION::Json const& settings);
	void							Shutdown();

#ifdef _D_EDITOR
	bool							OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options);
#endif

	void							Update(bool running, float dt);
	void							Flush();

	PhysicsScene* GetScene();
	physx::PxPhysics* GetCore();
	D_RESOURCE::ResourceHandle		GetDefaultMaterial();

	// Creates a convex mesh with association with the given uuid. It is best to have some kind
	// of relevance between the given uuid and the mesh the cooking is being performed on. Mesh
	// resource uuid is the best choice for it.
	NODISCARD MeshDataHandle		CreateConvexMesh(D_CORE::Uuid const& uuid, bool direct, physx::PxConvexMeshDesc const& desc);
	void							CreateConvexMeshAsync(D_CORE::Uuid const& uuid, bool direct, physx::PxConvexMeshDesc const& desc, MeshCreationCallback callback = nullptr, Darius::Job::CancellationToken* cancelleationToken = nullptr);
	NODISCARD MeshDataHandle		CreateTriangleMesh(D_CORE::Uuid const& uuid, bool direct, physx::PxTriangleMeshDesc const& desc,
		bool skipMeshCleanup = false, bool skipEdgeData = false, UINT numTrisPerLeaf = 4);
	void							CreateTriangleMeshAsync(D_CORE::Uuid const& uuid, bool direct, physx::PxTriangleMeshDesc const& desc, bool skipMeshCleanup = false, bool skipEdgeData = false, UINT numTrisPerLeaf = 4, MeshCreationCallback callback = nullptr, Darius::Job::CancellationToken* cancelleationToken = nullptr);

	NODISCARD MeshDataHandle FindConvexMesh(D_CORE::Uuid const& uuid);
	NODISCARD MeshDataHandle FindTriangleMesh(D_CORE::Uuid const& uuid);

#pragma region Math Converters
	INLINE physx::PxQuat	GetQuat(D_MATH::Quaternion const& quat)
	{
		return physx::PxQuat(quat.GetX(), quat.GetY(), quat.GetZ(), quat.GetW());
	}

	INLINE D_MATH::Quaternion GetQuat(physx::PxQuat const& quat)
	{
		return D_MATH::Quaternion(quat.x, quat.y, quat.z, quat.w);
	}

	INLINE physx::PxVec3	GetVec3(D_MATH::Vector3 const& vec3)
	{
		return physx::PxVec3(vec3.GetX(), vec3.GetY(), vec3.GetZ());
	}

	INLINE D_MATH::Vector3	GetVec3(physx::PxVec3 const& vec3)
	{
		return D_MATH::Vector3(vec3.x, vec3.y, vec3.z);
	}

	INLINE physx::PxVec4	GetVec4(D_MATH::Vector4 const& vec4)
	{
		return physx::PxVec4(vec4.GetX(), vec4.GetY(), vec4.GetZ(), vec4.GetW());
	}

	INLINE D_MATH::Vector4	GetVec4(physx::PxVec4 const& vec4)
	{
		return D_MATH::Vector4(vec4.x, vec4.y, vec4.z, vec4.w);
	}

	INLINE physx::PxTransform GetTransform(D_MATH::Transform const& trans)
	{
		return physx::PxTransform(GetVec3(trans.Translation), GetQuat(trans.Rotation));
	}

	INLINE D_MATH::Transform GetTransform(physx::PxTransform const& trans)
	{
		return D_MATH::Transform(GetVec3(trans.p), GetQuat(trans.q));
	}
#pragma endregion
}
