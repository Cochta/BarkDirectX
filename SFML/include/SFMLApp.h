#pragma once

#include <imgui-SFML.h>
#include <imgui.h>

#include <SFML/Graphics.hpp>
#include <vector>

#include "SampleManager.h"

class SFMLApp {
 private:
  sf::RenderWindow _window;
  sf::Clock _deltaClock;
  SampleManager _sampleManager;

 public:
  std::string Title;
  int Width, Height;
  XMVECTOR MousePos;

  SFMLApp(sf::String title, sf::Uint32 width, sf::Uint32 height) noexcept
      : _window(sf::VideoMode(width, height), title) {
    Width = width;
    Height = height;
    Title = title;
  }

  void SetUp();

  void TearDown() const noexcept;

  void Run() noexcept;

 private:
  void DrawCircle(XMVECTOR center, float radius, int segments,
                  const sf::Color &col) noexcept;

  void DrawRectangle(XMVECTOR minBound, XMVECTOR maxBound,
                     const sf::Color &col) noexcept;

  void DrawRectangleBorder(XMVECTOR minBound, XMVECTOR maxBound,
                           const sf::Color &col) noexcept;

  void DrawPolygon(const std::vector<XMVECTOR> &vertices,
                   const sf::Color &col);

  void DrawAllGraphicsData() noexcept;
};