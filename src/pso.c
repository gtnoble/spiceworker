#include <gsl/gsl_rng.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>

#include "sand.h"
#include "pso.h"

const double k_pi = 3.1415926535897932384626433;
static inline double square(double x) {
  return x * x;
}

RealVector *make_real_vector(size_t length, Arena *arena) {
  RealVector *vector = ARENA_ALLOCATE(1, RealVector, arena);
  vector->elements = ARENA_ALLOCATE(length, double, arena);
  vector->length = length;
  return vector;
}

RealVectors *make_real_vectors(size_t num_vectors, Arena *arena) {
  RealVectors *vectors = ARENA_ALLOCATE(1, RealVectors, arena);
  vectors->vectors = ARENA_ALLOCATE(num_vectors, RealVector *, arena);
  vectors->num_vectors = num_vectors;
  return vectors;
}

static inline double sample_normal_distribution_1d(double mean, double standard_deviation, gsl_rng *rng) {
  return mean + 
  (
    cos(2 * k_pi * gsl_rng_uniform(rng)) *
    sqrt(-2 * log(gsl_rng_uniform(rng))) *
    standard_deviation
  );
}

static inline RealVector *get_best_position(RealVectors *particle_positions, RealVector *particle_energies) {
  assert(particle_positions->num_vectors == particle_energies->length);

  RealVector *best_position_so_far = particle_positions->vectors[0];
  double best_position_energy = particle_energies->elements[0];

  for (size_t i = 1; i < particle_positions->num_vectors; i++) {
    double candidate_energy = particle_energies->elements[i];
    RealVector *candidate_position = particle_positions->vectors[i];
    if (isnan(best_position_energy) && ! isnan(candidate_energy)) {
      best_position_so_far = candidate_position;
      best_position_energy = candidate_energy;
    }
    else if (! isnan(best_position_energy) && isnan(candidate_energy)) {
      continue;
    }
    else if (best_position_energy < candidate_energy) {
      continue;
    }
    else {
      best_position_so_far = candidate_position;
      best_position_energy = candidate_energy;
    }
  }
  return best_position_so_far;
}

static inline void update_position(
  RealVector *particle_position,
  const RealVector *global_best_position,
  gsl_rng *rng
) {
  assert(particle_position->length == global_best_position->length);
  
  double difference_norm_squared = 0;

  for (size_t i = 0; i < particle_position->length; i++) {
    double particle_coordinate = particle_position->elements[i];
    double global_best_coordinate = global_best_position->elements[i];

    double mean = (particle_coordinate + global_best_coordinate) * 0.5;

    difference_norm_squared += square(particle_coordinate - global_best_coordinate);
    particle_position->elements[i] = mean;
  }
  
  double standard_deviation = sqrt(difference_norm_squared);

  for (size_t i = 0; i < particle_position->length; i++) {
    double mean = particle_position->elements[i];
    particle_position->elements[i] = sample_normal_distribution_1d(
      mean, standard_deviation, rng
    );
  }
}

RealVector *particle_swarm_optimize(
  RealVectors *positions, 
  ObjectiveFunction objective_function, 
  unsigned long max_iterations,
  gsl_rng *rng,
  Arena *arena
) {
  RealVector *energies = make_real_vector(positions->num_vectors, arena);
  RealVector *best_position;

  assert(positions->num_vectors > 0);

  assert(positions->num_vectors == energies->length);
  objective_function(positions, energies);
  assert(positions->num_vectors == energies->length);

  for (size_t i = 0; i < max_iterations; i++) {
    best_position = get_best_position(positions, energies);
    for (size_t j = 0; j < positions->num_vectors; j++) {
      update_position(positions->vectors[j], best_position, rng);
    }

    assert(positions->num_vectors == energies->length);
    objective_function(positions, energies);
    assert(positions->num_vectors == energies->length);
  }
  return best_position;
  
}

