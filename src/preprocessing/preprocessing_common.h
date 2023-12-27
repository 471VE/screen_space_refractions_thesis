#pragma once

#include <functional>
#include <vector>

#include <glm/glm.hpp>

#include "../config.h"

// Calculations have to be performed using double, and only then results should be casted to float.
// Otherwise, precision is lost.
std::vector<glm::dvec3> construct_hemisphere_hammersley_sequence(uint32_t numPoints);

std::vector<float> calculate_sh_terms(
  std::vector<glm::dvec3> hammersleySequence, std::function<DataToEncode(glm::dvec3)> getDataToEncode
);
