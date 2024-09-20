#include <assert.h>
#include <string.h>

#include "vector.h"
#include "sand.h"

int main(int argc, char *argv[]) {
  (void) argc;
  (void) argv;

  Arena arena = make_arena(10000);
  Vector *vector = make_vector(2, &arena);
  
  assert(vector->length == 0);
  assert(vector->max_length = 2);
  
  vector_push(69, vector, &arena);
  assert(vector->length == 1);
  assert(vector->max_length == 2);
  assert(vector->elements[0] == 69);
  
  vector_push(420, vector, &arena);
  vector_push(666, vector, &arena);
  
  assert(vector->length == 3);
  assert(vector->max_length == 4);
  assert(vector->elements[0] == 69);
  assert(vector->elements[1] == 420);
  assert(vector->elements[2] == 666);
  
  Arena element_scratch = make_arena(1000000);
  
  char *serialized_vector = serialize_vector(vector, &arena, &element_scratch);
  
  assert(
    strcmp(
      serialized_vector, 
    "6.900000e+01+0.000000e+00i,4.200000e+02+0.000000e+00i,6.660000e+02+0.000000e+00i"
    ) == 0
  );
  
  return 0; 
}
