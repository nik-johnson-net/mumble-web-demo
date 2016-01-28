#ifndef _MUMBLE_UDP_PING_H_
#define _MUMBLE_UDP_PING_H_

#include "uv_udp_ssl.h"

#include <uv.h>


typedef struct {
  uint64_t timestamp;
  int waiting;
} udp_ping_attempt_t;

// 1 byte header + 64bit int encoded as varint
#define PING_BUF_SIZE 8
typedef struct {
  uv_udp_ssl_t *socket;
  uv_timer_t period_timer;
  uv_timer_t reply_timer;
  const struct sockaddr_storage *sockaddr;
  int connected;
  int ping_period;
  int timeout;
  udp_ping_attempt_t last_attempt;
  char ping_buf[PING_BUF_SIZE];
} udp_ping_t;

void mumble_udp_ping_init(udp_ping_t *ping, uv_udp_ssl_t *socket, uv_loop_t *loop);
void mumble_udp_ping_init_ex(udp_ping_t *ping, uv_udp_ssl_t *socket, uv_loop_t *loop, int timeout, int ping_period);
void mumble_udp_ping_address(udp_ping_t *ping, const struct sockaddr_storage *sockaddr);
void mumble_udp_ping_recv(udp_ping_t *ping, uint64_t timestamp);
void mumble_udp_ping_start(udp_ping_t *ping);
void mumble_udp_ping_stop(udp_ping_t *ping);

#endif
