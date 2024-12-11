#include "Sample.h"

void Sample::SetUp() noexcept
{
    _timer.SetUp();
    _world.SetUp();
    SampleSetUp();
}

void Sample::Update() noexcept
{
    SampleUpdate();
    _timer.Tick();
    _world.Update(_timer.DeltaTime);
    _mouseLeftReleased = false;
    _mouseRightReleased = false;
}

void Sample::TearDown() noexcept
{
    SampleTearDown();
    _bodyRefs.clear();
    AllGraphicsData.clear();
    _colRefs.clear();
    _world.TearDown();
}

void Sample::GetMousePos(const XMVECTOR mousePos) noexcept
{
    _mousePos = mousePos;
}

void Sample::OnLeftClick() noexcept
{
    _mouseLeftReleased = true;
}

void Sample::OnRightClick() noexcept
{
    _mouseRightReleased = true;
}
