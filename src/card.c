#define _POSIX_C_SOURCE 200809L
#include <string.h>

#include "sand.h"
#include "pso.h"
#include "card.h"

const char *k_substitution_key = "%s";

ParsedTemplate *parse_template(const char *template, Arena *arena) {
  char *tokenized_template = arena_strdup(arena, template);
  char *saveptr;
  
  char *component;
  char **components = NULL;
  size_t num_components = 0;
  while (
    (
      component = strtok_r(
      tokenized_template, 
      k_substitution_key, 
      &saveptr)
    ) != NULL
  ) {
    if (tokenized_template != NULL) {
      components = ARENA_ALLOCATE(1, char *, arena);
      *components = component;
      tokenized_template = NULL;
    }
    else {
      ARENA_PUSH(component, arena);
    }
    num_components++;
  }
  
  ParsedTemplate *parsed_template = ARENA_ALLOCATE(1, ParsedTemplate, arena);
  parsed_template->components = components;
  parsed_template->num_components = num_components;
  return parsed_template;
}

char *substitute_template(ParsedTemplate *template, RealVector *vector, Arena *arena) {

  char *substituted = arena_sprintf(arena, "");
  for (size_t i = 0; i < template->num_components; i++) {
    arena_sprintf(arena, "%s", template->components[i]);
    if (i < vector->length) {
      arena_sprintf(arena, "%e", vector->elements[i]);
    }
  }
  arena_sprintf_null(arena, "");
  return substituted;
}
