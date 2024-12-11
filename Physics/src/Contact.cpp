#include "Contact.h"

void Contact::Resolve()
{
	switch (CollidingBodies[0].collider->Shape.index())
	{
	case static_cast<int>(ShapeType::Circle):
		switch (CollidingBodies[1].collider->Shape.index())
		{
		case static_cast<int>(ShapeType::Circle):
		{
			const CircleF& circle0 = std::get<CircleF>(CollidingBodies[0].collider->Shape);
			const CircleF& circle1 = std::get<CircleF>(CollidingBodies[1].collider->Shape);

			const auto delta = XMVectorSubtract(XMVectorSubtract(XMVectorAdd(CollidingBodies[0].body->Position, circle0.Center()), CollidingBodies[1].body->Position), circle1.Center());

			float length = XMVectorGetX(XMVector3Length(delta));

			if (length > 0.f)
			{
				Normal = XMVector3Normalize(delta);
			}
			else
			{
				Normal = g_XMIdentityR1; // vector up
			}
			Penetration = std::get<CircleF>(CollidingBodies[0].collider->Shape).Radius() +
				std::get<CircleF>(CollidingBodies[1].collider->Shape).Radius() - length;
		}
		break;
		case static_cast<int>(ShapeType::Rectangle):
		{
			const CircleF& circle = std::get<CircleF>(CollidingBodies[0].collider->Shape);
			const RectangleF& rectangle = std::get<RectangleF>(CollidingBodies[1].collider->Shape);



			auto closestX = Clamp(
				XMVectorGetX(CollidingBodies[0].body->Position) + XMVectorGetX(circle.Center()),
				XMVectorGetX(rectangle.MinBound()) + XMVectorGetX(CollidingBodies[1].body->Position),
				XMVectorGetX(rectangle.MaxBound()) + XMVectorGetX(CollidingBodies[1].body->Position)
			);

			auto closestY = Clamp(
				XMVectorGetY(CollidingBodies[0].body->Position) + XMVectorGetY(circle.Center()),
				XMVectorGetY(rectangle.MinBound()) + XMVectorGetY(CollidingBodies[1].body->Position),
				XMVectorGetY(rectangle.MaxBound()) + XMVectorGetY(CollidingBodies[1].body->Position)
			);

			const XMVECTOR closest = XMVectorSet(closestX, closestY, 0, 0);

			//const XMVECTOR delta = CollidingBodies[0].body->Position + circle.Center() - closest;

			XMVECTOR delta = XMVectorSubtract(
				XMVectorAdd(CollidingBodies[0].body->Position, circle.Center()),
				closest
			);

			const float distance = XMVectorGetX(XMVector3Length(delta));

			Penetration = circle.Radius() - distance;

			if (distance > 0.f)
			{
				Normal = XMVector3Normalize(delta);
			}
			else
			{
				Normal = g_XMIdentityR1;
			}

		}
		break;
		}
		break;
	case static_cast<int>(ShapeType::Rectangle):
		switch (CollidingBodies[1].collider->Shape.index())
		{
		case static_cast<int>(ShapeType::Circle):
		{
			std::swap(CollidingBodies[0], CollidingBodies[1]);
			Resolve();
		}
		break;
		case static_cast<int>(ShapeType::Rectangle):
		{
			const RectangleF& rect0 = std::get<RectangleF>(CollidingBodies[0].collider->Shape);
			const RectangleF& rect1 = std::get<RectangleF>(CollidingBodies[1].collider->Shape);

			const auto delta = XMVectorSubtract(XMVectorSubtract(XMVectorAdd(CollidingBodies[0].body->Position, rect0.Center()), CollidingBodies[1].body->Position), rect1.Center());

			const XMVECTOR penetration = XMVectorSubtract(XMVectorAdd(std::get<RectangleF>(CollidingBodies[0].collider->Shape).HalfSize(), std::get<RectangleF>(CollidingBodies[1].collider->Shape).HalfSize()), XMVectorAbs(delta));

			if (XMVectorGetX(penetration) < XMVectorGetY(penetration))
			{
				Penetration = XMVectorGetX(penetration);
				Normal = (XMVectorGetX(delta) > 0) ? XMVectorSet(1.0f, 0.0f, 0, 0) : XMVectorSet(-1.0f, 0.0f, 0, 0);
			}
			else
			{
				Penetration = XMVectorGetY(penetration);
				Normal = (XMVectorGetY(delta) > 0) ? XMVectorSet(0.0f, 1.0f, 0, 0) : XMVectorSet(0.0f, -1.0f, 0, 0);
			}
		}
		break;
		}
	}

	const auto mass1 = CollidingBodies[0].body->Mass, mass2 = CollidingBodies[1].body->Mass;
	const auto rest1 = CollidingBodies[0].collider->Restitution, rest2 = CollidingBodies[1].collider->Restitution;

	Restitution = (mass1 * rest1 + mass2 * rest2) / (mass1 + mass2);

	ResolveVelocityAndInterpenetration();
	ResolveInterpenetration();
}

float Contact::CalculateSeparateVelocity() const noexcept
{
	const auto relativeVelocity = XMVectorSubtract(CollidingBodies[0].body->Velocity, CollidingBodies[1].body->Velocity);
	float result = 0;
	XMStoreFloat(&result, XMVector3Dot(relativeVelocity, Normal));
	return result; // relativeVelocity.Dot(Normal);
}

void Contact::ResolveVelocityAndInterpenetration() const noexcept
{
	const float separatingVelocity = CalculateSeparateVelocity();

	if (separatingVelocity > 0) {
		return;
	}

	const float newSeparatingVelocity = -separatingVelocity * Restitution;

	const float deltaVelocity = newSeparatingVelocity - separatingVelocity;

	const float inverseMass1 = 1.f / CollidingBodies[0].body->Mass;
	const float inverseMass2 = 1.f / CollidingBodies[1].body->Mass;

	const float totalInverseMass = inverseMass1 + inverseMass2;

	if (totalInverseMass <= 0) {
		return;
	}

	const float impulse = deltaVelocity / totalInverseMass;
	const auto impulsePerIMass = XMVectorScale(Normal, impulse);

	if (CollidingBodies[0].body->Type == BodyType::DYNAMIC)
	{
		CollidingBodies[0].body->Velocity = XMVectorAdd(CollidingBodies[0].body->Velocity, XMVectorScale(impulsePerIMass, inverseMass1));
	}
	if (CollidingBodies[1].body->Type == BodyType::DYNAMIC)
	{
		CollidingBodies[1].body->Velocity = XMVectorSubtract(CollidingBodies[1].body->Velocity, XMVectorScale(impulsePerIMass, inverseMass2));
	}

	if (CollidingBodies[0].body->Type == BodyType::STATIC)
	{
		CollidingBodies[1].body->Velocity = XMVectorSubtract(CollidingBodies[1].body->Velocity, XMVectorScale(impulsePerIMass, inverseMass1));
	}
	if (CollidingBodies[1].body->Type == BodyType::STATIC)
	{
		CollidingBodies[0].body->Velocity = XMVectorAdd(CollidingBodies[0].body->Velocity, XMVectorScale(impulsePerIMass, inverseMass2));
	}
}

void Contact::ResolveInterpenetration() const noexcept
{
	if (Penetration <= 0) return;

	const float inverseMass1 = 1.f / CollidingBodies[0].body->Mass;
	const float inverseMass2 = 1.f / CollidingBodies[1].body->Mass;
	const float totalInverseMass = inverseMass1 + inverseMass2;

	if (totalInverseMass <= 0) {
		return;
	}

	const auto movePerIMass = XMVectorScale(Normal, (Penetration / totalInverseMass));

	if (CollidingBodies[0].body->Type == BodyType::DYNAMIC)
	{
		CollidingBodies[0].body->Position = XMVectorAdd(CollidingBodies[0].body->Position, XMVectorScale(movePerIMass, inverseMass1));
	}

	if (CollidingBodies[1].body->Type == BodyType::DYNAMIC)
	{
		CollidingBodies[1].body->Position = XMVectorSubtract(CollidingBodies[1].body->Position, XMVectorScale(movePerIMass, inverseMass2));
	}
}