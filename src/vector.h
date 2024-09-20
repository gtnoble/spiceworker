#ifndef SPICE_VECTOR
#define SPICE_VECTOR

#include <complex.h>
#include <stdlib.h>
#include "sand.h"

typedef struct {
  complex double *elements;
  size_t length;
  size_t max_length;
} Vector;

typedef struct {
  Vector **vectors;
  char **names;
  size_t num_vectors;
} Vectors;

Vector *make_vector(size_t max_length, Arena *arena);
void vector_push(complex double element, Vector *vector, Arena *arena);

char *serialize_vector(
  const Vector *vector, 
  Arena *arena
);

/**
 * @brief Serializes spice result vectors into a message payload
 * 
 * @param id Request ID
 * @param values Spice result vectors
 * @param arena Arena to allocate payload memory
 * @return message payload
 */
char *serialize_vectors(
  const Vectors *vectors, 
  Arena *arena
);

#endif

