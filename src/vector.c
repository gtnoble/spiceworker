#include <string.h>
#include <assert.h>
#include <complex.h>

#include "sand.h"
#include "vector.h"


Vector *make_vector(size_t length, const char *name, Arena *arena) {
  Vector *vector = ARENA_ALLOCATE(1, Vector, arena);
  vector->elements = ARENA_ALLOCATE(length, complex double, arena);
  vector->length = length;
  vector->name = arena_strdup(arena, name);
  return vector;
}

Plot *make_plot(size_t num_vectors, const char *name, Arena *arena) {
  Plot *plot = ARENA_ALLOCATE(1, Plot, arena);
  plot->vectors = ARENA_ALLOCATE(num_vectors, Vector *, arena);
  plot->num_vectors = num_vectors;
  plot->name = arena_strdup(arena, name);
  return plot;
}

Plots *make_plots(size_t num_plots, Arena *arena) {
  Plots *plots = ARENA_ALLOCATE(1, Plots, arena);
  plots->num_plots = num_plots;
  plots->plots = ARENA_ALLOCATE(num_plots, Plot *, arena);
  return plots;
}

char *serialize_plots(
  const Plots *plots,
  Arena *arena,
  Arena *plot_scratch,
  Arena *vector_scratch,
  Arena *element_scratch
 ) {
  size_t checkpoint = arena_checkpoint(plot_scratch);

  char *payload;
  for (size_t i = 0; i < plots->num_plots; i++) {
    char *plot = serialize_plot(
      plots->plots[i], 
      plot_scratch, 
      vector_scratch, 
      element_scratch
    );
    if (i == 0) {
      payload = arena_sprintf(arena, "%s", plot);
    }
    else {
      arena_sprintf(arena, PLOTS_SEPARATOR "%s", plot);
    }
  }
  arena_sprint_null(arena);

  arena_restore(checkpoint, plot_scratch);
  return payload;
}

/**
 * @brief Serializes spice result vectors into a message payload
 * 
 * @param id Request ID
 * @param values Spice result vectors
 * @param arena Arena to allocate payload memory
 * @return message payload
 */
char *serialize_plot(
  const Plot *plot, 
  Arena *arena,
  Arena *vector_scratch,
  Arena *element_scratch
) {

  size_t checkpoint = arena_checkpoint(vector_scratch);

  char *payload = arena_sprintf(arena, "%s", plot->name);
  for (size_t ii = 0; ii < plot->num_vectors; ii++) {
    char *vector = serialize_vector(
      plot->vectors[ii], 
      vector_scratch,
      element_scratch
    );

    arena_sprintf(
      arena, 
      PLOT_SEPARATOR "%s", 
      vector
    );

  }
  arena_sprint_null(arena); 

  arena_restore(checkpoint, vector_scratch);
  return payload;
}

char *serialize_complex_number(double complex number, Arena *arena) {
  return arena_sprintf_null(
    arena, 
    "%e+%ei", 
    creal(number), 
    cimag(number)
  );
}

char *serialize_vector(
  const Vector *vector, 
  Arena *arena,
  Arena *element_scratch
) {
  assert(vector->length > 0);

  size_t scratch_checkpoint = arena_checkpoint(element_scratch);

  char *serialized_vector = arena_sprintf(arena, "%s", vector->name);
  for (size_t i = 0; i < vector->length; i++) {
    arena_sprintf(
      arena, 
      VECTOR_SEPARATOR "%s", 
      serialize_complex_number(vector->elements[i], element_scratch)
    );
  }
  arena_sprint_null(arena);

  arena_restore(scratch_checkpoint, element_scratch);
  return serialized_vector;
}
