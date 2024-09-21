#define _POSIX_C_SOURCE 200809L

#include <stdbool.h>
#include <sharedspice.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "sand.h"
#include "vector.h"
#include "spice.h"

const size_t k_default_vector_size = 20;

/* Dvec flags. */
enum dvec_flags {
  VF_REAL = (1 << 0),       /* The data is real. */
  VF_COMPLEX = (1 << 1),    /* The data is complex. */
  VF_ACCUM = (1 << 2),      /* writedata should save this vector. */
  VF_PLOT = (1 << 3),       /* writedata should incrementally plot it. */
  VF_PRINT = (1 << 4),      /* writedata should print this vector. */
  VF_MINGIVEN = (1 << 5),   /* The v_minsignal value is valid. */
  VF_MAXGIVEN = (1 << 6),   /* The v_maxsignal value is valid. */
  VF_PERMANENT = (1 << 7),  /* Don't garbage collect this vector. */
  VF_EVENT_NODE = (1 << 8)  /* Derived from and XSPICE event node. */
};

static int exit_spice(
  int exit_status, 
  bool immediately_unload_dll, 
  bool exit_on_quit, 
  int caller_id,
  void *return_pointer
);

static int send_char(
  char *output,
  int caller_id,
  void *user_data
);

void init_spice(void) {
  ngSpice_Init(
    send_char, 
    NULL, 
    exit_spice, 
    NULL, 
    NULL, 
    NULL, 
    NULL
  );
}

Plots *get_results(Arena *arena);

void populate_circuit(const char *spice_card, Arena *arena) {
  size_t start = arena_checkpoint(arena);
  
  char *spice_card_mutable = arena_sprintf_null(
    arena, 
    "%s", 
    spice_card
  );

  char **lines = ARENA_ALLOCATE(1, char *, arena);
  char *strtok_state;
  char *line;

  size_t line_index = 0;
  while (
    (line = strtok_r(
      line_index == 0 ? spice_card_mutable : NULL, 
      "\n", 
      &strtok_state)
    ) != NULL
  ) {
    lines[line_index] = line;
    ARENA_ALLOCATE(1, char *, arena);
    line_index++;
  }
  lines[line_index] = NULL;
  
  ngSpice_Circ(lines);
  arena_restore(start, arena);
}

Plots *run_simulation(const char *simulation_card, Arena *arena) {

  populate_circuit(simulation_card, arena);
  ngSpice_Command("run");
  return get_results(arena);
}

size_t count_strings(char **strings) {
  size_t count = 0;
  while (strings[count] != NULL) {
    count++;
  }
  return count;
}

Vector *from_spice_vector(pvector_info spice_vector, Arena *arena) {
  bool is_complex = (spice_vector->v_flags & VF_COMPLEX) == VF_COMPLEX;
  bool is_real = (spice_vector->v_flags & VF_REAL) == VF_REAL;
  assert((is_complex && ! is_real) || (is_real && ! is_complex));

  Vector *vector = make_vector(
    spice_vector->v_length, 
    spice_vector->v_name, 
    arena
  );
  
  for (int k = 0; k < spice_vector->v_length; k++) {
    if (is_real) {
      double real_element = spice_vector->v_realdata[k];
      vector->elements[k] = real_element;
    }
    else {
      ngcomplex_t complex_element = spice_vector->v_compdata[k];
      vector->elements[k] = 
        complex_element.cx_real + complex_element.cx_imag * I;
    }
  }
  
  return vector;
}

Plots *get_results(Arena *arena) {
  char **all_plot_names = ngSpice_AllPlots();
  Plots *all_plots = make_plots(
    count_strings(all_plot_names), arena
  );

  for (size_t i = 0; all_plot_names[i] != NULL; i++) {

    char *current_plot_name = all_plot_names[i];
    char **all_vector_names = ngSpice_AllVecs(current_plot_name);

    Plot *current_plot = make_plot(
      count_strings(all_vector_names), 
      current_plot_name, 
      arena
    );
    all_plots->plots[i] = current_plot;

    for (size_t j = 0; j < current_plot->num_vectors; j++) {
      char *current_vector_name = all_vector_names[j];

      char *full_vector_name = arena_sprintf_null(
        arena, 
        "%s.%s", current_plot->name, current_vector_name 
      );

      pvector_info spice_vector = ngGet_Vec_Info(full_vector_name);
      current_plot->vectors[j] = from_spice_vector(spice_vector, arena);

    }
  }

  return all_plots;
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

static int send_char(char *output, int caller_id, void *user_data) {
  (void) caller_id;
  (void) user_data;

  fprintf(stderr, "%s\n", output);
  return EXIT_SUCCESS;
}
