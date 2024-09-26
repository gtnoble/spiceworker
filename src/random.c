#include <gsl/gsl_rng.h>

#include "sand.h"

gsl_rng *make_rng(const gsl_rng_type *rng_type, Arena *arena) {
  gsl_rng *rng = ARENA_ALLOCATE(1, gsl_rng, arena);
  rng->state = arena_allocate(rng_type->size, arena);
  rng->type = rng_type;
  gsl_rng_set(rng, gsl_rng_default_seed);
  return rng;
}
