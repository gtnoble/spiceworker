#include <assert.h>
#include <string.h>

#include "vector.h"
#include "sand.h"
#include "serialize.h"

int main(int argc, char *argv[]) {
  (void) argc;
  (void) argv;

  Arena arena = make_arena(10000);
  Vector *vector = make_vector(3, "derp", &arena);
  assert(vector->length == 3);
  assert(strcmp(vector->name, "derp") == 0);
  
  vector->elements[0] = 69;
  vector->elements[1] = 420;
  vector->elements[2] = 666;
  
  Arena plot_scratch = make_arena(1000000);
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
  
  
  
  
  return 0; 
}
