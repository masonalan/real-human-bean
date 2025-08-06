//
// Created by James Pickering on 6/26/25.
//

#pragma once

#include <glm/vec3.hpp>

auto perlin(double x, double y, double z) -> double;
auto octavePerlin(glm::vec<3, double> point, int octaves, double persistence) -> double;
auto octavePerlinSigned(glm::vec<3, double> point, int octaves, double persistence) -> double;