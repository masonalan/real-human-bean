//
// Created by James Pickering on 8/5/25.
//

#pragma once

#include "../quad.hpp"

// TODO: move to ui config
constexpr auto MinKnobRotation = glm::radians(90.f);
constexpr auto MaxKnobRotation = glm::radians(-180.f);

struct GraphicsContext;
struct Mouse;

struct Knob {
	Quad quad;
	bool hovered = false;
	bool rotating = false;
	float rotation = MinKnobRotation;
	float initRotation = 0.f;
	float value = 0.f;
};

auto knobUpdate(Knob& knob, const Mouse& mouse) -> void;
auto knobRender(const Knob& knob, const GraphicsContext& graphics) -> void;
auto knobInitWithValue(Knob& knob, float value) -> void;
auto knobValueToRotation(float value) -> float;
auto knobRotationToValue(float rotation) -> float;
auto knobApplyRotationToModel(const glm::mat4& model, float rotation)
	-> glm::mat4;
