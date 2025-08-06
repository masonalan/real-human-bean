//
// Created by James Pickering on 8/5/25.
//

#pragma once

#include <glm/glm.hpp>

struct Quad {
	glm::vec2 pos{};
	glm::vec2 size{};
};

auto containsPoint(const Quad& quad, glm::vec2 point) -> bool;