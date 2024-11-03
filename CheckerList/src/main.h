#pragma once
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

typedef enum { TIMING, CHANGING_TIME } Timer_state;

typedef struct {
	int8_t minutes;
	int8_t seconds;
} Timer;
