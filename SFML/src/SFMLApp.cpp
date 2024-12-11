#include "SFMLApp.h"

#ifdef TRACY_ENABLE
#include "Tracy.hpp"
#include "TracyC.h"
#endif

void SFMLApp::SetUp() {
	_window.create(sf::VideoMode(Width, Height), Title);
	ImGui::SFML::Init(_window);

	_sampleManager.SetUp();
}

void SFMLApp::TearDown() const noexcept { ImGui::SFML::Shutdown(); }

void SFMLApp::Run() noexcept {
	bool quit = false;

	bool adjustWindow = true;

	sf::Event e;

	while (!quit) {
		while (_window.pollEvent(e)) {
			ImGui::SFML::ProcessEvent(e);
			switch (e.type) {
			case sf::Event::Closed:
				quit = true;
				break;
			case sf::Event::KeyReleased:
				switch (e.key.code) {
				case sf::Keyboard::Left:
					_sampleManager.PreviousSample();
					break;
				case sf::Keyboard::Right:
					_sampleManager.NextSample();
					break;
				case sf::Keyboard::Space:
					_sampleManager.RegenerateSample();
					break;
				}
				break;
			case sf::Event::MouseButtonReleased:
				if (e.mouseButton.button == sf::Mouse::Left) {
					_sampleManager.GiveLeftMouseClickToSample();
				}
				else if (e.mouseButton.button == sf::Mouse::Right) {
					_sampleManager.GiveRightMouseClickToSample();
				}
				break;
			}
		}

		ImGui::SFML::Update(_window, _deltaClock.restart());

		if (adjustWindow) {
			ImGui::SetNextWindowSize(ImVec2(Metrics::Width / 3, Metrics::Height / 5));
			adjustWindow = false;
		}

		ImGui::Begin("Sample Manager");

		if (ImGui::BeginCombo(
			"Select a Sample",
			_sampleManager.GetSampleName(_sampleManager.GetCurrentIndex())
			.c_str())) {
			for (std::size_t index = 0; index < _sampleManager.GetSampleNbr();
				index++) {
				if (ImGui::Selectable(_sampleManager.GetSampleName(index).c_str(),
					_sampleManager.GetCurrentIndex() == index)) {
					_sampleManager.ChangeSample(index);
				}
			}
			ImGui::EndCombo();
		}

		ImGui::Spacing();

		ImGui::TextWrapped(
			_sampleManager.GetSampleDescription(_sampleManager.GetCurrentIndex())
			.c_str());

		ImGui::Spacing();

		ImGui::SetCursorPosY(ImGui::GetWindowHeight() -
			(ImGui::GetFrameHeightWithSpacing()));

		if (ImGui::ArrowButton("PreviousSample", ImGuiDir_Left)) {
			_sampleManager.PreviousSample();
		}

		ImGui::SameLine();

		if (ImGui::Button("Regenerate")) {
			_sampleManager.RegenerateSample();
		}

		ImGui::SameLine();

		if (ImGui::ArrowButton("NextSample", ImGuiDir_Right)) {
			_sampleManager.NextSample();
		}

		ImGui::End();

		_window.clear(sf::Color::Black);

		sf::Vector2i sfMousePos = sf::Mouse::getPosition(_window);
		MousePos = XMVectorSet(sfMousePos.x, sfMousePos.y, 0, 0);
		_sampleManager.GiveMousePositionToSample(MousePos);
		_sampleManager.UpdateSample();

		DrawAllGraphicsData();

		ImGui::SFML::Render(_window);

		_window.display();

#ifdef TRACY_ENABLE
		FrameMark;
#endif
	}
}

void SFMLApp::DrawCircle(const XMVECTOR center, const float radius,
	const int segments, const sf::Color& col) noexcept {
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	sf::CircleShape circle = sf::CircleShape(radius, segments);
	circle.setFillColor(col);
	circle.setOrigin(radius, radius);
	circle.setPosition(XMVectorGetX(center), XMVectorGetY(center));

	_window.draw(circle);
}

void SFMLApp::DrawRectangle(const XMVECTOR minBound,
	const XMVECTOR maxBound,
	const sf::Color& col) noexcept {
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	// Calculate the width and height of the rectangle
	float width = XMVectorGetX(maxBound) - XMVectorGetX(minBound);
	float height = XMVectorGetY(maxBound) - XMVectorGetY(minBound);

	// Create an SFML rectangle shape
	sf::RectangleShape rectangle(sf::Vector2f(width, height));

	// Set the position and color of the rectangle
	rectangle.setPosition(XMVectorGetX(minBound), XMVectorGetY(minBound));
	rectangle.setFillColor(col);

	// Draw the rectangle to the window
	_window.draw(rectangle);
}

void SFMLApp::DrawRectangleBorder(const XMVECTOR minBound, const XMVECTOR maxBound,
	const sf::Color& col) noexcept {
	// Calculate the width and height of the rectangle
	float width = XMVectorGetX(maxBound) - XMVectorGetX(minBound);
	float height = XMVectorGetY(maxBound) - XMVectorGetY(minBound);

	// Create an SFML rectangle shape
	sf::RectangleShape rectangle(sf::Vector2f(width, height));

	// Set the position and color of the rectangle
	rectangle.setPosition(XMVectorGetX(minBound), XMVectorGetY(minBound));
	rectangle.setOutlineColor(col);
	rectangle.setOutlineThickness(1);
	rectangle.setFillColor(sf::Color::Transparent);

	// Draw the rectangle to the window
	_window.draw(rectangle);
}

void SFMLApp::DrawPolygon(const std::vector<XMVECTOR>& vertices, const sf::Color& col) {
	if (vertices.size() < 3) {
		return;  // Don't draw if the polygon is invalid
	}

	sf::ConvexShape polygon(vertices.size());

	for (size_t i = 0; i < vertices.size(); ++i) {
		polygon.setPoint(i, sf::Vector2f(XMVectorGetX(vertices[i]), XMVectorGetY(vertices[i])));
	}

	polygon.setFillColor(col);

	_window.draw(polygon);
}

void SFMLApp::DrawAllGraphicsData() noexcept {
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif
	for (auto& bd : _sampleManager.GetSampleData()) {
		if (bd.Shape.index() == (int)ShapeType::Circle) {
			auto& circle = std::get<CircleF>(bd.Shape);
			DrawCircle(circle.Center(), circle.Radius(), 30,
				{ static_cast<sf::Uint8>(bd.Color.r),
				 static_cast<sf::Uint8>(bd.Color.g),
				 static_cast<sf::Uint8>(bd.Color.b),
				 static_cast<sf::Uint8>(bd.Color.a) });
		}
		else if (bd.Shape.index() == (int)ShapeType::Rectangle) {
			auto& rect = std::get<RectangleF>(bd.Shape);
			if (!bd.Filled) {
				DrawRectangleBorder(rect.MinBound(), rect.MaxBound(),
					{ static_cast<sf::Uint8>(bd.Color.r),
					 static_cast<sf::Uint8>(bd.Color.g),
					 static_cast<sf::Uint8>(bd.Color.b),
					 static_cast<sf::Uint8>(bd.Color.a) });
			}
			else {
				DrawRectangle(rect.MinBound(), rect.MaxBound(),
					{ static_cast<sf::Uint8>(bd.Color.r),
					 static_cast<sf::Uint8>(bd.Color.g),
					 static_cast<sf::Uint8>(bd.Color.b),
					 static_cast<sf::Uint8>(bd.Color.a) });
			}
		}
		else if (bd.Shape.index() == (int)ShapeType::Polygon) {
			auto& polygon = std::get<PolygonF>(bd.Shape);
			DrawPolygon(polygon.Vertices(), { static_cast<sf::Uint8>(bd.Color.r),
											 static_cast<sf::Uint8>(bd.Color.g),
											 static_cast<sf::Uint8>(bd.Color.b),
											 static_cast<sf::Uint8>(bd.Color.a) });
		}
	}
}
