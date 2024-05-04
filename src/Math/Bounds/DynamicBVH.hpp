#pragma once

#include "BoundingBox.hpp"
#include "BoundingPlane.hpp"
#include "Math/Ray.hpp"
#include "Math/Camera/Frustum.hpp"

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

	template<typename T = void*>
	class DynamicBVH
	{
		struct Node;

		// Returns whether it should continue the query
		typedef std::function<bool(T const&, Aabb const&)> QueryResult;

	public:
		struct ID
		{
			Node* node = nullptr;

		public:
			INLINE bool IsValid() const { return node != nullptr; }
		};

	private:
		struct Volume
		{
			Aabb Aabb;

			INLINE Vector3 GetCenter() const { return Aabb.GetCenter(); }
			INLINE Vector3 GetLength() const { return Aabb.GetDimensions(); }

			INLINE bool Contains(const Volume& a) const
			{
				return Aabb.Contains(a.Aabb) == ContainmentType::Contains;
			}

			INLINE Volume Merge(const Volume& b) const
			{
				Volume r {Aabb.Union(b.Aabb)};

				return r;
			}

			INLINE float GetSize() const
			{
				const Vector3 edges = GetLength();
				return edges.Sum() + edges.Mult();
			}

			INLINE bool IsNotEqualTo(const Volume& b) const
			{
				return !Aabb.Equals(b.Aabb);
			}

			INLINE float GetProximityTo(const Volume& b) const
			{
				const Vector3 d = Aabb.GetCenter() - b.GetCenter();
				return D_MATH::Abs(d).Sum() * 2;
			}

			INLINE int SelectByProximity(const Volume& a, const Volume& b) const
			{
				return (GetProximityTo(a) < GetProximityTo(b) ? 0 : 1);
			}

			//
			INLINE bool Intersects(const Volume& b) const
			{
				return Aabb.Intersects(b.Aabb);
			}

			INLINE bool IntersectsConvex(Plane const* planes, int planeCount, Vector3 const* points, int pointCount) const
			{
				Vector3 extents = Aabb.GetExtents();
				Vector3 center = GetCenter();

				using namespace DirectX;

				for(int i = 0; i < planeCount; i++)
				{
					const Plane& p = planes[i];
					Vector3 point = (D_MATH::Select(-extents, extents, p.GetNormal() > Vector3::Zero));
					point += GetCenter();
					if(p.IsPointOver(point))
					{
						return false;
					}
				}

				// Make sure all points in the shape aren't fully separated from the AABB on
				// each axis.
				int badPointCountsPositive[3] = {0};
				int badPointCountsNegative[3] = {0};

				for(int k = 0; k < 3; k++)
				{
					for(int i = 0; i < pointCount; i++)
					{
						if(points[i]._GetFast(k) > center._GetFast(k) + extents._GetFast(k))
						{
							badPointCountsPositive[k]++;
						}
						if(points[i]._GetFast(k) < center._GetFast(k) - extents._GetFast(k))
						{
							badPointCountsNegative[k]++;
						}
					}

					if(badPointCountsNegative[k] == pointCount)
					{
						return false;
					}
					if(badPointCountsPositive[k] == pointCount)
					{
						return false;
					}
				}

				return true;
			}
		};

		struct Node
		{
			Volume Volume;
			Node* Parent = nullptr;
			union
			{
				T		Data;
				Node* Child0;
			};
			Node* Child1;

			INLINE Node*& GetChild(int index /*0 or 1*/)
			{
				if(index == 0)
					return Child0;
				return Child1;
			}

			INLINE bool IsLeaf() const { return Child1 == nullptr; }
			INLINE bool IsInternal() const { return (!IsLeaf()); }

			INLINE int GetIndexInParent() const
			{
				if(!D_VERIFY(Parent))
					return 0;

				return (Parent->Child1 == this) ? 1 : 0;
			}

			void GetMaxDepth(int depth, int& maxdepth)
			{
				if(IsInternal())
				{
					Child0->GetMaxDepth(depth + 1, maxdepth);
					Child1->GetMaxDepth(depth + 1, maxdepth);
				}
				else
				{
					maxdepth = D_MATH::Max(maxdepth, depth);
				}
			}

			int CountLeaves() const
			{
				if(IsInternal())
				{
					return Child0->CountLeaves() + Child1->CountLeaves();
				}
				else
				{
					return (1);
				}
			}

			bool IsLeafOfAxis(const Vector3& org, const Vector3& axis) const
			{
				return D_MATH::Dot(axis, Volume.GetCenter() - org) <= 0.f;
			}

			Node()
			{
				Child0 = nullptr;
				Child1 = nullptr;
			}
		};

		D_MEMORY::PagedAllocator<Node> mNodeAllocator;
		// Fields
		Node* mBvhRoot = nullptr;
		int mLkhd = -1;
		int mTotalLeaves = 0;
		uint32_t mOpath = 0;
		uint32_t mIndex = 0;

		enum
		{
			ALLOCA_STACK_SIZE = 128
		};

		void DeleteNodeInternal(Node* node)
		{
			mNodeAllocator.Free(node);
		}

		void RecurseDeleteNodeInternal(Node* node)
		{
			if(!node->IsLeaf())
			{
				RecurseDeleteNodeInternal(node->Child0);
				RecurseDeleteNodeInternal(node->Child1);
			}
			if(node == mBvhRoot)
			{
				mBvhRoot = nullptr;
			}
			DeleteNodeInternal(node);
		}

		Node* CreateNodeInternal(Node* parent, T const& data)
		{
			Node* node = mNodeAllocator.Alloc();
			node->Parent = parent;
			node->Data = data;
			return node;
		}

		DynamicBVH::Node* CreateNodeWithVolumeInternal(Node* parent, Volume const& volume, T const& data)
		{
			Node* node = CreateNodeInternal(parent, data);
			node->Volume = volume;
			return node;
		}

		void InsertLeaf(Node* root, Node* leaf)
		{
			if(!mBvhRoot)
			{
				mBvhRoot = leaf;
				leaf->Parent = nullptr;
			}
			else
			{
				if(!root->IsLeaf())
				{
					do
					{
						root = root->GetChild(leaf->Volume.SelectByProximity(
							root->Child0->Volume,
							root->Child1->Volume));
					} while(!root->IsLeaf());
				}
				Node* prev = root->Parent;
				Node* node = CreateNodeWithVolumeInternal(prev, leaf->Volume.Merge(root->Volume), {});
				if(prev)
				{
					prev->GetChild(root->GetIndexInParent()) = node;
					node->Child0 = root;
					root->Parent = node;
					node->Child1 = leaf;
					leaf->Parent = node;
					do
					{
						if(!prev->Volume.Contains(node->Volume))
						{
							prev->Volume = prev->Child0->Volume.Merge(prev->Child1->Volume);
						}
						else
						{
							break;
						}
						node = prev;
					} while(nullptr != (prev = node->Parent));
				}
				else
				{
					node->Child0 = root;
					root->Parent = node;
					node->Child1 = leaf;
					leaf->Parent = node;
					mBvhRoot = node;
				}
			}
		}

		Node* RemoveLeaf(Node* leaf)
		{
			if(leaf == mBvhRoot)
			{
				mBvhRoot = nullptr;
				return nullptr;
			}
			else
			{
				Node* parent = leaf->Parent;
				Node* prev = parent->Parent;
				Node* sibling = parent->GetChild(1 - leaf->GetIndexInParent());
				if(prev)
				{
					prev->GetChild(parent->GetIndexInParent()) = sibling;
					sibling->Parent = prev;
					DeleteNodeInternal(parent);
					while(prev)
					{
						const Volume pb = prev->Volume;
						prev->Volume = prev->Child0->Volume.Merge(prev->Child1->Volume);
						if(pb.IsNotEqualTo(prev->Volume))
						{
							prev = prev->Parent;
						}
						else
						{
							break;
						}
					}
					return (prev ? prev : mBvhRoot);
				}
				else
				{
					mBvhRoot = sibling;
					sibling->Parent = nullptr;
					DeleteNodeInternal(parent);
					return (mBvhRoot);
				}
			}
		}

		void FetchLeaves(Node* root, D_CONTAINERS::DVector<Node*>& leaves, int depth = -1)
		{
			if(root->IsInternal() && depth)
			{
				FetchLeaves(root->Child0, leaves, depth - 1);
				FetchLeaves(root->Child1, leaves, depth - 1);
				DeleteNodeInternal(root);
			}
			else
			{
				leaves.push_back(root);
			}
		}

		void BottomUp(Node** leaves, int count)
		{
			while(count > 1)
			{
				float minsize = INFINITY;
				int minidx[2] = {-1, -1};
				for(int i = 0; i < count; ++i)
				{
					for(int j = i + 1; j < count; ++j)
					{
						const float sz = leaves[i]->Volume.Merge(leaves[j]->Volume).GetSize();
						if(sz < minsize)
						{
							minsize = sz;
							minidx[0] = i;
							minidx[1] = j;
						}
					}
				}
				Node* n[] = {leaves[minidx[0]], leaves[minidx[1]]};
				Node* p = CreateNodeWithVolumeInternal(nullptr, n[0]->Volume.Merge(n[1]->Volume), {});
				p->Child0 = n[0];
				p->Child1 = n[1];
				n[0]->Parent = p;
				n[1]->Parent = p;
				leaves[minidx[0]] = p;
				leaves[minidx[1]] = leaves[count - 1];
				--count;
			}
		}

		Node* TopDown(Node** leaves, int count, int buThreshold)
		{
			static const Vector3 axis[] = {Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1)};

			if(buThreshold <= 1)
				return nullptr;

			if(count > 1)
			{
				if(count > buThreshold)
				{
					const Volume vol = Bounds(leaves, count);
					const Vector3 org = vol.GetCenter();
					int partition;
					int bestaxis = -1;
					int bestmidp = count;
					int splitcount[3][2] = {{ 0, 0 }, { 0, 0 }, { 0, 0 }};
					int i;
					for(i = 0; i < count; ++i)
					{
						const Vector3 x = leaves[i]->Volume.GetCenter() - org;
						for(int j = 0; j < 3; ++j)
						{
							++splitcount[j][Dot(x, axis[j]) > 0.f ? 1 : 0];
						}
					}
					for(i = 0; i < 3; ++i)
					{
						if((splitcount[i][0] > 0) && (splitcount[i][1] > 0))
						{
							const int midp = (int)Abs((float)(splitcount[i][0] - splitcount[i][1]));
							if(midp < bestmidp)
							{
								bestaxis = i;
								bestmidp = midp;
							}
						}
					}
					if(bestaxis >= 0)
					{
						partition = Split(leaves, count, org, axis[bestaxis]);
						if(partition == 0 || partition == count)
							return nullptr;
					}
					else
					{
						partition = count / 2 + 1;
					}

					Node* node = CreateNodeWithVolumeInternal(nullptr, vol, {});
					node->Child0 = TopDown(&leaves[0], partition, buThreshold);
					node->Child1 = TopDown(&leaves[partition], count - partition, buThreshold);
					node->Child0->Parent = node;
					node->Child1->Parent = node;
					return (node);
				}
				else
				{
					BottomUp(leaves, count);
					return (leaves[0]);
				}
			}
			return (leaves[0]);
		}

		Node* NodeSort(Node* n, Node*& r)
		{
			Node* p = n->Parent;
			if(!n->IsInternal())
				return nullptr;

			if(p > n)
			{
				const int i = n->GetIndexInParent();
				const int j = 1 - i;
				Node* s = p->GetChild(j);
				Node* q = p->Parent;
				if(n != p->GetChild(i))
					return nullptr;

				if(q)
				{
					q->GetChild(p->GetIndexInParent()) = n;
				}
				else
				{
					r = n;
				}
				s->Parent = n;
				p->Parent = n;
				n->Parent = q;
				p->Child0 = n->Child0;
				p->Child1 = n->Child1;
				n->Child0->Parent = p;
				n->Child1->Parent = p;
				n->GetChild(i) = p;
				n->GetChild(j) = s;

				DynamicBVH::Volume aux = p->Volume;
				p->Volume = n->Volume;
				n->Volume = aux;
				return (p);
			}
			return (n);
		}

		void UpdateInternal(Node* leaf, int lookahead = -1)
		{
			Node* root = RemoveLeaf(leaf);
			if(root)
			{
				if(lookahead >= 0)
				{
					for(int i = 0; (i < lookahead) && root->Parent; ++i)
					{
						root = root->Parent;
					}
				}
				else
				{
					root = mBvhRoot;
				}
			}
			InsertLeaf(root, leaf);
		}

		void ExtractLeaves(Node* node, D_CONTAINERS::DList<ID>* elements)
		{
			if(node->IsInternal())
			{
				ExtractLeaves(node->Child0, elements);
				ExtractLeaves(node->Child1, elements);
			}
			else
			{
				ID id;
				id.node = node;
				elements->push_back(id);
			}
		}

		INLINE bool RayAabbInternal(Vector3 const& rayFrom, Vector3 const& rayInvDirection, const unsigned int raySign[3], const Vector3 bounds[2], float& tmin, float lambdaMin, float lambdaMax) const
		{
			float tmax, tymin, tymax, tzmin, tzmax;
			tmin = (bounds[raySign[0]].GetX() - rayFrom.GetX()) * rayInvDirection.GetX();
			tmax = (bounds[1 - raySign[0]].GetX() - rayFrom.GetX()) * rayInvDirection.GetX();
			tymin = (bounds[raySign[1]].GetY() - rayFrom.GetY()) * rayInvDirection.GetY();
			tymax = (bounds[1 - raySign[1]].GetY() - rayFrom.GetY()) * rayInvDirection.GetY();

			if((tmin > tymax) || (tymin > tmax))
			{
				return false;
			}

			if(tymin > tmin)
			{
				tmin = tymin;
			}

			if(tymax < tmax)
			{
				tmax = tymax;
			}

			tzmin = (bounds[raySign[2]].GetZ() - rayFrom.GetZ()) * rayInvDirection.GetZ();
			tzmax = (bounds[1 - raySign[2]].GetZ() - rayFrom.GetZ()) * rayInvDirection.GetZ();

			if((tmin > tzmax) || (tzmin > tmax))
			{
				return false;
			}
			if(tzmin > tmin)
			{
				tmin = tzmin;
			}
			if(tzmax < tmax)
			{
				tmax = tzmax;
			}
			return ((tmin < lambdaMax) && (tmax > lambdaMin));
		}

		// Partitions leaves such that leaves[0, n) are on the
		// left of axis, and leaves[n, count) are on the right
		// of axis. returns N.
		static int Split(Node** leaves, int count, Vector3 const& org, const Vector3& axis)
		{
			int begin = 0;
			int end = count;
			for(;;)
			{
				while(begin != end && leaves[begin]->IsLeafOfAxis(org, axis))
				{
					++begin;
				}

				if(begin == end)
				{
					break;
				}

				while(begin != end && !leaves[end - 1]->IsLeafOfAxis(org, axis))
				{
					--end;
				}

				if(begin == end)
				{
					break;
				}

				// swap out of place nodes
				--end;
				Node* temp = leaves[begin];
				leaves[begin] = leaves[end];
				leaves[end] = temp;
				++begin;
			}

			return begin;
		}

		static Volume Bounds(Node** leaves, int count)
		{
			Volume volume = leaves[0]->Volume;
			for(int i = 1, ni = count; i < ni; ++i)
			{
				volume = volume.Merge(leaves[i]->Volume);
			}
			return (volume);
		}

	public:
		// Methods
		void Clear()
		{
			if(mBvhRoot)
			{
				RecurseDeleteNodeInternal(mBvhRoot);
			}
			mLkhd = -1;
			mOpath = 0;
		}

		bool IsEmpty() const { return (nullptr == mBvhRoot); }
		void OptimizeBottomUp()
		{
			if(mBvhRoot)
			{
				D_CONTAINERS::DVector<Node*> leaves;
				FetchLeaves(mBvhRoot, leaves);
				BottomUp(&leaves[0], (int)leaves.size());
				mBvhRoot = leaves[0];
			}
		}

		void OptimizeTopDown(int buThreshold = 128)
		{
			if(mBvhRoot)
			{
				D_CONTAINERS::DVector<Node*> leaves;
				FetchLeaves(mBvhRoot, leaves);
				mBvhRoot = TopDown(&leaves[0], (int)leaves.size(), buThreshold);
			}
		}

		void OptimizeIncremental(int passes)
		{
			if(passes < 0)
			{
				passes = mTotalLeaves;
			}
			if(passes > 0)
			{
				do
				{
					if(!mBvhRoot)
					{
						break;
					}
					Node* node = mBvhRoot;
					unsigned bit = 0;
					while(node->IsInternal())
					{
						node = NodeSort(node, mBvhRoot)->GetChild((mOpath >> bit) & 1);
						bit = (bit + 1) & (sizeof(unsigned) * 8 - 1);
					}
					UpdateInternal(node);
					++mOpath;
				} while(--passes);
			}
		}

		ID Insert(Aabb const& box, T const& userdata)
		{
			Volume volume {box};

			Node* leaf = CreateNodeWithVolumeInternal(nullptr, volume, userdata);
			InsertLeaf(mBvhRoot, leaf);
			++mTotalLeaves;

			ID id;
			id.node = leaf;

			return id;
		}

		bool Update(ID const& id, Aabb const& box)
		{
			if(!id.IsValid())
				return false;

			Node* leaf = id.node;

			Volume volume {box};


			if(leaf->Volume.Aabb.NearEquals(box, 0.00001f))
			{
				// noop
				return false;
			}

			Node* base = RemoveLeaf(leaf);
			if(base)
			{
				if(mLkhd >= 0)
				{
					for(int i = 0; (i < mLkhd) && base->Parent; ++i)
					{
						base = base->Parent;
					}
				}
				else
				{
					base = mBvhRoot;
				}
			}
			leaf->Volume = volume;
			InsertLeaf(base, leaf);
			return true;
		}

		void Remove(ID const& id)
		{
			if(!id.IsValid())
				return;

			Node* leaf = id.node;
			RemoveLeaf(leaf);
			DeleteNodeInternal(leaf);
			--mTotalLeaves;
		}

		void GetElements(D_CONTAINERS::DList<ID>* elements)
		{
			if(mBvhRoot)
			{
				ExtractLeaves(mBvhRoot, elements);
			}
		}

		int GetLeafCount() const { return mTotalLeaves; }
		int GetMaxDepth() const
		{
			if(mBvhRoot)
			{
				int depth = 1;
				int max_depth = 0;
				mBvhRoot->GetMaxDepth(depth, max_depth);
				return max_depth;
			}
			else
			{
				return 0;
			}
		}

		INLINE void FrustumQuery(D_MATH_CAMERA::Frustum const& frustum, QueryResult result) const;
		INLINE void AabbQuery(Aabb const& aabb, QueryResult result) const;
		INLINE void ConvexQuery(Plane const* planes, int planeCount, Vector3 const* points, int pointCount, QueryResult result) const;
		INLINE void RayQuery(Vector3 const& from, Vector3 const& to, QueryResult result) const;

		void SetIndex(uint32_t index)
		{
			if(mBvhRoot != nullptr)
				return;

			mIndex = index;
		}

		uint32_t GetIndex() const { return mIndex; }

		~DynamicBVH()
		{
			Clear();
		}
	};

	template <typename T>
	void DynamicBVH<T>::AabbQuery(Aabb const& box, QueryResult result) const
	{
		if(!mBvhRoot)
		{
			return;
		}

		Volume volume;
		volume.Aabb = box;

		const Node** stack = (const Node**)alloca(ALLOCA_STACK_SIZE * sizeof(const Node*));
		stack[0] = mBvhRoot;
		int32_t depth = 1;
		int32_t threshold = ALLOCA_STACK_SIZE - 2u;

		D_CONTAINERS::DVector<const Node*> auxStack; //only used in rare occasions when you run out of alloca memory because tree is too unbalanced. Should correct itself over time.

		do
		{
			depth--;
			const Node* n = stack[depth];
			if(n->Volume.Intersects(volume))
			{
				if(n->IsInternal())
				{
					if(depth > threshold)
					{
						if(auxStack.empty())
						{
							auxStack.resize(ALLOCA_STACK_SIZE * 2);
							memcpy(auxStack.data(), stack, ALLOCA_STACK_SIZE * sizeof(const Node*));
						}
						else
						{
							auxStack.resize(auxStack.size() * 2);
						}
						stack = auxStack.data();
						threshold = (UINT)auxStack.size() - 2;
					}
					stack[depth++] = n->Child0;
					stack[depth++] = n->Child1;
				}
				else
				{
					if(!result(n->Data, n->Volume.Aabb))
					{
						return;
					}
				}
			}
		} while(depth > 0);
	}

	template <typename T>
	void DynamicBVH<T>::FrustumQuery(D_MATH_CAMERA::Frustum const& frustum, QueryResult result) const
	{
		if(!mBvhRoot)
		{
			return;
		}

		//generate a volume anyway to improve pre-testing
		Volume volume;
		for(int i = 0; i < (int)D_MATH_CAMERA::Frustum::_kNumCorners; i++)
		{
			if(i == 0)
			{
				volume.Aabb = Aabb(frustum.GetFrustumCorner((D_MATH_CAMERA::Frustum::CornerID)0));
			}
			else
			{
				volume.Aabb.AddPoint(frustum.GetFrustumCorner((D_MATH_CAMERA::Frustum::CornerID)i));
			}
		}

		Node const** stack = (Node const**)alloca(ALLOCA_STACK_SIZE * sizeof(Node const*));
		stack[0] = mBvhRoot;
		int32_t depth = 1;
		int32_t threshold = ALLOCA_STACK_SIZE - 2;

		D_CONTAINERS::DVector<Node const*> auxStack; //only used in rare occasions when you run out of alloca memory because tree is too unbalanced. Should correct itself over time.

		do
		{
			depth--;
			const Node* n = stack[depth];
			if(n->Volume.Intersects(volume) && frustum.Intersects(n->Volume.Aabb))
			{
				if(n->IsInternal())
				{
					if(depth > threshold)
					{
						if(auxStack.empty())
						{
							auxStack.resize(ALLOCA_STACK_SIZE * 2);
							memcpy(auxStack.data(), stack, ALLOCA_STACK_SIZE * sizeof(const Node*));
						}
						else
						{
							auxStack.resize(auxStack.size() * 2);
						}
						stack = auxStack.data();
						threshold = (UINT)auxStack.size() - 2;
					}
					stack[depth++] = n->Child0;
					stack[depth++] = n->Child1;
				}
				else
				{
					if(!result(n->Data, n->Volume.Aabb))
					{
						return;
					}
				}
			}
		} while(depth > 0);
	}

	template <typename T>
	void DynamicBVH<T>::ConvexQuery(Plane const* planes, int planeCount, Vector3 const* points, int pointCount, QueryResult result) const
	{
		if(!mBvhRoot)
		{
			return;
		}

		//generate a volume anyway to improve pre-testing
		Volume volume;
		for(int i = 0; i < pointCount; i++)
		{
			if(i == 0)
			{
				volume.Aabb = Aabb(points[0]);
			}
			else
			{
				volume.Aabb.AddPoint(points[i]);
			}
		}

		Node const** stack = (Node const**)alloca(ALLOCA_STACK_SIZE * sizeof(Node const*));
		stack[0] = mBvhRoot;
		int32_t depth = 1;
		int32_t threshold = ALLOCA_STACK_SIZE - 2;

		D_CONTAINERS::DVector<Node const*> auxStack; //only used in rare occasions when you run out of alloca memory because tree is too unbalanced. Should correct itself over time.

		do
		{
			depth--;
			const Node* n = stack[depth];
			if(n->Volume.Intersects(volume) && n->Volume.IntersectsConvex(planes, planeCount, points, pointCount))
			{
				if(n->IsInternal())
				{
					if(depth > threshold)
					{
						if(auxStack.empty())
						{
							auxStack.resize(ALLOCA_STACK_SIZE * 2);
							memcpy(auxStack.data(), stack, ALLOCA_STACK_SIZE * sizeof(const Node*));
						}
						else
						{
							auxStack.resize(auxStack.size() * 2);
						}
						stack = auxStack.data();
						threshold = (UINT)auxStack.size() - 2;
					}
					stack[depth++] = n->Child0;
					stack[depth++] = n->Child1;
				}
				else
				{
					if(!result(n->Data, n->Volume.Aabb))
					{
						return;
					}
				}
			}
		} while(depth > 0);
	}

	template <typename T>
	void DynamicBVH<T>::RayQuery(Vector3 const& from, Vector3 const& to, QueryResult result) const
	{
		if(!mBvhRoot)
		{
			return;
		}

		Vector3 rayDir = (to - from);
		rayDir.Normalize();

		///what about division by zero? --> just set rayDirection[i] to INF/B3_LARGE_FLOAT
		Vector3 invDir = Select(Vector3(1e20f), 1.f / rayDir, rayDir == Vector3::Zero);
		unsigned int signs[3] = {invDir.GetX() < 0.f, invDir.GetY() < 0.f, invDir.GetZ() < 0.f};

		float lambdaMax = Dot(rayDir, to - from);

		Vector3 bounds[2];

		const Node** stack = (const Node**)alloca(ALLOCA_STACK_SIZE * sizeof(const Node*));
		stack[0] = mBvhRoot;
		int32_t depth = 1;
		int32_t threshold = ALLOCA_STACK_SIZE - 2;

		D_CONTAINERS::DVector<const Node*> auxStack; //only used in rare occasions when you run out of alloca memory because tree is too unbalanced. Should correct itself over time.

		do
		{
			depth--;
			Node const* node = stack[depth];
			bounds[0] = node->Volume.Aabb.GetMin();
			bounds[1] = node->Volume.Aabb.GetMax();
			float tmin = 1.f, lambdaMin = 0.f;
			unsigned int result1 = false;
			result1 = RayAabbInternal(from, invDir, signs, bounds, tmin, lambdaMin, lambdaMax);
			if(result1)
			{
				if(node->IsInternal())
				{
					if(depth > threshold)
					{
						if(auxStack.empty())
						{
							auxStack.resize(ALLOCA_STACK_SIZE * 2);
							memcpy(auxStack.data(), stack, ALLOCA_STACK_SIZE * sizeof(const Node*));
						}
						else
						{
							auxStack.resize(auxStack.size() * 2);
						}
						stack = auxStack.data();
						threshold = (UINT)auxStack.size() - 2;
					}
					stack[depth++] = node->Child0;
					stack[depth++] = node->Child1;
				}
				else
				{
					if(!result(node->Data, node->Volume.Aabb))
					{
						return;
					}
				}
			}
		} while(depth > 0);
	}
}
