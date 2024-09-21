#ifndef SPICE_VECTOR
#define SPICE_VECTOR

#include <complex.h>
#include <stdlib.h>
#include "sand.h"

#define VECTOR_SEPARATOR "␟"
#define PLOT_SEPARATOR "␞"
#define PLOTS_SEPARATOR "␝"
#define ELEMENT_SEPARATOR ","
typedef struct {
  char *name;
  complex double *elements;
  size_t length;
} Vector;

typedef struct {
  char *name;
  Vector **vectors;
  size_t num_vectors;
} Plot;

typedef struct {
  Plot **plots;
  size_t num_plots;
} Plots;

Plots *make_plots(size_t num_plots, Arena *arena);
Plot *make_plot(size_t num_vectors, const char *name, Arena *arena);
Vector *make_vector(size_t length, const char *name, Arena *arena);

char *serialize_vector(
  const Vector *vector, 
  Arena *arena,
  Arena *element_scratch
);

/**
 * @brief Serializes spice result vectors into a message payload
 * 
 * @param id Request ID
 * @param values Spice result vectors
 * @param arena Arena to allocate payload memory
 * @return message payload
 */
char *serialize_plot(
  const Plot *vectors, 
  Arena *arena,
  Arena *vector_scratch,
  Arena *element_scratch
);

char *serialize_plots(
  const Plots *plots,
  Arena *arena,
  Arena *plot_scratch,
  Arena *vector_scratch,
  Arena *element_scratch
);

#endif

