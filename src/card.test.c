#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "card.h"
#include "pso.h"
#include "sand.h"

const char *template = "herp%sderp%spopp%s";
const char *template_actual = "herp%ederp%epopp%e";
const size_t length = 3;
const double substututed_numbers[] = {420.0, 69.0, 666.0};

int main(int argc, char *argv[]) {
  (void) argc;
  (void) argv;
  
  Arena arena = make_arena(100000);
  
  RealVector *vector = make_real_vector(length, &arena);

  memcpy(
    vector->elements, 
    substututed_numbers, 
    vector->length * sizeof(double)
  );
  
  ParsedTemplate *parsed_template = parse_template(template, &arena);
  char *substituted_template = substitute_template(
    parsed_template, 
    vector, 
    &arena
  );
  
  char *expected_template = arena_sprintf_null(
    &arena, 
    template_actual, 
    substututed_numbers[0],
    substututed_numbers[1],
    substututed_numbers[2]
  );

  assert(strcmp(substituted_template, expected_template) == 0);
  return 0;
}
