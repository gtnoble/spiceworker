#include <mqueue.h>

#include "sand.h"

typedef struct SpiceVector {
  char *name;
  bool is_scale;
  double complex data[];
}

char *deserialize_response(char *