//
// Created by James Pickering on 8/5/25.
//

#pragma once

#include "../quad.hpp"

struct Button {
	Quad quad;
	int events = 0;
	bool hovered = false;
	bool pressed = false;
};
