//
// Created by James Pickering on 7/21/25.
//

#pragma once

#include "mouse.hpp"
#include "quad.hpp"
#include "widget/button.hpp"
#include "widget/knob.hpp"

#include <glm/glm.hpp>

#include <vector>

struct State;
struct GraphicsContext;

struct Ui {
	glm::ivec2 windowSize;
	bool isFresh = true;

	Mouse mouse;
	Knob knobAlpha;
	Knob knobSteps;
	Knob knobVariance;
	Knob knobLookahead;
	Quad graphPowerSpectrum;
	Quad graphOffset;
	Quad diagramOffset;
	Quad labelKnobDesc;
	Button buttonReseed;
	std::vector<Quad> cells;

	unsigned int currDescTex = 0;
};

auto setupUi(Ui& ui) -> void;
auto updateUi(Ui& ui, State& state, const GraphicsContext& graphics) -> void;
auto renderUi(Ui& ui, const State& state, const GraphicsContext& graphics)
	-> void;
