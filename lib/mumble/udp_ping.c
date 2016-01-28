#include "udp_ping.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "util.h"

#define DEFAULT_PING_PERIOD 5000
#define DEFAULT_PING_TIMEOUT 1000

/* The ping system is a state machine of four states:
 * Waiting 0, Connected 0: Confirmed dead (Initial state on creation)
 * Waiting 0, Connected 1: Confirmed alive
 * Waiting 1, Connected 0: Attempting to reconnect
 * Waiting 1, Connected 1: Attempting to confirm liveliness
 */
static void udp_ping_attempt_init(udp_ping_attempt_t *last_attempt) {
  fprintf(stderr, "Beginning ping attempt\n");
  last_attempt->timestamp = time(NULL);
  last_attempt->waiting = 1;
}

/* On a timeout, consider the ping lost and state is disconnected. */
static void reply_timer_cb(uv_timer_t *handle) {
  fprintf(stderr, "Failing ping attempt\n");
  udp_ping_t *ping = handle->data;
  ping->connected = 0;
  ping->last_attempt.waiting = 0;
}

static void period_timer_cb(uv_timer_t *handle) {
  udp_ping_t *ping = handle->data;

  udp_ping_attempt_init(&ping->last_attempt);

  // write ping packet
  int written;
  int ret = varint_encode(time(NULL), ping->ping_buf + 1, PING_BUF_SIZE - 2, &written);
  assert(ret == 0);
  assert(written + 1 <= PING_BUF_SIZE);
  uv_udp_ssl_write(ping->socket, (struct sockaddr*)ping->sockaddr, ping->ping_buf, written + 1);

  // Start the timeout timer
  uv_timer_start(&ping->reply_timer, reply_timer_cb, ping->timeout, 0);
}

void mumble_udp_ping_init(udp_ping_t *ping, uv_udp_ssl_t *socket, uv_loop_t *loop) {
  mumble_udp_ping_init_ex(ping, socket, loop, DEFAULT_PING_TIMEOUT, DEFAULT_PING_PERIOD);
}

void mumble_udp_ping_init_ex(udp_ping_t *ping, uv_udp_ssl_t *socket, uv_loop_t *loop, int timeout, int ping_period) {
  assert(timeout > 0);
  assert(ping_period > timeout);
  assert(socket != NULL);
  memset(ping, 0, sizeof(udp_ping_t));

  ping->ping_period = ping_period;
  ping->socket = socket;
  ping->timeout = timeout;

  uv_timer_init(loop, &ping->period_timer);
  ping->period_timer.data = ping;

  uv_timer_init(loop, &ping->reply_timer);
  ping->reply_timer.data = ping;

  // Header is static, so set once
  // Translates to Type = PING, Target = normal
  ping->ping_buf[0] = 0x20;
}

void mumble_udp_ping_address(udp_ping_t *ping, const struct sockaddr_storage *sockaddr) {
  ping->sockaddr = sockaddr;
}

void mumble_udp_ping_recv(udp_ping_t *ping, uint64_t timestamp) {
  if (ping->last_attempt.waiting) {
    if (timestamp == ping->last_attempt.timestamp) {
      fprintf(stderr, "Succeeding ping attempt\n");
      ping->connected = 1;
      ping->last_attempt.waiting = 0;
      uv_timer_stop(&ping->reply_timer);
    } else {
      fprintf(stderr, "ping reply does not match timestamp\n");
    }
  } else {
    fprintf(stderr, "ping reply recv when not waiting\n");
  }
}

void mumble_udp_ping_start(udp_ping_t *ping) {
  uv_timer_start(&ping->period_timer, period_timer_cb, 0, ping->ping_period);
}

void mumble_udp_ping_stop(udp_ping_t *ping) {
  uv_timer_stop(&ping->period_timer);
  uv_timer_stop(&ping->reply_timer);
  ping->connected = 0;
}
