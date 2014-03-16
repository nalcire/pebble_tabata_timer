#define PERSIST_KEY_CYCLE 0 
#define PERSIST_KEY_WORK 1
#define PERSIST_KEY_REST 2

#define TIMER_DEFAULT_CYCLE 8
#define TIMER_DEFAULT_WORK 20
#define TIMER_DEFAULT_REST 10

#define TIMER_TICK 59

#define TIMER_CYCLE 0
#define TIMER_WORK 1
#define TIMER_REST 2
#define TIMER_NONE 3

#define TIMER_STATE_IDLE 0
#define TIMER_STATE_RUNNING 1
#define TIMER_STATE_PAUSED 2
#define TIMER_STATE_DONE 3
#define TIMER_STATE_COUNTDOWN 4

void window_timer_init();
void window_timer_deinit();
void window_timer_load(Window *window);
void load_values();
void persist_values();
void timer_config_provider(void *context);
void window_set_view();
void down_click(ClickRecognizerRef recognizer, void *context);
void up_click(ClickRecognizerRef recognizer, void *context);
void timer_config(ClickRecognizerRef recognizer, void *context);
void timer_config_stop(ClickRecognizerRef recognizer, void *context);
void toggle_timer();
void start_timer();
void start_countdown();
void reset_timer();
void timer_handle_timer();
void countdown_handle_timer();
void update_display(int section);
void update_countdown(int second);
void reset_display(int section);
void convert_seconds_to_text(int seconds, char *text);
void convert_cycles_to_text(int cycles, char *text);
