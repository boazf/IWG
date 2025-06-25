/*
* This file was copied and a bit modified to make it suitable for this
* project.
*
* Original Author: Jaume Olivé (jolive@iberoxarxa.com / jolive@whitecatboard.org)
*
*/

#ifdef USE_WIFI
#ifndef _PING_
#define _PING_
#include <Arduino.h>

typedef void(*ping_recv_function)(void* arg, void *pdata);
typedef void(*ping_sent_function)(void* arg, void *pdata);

struct ping_option {
	uint32_t count;
	uint32_t ip;
	uint32_t coarse_time;
	ping_recv_function recv_function;
	ping_sent_function sent_function;
	void* reverse;
};

struct ping_resp {
	uint32_t total_count;
	uint32_t resp_time;
	uint32_t seqno;
	uint32_t timeout_count;
	uint32_t bytes;
	uint32_t total_bytes;
	uint32_t total_time;
	int8_t  ping_err;
};

bool ping_start(struct ping_option *ping_opt);
void ping(const char *name, int count, int interval, int size, int timeout);
bool ping_start(IPAddress adr, int count = 0, int interval = 0, int size = 0, int timeout = 0);

#endif // _PING_
#endif // USE_WIFI