#pragma once

#include "Sample.h"
#include "Random.h"

class FormsTriggerSample : public Sample, public ContactListener
{
private:
    std::vector<int> _triggerNbrPerCollider;

    BodyRef _movableTriangleRef{};

public:
    std::string GetName() noexcept override;
    std::string GetDescription() noexcept override;

    void OnTriggerEnter(ColliderRef col1, ColliderRef col2) noexcept override;

    void OnTriggerExit(ColliderRef col1, ColliderRef col2) noexcept override;

    void OnCollisionEnter(ColliderRef col1, ColliderRef col2) noexcept override {};

    void OnCollisionExit(ColliderRef col1, ColliderRef col2) noexcept override {};

protected:
    void SampleSetUp() noexcept override;

    void SampleUpdate() noexcept override;

    void SampleTearDown() noexcept override;
};