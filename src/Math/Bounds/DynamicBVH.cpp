#include "Math/pch.hpp"
#include "DynamicBVH.hpp"

using namespace D_CONTAINERS;

/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */

namespace Darius::Math::Bounds
{

	void DynamicBVH::DeleteNodeInternal(Node* node) {
		mNodeAllocator.Free(node);
	}

	void DynamicBVH::RecurseDeleteNodeInternal(Node* node) {
		if (!node->IsLeaf()) {
			RecurseDeleteNodeInternal(node->Children[0]);
			RecurseDeleteNodeInternal(node->Children[1]);
		}
		if (node == mBvhRoot) {
			mBvhRoot = nullptr;
		}
		DeleteNodeInternal(node);
	}

	DynamicBVH::Node* DynamicBVH::CreateNodeInternal(Node* parent, void* data) {
		Node* node = mNodeAllocator.Alloc();
		node->Parent = parent;
		node->Data = data;
		return node;
	}

	DynamicBVH::Node* DynamicBVH::CreateNodeWithVolumeInternal(Node* parent, Volume const& volume, void* data) {
		Node* node = CreateNodeInternal(parent, data);
		node->Volume = volume;
		return node;
	}

	void DynamicBVH::InsertLeaf(Node* root, Node* leaf) {
		if (!mBvhRoot) {
			mBvhRoot = leaf;
			leaf->Parent = nullptr;
		}
		else {
			if (!root->IsLeaf()) {
				do {
					root = root->Children[leaf->Volume.SelectByProximity(
						root->Children[0]->Volume,
						root->Children[1]->Volume)];
				} while (!root->IsLeaf());
			}
			Node* prev = root->Parent;
			Node* node = CreateNodeWithVolumeInternal(prev, leaf->Volume.Merge(root->Volume), nullptr);
			if (prev) {
				prev->Children[root->GetIndexInParent()] = node;
				node->Children[0] = root;
				root->Parent = node;
				node->Children[1] = leaf;
				leaf->Parent = node;
				do {
					if (!prev->Volume.Contains(node->Volume)) {
						prev->Volume = prev->Children[0]->Volume.Merge(prev->Children[1]->Volume);
					}
					else {
						break;
					}
					node = prev;
				} while (nullptr != (prev = node->Parent));
			}
			else {
				node->Children[0] = root;
				root->Parent = node;
				node->Children[1] = leaf;
				leaf->Parent = node;
				mBvhRoot = node;
			}
		}
	}

	DynamicBVH::Node* DynamicBVH::RemoveLeaf(Node* leaf) {
		if (leaf == mBvhRoot) {
			mBvhRoot = nullptr;
			return (nullptr);
		}
		else {
			Node* parent = leaf->Parent;
			Node* prev = parent->Parent;
			Node* sibling = parent->Children[1 - leaf->GetIndexInParent()];
			if (prev) {
				prev->Children[parent->GetIndexInParent()] = sibling;
				sibling->Parent = prev;
				DeleteNodeInternal(parent);
				while (prev) {
					const Volume pb = prev->Volume;
					prev->Volume = prev->Children[0]->Volume.Merge(prev->Children[1]->Volume);
					if (pb.IsNotEqualTo(prev->Volume)) {
						prev = prev->Parent;
					}
					else {
						break;
					}
				}
				return (prev ? prev : mBvhRoot);
			}
			else {
				mBvhRoot = sibling;
				sibling->Parent = nullptr;
				DeleteNodeInternal(parent);
				return (mBvhRoot);
			}
		}
	}

	void DynamicBVH::FetchLeaves(Node* root, DVector<Node*>& r_leaves, int p_depth) {
		if (root->IsInternal() && p_depth) {
			FetchLeaves(root->Children[0], r_leaves, p_depth - 1);
			FetchLeaves(root->Children[1], r_leaves, p_depth - 1);
			DeleteNodeInternal(root);
		}
		else {
			r_leaves.push_back(root);
		}
	}

	// Partitions leaves such that leaves[0, n) are on the
	// left of axis, and leaves[n, count) are on the right
	// of axis. returns N.
	int DynamicBVH::Split(Node** leaves, int count, const Vector3& p_org, const Vector3& p_axis) {
		int begin = 0;
		int end = count;
		for (;;) {
			while (begin != end && leaves[begin]->IsLeafOfAxis(p_org, p_axis)) {
				++begin;
			}

			if (begin == end) {
				break;
			}

			while (begin != end && !leaves[end - 1]->IsLeafOfAxis(p_org, p_axis)) {
				--end;
			}

			if (begin == end) {
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

	DynamicBVH::Volume DynamicBVH::Bounds(Node** leaves, int count) {
		Volume volume = leaves[0]->Volume;
		for (int i = 1, ni = count; i < ni; ++i) {
			volume = volume.Merge(leaves[i]->Volume);
		}
		return (volume);
	}

	void DynamicBVH::BottomUp(Node** leaves, int count) {
		while (count > 1) {
			float minsize = INFINITY;
			int minidx[2] = { -1, -1 };
			for (int i = 0; i < count; ++i) {
				for (int j = i + 1; j < count; ++j) {
					const float sz = leaves[i]->Volume.Merge(leaves[j]->Volume).GetSize();
					if (sz < minsize) {
						minsize = sz;
						minidx[0] = i;
						minidx[1] = j;
					}
				}
			}
			Node* n[] = { leaves[minidx[0]], leaves[minidx[1]] };
			Node* p = CreateNodeWithVolumeInternal(nullptr, n[0]->Volume.Merge(n[1]->Volume), nullptr);
			p->Children[0] = n[0];
			p->Children[1] = n[1];
			n[0]->Parent = p;
			n[1]->Parent = p;
			leaves[minidx[0]] = p;
			leaves[minidx[1]] = leaves[count - 1];
			--count;
		}
	}

	DynamicBVH::Node* DynamicBVH::TopDown(Node** leaves, int count, int buThreshold) {
		static const Vector3 axis[] = { Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1) };

		if (buThreshold <= 1)
			return nullptr;

		if (count > 1) {
			if (count > buThreshold) {
				const Volume vol = Bounds(leaves, count);
				const Vector3 org = vol.GetCenter();
				int partition;
				int bestaxis = -1;
				int bestmidp = count;
				int splitcount[3][2] = { { 0, 0 }, { 0, 0 }, { 0, 0 } };
				int i;
				for (i = 0; i < count; ++i) {
					const Vector3 x = leaves[i]->Volume.GetCenter() - org;
					for (int j = 0; j < 3; ++j) {
						++splitcount[j][Dot(x, axis[j]) > 0.f ? 1 : 0];
					}
				}
				for (i = 0; i < 3; ++i) {
					if ((splitcount[i][0] > 0) && (splitcount[i][1] > 0)) {
						const int midp = (int)Abs((float)(splitcount[i][0] - splitcount[i][1]));
						if (midp < bestmidp) {
							bestaxis = i;
							bestmidp = midp;
						}
					}
				}
				if (bestaxis >= 0) {
					partition = Split(leaves, count, org, axis[bestaxis]);
					if (partition == 0 || partition == count)
						return nullptr;
				}
				else {
					partition = count / 2 + 1;
				}

				Node* node = CreateNodeWithVolumeInternal(nullptr, vol, nullptr);
				node->Children[0] = TopDown(&leaves[0], partition, buThreshold);
				node->Children[1] = TopDown(&leaves[partition], count - partition, buThreshold);
				node->Children[0]->Parent = node;
				node->Children[1]->Parent = node;
				return (node);
			}
			else {
				BottomUp(leaves, count);
				return (leaves[0]);
			}
		}
		return (leaves[0]);
	}

	DynamicBVH::Node* DynamicBVH::NodeSort(Node* n, Node*& r) {
		Node* p = n->Parent;
		if (!n->IsInternal())
			return nullptr;

		if (p > n) {
			const int i = n->GetIndexInParent();
			const int j = 1 - i;
			Node* s = p->Children[j];
			Node* q = p->Parent;
			if (n != p->Children[i])
				return nullptr;

			if (q) {
				q->Children[p->GetIndexInParent()] = n;
			}
			else {
				r = n;
			}
			s->Parent = n;
			p->Parent = n;
			n->Parent = q;
			p->Children[0] = n->Children[0];
			p->Children[1] = n->Children[1];
			n->Children[0]->Parent = p;
			n->Children[1]->Parent = p;
			n->Children[i] = p;
			n->Children[j] = s;

			DynamicBVH::Volume aux = p->Volume;
			p->Volume = n->Volume;
			n->Volume = aux;
			return (p);
		}
		return (n);
	}

	void DynamicBVH::Clear() {
		if (mBvhRoot) {
			RecurseDeleteNodeInternal(mBvhRoot);
		}
		mLkhd = -1;
		mOpath = 0;
	}

	void DynamicBVH::OptimizeBottomUp() {
		if (mBvhRoot) {
			DVector<Node*> leaves;
			FetchLeaves(mBvhRoot, leaves);
			BottomUp(&leaves[0], (int)leaves.size());
			mBvhRoot = leaves[0];
		}
	}

	void DynamicBVH::OptimizeTopDown(int bu_threshold) {
		if (mBvhRoot) {
			DVector<Node*> leaves;
			FetchLeaves(mBvhRoot, leaves);
			mBvhRoot = TopDown(&leaves[0], (int)leaves.size(), bu_threshold);
		}
	}

	void DynamicBVH::OptimizeIncremental(int passes) {
		if (passes < 0) {
			passes = mTotalLeaves;
		}
		if (passes > 0) {
			do {
				if (!mBvhRoot) {
					break;
				}
				Node* node = mBvhRoot;
				unsigned bit = 0;
				while (node->IsInternal()) {
					node = NodeSort(node, mBvhRoot)->Children[(mOpath >> bit) & 1];
					bit = (bit + 1) & (sizeof(unsigned) * 8 - 1);
				}
				UpdateInternal(node);
				++mOpath;
			} while (--passes);
		}
	}

	DynamicBVH::ID DynamicBVH::Insert(Aabb const& box, void* userdata) {
		Volume volume{ box };

		Node* leaf = CreateNodeWithVolumeInternal(nullptr, volume, userdata);
		InsertLeaf(mBvhRoot, leaf);
		++mTotalLeaves;

		ID id;
		id.node = leaf;

		return id;
	}

	void DynamicBVH::UpdateInternal(Node* leaf, int lookahead) {
		Node* root = RemoveLeaf(leaf);
		if (root) {
			if (lookahead >= 0) {
				for (int i = 0; (i < lookahead) && root->Parent; ++i) {
					root = root->Parent;
				}
			}
			else {
				root = mBvhRoot;
			}
		}
		InsertLeaf(root, leaf);
	}

	bool DynamicBVH::Update(const ID& id, const Aabb& box) {
		if (!id.IsValid())
			return false;

		Node* leaf = id.node;

		Volume volume{ box };

		
		if (volume.Aabb.NearEquals(box, 0.00001f)) {
			// noop
			return false;
		}

		Node* base = RemoveLeaf(leaf);
		if (base) {
			if (mLkhd >= 0) {
				for (int i = 0; (i < mLkhd) && base->Parent; ++i) {
					base = base->Parent;
				}
			}
			else {
				base = mBvhRoot;
			}
		}
		leaf->Volume = volume;
		InsertLeaf(base, leaf);
		return true;
	}

	void DynamicBVH::Remove(ID const& id) {
		if (!id.IsValid())
			return;

		Node* leaf = id.node;
		RemoveLeaf(leaf);
		DeleteNodeInternal(leaf);
		--mTotalLeaves;
	}

	void DynamicBVH::ExtractLeaves(Node* p_node, DList<ID>* elements) {
		if (p_node->IsInternal()) {
			ExtractLeaves(p_node->Children[0], elements);
			ExtractLeaves(p_node->Children[1], elements);
		}
		else {
			ID id;
			id.node = p_node;
			elements->push_back(id);
		}
	}

	void DynamicBVH::SetIndex(uint32_t index) {
		if (mBvhRoot != nullptr)
			return;

		mIndex = index;
	}

	uint32_t DynamicBVH::GetIndex() const {
		return mIndex;
	}

	void DynamicBVH::GetElements(DList<ID>* r_elements) {
		if (mBvhRoot) {
			ExtractLeaves(mBvhRoot, r_elements);
		}
	}

	int DynamicBVH::GetLeafCount() const {
		return mTotalLeaves;
	}
	int DynamicBVH::GetMaxDepth() const {
		if (mBvhRoot) {
			int depth = 1;
			int max_depth = 0;
			mBvhRoot->GetMaxDepth(depth, max_depth);
			return max_depth;
		}
		else {
			return 0;
		}
	}

	DynamicBVH::~DynamicBVH() {
		Clear();
	}

}
