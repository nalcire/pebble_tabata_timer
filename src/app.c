#include <pebble.h>
#include "timer.h"

#define MY_UUID { 0x11, 0x3A, 0xE7, 0x17, 0xFB, 0x8D, 0x42, 0x2A, 0xA4, 0xE3, 0x79, 0x7C, 0xDD, 0xC6, 0xBE, 0x91 }

void handle_init() {
    window_timer_init();
}

void handle_deinit() {
    window_timer_deinit();
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
