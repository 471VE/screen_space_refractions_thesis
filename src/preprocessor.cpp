
#include <iostream>
#include "preprocessing/preprocessing_common.h"

#define NUM_POINTS 1000000

int main()
{
  std::vector<glm::dvec3> hammersleySequence = construct_hemisphere_hammersley_sequence(NUM_POINTS);
  // std::vector<float> shTerms = calculate_sh_terms(hammersleySequence, sphere_width);

  return 0;
}