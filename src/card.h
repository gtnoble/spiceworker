#ifndef SPICE_CARD
#define SPICE_CARD

#include <stdlib.h>
#include "pso.h"

typedef struct {
  char **components;
  size_t num_components;
} ParsedTemplate;

extern const char *k_substitution_key;

ParsedTemplate *parse_template(const char *template, Arena *arena);
char *substitute_template(ParsedTemplate *template, RealVector *vector, Arena *arena);

#endif
