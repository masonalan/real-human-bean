//
// Created by James Pickering on 8/5/25.
//

#include "quad.hpp"

#include "config.hpp"
#include "ui.hpp"

#include <glm/ext/matrix_transform.hpp>

#include "engine.hpp"

auto quadContainsPoint(const Quad& quad, const glm::vec2 point) -> bool {
	const auto halfExtents = quad.size / 2.f;
	return point.x < quad.pos.x + halfExtents.x &&
		   point.x > quad.pos.x - halfExtents.x &&
		   point.y < quad.pos.y + halfExtents.y &&
		   point.y > quad.pos.y - halfExtents.y;
}

auto quadToModel(const Quad& quad) -> glm::mat4 {
	auto model = glm::mat4{1.f};
	model = glm::translate(model,
						   glm::vec3{quad.pos / config::HalfWindowSize, 0.1f});
	model =
		glm::scale(model, glm::vec3{quad.size / config::HalfWindowSize, 1.f});
	return model;
}

auto quadFromPsQuad(const glm::vec2 psPos, const glm::vec2 psSize) -> Quad {
	auto quad = Quad{};
	quad.size = sizeFromPsSize(psSize);
	quad.pos = posFromPsPos(psPos, psSize);
	return quad;
}

auto posFromPsPos(const glm::vec2 psPos, const glm::vec2 psSize) -> glm::vec2 {
	const auto diff = glm::vec2{psSize / 2.f - ImgSize / 2.f};
	auto pos = (psPos + diff) * WinRatio;
	pos.y = -pos.y;
	return pos;
}

auto sizeFromPsSize(const glm::vec2 psSize) -> glm::vec2 {
	return psSize * WinRatio;
}
