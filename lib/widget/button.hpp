//
// Created by James Pickering on 8/5/25.
//

#pragma once

#include "../quad.hpp"

struct GraphicsContext;
struct Mouse;

struct Button {
	Quad quad;
	int events = 0;
	bool hovered = false;
	bool pressed = false;
};

auto buttonUpdate(Button& button, const Mouse& mouse) -> void;
auto buttonRender(const Button& button, const GraphicsContext& graphics)
	-> void;