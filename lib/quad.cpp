//
// Created by James Pickering on 8/5/25.
//

#include "quad.hpp"

auto containsPoint(const Quad& quad, const glm::vec2 point) -> bool {
	const auto halfExtents = quad.size / 2.f;
	return point.x < quad.pos.x + halfExtents.x &&
		   point.x > quad.pos.x - halfExtents.x &&
		   point.y < quad.pos.y + halfExtents.y &&
		   point.y > quad.pos.y - halfExtents.y;
}