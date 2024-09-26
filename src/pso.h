#include <gsl/gsl_rng.h>
#include <stdlib.h>

#include "sand.h"

typedef struct {
  size_t length;
  double *elements;
} RealVector;

typedef struct {
  size_t num_vectors;
  RealVector **vectors;
} RealVectors;

typedef void ObjectiveFunction(
  const RealVectors *positions, 
  RealVector *resulting_energies
);

RealVector *particle_swarm_optimize(
  RealVectors *positions, 
  ObjectiveFunction objective_function, 
  unsigned long max_iterations,
  gsl_rng *rng,
  Arena *arena
);

RealVector *make_real_vector(size_t length, Arena *arena);
RealVectors *make_real_vectors(size_t num_vectors, Arena *arena);
