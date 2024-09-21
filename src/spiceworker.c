#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdbool.h>
#include <sharedspice.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <mqueue.h>
#include <string.h>
#include <sys/poll.h>
#include <errno.h>
#include <complex.h>

#include "sand.h"
#include "serialize.h"
#include "spice.h"

int main(int argc, char *argv[]) {
  (void) argc;
  (void) argv;

  init_spice();
}

void process_messages(mqd_t input_queue, mqd_t output_queue) {
  struct mq_attr input_queue_attributes;
  mq_getattr(input_queue, &input_queue_attributes);
  char *output_payload = NULL;

  struct pollfd fds[] = {
    {.fd = input_queue, .events = POLLIN}, 
    {.fd = output_queue, .events = POLLOUT}
  };
  const size_t input_fd_idx = 0;
  const size_t output_fd_idx = 1;
  
  Arena request_arena = make_arena(100000000);
  
  while (fds[input_fd_idx].fd != -1 && fds[output_fd_idx].fd != -1) {
    poll(fds, 2, -1);
    
    if ((fds[input_fd_idx].revents & POLLIN) == POLLIN && output_payload == NULL) {
      SpiceRequest *request = receive_message(
        input_queue, 
        input_queue_attributes.mq_msgsize, 
        &request_arena
      );
      
      if (request != NULL) {
        populate_circuit(request->spice_card, &request_arena);
        ngSpice_Command("run");
        output_payload = serialize_result(request->request_id, most_recent_values, &request_arena);
      }
    }
    
    if ((fds[output_fd_idx].revents & POLLOUT) == POLLOUT && output_payload != NULL) {
      if (send_message(output_payload, output_queue)) {
        output_payload = NULL;
        arena_reset(&request_arena);
      }
    }

  }

}



