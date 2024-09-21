#ifndef SPICE_SPICE
#define SPICE_SPICE

#include "vector.h"
#include "sand.h"


void init_spice(void);
Plots *run_simulation(const char *simulation_card, Arena *arena);
#endif
