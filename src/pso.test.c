#include <gsl/gsl_rng.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#include "pso.h"
#include "sand.h"
#include "random.h"

static inline double square(double x);

void objective(
  const RealVectors *particle_positions, 
  RealVector *particle_energies
);

const double initial_positions[] = {-1.0, -2.0, 4.0, 6.0};
const unsigned long k_max_iterations = 1000000;

int main(int argc, char *argv[]) {
  (void) argc;
  (void) argv;

  Arena arena = make_arena(10000000);

  RealVectors *particle_positions = make_real_vectors(4, &arena);
  for (size_t i = 0; i < 4; i++) {
    RealVector *position = make_real_vector(1, &arena);
    position->elements[0] = initial_positions[i];
    particle_positions->vectors[i] = position;
  }
  
  gsl_rng *rng = make_rng(gsl_rng_mt19937, &arena);
  
  RealVector *result = particle_swarm_optimize(
    particle_positions, 
    objective,
    k_max_iterations, 
    rng, 
    &arena
  );
  
  assert(fabs(result->elements[0]) < 0.01);


}

void objective(
  const RealVectors *particle_positions, 
  RealVector *particle_energies
) {
 for (size_t i = 0; i < particle_positions->num_vectors; i++) {
  particle_energies->elements[i] = square(particle_positions->vectors[i]->elements[0]);
 }
}

static inline double square(double x) {
  return x * x;
}
