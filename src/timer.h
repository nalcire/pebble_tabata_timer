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

void window_timer_init();
void window_timer_load(Window *window);
void timer_config_provider(ClickConfig **config, Window *window);
void window_set_view();
void down_click(ClickRecognizerRef recognizer, Window *window);
void up_click(ClickRecognizerRef recognizer, Window *window);
void timer_config(ClickRecognizerRef recognizer, Window *window);
void timer_config_stop(ClickRecognizerRef recognizer, Window *window);
void toggle_timer();
void start_timer();
void reset_timer();
void timer_handle_timer(AppContextRef ctx, AppTimerHandle handle);
void update_display(int section);
void convert_seconds_to_text(int seconds, char *text);
void convert_cycles_to_text(int cycles, char *text);
