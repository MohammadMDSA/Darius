#pragma once

#include "BoundingBox.hpp"
#include "BoundingPlane.hpp"
#include "Math/Ray.hpp"

#include <Core/Containers/List.hpp>
#include <Core/Memory/Allocators/PagedAllocator.hpp>

#ifndef D_MATH_BOUNDS
#define D_MATH_BOUNDS Darius::Math::Bounds
#endif

namespace Darius::Math::Bounds
{
	// Based on bullet Dbvh

	/*
	Bullet Continuous Collision Detection and Physics Library
	Copyright (c) 2003-2013 Erwin Coumans  http://bulletphysics.org

	This software is provided 'as-is', without any express or implied warranty.
	In no event will the authors be held liable for any damages arising from the use of this software.
	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it freely,
	subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
	2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
	3. This notice may not be removed or altered from any source distribution.
	*/

	///DynamicBVH implementation by Nathanael Presson
	// The DynamicBVH class implements a fast dynamic bounding volume tree based on axis aligned bounding boxes (aabb tree).

	class DynamicBVH {
		struct Node;

	public:
		struct ID {
			Node* node = nullptr;

		public:
			INLINE bool IsValid() const { return node != nullptr; }
		};

	private:
		struct Volume {
			Aabb Aabb;

			INLINE Vector3 GetCenter() const { return Aabb.GetCenter(); }
			INLINE Vector3 GetLength() const { return Aabb.GetDimensions(); }

			INLINE bool Contains(const Volume& a) const {
				return Aabb.Contains(a.Aabb) == ContainmentType::Contains;
			}

			INLINE Volume Merge(const Volume& b) const {
				Volume r{ Aabb.Union(b.Aabb) };
				
				return r;
			}

			INLINE float GetSize() const {
				const Vector3 edges = GetLength();
				return edges.Sum() + edges.Mult();
			}

			INLINE bool IsNotEqualTo(const Volume& b) const {
				return !Aabb.Equals(b.Aabb);
			}

			INLINE float GetProximityTo(const Volume& b) const {
				const Vector3 d = Aabb.GetCenter() - b.GetCenter();
				return D_MATH::Abs(d).Sum() * 2;
			}

			INLINE int SelectByProximity(const Volume& a, const Volume& b) const {
				return (GetProximityTo(a) < GetProximityTo(b) ? 0 : 1);
			}

			//
			INLINE bool Intersects(const Volume& b) const {
				return Aabb.Intersects(b.Aabb);
			}

			INLINE bool IntersectsConvex(Plane const* planes, int planeCount, Vector3 const* points, int pointCount) const {
				Vector3 extents = Aabb.GetExtents();
				Vector3 center = GetCenter();

				using namespace DirectX;

				for (int i = 0; i < planeCount; i++) {
					const Plane& p = planes[i];
					Vector3 point = (D_MATH::Select(-extents, extents, p.GetNormal() > Vector3::Zero));
					point += GetCenter();
					if (p.IsPointOver(point)) {
						return false;
					}
				}

				// Make sure all points in the shape aren't fully separated from the AABB on
				// each axis.
				int badPointCountsPositive[3] = { 0 };
				int badPointCountsNegative[3] = { 0 };

				for (int k = 0; k < 3; k++) {
					for (int i = 0; i < pointCount; i++) {
						if (points[i]._GetFast(k) > center._GetFast(k) + extents._GetFast(k)) {
							badPointCountsPositive[k]++;
						}
						if (points[i]._GetFast(k) < center._GetFast(k) - extents._GetFast(k)) {
							badPointCountsNegative[k]++;
						}
					}

					if (badPointCountsNegative[k] == pointCount) {
						return false;
					}
					if (badPointCountsPositive[k] == pointCount) {
						return false;
					}
				}

				return true;
			}
		};

		struct Node {
			Volume Volume;
			Node* Parent = nullptr;
			union {
				Node* Children[2];
				void* Data;
			};

			INLINE bool IsLeaf() const { return Children[1] == nullptr; }
			INLINE bool IsInternal() const { return (!IsLeaf()); }

			INLINE int GetIndexInParent() const {
				if(!D_VERIFY(Parent))
					return 0;

				return (Parent->Children[1] == this) ? 1 : 0;
			}
			void GetMaxDepth(int depth, int& maxdepth) {
				if (IsInternal()) {
					Children[0]->GetMaxDepth(depth + 1, maxdepth);
					Children[1]->GetMaxDepth(depth + 1, maxdepth);
				}
				else {
					maxdepth = D_MATH::Max(maxdepth, depth);
				}
			}

			//
			int CountLeaves() const {
				if (IsInternal()) {
					return Children[0]->CountLeaves() + Children[1]->CountLeaves();
				}
				else {
					return (1);
				}
			}

			bool IsLeafOfAxis(const Vector3& org, const Vector3& axis) const {
				return D_MATH::Dot(axis, Volume.GetCenter() - org) <= 0.f;
			}

			Node() {
				Children[0] = nullptr;
				Children[1] = nullptr;
			}
		};

		D_MEMORY::PagedAllocator<Node> mNodeAllocator;
		// Fields
		Node* mBvhRoot = nullptr;
		int mLkhd = -1;
		int mTotalLeaves = 0;
		uint32_t mOpath = 0;
		uint32_t mIndex = 0;

		enum {
			ALLOCA_STACK_SIZE = 128
		};

		void DeleteNodeInternal(Node* node);
		void RecurseDeleteNodeInternal(Node* node);
		Node* CreateNodeInternal(Node* parent, void* data);
		DynamicBVH::Node* CreateNodeWithVolumeInternal(Node* parent, Volume const& volume, void* data);
		void InsertLeaf(Node* root, Node* leaf);
		Node* RemoveLeaf(Node* leaf);
		void FetchLeaves(Node* root, D_CONTAINERS::DVector<Node*>& leaves, int depth = -1);
		static int Split(Node** leaves, int count, Vector3 const& org, const Vector3& axis);
		static Volume Bounds(Node** leaves, int count);
		void BottomUp(Node** leaves, int count);
		Node* TopDown(Node** leaves, int count, int buThreshold);
		Node* NodeSort(Node* n, Node*& r);

		void UpdateInternal(Node* leaf, int lookahead = -1);

		void ExtractLeaves(Node* node, D_CONTAINERS::DList<ID>* elements);

		INLINE bool RayAabbInternal(Vector3 const& rayFrom, Vector3 const& rayInvDirection, const unsigned int raySign[3], const Vector3 bounds[2], float& tmin, float lambdaMin, float lambdaMax) {
			float tmax, tymin, tymax, tzmin, tzmax;
			tmin = (bounds[raySign[0]].GetX() - rayFrom.GetX()) * rayInvDirection.GetX();
			tmax = (bounds[1 - raySign[0]].GetX() - rayFrom.GetX()) * rayInvDirection.GetX();
			tymin = (bounds[raySign[1]].GetY() - rayFrom.GetY()) * rayInvDirection.GetY();
			tymax = (bounds[1 - raySign[1]].GetY() - rayFrom.GetY()) * rayInvDirection.GetY();

			if ((tmin > tymax) || (tymin > tmax)) {
				return false;
			}

			if (tymin > tmin) {
				tmin = tymin;
			}

			if (tymax < tmax) {
				tmax = tymax;
			}

			tzmin = (bounds[raySign[2]].GetZ() - rayFrom.GetZ()) * rayInvDirection.GetZ();
			tzmax = (bounds[1 - raySign[2]].GetZ() - rayFrom.GetZ()) * rayInvDirection.GetZ();

			if ((tmin > tzmax) || (tzmin > tmax)) {
				return false;
			}
			if (tzmin > tmin) {
				tmin = tzmin;
			}
			if (tzmax < tmax) {
				tmax = tzmax;
			}
			return ((tmin < lambdaMax) && (tmax > lambdaMin));
		}

	public:
		// Methods
		void Clear();
		bool IsEmpty() const { return (nullptr == mBvhRoot); }
		void OptimizeBottomUp();
		void OptimizeTopDown(int buThreshold = 128);
		void OptimizeIncremental(int passes);
		ID Insert(Aabb const& box, void* userdata);
		bool Update(ID const& id, Aabb const& box);
		void Remove(ID const& id);
		void GetElements(D_CONTAINERS::DList<ID>* elements);

		int GetLeafCount() const;
		int GetMaxDepth() const;

		/* Discouraged, but works as a reference on how it must be used */
		struct DefaultQueryResult {
			virtual bool operator()(void* data) = 0; //return true whether you want to continue the query
			virtual ~DefaultQueryResult() {}
		};

		template <class QueryResult>
		INLINE void AabbQuery(Aabb const& aabb, QueryResult& r_result);
		template <class QueryResult>
		INLINE void ConvexQuery(Plane const* planes, int planeCount, Vector3 const* points, int pointCount, QueryResult& result);
		template <class QueryResult>
		INLINE void RayQuery(Vector3 const& from, Vector3 const& to, QueryResult& result);

		void SetIndex(uint32_t index);
		uint32_t GetIndex() const;

		~DynamicBVH();
	};

	template <class QueryResult>
	void DynamicBVH::AabbQuery(Aabb const& box, QueryResult& result) {
		if (!mBvhRoot) {
			return;
		}

		Volume volume;
		volume.Aabb = box;

		const Node** stack = (const Node**)alloca(ALLOCA_STACK_SIZE * sizeof(const Node*));
		stack[0] = mBvhRoot;
		int32_t depth = 1;
		int32_t threshold = ALLOCA_STACK_SIZE - 2u;

		D_CONTAINERS::DVector<const Node*> auxStack; //only used in rare occasions when you run out of alloca memory because tree is too unbalanced. Should correct itself over time.

		do {
			depth--;
			const Node* n = stack[depth];
			if (n->Volume.Intersects(volume)) {
				if (n->IsInternal()) {
					if (depth > threshold) {
						if (auxStack.empty()) {
							auxStack.resize(ALLOCA_STACK_SIZE * 2);
							memcpy(auxStack.data(), stack, ALLOCA_STACK_SIZE * sizeof(const Node*));
						}
						else {
							auxStack.resize(auxStack.size() * 2);
						}
						stack = auxStack.data();
						threshold = (UINT)auxStack.size() - 2;
					}
					stack[depth++] = n->Children[0];
					stack[depth++] = n->Children[1];
				}
				else {
					if (result(n->Data)) {
						return;
					}
				}
			}
		} while (depth > 0);
	}

	template <class QueryResult>
	void DynamicBVH::ConvexQuery(Plane const* planes, int planeCount, Vector3 const* points, int pointCount, QueryResult& result) {
		if (!mBvhRoot) {
			return;
		}

		//generate a volume anyway to improve pre-testing
		Volume volume;
		for (int i = 0; i < pointCount; i++) {
			if (i == 0) {
				volume.Aabb = Aabb(points[0]);
			}
			else {
				volume.Aabb.AddPoint(points[i]);
			}
		}

		Node const** stack = (Node const**)alloca(ALLOCA_STACK_SIZE * sizeof(Node const*));
		stack[0] = mBvhRoot;
		int32_t depth = 1;
		int32_t threshold = ALLOCA_STACK_SIZE - 2;

		D_CONTAINERS::DVector<Node const*> auxStack; //only used in rare occasions when you run out of alloca memory because tree is too unbalanced. Should correct itself over time.

		do {
			depth--;
			const Node* n = stack[depth];
			if (n->Volume.Intersects(volume) && n->Volume.IntersectsConvex(planes, planeCount, points, pointCount)) {
				if (n->IsInternal()) {
					if (depth > threshold) {
						if (auxStack.empty()) {
							auxStack.resize(ALLOCA_STACK_SIZE * 2);
							memcpy(auxStack.data(), stack, ALLOCA_STACK_SIZE * sizeof(const Node*));
						}
						else {
							auxStack.resize(auxStack.size() * 2);
						}
						stack = auxStack.data();
						threshold = (UINT)auxStack.size() - 2;
					}
					stack[depth++] = n->Children[0];
					stack[depth++] = n->Children[1];
				}
				else {
					if (result(n->Data)) {
						return;
					}
				}
			}
		} while (depth > 0);
	}
	template <class QueryResult>
	void DynamicBVH::RayQuery(Vector3 const& from, Vector3 const& to, QueryResult& result) {
		if (!mBvhRoot) {
			return;
		}

		Vector3 rayDir = (to - from);
		rayDir.Normalize();

		///what about division by zero? --> just set rayDirection[i] to INF/B3_LARGE_FLOAT
		Vector3 invDir = Select(Vector3(1e20), 1.f / rayDir, rayDir == Vector3::Zero);
		unsigned int signs[3] = { invDir.GetX() < 0.f, invDir.GetY() < 0.f, invDir.GetZ() < 0.f };

		float lambdaMax = Dot(rayDir, to - from);

		Vector3 bounds[2];

		const Node** stack = (const Node**)alloca(ALLOCA_STACK_SIZE * sizeof(const Node*));
		stack[0] = mBvhRoot;
		int32_t depth = 1;
		int32_t threshold = ALLOCA_STACK_SIZE - 2;

		D_CONTAINERS::DVector<const Node*> auxStack; //only used in rare occasions when you run out of alloca memory because tree is too unbalanced. Should correct itself over time.

		do {
			depth--;
			Node const* node = stack[depth];
			bounds[0] = node->Volume.Aabb.GetMin();
			bounds[1] = node->Volume.Aabb.GetMax();
			float tmin = 1.f, lambdaMin = 0.f;
			unsigned int result1 = false;
			result1 = RayAabbInternal(from, invDir, signs, bounds, tmin, lambdaMin, lambdaMax);
			if (result1) {
				if (node->IsInternal()) {
					if (depth > threshold) {
						if (auxStack.empty()) {
							auxStack.resize(ALLOCA_STACK_SIZE * 2);
							memcpy(auxStack.data(), stack, ALLOCA_STACK_SIZE * sizeof(const Node*));
						}
						else {
							auxStack.resize(auxStack.size() * 2);
						}
						stack = auxStack.data();
						threshold = (UINT)auxStack.size() - 2;
					}
					stack[depth++] = node->Children[0];
					stack[depth++] = node->Children[1];
				}
				else {
					if (result(node->Data)) {
						return;
					}
				}
			}
		} while (depth > 0);
	}
}
