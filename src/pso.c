#include "sand.h"
#include <gsl/gsl_rng.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>

typedef struct {
  size_t length;
  double *elements;
} RealVector;

typedef struct {
  size_t num_vectors;
  RealVector **vectors;
} RealVectors;

double square(double x) {
  return x * x;
}

typedef void ObjectiveFunction(const RealVectors *positions, RealVector *resulting_energies);

RealVector *make_real_vector(size_t length, Arena *arena) {
  RealVector *vector = ARENA_ALLOCATE(1, RealVector, arena);
  vector->elements = ARENA_ALLOCATE(length, double, arena);
  vector->length = length;
  return vector;
}

double sample_normal_distribution_1d(double mean, double standard_deviation, gsl_rng *rng) {
  return mean + 
  (
    cos(2 * PI * gsl_rng_uniform(rng)) *
    sqrt(-2 * log(gsl_rng_uniform(rng))) *
    standard_deviation
  );
}

RealVector *select_optimum_energy_position(RealVector *position1, double energy_1, RealVector *position2, double energy_2) {
  if (isnan(energy_1) && ! isnan(energy_2)) {
    return position2;
  }
  else if (! isnan(energy_1) && isnan(energy_2)) {
    return position1;
  }
  else if (energy_1 < energy_2) {
    return position1;
  }
  else {
    return position2;
  }
}

double vector_norm(RealVector *vector) {
  double sum = 0;
  for (size_t i = 0; i < vector->length; i++) {
    sum += square(vector->elements[i]);
  }
  return sqrt(sum);
}

RealVector *vector_add(const RealVector *vector1, const RealVector *vector2, Arena *arena) {
  assert(vector1->length == vector2->length);
  RealVector *result = make_real_vector(vector1->length, arena);

  for (size_t i = 0; i < vector1->length; i++) {
    result->elements[i] = vector1->elements[i] + vector2->elements[i];
  }
  return result;
}

RealVector *vector_subtract(const RealVector *vector1, const RealVector *vector2, Arena *arena) {
  assert(vector1->length == vector2->length);
  RealVector *result = make_real_vector(vector1->length, arena);

  for (size_t i = 0; i < vector1->length; i++) {
    result->elements[i] = vector1->elements[i] - vector2->elements[i];
  }
  return result;
}

RealVector *vector_scale(double scalar, const RealVector *vector, Arena *arena) {
  RealVector *result = make_real_vector(vector->length, arena);
  for (size_t i = 0; i < vector->length; i++) {
    result->elements[i] = vector->elements[i] * scalar;
  }
}

RealVector *get_best_position(RealVectors *particle_positions, RealVector *particle_energies) {
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

void update_position(
  RealVector *particle_position,
  const RealVector *global_best_position,
  gsl_rng *rng,
  Arena *scratch
) {
  size_t checkpoint = arena_checkpoint(scratch);
  RealVector *mean = vector_scale(
    .5, 
    vector_add(
      particle_position, 
      global_best_position, 
      scratch
    ), 
    scratch
  );
  double standard_deviation = vector_norm(
  vector_subtract(
    particle_position, 
    global_best_position, 
    scratch)
  );
  
  for (size_t i = 0; i < particle_position->length; i++) {
    particle_position->elements[i] = sample_normal_distribution_1d(
      mean->elements[i], 
      standard_deviation, 
      rng
    );
  }
  arena_restore(checkpoint, scratch);
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
  
  objective_function(positions, energies);
  for (size_t i = 0; i < max_iterations; i++) {
    best_position = get_best_position(positions, energies);
    for (size_t j = 0; j < positions->num_vectors; j++) {
      update_position(positions->vectors[j], best_position, rng, arena);
    }
    objective_function(positions, energies);
  }
  return best_position;
  
}

