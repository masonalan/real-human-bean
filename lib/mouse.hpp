//
// Created by James Pickering on 8/5/25.
//

#pragma once

#include <glm/glm.hpp>

struct Mouse {
	glm::vec2 pos{};
	glm::vec2 mouseDownPos;
	bool isPressed = false;
	int events = 0;
};