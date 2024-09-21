#include <fcntl.h>
#include <stdio.h>
#include <mqueue.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "sand.h"

#define FIELD_SEPARATOR "␝"

const unsigned int k_result_message_priority = 0;

typedef struct {
  unsigned long request_id;
  char *spice_card;
} SpiceRequest;

/**
 * @brief Deserialized a request message payload
 * 
 * @param payload message payload
 * @param payload_length message payload length
 * @param arena arena to allocate deserialized data
 * @return SpiceRequest* 
 */
SpiceRequest *deserialize_request_payload(char *payload, Arena *arena) {
  SpiceRequest *request = ARENA_ALLOCATE(1, SpiceRequest, arena);
  sscanf(
    payload, 
    "%ld" FIELD_SEPARATOR "%s", 
    &request->request_id, 
    request->spice_card
  );
  return request;
}

mqd_t open_receive_mq(const char *mq_name) {
  
  mqd_t mq = mq_open(mq_name, O_CREAT | O_RDONLY | O_NONBLOCK);
  if (mq == -1) {
    fprintf(
      stderr, 
      "error: could not open receive message queue: %s", 
      strerror(errno)
    );
    exit(1);
  }
  else {
    return mq;
  }
}

mqd_t open_send_mq(const char *mq_name) {
  
  mqd_t mq = mq_open(mq_name, O_CREAT | O_WRONLY | O_NONBLOCK);
  if (mq == -1) {
    fprintf(
      stderr, 
      "error: could not open send message queue: %s", 
      strerror(errno)
    );
    exit(1);
  }
  else {
    return mq;
  }
}


char *receive_message(
  mqd_t message_queue, 
  size_t max_message_length, 
  Arena *arena
) {
  
  size_t checkpoint = arena_checkpoint(arena);
  char *payload = ARENA_ALLOCATE(max_message_length, char, arena);

  ssize_t payload_length = mq_receive(
    message_queue, 
    payload, 
    max_message_length, 
    NULL
  );
  
  if (payload_length == -1) {
    if (errno == EAGAIN) {
      arena_restore(checkpoint, arena);
      return NULL;
    }
    else {
      fprintf(stderr, "error: could not receive message: %s", strerror(errno));
      exit(EXIT_FAILURE);
    }
  }
  
  assert(payload_length >= 0);
  payload[payload_length] = '\0';
  return payload;
}

bool send_message(
  mqd_t message_queue,
  const char *serialized_result
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
