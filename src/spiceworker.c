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


const unsigned int k_result_message_priority = 0;

typedef struct {
  unsigned long request_id;
  char *spice_card;
} SpiceRequest;


int main(int argc, char *argv[]) {
  (void) argc;
  (void) argv;

  init_spice();
}

/**
 * @brief Deserialized a request message payload
 * 
 * @param payload message payload
 * @param payload_length message payload length
 * @param arena arena to allocate deserialized data
 * @return SpiceRequest* 
 */
SpiceRequest *deserialize_request_payload(char *payload, size_t payload_length, Arena *arena) {
  payload[payload_length - 1] = '\0';
  SpiceRequest *request = ARENA_ALLOCATE(1, SpiceRequest, arena);
  sscanf(
    payload, 
    "%ld" FIELD_SEPARATOR "%s", 
    &request->request_id, 
    request->spice_card
  );
  return request;
}


SpiceRequest *receive_message(
  mqd_t message_queue, 
  size_t max_message_length, 
  Arena *arena
) {
  
  char *payload = ARENA_ALLOCATE(max_message_length, char, arena);

  ssize_t payload_length = mq_receive(
    message_queue, 
    payload, 
    max_message_length, 
    NULL
  );
  
  if (payload_length == -1) {
    if (errno == EAGAIN) {
      return NULL;
    }
    else {
      fprintf(stderr, "error: could not receive message: %s", strerror(errno));
      exit(EXIT_FAILURE);
    }
  }
  
  return deserialize_request_payload(payload, payload_length, arena);
}

bool send_message(
  char *serialized_result,
  mqd_t message_queue
) {
  if (mq_send(
    message_queue, 
    serialized_result, 
    strlen(serialized_result), 
    k_result_message_priority
  ) == -1) {
    if (errno == EAGAIN) {
      return false;
    }
    else {
      fprintf(stderr, "error: could not send message: %s", strerror(errno));
    }
  }
  return true;
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



