//
// Created by James Pickering on 8/5/25.
//

#pragma once

#include <glm/glm.hpp>

struct Quad {
	glm::vec2 pos{};
	glm::vec2 size{};
};

auto quadContainsPoint(const Quad& quad, glm::vec2 point) -> bool;
auto quadToModel(const Quad& quad) -> glm::mat4;
auto quadFromPsQuad(const glm::vec2 psPos, const glm::vec2 psSize) -> Quad;
auto posFromPsPos(const glm::vec2 psPos, const glm::vec2 psSize) -> glm::vec2;
auto sizeFromPsSize(const glm::vec2 psSize) -> glm::vec2;
