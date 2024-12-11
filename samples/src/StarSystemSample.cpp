#include "StarSystemSample.h"

std::string StarSystemSample::GetName() noexcept
{
	return "Star System";
}

std::string StarSystemSample::GetDescription() noexcept
{
	return "Randomly generated physical objects rotating around a \"sun\" using forces.";
}

void StarSystemSample::SampleSetUp() noexcept
{
	{
		_sunRef = _world.CreateBody();
		auto& sun = _world.GetBody(_sunRef);
		sun.Position = { static_cast<float>(Metrics::Width) / 2, static_cast<float>(Metrics::Height) / 2 };
		sun.Mass = 1000000;

		_bodyRefs.push_back(_sunRef);
		GraphicsData sgd;
		_circles.emplace_back(XMVectorZero(), Metrics::MetersToPixels(0.03f));
		sgd.Color = { 255, 255, 0, 255 };
		AllGraphicsData.push_back(sgd);

		for (std::size_t i = 0; i < PLANET_NBR; ++i)
		{
			// Engine
			auto bodyRef = _world.CreateBody();
			auto& body = _world.GetBody(bodyRef);
			body.Position = { Random::Range(100.f, Metrics::Width - 100.f),
							 Random::Range(100.f, Metrics::Height - 100.f) };
			auto r = XMVectorSubtract(_world.GetBody(_sunRef).Position, body.Position);
			auto v = sqrt(G * (_world.GetBody(_sunRef).Mass / XMVectorGetX(XMVector3Length(r))));
			body.Velocity = XMVectorScale(XMVector3Normalize(XMVectorSet(-XMVectorGetY(r), XMVectorGetX(r), 0, 0)), v);
			//body.Velocity = XMVECTOR(-r.Y, r.X).Normalized() * v;
			body.Mass = 10.f;

			// Graphics
			_bodyRefs.push_back(bodyRef);
			GraphicsData gd;
			_circles.emplace_back(XMVectorZero(),
				Random::Range(
					Metrics::MetersToPixels(0.05f),
					Metrics::MetersToPixels(0.15f)));
			gd.Color = {
					Random::Range(0, 255),
					Random::Range(0, 255),
					Random::Range(0, 255),
					255 };

			AllGraphicsData.push_back(gd);
		}
	}
}

void StarSystemSample::CalculateGravitationalForce(const Body& sun, Body& body) noexcept
{

	auto m1m2 = sun.Mass * body.Mass;
	auto r = XMVectorGetX(XMVector3Length(XMVectorSubtract(sun.Position, body.Position)));
	auto r2 = r * r;
	auto F = G * (m1m2 / r2);

	XMVECTOR forceDirection = XMVector3Normalize(XMVectorSubtract(sun.Position, body.Position));
	XMVECTOR force = XMVectorScale(forceDirection, F);
	body.ApplyForce(force);

}

void StarSystemSample::SampleUpdate() noexcept
{
	auto& sun = _world.GetBody(_sunRef);

	for (std::size_t i = 0; i < _bodyRefs.size(); ++i)
	{
		auto& body = _world.GetBody(_bodyRefs[i]);

		AllGraphicsData[i].Shape = _circles[i] + body.Position;

		if (_bodyRefs[i] == _sunRef) continue; // Skip the Sun

		CalculateGravitationalForce(sun, body);

	}
}

void StarSystemSample::SampleTearDown() noexcept
{
	_circles.clear();
}

