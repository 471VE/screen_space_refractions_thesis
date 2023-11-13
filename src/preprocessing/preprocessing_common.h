#pragma once

#include <functional>
#include <vector>

#include <glm/glm.hpp>

std::vector<glm::dvec3> construct_hemisphere_hammersley_sequence(uint32_t numPoints);
std::vector<double> calculate_sh_terms(
  std::vector<glm::dvec3> hammersleySequence, std::function<double(glm::dvec3)> getObjectWidth
);