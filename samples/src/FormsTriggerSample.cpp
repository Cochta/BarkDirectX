#include "FormsTriggerSample.h"

std::string FormsTriggerSample::GetName() noexcept
{
	return "Forms intersect";
}

std::string FormsTriggerSample::GetDescription() noexcept
{
	return "CONTROLS: Move the triangle using the mouse.\n\nForms become green if they detect a trigger otherwise they stay blue. ";
}

void FormsTriggerSample::OnTriggerEnter(ColliderRef col1, ColliderRef col2) noexcept
{
	_triggerNbrPerCollider[col1.Index]++;
	_triggerNbrPerCollider[col2.Index]++;
}

void FormsTriggerSample::OnTriggerExit(ColliderRef col1, ColliderRef col2) noexcept
{
	_triggerNbrPerCollider[col1.Index]--;
	_triggerNbrPerCollider[col2.Index]--;
}

void FormsTriggerSample::SampleSetUp() noexcept
{
	_world.SetContactListener(this);

	_triggerNbrPerCollider.resize(4, 0);

	auto triangleRef = _world.CreateBody();
	auto& triangle = _world.GetBody(triangleRef);
	triangle.Position = { static_cast<float>(Metrics::Width - Metrics::MetersToPixels(2)),
						 static_cast<float>(Metrics::Height - Metrics::MetersToPixels(2)) };
	triangle.Mass = 4.f;
	_bodyRefs.push_back(triangleRef);

	auto triangleColRef = _world.CreateCollider(triangleRef);
	_colRefs.push_back(triangleColRef);
	auto& triangleCol = _world.GetCollider(triangleColRef);

	std::vector<XMVECTOR> verticesTriangle = { {0.f,                         0.f},
												 {-Metrics::MetersToPixels(2), -Metrics::MetersToPixels(1)},
												 {-Metrics::MetersToPixels(1), -Metrics::MetersToPixels(2)} };

	triangleCol.Shape = PolygonF(verticesTriangle);
	triangleCol.IsTrigger = true;

	GraphicsData gdTriangle;
	gdTriangle.Shape = PolygonF(verticesTriangle) + triangle.Position;
	AllGraphicsData.push_back(gdTriangle);


	_movableTriangleRef = _world.CreateBody();
	auto& movableTriangleBody = _world.GetBody(_movableTriangleRef);
	movableTriangleBody.Position = { static_cast<float>(Metrics::Width) / 2,
						 static_cast<float>(Metrics::Height) / 2 };
	movableTriangleBody.Mass = 1.f;
	_bodyRefs.push_back(_movableTriangleRef);

	auto movableTriangleColRef = _world.CreateCollider(_movableTriangleRef);
	_colRefs.push_back(movableTriangleColRef);
	auto& movableTriangleCol = _world.GetCollider(movableTriangleColRef);

	std::vector<XMVECTOR> verticesMovableTriangle = { {0, Metrics::MetersToPixels(0.5f)},
											 {-Metrics::MetersToPixels(0.5f), Metrics::MetersToPixels(0.5f)},
											 {Metrics::MetersToPixels(0.5f), -Metrics::MetersToPixels(0.5f)} };

	movableTriangleCol.Shape = PolygonF(verticesMovableTriangle);
	movableTriangleCol.IsTrigger = true;

	GraphicsData gdMovableTriangle;
	gdMovableTriangle.Shape = PolygonF(verticesMovableTriangle) + movableTriangleBody.Position;
	AllGraphicsData.push_back(gdMovableTriangle);


	auto circleBodyRef = _world.CreateBody();
	_bodyRefs.push_back(circleBodyRef);
	auto& circleBody = _world.GetBody(circleBodyRef);
	circleBody.Position = { Metrics::MetersToPixels(7), Metrics::MetersToPixels(2) };
	auto circleColRef = _world.CreateCollider(circleBodyRef);
	_colRefs.push_back(circleColRef);
	auto& circleCol = _world.GetCollider(circleColRef);
	circleCol.Shape = Circle(XMVectorZero(), Metrics::MetersToPixels(0.3f));
	circleCol.IsTrigger = true;

	GraphicsData gdCircle;
	gdCircle.Shape = Circle(XMVectorZero(), Metrics::MetersToPixels(0.3f)) + circleBody.Position;
	AllGraphicsData.push_back(gdCircle);

	auto rectRef = _world.CreateBody();
	auto& rect = _world.GetBody(rectRef);
	rect.Position = XMVectorSet(Metrics::MetersToPixels(2), Metrics::MetersToPixels(2), 0, 0);

	_bodyRefs.push_back(rectRef);

	auto colRefRect = _world.CreateCollider(rectRef);
	_colRefs.push_back(colRefRect);
	auto& colRect = _world.GetCollider(colRefRect);

	colRect.Shape = RectangleF(XMVectorZero(),
		XMVectorSet(
			Metrics::MetersToPixels(2),
			Metrics::MetersToPixels(2), 0, 0));
	colRect.IsTrigger = true;

	GraphicsData gdRect;
	gdRect.Shape = RectangleF(XMVectorZero(),
		XMVectorSet(
			Metrics::MetersToPixels(2),
			Metrics::MetersToPixels(2), 0, 0)) + rect.Position;
	AllGraphicsData.push_back(gdRect);
}

void FormsTriggerSample::SampleUpdate() noexcept
{
	auto& movableTriangleBody = _world.GetBody(_movableTriangleRef);
	movableTriangleBody.Position = _mousePos;

	AllGraphicsData[1].Shape = std::get<PolygonF>(_world.GetCollider(_colRefs[1]).Shape) + movableTriangleBody.Position;
	for (int i = 0; i < _colRefs.size(); ++i)
	{
		if (_triggerNbrPerCollider[i] > 0)
		{
			AllGraphicsData[i].Color = { 0, 255, 0, 255 };
		}
		else
		{
			AllGraphicsData[i].Color = { 0, 0, 255, 255 };
		}
	}
}

void FormsTriggerSample::SampleTearDown() noexcept
{
	_triggerNbrPerCollider.clear();
}
