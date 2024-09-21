#include <assert.h>

#include "spice.h"
#include "sand.h"
#include "vector.h"

const char *spice_card =
"* Super simple RC test circuit\n"
"V1 VIN 0 0 AC 1 0\n"
"R1 VOUT VIN 1k\n"
"C1 VOUT 0 159n\n"
".ac dec 10 1 1Meg\n"
".plot ac VOUT\n"
".end\n";

int main(int argc, char *argv[]) {
  (void) argc;
  (void) argv;

  Arena arena = make_arena(100000000);
  Arena plot_scratch = make_arena(100000000);
  Arena vector_scratch = make_arena(100000000);
  Arena element_scratch = make_arena(100000000);
  
  init_spice();
  Plots *result = run_simulation(spice_card, &arena);
  assert(result);

  char *serialized_result = serialize_plots(
    result, 
    &arena, 
    &plot_scratch, 
    &vector_scratch, 
    &element_scratch
  );
  assert(serialized_result);

  return 0;
}


