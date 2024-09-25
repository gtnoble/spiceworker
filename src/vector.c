#include <string.h>
#include <assert.h>
#include <complex.h>

#include "cwpack.h"
#include "cwpack_utils.h"
#include "sand.h"
#include "vector.h"

const char *k_vector_name_key = "name";
const char *k_vector_elements_key = "elements";

const char *k_plot_name_key = "name";
const char *k_plot_vectors_key = "plots";

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

void unpack_and_validate_key(cw_unpack_context *mpack_context, const char *key);

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

void serialize_plots_mpack(const Plots *plots, cw_pack_context *mpack_context) {
  cw_pack_array_size(mpack_context, plots->num_plots);
  for (size_t i = 0; i < plots->num_plots; i++) {
    serialize_plot_mpack(plots->plots[i], mpack_context);
  }
}

Plots *deserialize_plots_mpack(cw_unpack_context *mpack_context, Arena *arena) {
  size_t num_plots = cw_unpack_next_array_size(mpack_context);
  Plots *plots = make_plots(num_plots, arena);
  for (size_t i = 0; i < num_plots; i++) {
    plots->plots[i] = deserialize_plot_mpack(mpack_context, arena);
  }
  return plots;
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

void serialize_plot_mpack(const Plot *plot, cw_pack_context *mpack_context) {
  cw_pack_map_size(mpack_context, 2);
  
  cw_pack_str(mpack_context, k_plot_name_key, strlen(k_plot_name_key));
  cw_pack_str(mpack_context, plot->name, strlen(plot->name));
  
  cw_pack_str(mpack_context, k_plot_vectors_key, strlen(k_plot_vectors_key));
  cw_pack_array_size(mpack_context, plot->num_vectors);
  for (size_t i = 0; i < plot->num_vectors; i++) {
    serialize_vector_mpack(plot->vectors[i], mpack_context);
  }
}

Plot *deserialize_plot_mpack(cw_unpack_context *mpack_context, Arena *arena) {
  unsigned int map_size = cw_unpack_next_map_size(mpack_context);
  assert(map_size == 2);

  unpack_and_validate_key(mpack_context, k_vector_name_key);
  size_t name_length = cw_unpack_next_str_lengh(mpack_context);
  char *name = arena_strndup(
    arena, 
    mpack_context->item.as.str.start, 
    name_length
  );
  
  unpack_and_validate_key(mpack_context, k_plot_vectors_key);
  size_t num_vectors = cw_unpack_next_array_size(mpack_context);
  Plot *plot = make_plot(num_vectors, name, arena);

  
  for (size_t i = 0; i < num_vectors; i++) {
    plot->vectors[i] = deserialize_vector_mpack(mpack_context, arena);
  }

  return plot;
}

char *serialize_complex_number(double complex number, Arena *arena) {
  return arena_sprintf_null(
    arena, 
    "%e+%ei", 
    creal(number), 
    cimag(number)
  );
}

void serialize_complex_number_mpack(double complex number, cw_pack_context *mpack_context) {
  cw_pack_array_size(mpack_context, 2);
  cw_pack_double(mpack_context, creal(number));
  cw_pack_double(mpack_context, cimag(number));
}

double complex deserialize_complex_number_mpack(cw_unpack_context *mpack_context) {
  assert(cw_unpack_next_array_size(mpack_context) == 2);
  double real = cw_unpack_next_double(mpack_context);
  double imag = cw_unpack_next_double(mpack_context);
  return real + imag * I;
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

Vector *deserialize_vector_mpack(cw_unpack_context *mpack_context, Arena *arena) {
  size_t vector_map_size = cw_unpack_next_map_size(mpack_context);
  assert(vector_map_size == 2);

  unpack_and_validate_key(mpack_context, k_vector_name_key);
  size_t name_length = cw_unpack_next_str_lengh(mpack_context);
  char *name = arena_strndup(
    arena, 
    mpack_context->item.as.str.start, 
    name_length
  );

  unpack_and_validate_key(mpack_context, k_vector_elements_key);
  size_t num_elements = cw_unpack_next_array_size(mpack_context);
  Vector *vector = make_vector(num_elements, name, arena);
  
  for (size_t i = 0; i < num_elements; i++) {
    vector->elements[i] = deserialize_complex_number_mpack(mpack_context);
  }
  
  return vector;
}

void serialize_vector_mpack(const Vector *vector, cw_pack_context *mpack_context) {
  cw_pack_map_size(mpack_context, 2);

  cw_pack_str(mpack_context, k_vector_name_key,  strlen(k_vector_name_key));
  cw_pack_str(mpack_context, vector->name, strlen(vector->name));

  cw_pack_str(mpack_context, k_vector_elements_key, strlen(k_vector_elements_key));
  cw_pack_array_size(mpack_context, vector->length);
  for (size_t i = 0; i < vector->length; i++) {
    serialize_complex_number_mpack(vector->elements[i], mpack_context);
  }
}

void unpack_and_validate_key(cw_unpack_context *mpack_context, const char *key) {
  size_t key_length = cw_unpack_next_str_lengh(mpack_context);
  assert(key_length == strlen(key));
  assert(strncmp(mpack_context->item.as.str.start, key, key_length) == 0);
}
