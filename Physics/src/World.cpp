#include "World.h"

#ifdef TRACY_ENABLE
#include <Tracy.hpp>
#include <TracyC.h>
#endif 

void World::SetUp(int initSize) noexcept
{
	_bodies.resize(initSize);
	BodyGenIndices.resize(initSize, 0);

	_colliders.resize(initSize);
	ColliderGenIndices.resize(initSize, 0);
}

void World::TearDown() noexcept
{
	_bodies.clear();
	BodyGenIndices.clear();
	_colliders.clear();

	ColliderGenIndices.clear();

	_colRefPairs.clear();
}

void World::Update(const float deltaTime) noexcept
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	UpdateBodies(deltaTime);

	SetUpQuadTree();

	UpdateQuadTreeCollisions(QuadTree.Nodes[0]);
}

[[nodiscard]] BodyRef World::CreateBody() noexcept
{
	const auto it = std::find_if(_bodies.begin(), _bodies.end(), [](const Body& body) {
		return !body.IsEnabled(); // Get first disabled body
		});

	if (it != _bodies.end())
	{
		const std::size_t index = std::distance(_bodies.begin(), it);
		const auto bodyRef = BodyRef{ index, BodyGenIndices[index] };
		GetBody(bodyRef).Enable();
		return bodyRef;
	}

	const std::size_t previousSize = _bodies.size();

	_bodies.resize(previousSize * 2, Body());
	BodyGenIndices.resize(previousSize * 2, 0);

	const BodyRef bodyRef = { previousSize, BodyGenIndices[previousSize] };
	GetBody(bodyRef).Enable();
	return bodyRef;
}

void World::DestroyBody(const BodyRef bodyRef)
{
	if (BodyGenIndices[bodyRef.Index] != bodyRef.GenIndex)
	{
		throw std::runtime_error("No body found !");
	}

	_bodies[bodyRef.Index].Disable();
}

[[nodiscard]] Body& World::GetBody(const BodyRef bodyRef)
{
	if (BodyGenIndices[bodyRef.Index] != bodyRef.GenIndex)
	{
		throw std::runtime_error("No body found !");
	}

	return _bodies[bodyRef.Index];
}

ColliderRef World::CreateCollider(const BodyRef bodyRef) noexcept
{
	const auto it = std::find_if(_colliders.begin(), _colliders.end(), [](const Collider& collider) {
		return !collider.IsAttached; // Get first disabled collider
		});

	if (it != _colliders.end())
	{
		const std::size_t index = std::distance(_colliders.begin(), it);
		const auto colRef = ColliderRef{ index, ColliderGenIndices[index] };
		auto& col = GetCollider(colRef);
		col.IsAttached = true;
		col.BodyRef = bodyRef;

		return colRef;
	}

	const std::size_t previousSize = _colliders.size();

	_colliders.resize(previousSize * 2, Collider());
	ColliderGenIndices.resize(previousSize * 2, 0);

	const ColliderRef colRef = { previousSize, ColliderGenIndices[previousSize] };
	auto& col = GetCollider(colRef);
	col.IsAttached = true;
	col.BodyRef = bodyRef;
	return colRef;
}

Collider& World::GetCollider(const ColliderRef colRef)
{
	if (ColliderGenIndices[colRef.Index] != colRef.GenIndex)
	{
		throw std::runtime_error("No collider found !");
	}

	return _colliders[colRef.Index];
}

void World::DestroyCollider(const ColliderRef colRef)
{
	if (ColliderGenIndices[colRef.Index] != colRef.GenIndex)
	{
		throw std::runtime_error("No collider found !");
	}
	_colliders[colRef.Index].IsAttached = false;
}

void World::UpdateBodies(const float deltaTime) noexcept
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	for (auto& body : _bodies)
	{
		if (!body.IsEnabled())
		{
			continue;
		}
		if (body.Type == BodyType::STATIC)
		{
			continue;
		}
		auto acceleration = XMVectorScale(body.GetForce(), 1 / body.Mass);
		body.Velocity = XMVectorAdd(body.Velocity, XMVectorScale(acceleration, deltaTime));
		body.Position = XMVectorAdd(body.Position, XMVectorScale(body.Velocity, deltaTime));

		body.ResetForce();
	}
}

void World::SetUpQuadTree() noexcept {
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	XMVECTOR maxBounds = XMVectorSet(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), 0, 0);
	XMVECTOR minBounds = XMVectorSet(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 0, 0);

	for (auto& collider : _colliders) {
		if (!collider.IsAttached) {
			continue;
		}

		collider.BodyPosition = GetBody(collider.BodyRef).Position;

		const auto bounds = collider.GetBounds();

		minBounds = XMVectorSetX(minBounds, std::min(XMVectorGetX(minBounds), XMVectorGetX(bounds.MinBound())));
		minBounds = XMVectorSetY(minBounds, std::min(XMVectorGetY(minBounds), XMVectorGetY(bounds.MinBound())));
		maxBounds = XMVectorSetX(maxBounds, std::max(XMVectorGetX(maxBounds), XMVectorGetX(bounds.MaxBound())));
		maxBounds = XMVectorSetY(maxBounds, std::max(XMVectorGetY(maxBounds), XMVectorGetY(bounds.MaxBound())));
	}

	QuadTree.SetUpRoot(RectangleF(minBounds, maxBounds));
#ifdef TRACY_ENABLE
	ZoneNamedN(Insert, "Insert in QuadTree", true);
#endif
	for (std::size_t i = 0; i < _colliders.size(); ++i) {
		if (_colliders[i].IsAttached) {
			QuadTree.Insert(QuadTree.Nodes[0], { _colliders[i].GetBounds(), { i, ColliderGenIndices[i] } });
		}
	}
}

void World::UpdateQuadTreeCollisions(const QuadNode& node) noexcept
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	if (node.Children[0] == nullptr)
	{
		if (node.ColliderRefAabbs.empty())
		{
			return;
		}
		for (std::size_t i = 0; i < node.ColliderRefAabbs.size() - 1; ++i)
		{
			auto& col1 = GetCollider(node.ColliderRefAabbs[i].ColRef);

			for (std::size_t j = i + 1; j < node.ColliderRefAabbs.size(); ++j)
			{
				auto& col2 = GetCollider(node.ColliderRefAabbs[j].ColRef);

				if (!col2.IsTrigger && !col1.IsTrigger) // Physical collision
				{
					if (Overlap(col1, col2))
					{
						Contact contact;
						contact.CollidingBodies[0] = { &GetBody(col1.BodyRef), &col1 };
						contact.CollidingBodies[1] = { &GetBody(col2.BodyRef), &col2 };
						contact.Resolve();
						if (_contactListener != nullptr)
						{
							_contactListener->OnCollisionEnter(node.ColliderRefAabbs[i].ColRef, node.ColliderRefAabbs[j].ColRef);
						}
					}
					else
					{
						if (_contactListener != nullptr)
						{
							_contactListener->OnCollisionExit(node.ColliderRefAabbs[i].ColRef, node.ColliderRefAabbs[j].ColRef);
						}
					}
					continue;
				}

				if (_contactListener == nullptr)
				{
					continue;
				}
				// Trigger collision
				const ColliderRefPair& colPair = { node.ColliderRefAabbs[i].ColRef, node.ColliderRefAabbs[j].ColRef };

				if (_colRefPairs.find(colPair) != _colRefPairs.end())
				{
					if (!Overlap(col1, col2))
					{
						_contactListener->OnTriggerExit(colPair.ColRefA, colPair.ColRefB);
						_colRefPairs.erase(colPair);
					}
					continue;
				}

				if (Overlap(col1, col2))
				{
					_contactListener->OnTriggerEnter(colPair.ColRefA, colPair.ColRefB);
					_colRefPairs.emplace(colPair);
				}
			}
		}
	}
	else
	{
		for (const auto& child : node.Children)
		{
			UpdateQuadTreeCollisions(*child);
		}
	}
}

[[nodiscard]] bool World::Overlap(const Collider& colA, const Collider& colB) noexcept
{
	const auto ShapeA = static_cast<ShapeType>(colA.Shape.index());
	const auto ShapeB = static_cast<ShapeType>(colB.Shape.index());

#ifdef TRACY_ENABLE
	ZoneScoped;
	/*static constexpr const char* names[] = { "Circle", "Rectangle", "Polygon", "None" };
	const auto log = fmt::format("Shape A: {}, Shape B: {}", names[static_cast<int>(ShapeA)], names[static_cast<int>(ShapeB)]);
	ZoneText(log.data(), log.size());*/
#endif

	switch (ShapeA)
	{
	case ShapeType::Circle:
	{
		CircleF circle = std::get<CircleF>(colA.Shape) + GetBody(colA.BodyRef).Position;
		switch (ShapeB)
		{
		case ShapeType::Circle:
			return Intersect(circle, std::get<CircleF>(colB.Shape) + GetBody(colB.BodyRef).Position);
		case ShapeType::Rectangle:
			return Intersect(circle, std::get<RectangleF>(colB.Shape) + GetBody(colB.BodyRef).Position);
			/*case ShapeType::Polygon:
				return Intersect(circle, std::get<PolygonF>(colB.Shape) + GetBody(colB.BodyRef).Position);*/
		}
		break;
	}
	case ShapeType::Rectangle:
	{
		RectangleF rect = std::get<RectangleF>(colA.Shape) + GetBody(colA.BodyRef).Position;
		switch (ShapeB)
		{
		case ShapeType::Circle:
			return Intersect(rect, std::get<CircleF>(colB.Shape) + GetBody(colB.BodyRef).Position);
		case ShapeType::Rectangle:
			return Intersect(rect, std::get<RectangleF>(colB.Shape) + GetBody(colB.BodyRef).Position);
			case ShapeType::Polygon:
				return Intersect(rect, std::get<PolygonF>(colB.Shape) + GetBody(colB.BodyRef).Position);
		}
		break;
	}
	case ShapeType::Polygon:
	{
		PolygonF pol = std::get<PolygonF>(colA.Shape) + GetBody(colA.BodyRef).Position;
		switch (ShapeB)
		{
		case ShapeType::Circle:
			return Intersect(pol, std::get<CircleF>(colB.Shape) + GetBody(colB.BodyRef).Position);
		case ShapeType::Rectangle:
			return Intersect(pol, std::get<RectangleF>(colB.Shape) + GetBody(colB.BodyRef).Position);
		case ShapeType::Polygon:
			return Intersect(pol, std::get<PolygonF>(colB.Shape) + GetBody(colB.BodyRef).Position);
		}
		break;
	}
	}
	return false;
}
