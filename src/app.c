#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "app.h"
#include "timer.h"

#define MY_UUID { 0x11, 0x3A, 0xE7, 0x17, 0xFB, 0x8D, 0x42, 0x2A, 0xA4, 0xE3, 0x79, 0x7C, 0xDD, 0xC6, 0xBE, 0x91 }
PBL_APP_INFO(MY_UUID,
             "Tabata Timer", "nalcire",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_STANDARD_APP);

AppContextRef appCtx;

void handle_init(AppContextRef ctx) {
  (void)ctx;
    appCtx = ctx;
    window_timer_init();
}

void handle_timer(AppContextRef ctx, AppTimerHandle handle, uint32_t cookie) {
    switch(cookie) {
	case COOKIE_TIMER: 
	    timer_handle_timer(ctx, handle);
	    break;
	case COOKIE_COUNTDOWN:
	    countdown_handle_timer(ctx, handle);
	    break;
    }
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .timer_handler = &handle_timer
  };
  app_event_loop(params, &handlers);
}
