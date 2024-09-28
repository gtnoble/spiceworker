#include <assert.h>
#include <stdlib.h>

#include "card.h"
#include "sand.h"
#include "vector.h"
#include "pso.h"
#include "spice.h"

ParsedTemplate *template;
Arena *arena;

RealVector *complex_magnitudes(Vector *vector, Arena *arena) {
  RealVector *magnitudes = make_real_vector(vector->length, arena);
  for (size_t i = 0; i < vector->length; i++) {
    magnitudes->elements[i] = cabs(vector->elements[i]);
  }
  return magnitudes;
}

double minimum(RealVector *vector) {
  assert(vector->length > 0);

  double minimum = vector->elements[0];
  for (size_t i = 0; i < vector->length; i++) {
    if (vector->elements[i] < minimum) {
      minimum = vector->elements[i];
    }
  }
  return minimum;
}

double evaluate(RealVector *position) {
  size_t checkpoint = arena_checkpoint(arena);

  char *card = substitute_template(template, position, arena);
  Plots *plots = run_simulation(card, arena);
  Plot *ac_plot = lookup_plot("ac1", plots);
  Vector *vout = lookup_vector("vout", ac_plot);
  double result = -minimum(complex_magnitudes(vout, arena));
  
  arena_restore(checkpoint, arena);
  return result;
}

void objective(RealVectors *positions, RealVector *energies) {
  assert(positions->num_vectors == energies->length);
  
  for (size_t i = 0; i < positions->num_vectors; i++) {
    energies->elements[i] = evaluate(positions->vectors[i]);
  }
}


