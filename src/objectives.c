#include <assert.h>

#include "sand.h"
#include "vector.h"

size_t vector_arg_min(Vector *vector) {
  assert(vector->length > 0);
  
  size_t min_i = 0;
  double min = vector->elements[min_i];
  for (size_t i = 0; i < vector->length; i++) {
    double candidate = vector->elements[i];
    if (candidate < min) {
      min = candidate;
      min_i = i;
    }
  }
  return min_i;
}

double vector_min(Vector *vector) {
  return vector->elements[vector_arg_min(vector)];
}



