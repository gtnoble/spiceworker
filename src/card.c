#define _POSIX_C_SOURCE 200809L
#include <string.h>

#include "sand.h"
#include "vector.h"
#include "string.h"

const char *k_substitution_key = "%s";

typedef struct {
  char **components;
  size_t num_components;
} ParsedTemplate;

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

char *substitute_template(ParsedTemplate *template, Vector *vector, Arena *arena) {

  char *substituted;
  for (size_t i = 0; i < template->num_components; i++) {
    char *next_component = template->components[i];
    if (i == 0) {
      substituted = arena_sprintf(arena, "%s%e", next_component, arena);
    }

  }
  

}
