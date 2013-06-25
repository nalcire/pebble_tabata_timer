#define TIMER_DEFAULT_CYCLE 8
#define TIMER_DEFAULT_WORK 30
#define TIMER_DEFAULT_REST 10

#define TIMER_BLINK_RATE 300
#define TIMER_TICK 100

#define TIMER_CYCLE 0
#define TIMER_WORK 1
#define TIMER_REST 2

#define TIMER_STATE_IDLE 0
#define TIMER_STATE_RUNNING 1
#define TIMER_STATE_PAUSED 2

#define TIMER_CONFIG_NONE 0
#define TIMER_CONFIG_CYCLE 1
#define TIMER_CONFIG_WORK_MINUTE 2
#define TIMER_CONFIG_WORK_SECOND 3
#define TIMER_CONFIG_REST_MINUTE 4
#define TIMER_CONFIG_REST_SECOND 5

void window_timer_init();
void window_timer_load(Window *window);
void timer_config_provider(ClickConfig **config, Window *window);
void down_click(ClickRecognizerRef recognizer, Window *window);
void up_click(ClickRecognizerRef recognizer, Window *window);
void timer_config(ClickRecognizerRef recognizer, Window *window);
void timer_config_stop(ClickRecognizerRef recognizer, Window *window);
void timer_handle_blink(AppContextRef ctx, AppTimerHandle handle);
void timer_toggle();
void timer_reset();
void timer_handle_timer(AppContextRef ctx, AppTimerHandle handle);
void update_display(int section);
char *blink_time_text();
void convert_seconds_to_text(int seconds, char *text);
void convert_cycles_to_text(int cycles, char *text);
