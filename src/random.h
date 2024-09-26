#include <gsl/gsl_rng.h>
#include "sand.h"

gsl_rng *make_rng(const gsl_rng_type *rng_type, Arena *arena);
