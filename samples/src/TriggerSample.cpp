#include "TriggerSample.h"

std::string TriggerSample::GetName() noexcept
{
	return "Trigger";
}

std::string TriggerSample::GetDescription() noexcept
{
	return "Randomly generated objects, they become green if they detect a trigger otherwise they stay blue, the trigger detection uses a QuadTree. ";
}

void TriggerSample::OnTriggerEnter(ColliderRef col1, ColliderRef col2) noexcept
{
	_triggerNbrPerCollider[col1.Index]++;
	_triggerNbrPerCollider[col2.Index]++;
}

void TriggerSample::OnTriggerExit(ColliderRef col1, ColliderRef col2) noexcept
{
	_triggerNbrPerCollider[col1.Index]--;
	_triggerNbrPerCollider[col2.Index]--;
}

void TriggerSample::SampleSetUp() noexcept
{
	_world.SetContactListener(this);
	_nbObjects = CIRCLE_NBR + RECTANGLE_NBR + TRIANGLE_NBR;
	_triggerNbrPerCollider.resize(_nbObjects, 0);
	AllGraphicsData.reserve(_nbObjects);
	_bodyRefs.reserve(_nbObjects);
	_colRefs.reserve(_nbObjects);

	//Create Circles
	for (std::size_t i = 0; i < CIRCLE_NBR; ++i)
	{
		const auto circleBodyRef = _world.CreateBody();
		_bodyRefs.push_back(circleBodyRef);
		auto& circleBody = _world.GetBody(circleBodyRef);

		circleBody.Velocity = XMVectorScale(XMVectorSet(Random::Range(-1.f, 1.f), Random::Range(-1.f, 1.f), 0, 0), SPEED);

		circleBody.Position = { Random::Range(100.f, Metrics::Width - 100.f),
						  Random::Range(100.f, Metrics::Height - 100.f) };

		const auto circleColRef = _world.CreateCollider(circleBodyRef);
		_colRefs.push_back(circleColRef);
		auto& circleCol = _world.GetCollider(circleColRef);
		circleCol.Shape = Circle(XMVectorZero(), CIRCLE_RADIUS);
		circleCol.BodyPosition = circleBody.Position;
		circleCol.IsTrigger = true;

		GraphicsData cgd;
		cgd.Shape = Circle(XMVectorZero(), CIRCLE_RADIUS) + circleBody.Position;
		AllGraphicsData.push_back(cgd);
	}

	//Create Rectangles
	for (std::size_t i = 0; i < RECTANGLE_NBR; ++i)
	{
		const auto rectBodyRef = _world.CreateBody();
		_bodyRefs.push_back(rectBodyRef);
		auto& rectBody = _world.GetBody(rectBodyRef);

		rectBody.Velocity = XMVectorScale(XMVectorSet(Random::Range(-1.f, 1.f), Random::Range(-1.f, 1.f), 0, 0), SPEED);

		rectBody.Position = { Random::Range(100.f, Metrics::Width - 100.f),
						  Random::Range(100.f, Metrics::Height - 100.f) };

		const auto rectColRef = _world.CreateCollider(rectBodyRef);
		_colRefs.push_back(rectColRef);
		auto& rectCol = _world.GetCollider(rectColRef);
		rectCol.Shape = RectangleF(XMVectorZero(), RECTANGLE_BOUNDS);
		rectCol.BodyPosition = rectBody.Position;
		rectCol.IsTrigger = true;

		GraphicsData rbd;
		rbd.Shape = RectangleF(XMVectorZero(), RECTANGLE_BOUNDS) + rectBody.Position;
		AllGraphicsData.push_back(rbd);
	}
	// Create Triangles
	for (std::size_t i = 0; i < TRIANGLE_NBR; ++i)
	{
		const auto triangleBodyRef = _world.CreateBody();
		_bodyRefs.push_back(triangleBodyRef);
		auto& triangleBody = _world.GetBody(triangleBodyRef);

		triangleBody.Velocity = XMVectorScale(XMVectorSet(Random::Range(-1.f, 1.f), Random::Range(-1.f, 1.f), 0, 0), SPEED);

		triangleBody.Position = { Random::Range(100.f, Metrics::Width - 100.f),
						  Random::Range(100.f, Metrics::Height - 100.f) };

		const auto triangleColRef = _world.CreateCollider(triangleBodyRef);
		_colRefs.push_back(triangleColRef);
		auto& triangleCol = _world.GetCollider(triangleColRef);
		triangleCol.Shape = PolygonF(TRIANGLE_VERTICES);
		triangleCol.BodyPosition = triangleBody.Position;
		triangleCol.IsTrigger = true;

		GraphicsData tbd;
		tbd.Shape = PolygonF(TRIANGLE_VERTICES) + triangleBody.Position;
		AllGraphicsData.push_back(tbd);
	}
}

void TriggerSample::DrawQuadtree(const QuadNode& node) noexcept
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

void TriggerSample::SampleUpdate() noexcept
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
			AllGraphicsData[i].Shape = std::get<CircleF>(shape) + body.Position;
			break;
		case static_cast<int>(ShapeType::Rectangle):
			AllGraphicsData[i].Shape = std::get<RectangleF>(shape) + body.Position;
			break;
		case static_cast<int>(ShapeType::Polygon):
			AllGraphicsData[i].Shape = std::get<PolygonF>(shape) + body.Position;
			break;
		}

		if (_triggerNbrPerCollider[i] > 0)
		{
			AllGraphicsData[i].Color = { 0, 255, 0, 255 };
		}
		else
		{
			AllGraphicsData[i].Color = { 0, 0, 255, 255 };
		}
	}

	_quadTreeGraphicsData.clear();
	DrawQuadtree(_world.QuadTree.Nodes[0]);
	AllGraphicsData.insert(AllGraphicsData.end(), _quadTreeGraphicsData.begin(), _quadTreeGraphicsData.end());
}

void TriggerSample::SampleTearDown() noexcept
{
	_triggerNbrPerCollider.clear();
	_quadTreeGraphicsData.clear();
}
