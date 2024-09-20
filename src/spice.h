#ifndef SPICE_SPICE
#define SPICE_SPICE

#include "vector.h"
#include "sand.h"

void init_spice(void);
Vectors *run_simulation(char *simulation_card, Arena *arena);
#endif
