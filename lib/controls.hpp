//
// Created by James Pickering on 7/21/25.
//

#pragma once

#include "quad.hpp"

#include <glm/glm.hpp>

#include <vector>

constexpr auto MinKnobRotation = glm::radians(90.f);
constexpr auto MaxKnobRotation = glm::radians(-180.f);
constexpr auto KnobLeftX = -375;
constexpr auto KnobMiddleX = -124;
constexpr auto KnobY = 567;
constexpr auto KnobSize = glm::vec2{65, 65};
constexpr auto BarSize = glm::vec2{44, 11};
constexpr auto GraphSize = glm::vec2{182, 118};
constexpr auto ImgSize = glm::vec2{1500, 750};
constexpr auto WinSize = glm::vec2{1000, 500};
constexpr auto WinRatio = WinSize / ImgSize;
constexpr auto ReseedButtonSize = glm::vec2{41, 42};

enum class Alignment {
	Left,
	Center,
};

constexpr auto fromPsPos(const glm::vec2 psPos,
						 const glm::vec2 psSize,
						 const Alignment alignment = Alignment::Left)
	-> glm::vec2 {
	auto diff = glm::vec2{};
	switch (alignment) {
		case Alignment::Left:
			diff = glm::vec2{psSize / 2.f - ImgSize / 2.f};
			break;
		case Alignment::Center:
			diff = glm::vec2{psSize.x / 2.f - ImgSize.x / 2.f,
							 (psSize / 2.f - ImgSize / 2.f).y};
			break;
		default:
			break;
	}
	auto pos = (psPos + diff) * WinRatio;
	pos.y = -pos.y;
	return pos;
}

constexpr auto fromPsSize(const glm::vec2 psSize) -> glm::vec2 {
	return psSize * WinRatio;
}

constexpr auto fromPsQuad(const glm::vec2 psPos, const glm::vec2 psSize)
	-> Quad {
	auto quad = Quad{};
	quad.size = fromPsSize(psSize);
	quad.pos = fromPsPos(psPos, psSize, Alignment::Left);
	return quad;
}

struct Knob {
	Quad quad;
	bool hovered = false;
	bool rotating = false;
	float rotation = MinKnobRotation;
	float initRotation = 0.f;
	float value = 0.f;
};

auto knobInitWithValue(Knob& knob, float value) -> void;
auto knobValueToRotation(float value) -> float;
auto knobRotationToValue(float rotation) -> float;

struct ShiftPair {
	Quad onBeat;
	Quad shifted;
};

enum Event {
	MousePressEvent = 1 << 0,
	MouseReleaseEvent = 1 << 1,
};

struct Button {
	Quad quad;
	int events = 0;
	bool hovered = false;
	bool pressed = false;
};

auto hasEvent(int events, int event) -> bool;
auto clearEvent(int& events, int event) -> void;

struct State;
struct GraphicsContext;

struct Mouse {
	glm::vec2 pos{};
	glm::vec2 mouseDownPos;
	bool isPressed = false;
	int events = 0;
};

struct Ui {
	glm::ivec2 size;
	Mouse mouse;
	Knob knobAlpha;
	Knob knobSteps;
	Knob knobVariance;
	Knob knobLookahead;
	Quad graphPowerSpectrum;
	Quad graphOffset;
	Quad diagramOffset;
	Button buttonReseed;
	std::vector<ShiftPair> shiftPairs;

	bool isFresh = true;
};

auto setupUi(Ui& ui) -> void;
auto updateUi(Ui& ui, State& state, const GraphicsContext& graphics) -> void;
auto renderUi(Ui& ui, const State& state, const GraphicsContext& graphics)
	-> void;
