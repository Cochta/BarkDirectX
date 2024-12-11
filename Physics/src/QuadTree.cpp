#include "QuadTree.h"

QuadNode::QuadNode(Allocator& alloc) noexcept : ColliderRefAabbs(StandardAllocator<ColliderRefPair>{alloc})
{
	ColliderRefAabbs.reserve(16);
}

QuadNode::QuadNode(const RectangleF& bounds, Allocator& alloc) noexcept : ColliderRefAabbs(StandardAllocator<ColliderRefPair>{ alloc }), Bounds(bounds)
{
	ColliderRefAabbs.reserve(16);
}

QuadTree::QuadTree(Allocator& alloc) noexcept : _alloc(alloc), Nodes{ StandardAllocator<QuadNode>{alloc} }
{
	std::size_t result = 0;
	for (size_t i = 0; i <= MAX_DEPTH; i++)
	{
		result += Pow(4, i);
	}
	Nodes.resize(result, QuadNode(_alloc));
}

void QuadTree::SubdivideNode(QuadNode& node) noexcept
{
	const XMVECTOR halfSize = XMVectorDivide(XMVectorSubtract(node.Bounds.MaxBound(), node.Bounds.MinBound()), XMVectorSet(2, 2, 2, 2));
	const XMVECTOR minBound = node.Bounds.MinBound();

	node.Children[0] = &Nodes[_nodeIndex];
	node.Children[0]->Bounds = { minBound, XMVectorAdd(minBound , halfSize) };

	node.Children[1] = &Nodes[_nodeIndex + 1];
	node.Children[1]->Bounds = { {XMVectorGetX(minBound), XMVectorGetY(minBound) + XMVectorGetY(halfSize)},
		{XMVectorGetX(minBound) + XMVectorGetX(halfSize), XMVectorGetY(minBound) + 2 * XMVectorGetY(halfSize)} };

	node.Children[2] = &Nodes[_nodeIndex + 2];
	node.Children[2]->Bounds = { {XMVectorGetX(minBound) + XMVectorGetX(halfSize), XMVectorGetY(minBound)},
		{XMVectorGetX(minBound) + 2 * XMVectorGetX(halfSize), XMVectorGetY(minBound) + XMVectorGetY(halfSize)} };

	node.Children[3] = &Nodes[_nodeIndex + 3];
	node.Children[3]->Bounds = { {XMVectorGetX(minBound) + XMVectorGetX(halfSize),
		XMVectorGetY(minBound) + XMVectorGetY(halfSize)}, node.Bounds.MaxBound() };


	for (const auto& child : node.Children)
	{
		child->Depth = node.Depth + 1;
	}
	_nodeIndex += 4;
}

void QuadTree::Insert(QuadNode& node, const ColliderRefAabb& colliderRefAabb) noexcept
{
	if (node.Children[0] != nullptr)
	{
		for (const auto& child : node.Children)
		{
			if (Intersect(colliderRefAabb.Aabb, child->Bounds))
			{
				Insert(*child, colliderRefAabb);
			}
		}
	}
	else if (node.ColliderRefAabbs.size() >= MAX_COL_NBR && node.Depth < MAX_DEPTH)
	{
		SubdivideNode(node);
		node.ColliderRefAabbs.push_back(colliderRefAabb);
		for (const auto& child : node.Children)
		{
			for (auto& col : node.ColliderRefAabbs)
			{
				if (Intersect(col.Aabb, child->Bounds))
				{
					Insert(*child, col);
				}
			}
		}
		node.ColliderRefAabbs.clear();
	}
	else
	{
		node.ColliderRefAabbs.push_back(colliderRefAabb);
	}
}

void QuadTree::SetUpRoot(const RectangleF& bounds) noexcept
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	for (auto& node : Nodes)
	{
		node.ColliderRefAabbs.clear();
		std::fill(node.Children.begin(), node.Children.end(), nullptr);
	}
	Nodes[0].Bounds = bounds;

	_nodeIndex = 1;
}
