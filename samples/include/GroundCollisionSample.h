#pragma once

#include <iostream>
#include "Sample.h"
#include "Random.h"

#ifdef SDL_ENABLE
static constexpr float SPEED = -500;
#else
static constexpr float SPEED = 500;
#endif

class GroundCollisionSample : public Sample, public ContactListener
{
private:
	

public:
	std::string GetName() noexcept override;
	std::string GetDescription() noexcept override;

	void OnTriggerEnter(ColliderRef col1, ColliderRef col2) noexcept override {};

	void OnTriggerExit(ColliderRef col1, ColliderRef col2) noexcept override {};

	void OnCollisionEnter(ColliderRef col1, ColliderRef col2) noexcept override;

	void OnCollisionExit(ColliderRef col1, ColliderRef col2) noexcept override;

protected:
	void SampleSetUp() noexcept override;

	void SampleUpdate() noexcept override;

	void SampleTearDown() noexcept override;

private:
	void CreateBall(XMVECTOR position) noexcept;
	void CreateRect(XMVECTOR position) noexcept;

};