
#include <iostream>
#include "preprocessing/preprocessing_common.h"

#define SPHERE_RADIUS 1.
#define NUM_POINTS 10000000

static double sphere_width(glm::dvec3 direction) { return 2. * SPHERE_RADIUS * direction.z; }

int main()
{
  std::vector<glm::dvec3> hammersleySequence = construct_hemisphere_hammersley_sequence(NUM_POINTS);
  std::vector<double> shTerms = calculate_sh_terms(hammersleySequence, sphere_width);
  
  std::cout << "Spherical harmonics expansion terms:\n";
  for (double &term : shTerms)
    std::cout << term << '\n';

  return 0;
}