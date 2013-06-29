#define COOKIE_TIMER 1
#define COOKIE_COUNTDOWN 2

extern AppContextRef appCtx;
void handle_timer(AppContextRef ctx, AppTimerHandle handle, uint32_t cookie);
