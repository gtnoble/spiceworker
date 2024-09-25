#include <assert.h>
#include <complex.h>
#include <string.h>

#include "vector.h"
#include "cwpack.h"
#include "sand.h"

double k_tolerance = 0.0001;

typedef struct {
  cw_pack_context context;
  char *buffer;
} SerializeMpack;

typedef struct {
  cw_unpack_context context;
  char *buffer;
} DeserializeMpack;

static bool complex_equal(double complex x, double complex y);
static bool vector_equal(Vector *x, Vector *y);
static bool plot_equal(Plot *x, Plot *y);
static bool plots_equal(Plots *x, Plots *y);

void test_mpack_serialization(Arena *arena);
void test_complex_mpack_serialization(Arena *arena);
void test_vector_mpack_serialization(Arena *arena);
void test_plot_mpack_serialization(Arena *arena);
void test_plots_mpack_serialization(Arena *arena);

SerializeMpack *make_serialize_mpack(size_t buffer_length, Arena *arena);
DeserializeMpack *make_deserialize_mpack(SerializeMpack *serialize, Arena *arena);

int main(int argc, char *argv[]) {
  (void) argc;
  (void) argv;

  Arena arena = make_arena(100000);
  Vector *vector = make_vector(3, "derp", &arena);
  assert(vector->length == 3);
  assert(strcmp(vector->name, "derp") == 0);
  
  vector->elements[0] = 69;
  vector->elements[1] = 420;
  vector->elements[2] = 666;
  
  Arena vector_scratch = make_arena(1000000);
  Arena element_scratch = make_arena(1000000);
  
  char *serialized_vector = serialize_vector(vector, &arena, &element_scratch);
  
  const char *expected_serialized_vector =
  "derp" VECTOR_SEPARATOR 
  "6.900000e+01+0.000000e+00i" VECTOR_SEPARATOR 
  "4.200000e+02+0.000000e+00i" VECTOR_SEPARATOR 
  "6.660000e+02+0.000000e+00i";
  
  assert(
    strcmp(serialized_vector, expected_serialized_vector) == 0
  );

  Plot *plot = make_plot(2, "purp", &arena);
  plot->vectors[0] = vector;
  plot->vectors[1] = vector;
  
  char *expected_serialized_plot = 
    arena_sprintf_null(
      &arena, 
      "%s" PLOT_SEPARATOR "%s" PLOT_SEPARATOR "%s", 
      plot->name, expected_serialized_vector, expected_serialized_vector
    );

  char *serialized_plot = serialize_plot(
    plot, 
    &arena, 
    &vector_scratch, 
    &element_scratch
  );
  
  assert(
    strcmp(serialized_plot, expected_serialized_plot) == 0
  );
  
  arena_reset(&arena);
  
  test_mpack_serialization(&arena);
  
  
  
  
  return 0; 
}

void test_mpack_serialization(Arena *arena) {
  (void) plots_equal;

  test_complex_mpack_serialization(arena);
  test_vector_mpack_serialization(arena);
  test_plot_mpack_serialization(arena);
  test_plots_mpack_serialization(arena);
}

void test_complex_mpack_serialization(Arena *arena) {
  
  double complex test_complex = 69 + 420 * I;
  
  const size_t k_buffer_size = 10000;
  SerializeMpack *serialize = make_serialize_mpack(k_buffer_size, arena);
  serialize_complex_number_mpack(test_complex, &serialize->context);

  DeserializeMpack *deserialize = make_deserialize_mpack(serialize, arena);
  double complex reconstructed_complex = 
    deserialize_complex_number_mpack(&deserialize->context);
  assert(complex_equal(test_complex, reconstructed_complex));

}

Vector *make_test_vector(Arena *arena) {
  Vector *vector = make_vector(3, "derp", arena);
  assert(vector->length == 3);
  assert(strcmp(vector->name, "derp") == 0);
  
  vector->elements[0] = 69;
  vector->elements[1] = 420;
  vector->elements[2] = 666;
  
  return vector;
}

void test_vector_mpack_serialization(Arena *arena) {

  Vector *vector = make_test_vector(arena);

  const size_t k_buffer_size = 10000;
  SerializeMpack *serialize = make_serialize_mpack(k_buffer_size, arena);
  serialize_vector_mpack(vector, &serialize->context);

  DeserializeMpack *deserialize = make_deserialize_mpack(serialize, arena);
  Vector *reconstructed_vector = 
    deserialize_vector_mpack(&deserialize->context, arena);
  assert(vector_equal(vector, reconstructed_vector));
}

Plot *make_test_plot(Arena *arena) {
  Vector *vector = make_test_vector(arena);

  Plot *plot = make_plot(2, "purp", arena);
  plot->vectors[0] = vector;
  plot->vectors[1] = vector;
  return plot;
}

void test_plot_mpack_serialization(Arena *arena) {

  Plot *plot = make_test_plot(arena);
  
  const size_t k_buffer_size = 10000;
  SerializeMpack *serialize = make_serialize_mpack(k_buffer_size, arena);
  serialize_plot_mpack(plot, &serialize->context);
  
  DeserializeMpack *deserialize = make_deserialize_mpack(serialize, arena);
  Plot *reconstructed_plot = deserialize_plot_mpack(&deserialize->context, arena);
  assert(plot_equal(plot, reconstructed_plot));
}

void test_plots_mpack_serialization(Arena *arena) {
  
  Plots *plots = make_plots(2, arena);
  Plot *plot = make_test_plot(arena);
  plots->plots[0] = plot;
  plots->plots[1] = plot;

  const size_t k_buffer_size = 10000;
  SerializeMpack *serialize = make_serialize_mpack(k_buffer_size, arena);
  serialize_plots_mpack(plots, &serialize->context);
  
  DeserializeMpack *deserialize = make_deserialize_mpack(serialize, arena);
  Plots *reconstructed_plots = deserialize_plots_mpack(&deserialize->context, arena);
  assert(plots_equal(plots, reconstructed_plots));
}

SerializeMpack *make_serialize_mpack(size_t buffer_length, Arena *arena) {
  SerializeMpack *serialize = ARENA_ALLOCATE(1, SerializeMpack, arena);
  serialize->buffer = ARENA_ALLOCATE(buffer_length, char, arena);
  cw_pack_context_init(
    &serialize->context, 
    serialize->buffer, 
    buffer_length, 
    NULL
  );
  return serialize;
}

DeserializeMpack *make_deserialize_mpack(SerializeMpack *serialize, Arena *arena) {
  DeserializeMpack *deserialize = ARENA_ALLOCATE(1, DeserializeMpack, arena);
  deserialize->buffer = (char *) serialize->context.start;
  cw_unpack_context_init(
    &deserialize->context, 
    deserialize->buffer, 
    serialize->context.end - serialize->context.start, 
    NULL
  );
  return deserialize;
}


static bool complex_equal(double complex x, double complex y) {
  return cabs(x - y) < k_tolerance;
}

static bool vector_equal(Vector *x, Vector *y) {
  bool names_equal = strcmp(x->name, y->name) == 0;
  bool lengths_equal = x->length == y->length;
  if (! (names_equal && lengths_equal)) {
    return false;
  }

  for (size_t i = 0; i < x->length; i++) {
    if (! (complex_equal(x->elements[i], y->elements[i]))) {
      return false;
    }
  }
  return true;
}

static bool plot_equal(Plot *x, Plot *y) {
  bool names_equal = strcmp(x->name, y->name) == 0;
  bool lengths_equal = x->num_vectors == y->num_vectors;
  if (! (names_equal && lengths_equal)) {
    return false;
  }
  
  for (size_t i = 0; i < x->num_vectors; i++) {
    if (! (vector_equal(x->vectors[i], y->vectors[i]))) {
      return false;
    }
  }
  return true;
}

static bool plots_equal(Plots *x, Plots *y) {
  bool lengths_equal = x->num_plots == y->num_plots;
  if (! lengths_equal) {
    return false;
  }
  
  for (size_t i = 0; i < x->num_plots; i++) {
    if (! (plot_equal(x->plots[i], y->plots[i]))) {
      return false;
    }
  }
  return true;
}
