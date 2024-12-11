#pragma once

#include "Collider.h"
#include "Allocators.h"
#include "UniquePtr.h"

#include <memory>
#include <array>
/**
 * @brief Structure representing a bounding box (AABB) associated with a collider reference.
 */
struct ColliderRefAabb
{
	const RectangleF Aabb;    /**< The bounding box (AABB). */
	const ColliderRef ColRef;       /**< The reference to a collider. */
};

/**
 * @brief Class representing a node in a quadtree data structure for collision detection.
 */
class QuadNode
{
public:
	CustomlyAllocatedVector<ColliderRefAabb> ColliderRefAabbs;  /**< Vector of collider references with AABBs. */
	RectangleF Bounds{ XMVectorZero(), XMVectorZero() }; /**< The bounds of the quadtree node. */
	std::array<QuadNode*, 4> Children{ nullptr, nullptr, nullptr, nullptr }; /**< Array of child nodes. */
	int Depth = 0; /**< The depth of the node in the quadtree.*/

	/**
	 * @brief Constructor for QuadNode, allocating memory using a specified allocator.
	 * @param alloc The allocator for memory allocation.
	 */
	QuadNode(Allocator& alloc) noexcept;

	/**
	 * @brief Constructor for QuadNode with a specified bounds, allocating memory using a specified allocator.
	 * @param bounds The bounds of the quadtree node.
	 * @param alloc The allocator for memory allocation.
	 */
	explicit QuadNode(const RectangleF& bounds, Allocator& alloc) noexcept;
};

/**
 * @brief Class representing a quadtree data structure for collision detection.
 */
class QuadTree {
public:
	CustomlyAllocatedVector<QuadNode> Nodes; /**< Vector of quadtree nodes. */

private:
	static constexpr int MAX_COL_NBR = 16; /**< Maximum number of colliders in a quadtree node. */
	static constexpr int MAX_DEPTH = 5; /**< Maximum depth of the quadtree. */
	int _nodeIndex = 1; /**< The index of the current node in the quadtree. */
	Allocator& _alloc; /**< The allocator for memory allocation.*/
public:
	/**
	 * @brief Constructor for QuadTree, allocating memory using a specified allocator.
	 * @param alloc The allocator for memory allocation.
	 */
	QuadTree(Allocator& alloc) noexcept;

	/**
	 * @brief Set up the root of the quadtree with the specified bounds.
	 * @param bounds The bounds of the root node.
	 */
	void SetUpRoot(const RectangleF& bounds) noexcept;

	/**
	 * @brief Insert a collider reference with an associated AABB into the quadtree.
	 * @param node The node to insert into.
	 * @param colliderRefAabb The collider reference with an AABB to insert.
	 */
	void Insert(QuadNode& node, const ColliderRefAabb& colliderRefAabb) noexcept;
private:
	/**
	 * @brief Subdivide a quadtree node into smaller child nodes.
	 * @param node The node to subdivide.
	 */
	void SubdivideNode(QuadNode& node) noexcept;
};