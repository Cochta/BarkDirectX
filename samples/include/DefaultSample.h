#pragma once

#include "Sample.h"

class DefaultSample : public Sample {
public:
	std::string GetName() noexcept override { return "None"; };
	std::string GetDescription() noexcept override { return "Using: SDL2 and Dear ImGui\n\nAuthor: Constantin Verine"; };
protected:
	void SampleSetUp() noexcept override {};

	void SampleUpdate() noexcept override {};

	void SampleTearDown() noexcept override {};
};