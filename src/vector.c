#include <string.h>
#include <assert.h>

#include "sand.h"
#include "serialize.h"
#include "vector.h"


Vector *make_vector(size_t max_length, Arena *arena) {
  Vector *vector = ARENA_ALLOCATE(1, Vector, arena);
  vector->elements = ARENA_ALLOCATE(max_length, complex double, arena);
  vector->max_length = max_length;
  vector->length = 0;
  return vector;
}


void vector_push(complex double element, Vector *vector, Arena *arena) {
  assert(vector->length <= vector->max_length);

  if (vector->length == vector->max_length) {
    size_t new_max_length = vector->max_length * 2;
    complex double *new_elements = ARENA_ALLOCATE(new_max_length, complex double, arena);
    memcpy(new_elements, vector->elements, sizeof(complex double) * vector->length);
    vector->max_length = new_max_length;
  }
    vector->elements[vector->length] = element;
    vector->length++;
}

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
) {

  char *payload;
  Arena scratch = make_arena(100000000);
  for (size_t ii = 0; ii < vectors->num_vectors; ii++) {
    char *name = vectors->names[ii];
    char *elements = serialize_vector(
      vectors->vectors[ii], 
      &scratch
    );

    if (ii == 0)
      payload = arena_sprintf(
        arena, 
        VECTOR_SEPARATOR "%s" FIELD_SEPARATOR "%s", 
        name,
        elements
      );
    else
      arena_sprintf(
        arena, 
        FIELD_SEPARATOR "%s", 
        elements
      );

    arena_reset(&scratch);
  }
  
  return payload;
}

char *serialize_complex_number(double complex number, Arena *arena) {
  return arena_sprintf(
    arena, 
    "%e + %ei", 
    creal(number), 
    cimag(number)
  );
}

char *serialize_vector(
  const Vector *vector, 
  Arena *arena
) {
  assert(vector->length > 0);
  char *serialized_elements = serialize_complex_number(
    vector->elements[0], arena
  );

  Arena scratch = make_arena(1000000);
  for (size_t i = 1; i < vector->length; i++) {
    arena_sprintf(
      arena, 
      ELEMENT_SEPARATOR "%s", 
      serialize_complex_number(vector->elements[i], &scratch)
    );
    arena_reset(&scratch);
  }

  return serialized_elements;
}
