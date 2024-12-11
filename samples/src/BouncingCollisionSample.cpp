#include "BouncingCollisionSample.h"

std::string BouncingCollisionSample::GetName() noexcept
{
	return "Collision";
}

std::string BouncingCollisionSample::GetDescription() noexcept
{
	return "Randomly generated objects, they change color on contact with another object, the collision detection uses a QuadTree. ";
}

void BouncingCollisionSample::OnCollisionEnter(ColliderRef col1, ColliderRef col2) noexcept
{

	Color color = {
	Random::Range(0, 255),
	Random::Range(0, 255),
	Random::Range(0, 255),
	255 };
	AllGraphicsData[col1.Index].Color = color;
	AllGraphicsData[col2.Index].Color = color;
}

void BouncingCollisionSample::OnCollisionExit(ColliderRef col1, ColliderRef col2) noexcept
{

}

void BouncingCollisionSample::SampleSetUp() noexcept
{
	_world.SetContactListener(this);
	_nbObjects = CIRCLE_NBR + RECTANGLE_NBR;
	_collisionNbrPerCollider.resize(_nbObjects, 0);
	AllGraphicsData.reserve(_nbObjects);
	_bodyRefs.reserve(_nbObjects);
	_colRefs.reserve(_nbObjects);

	//Create Circles
	for (std::size_t i = 0; i < CIRCLE_NBR; ++i)
	{
		auto bodyRef1 = _world.CreateBody();
		_bodyRefs.push_back(bodyRef1);
		auto& body1 = _world.GetBody(bodyRef1);

		body1.Mass = 1;

		body1.Velocity = XMVectorScale(XMVectorSet(Random::Range(-1.f, 1.f), Random::Range(-1.f, 1.f), 0, 0), SPEED);


		body1.Position = { Random::Range(100.f, Metrics::Width - 100.f),
						  Random::Range(100.f, Metrics::Height - 100.f) };

		auto colRef1 = _world.CreateCollider(bodyRef1);
		_colRefs.push_back(colRef1);
		auto& col1 = _world.GetCollider(colRef1);
		col1.Shape = Circle(XMVectorZero(), CIRCLE_RADIUS);
		col1.BodyPosition = body1.Position;

		GraphicsData bd;
		bd.Shape = Circle(XMVectorZero(), CIRCLE_RADIUS) + body1.Position;
		AllGraphicsData.push_back(bd);
	}

	//Create Rectangles
	for (std::size_t i = 0; i < RECTANGLE_NBR; ++i)
	{
		auto bodyRef1 = _world.CreateBody();
		_bodyRefs.push_back(bodyRef1);
		auto& body1 = _world.GetBody(bodyRef1);

		body1.Velocity = XMVectorScale(XMVectorSet(Random::Range(-1.f, 1.f), Random::Range(-1.f, 1.f), 0, 0), SPEED);

		body1.Position = { Random::Range(100.f, Metrics::Width - 100.f),
						  Random::Range(100.f, Metrics::Height - 100.f) };

		auto colRef1 = _world.CreateCollider(bodyRef1);
		_colRefs.push_back(colRef1);
		auto& col1 = _world.GetCollider(colRef1);
		col1.Shape = RectangleF(XMVectorZero(), RECTANGLE_BOUNDS);
		col1.BodyPosition = body1.Position;

		GraphicsData bd;
		bd.Shape = RectangleF(XMVectorZero(), RECTANGLE_BOUNDS) + body1.Position;
		AllGraphicsData.push_back(bd);
	}
}

void BouncingCollisionSample::DrawQuadtree(const QuadNode& node) noexcept
{
	if (node.Children[0] == nullptr)
	{
		_quadTreeGraphicsData.push_back({ RectangleF(node.Bounds), false });
	}
	else
	{
		for (int i = 0; i < 4; i++)
		{
			DrawQuadtree(*node.Children[i]);
		}
	}
}

void BouncingCollisionSample::SampleUpdate() noexcept
{
	if (_nbObjects < AllGraphicsData.size())
	{
		AllGraphicsData.erase(AllGraphicsData.begin() + _nbObjects, AllGraphicsData.end());
	}

	for (std::size_t i = 0; i < _colRefs.size(); ++i)
	{
		auto& col = _world.GetCollider(_colRefs[i]);
		auto bounds = col.GetBounds();
		auto& body = _world.GetBody(col.BodyRef);

		if (XMVectorGetX(bounds.MinBound()) <= 0)
		{
			body.Velocity = XMVectorSetX(body.Velocity, Abs(XMVectorGetX(body.Velocity)));
		}
		else if (XMVectorGetX(bounds.MaxBound()) >= Metrics::Width)
		{
			body.Velocity = XMVectorSetX(body.Velocity, -Abs(XMVectorGetX(body.Velocity)));
		}
		if (XMVectorGetY(bounds.MinBound()) <= 0)
		{
			body.Velocity = XMVectorSetY(body.Velocity, Abs(XMVectorGetY(body.Velocity)));
		}
		else if (XMVectorGetY(bounds.MaxBound()) >= Metrics::Height)
		{
			body.Velocity = XMVectorSetY(body.Velocity, -Abs(XMVectorGetY(body.Velocity)));
		}

		auto& shape = _world.GetCollider(_colRefs[i]).Shape;

		switch (shape.index())
		{
		case static_cast<int>(ShapeType::Circle):
			AllGraphicsData[i].Shape = std::get<CircleF>(shape) + col.BodyPosition;
			break;
		case static_cast<int>(ShapeType::Rectangle):
			AllGraphicsData[i].Shape = std::get<RectangleF>(shape) + col.BodyPosition;
			break;
		}
	}

	_quadTreeGraphicsData.clear();
	DrawQuadtree(_world.QuadTree.Nodes[0]);
	AllGraphicsData.insert(AllGraphicsData.end(), _quadTreeGraphicsData.begin(), _quadTreeGraphicsData.end());
}

void BouncingCollisionSample::SampleTearDown() noexcept
{
	_collisionNbrPerCollider.clear();
	_quadTreeGraphicsData.clear();
}
