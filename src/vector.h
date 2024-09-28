#ifndef SPICE_VECTOR
#define SPICE_VECTOR

#include <complex.h>
#include <stdlib.h>
#include "sand.h"
#include "cwpack.h"

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

Plot *lookup_plot(const char *name, const Plots *plots);
Vector *lookup_vector(const char *name, const Plot *plot);

char *serialize_vector(
  const Vector *vector, 
  Arena *arena,
  Arena *element_scratch
);
void serialize_vector_mpack(
  const Vector *vector, 
  cw_pack_context *mpack_context
);
Vector *deserialize_vector_mpack(cw_unpack_context *mpack_context, Arena *arena);

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
void serialize_plot_mpack(const Plot *plot, cw_pack_context *mpack_context);
Plot *deserialize_plot_mpack(cw_unpack_context *mpack_context, Arena *arena);

char *serialize_plots(
  const Plots *plots,
  Arena *arena,
  Arena *plot_scratch,
  Arena *vector_scratch,
  Arena *element_scratch
);
void serialize_plots_mpack(const Plots *plots, cw_pack_context *mpack_context);
Plots *deserialize_plots_mpack(cw_unpack_context *mpack_context, Arena *arena);

void serialize_complex_number_mpack(double complex number, cw_pack_context *mpack_context);
double complex deserialize_complex_number_mpack(cw_unpack_context *mpack_context);

#endif

