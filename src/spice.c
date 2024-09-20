#define _POSIX_C_SOURCE 200809L

#include <stdbool.h>
#include <sharedspice.h>
#include <string.h>

#include "vector.h"

const size_t k_default_vector_size = 20;

Vectors *simulation_results = NULL;
Arena *simulation_arena = NULL;

static int exit_spice(
  int exit_status, 
  bool immediately_unload_dll, 
  bool exit_on_quit, 
  int caller_id,
  void *return_pointer
);
static int send_data(
  struct vecvaluesall *values, 
  int number_of_vectors, 
  int caller_id, 
  void *user_data
);

void init_spice(void) {
  ngSpice_Init(NULL, NULL, exit_spice, send_data, NULL, NULL, NULL);
}

void populate_circuit(char *spice_card, Arena *arena) {
  size_t start = arena_checkpoint(arena);

  char **lines = ARENA_ALLOCATE(1, char *, arena);

  char *strtok_state;
  char *line;

  size_t line_index = 0;
  while ((line = strtok_r(spice_card, "\n", &strtok_state))) {
    lines[line_index] = line;
    ARENA_ALLOCATE(1, char *, arena);
    line_index++;
  }
  lines[line_index] = NULL;
  
  ngSpice_Circ(lines);
  arena_restore(start, arena);
}

Vectors *run_simulation(char *simulation_card, Arena *arena) {
  simulation_arena = arena;
  simulation_results = ARENA_ALLOCATE(1, Vectors, arena);
  simulation_results->num_vectors = 0;
  simulation_results->vectors = NULL;

  populate_circuit(simulation_card, arena);
  ngSpice_Command("run");
  return simulation_results;
}



static int exit_spice(
  int exit_status, 
  bool immediately_unload_dll, 
  bool exit_on_quit, 
  int caller_id,
  void *return_pointer
) {
  
  (void) immediately_unload_dll;
  (void) exit_on_quit;
  (void) caller_id;
  (void) return_pointer;

  exit(exit_status);
  return exit_status;
}

static int send_data(
  struct vecvaluesall *values, 
  int number_of_vectors, 
  int caller_id, 
  void *user_data
) {
  

  (void) caller_id;
  (void) user_data;

  if (simulation_results->vectors == NULL) {
    simulation_results->vectors = 
      ARENA_ALLOCATE(number_of_vectors, Vector *, simulation_arena);

    for (int ii = 0; ii < number_of_vectors; ii++) {
      simulation_results->vectors[ii] = make_vector(
        k_default_vector_size, 
        simulation_arena
      );
      simulation_results->names[ii] = arena_strdup(
        simulation_arena, 
        values->vecsa[ii]->name
      );
    }
  }
  
  for (int ii = 0; ii < number_of_vectors; ii++) {
    pvecvalues spice_vector_element = values->vecsa[ii];

    complex double element_value;
    if (spice_vector_element->is_complex)
      element_value = 
        spice_vector_element->creal + spice_vector_element->cimag * I;
    else 
      element_value = spice_vector_element->creal;

    vector_push( 
      element_value,
      simulation_results->vectors[ii],
      simulation_arena
    );
  }
  
  return EXIT_SUCCESS;
}
