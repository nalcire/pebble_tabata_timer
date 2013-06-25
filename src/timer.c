#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "app.h"
#include "timer.h"
#include "lib.h"

Window window_timer;
TextLayer layers[3];
TextLayer cycle_text;
TextLayer work_text;
TextLayer rest_text;
AppTimerHandle timer_handle;

int config_mode;
int timer_state;
int timer_start;
int pause_offset;
int timer_elapsed;
int elapsed_time_in_cycle;
int previous_mode;
int completed_cycles;
int timer_value[] = {TIMER_DEFAULT_CYCLE, TIMER_DEFAULT_WORK, TIMER_DEFAULT_REST};
int cycle_length;
char timer_text[][15] = {"", "", ""};

void window_timer_init() {
    window_init(&window_timer, "timer");
    window_set_click_config_provider(&window_timer, (ClickConfigProvider) timer_config_provider);
    WindowHandlers window_handlers = {
	.load = &window_timer_load
    };
    window_set_window_handlers(&window_timer, window_handlers);
    window_stack_push(&window_timer, true);
}

void window_timer_load(Window *window) {
    Layer *root = window_get_root_layer(window);
    
    config_mode = TIMER_NONE;
    timer_state = TIMER_STATE_IDLE;
    timer_elapsed = 0;
    pause_offset = 0;
    completed_cycles = 0;
    elapsed_time_in_cycle = 0;
    cycle_length = timer_value[TIMER_WORK] + timer_value[TIMER_REST];

    convert_cycles_to_text(timer_value[TIMER_CYCLE], timer_text[TIMER_CYCLE]);
    convert_seconds_to_text(timer_value[TIMER_WORK], timer_text[TIMER_WORK]);
    convert_seconds_to_text(timer_value[TIMER_REST], timer_text[TIMER_REST]);

    text_layer_init(&layers[TIMER_CYCLE], GRect(28,25,27,30));
    text_layer_set_font(&layers[TIMER_CYCLE], fonts_get_system_font(FONT_KEY_GOTHIC_28));
    text_layer_set_text_color(&layers[TIMER_CYCLE], GColorBlack);
    text_layer_set_text(&layers[TIMER_CYCLE], timer_text[TIMER_CYCLE]);
    layer_add_child(root, &layers[TIMER_CYCLE].layer);

    text_layer_init(&cycle_text, GRect(55,25,80,30));
    text_layer_set_font(&cycle_text, fonts_get_system_font(FONT_KEY_GOTHIC_28));
    text_layer_set_text(&cycle_text, "cycles");
    layer_add_child(root, &cycle_text.layer);
 
    text_layer_init(&layers[TIMER_WORK], GRect(63,55,53,30));
    text_layer_set_font(&layers[TIMER_WORK], fonts_get_system_font(FONT_KEY_GOTHIC_28));
    text_layer_set_text_color(&layers[TIMER_WORK], GColorBlack);
    text_layer_set_text(&layers[TIMER_WORK], timer_text[TIMER_WORK]);
    layer_add_child(root, &layers[TIMER_WORK].layer);

    text_layer_init(&work_text, GRect(28,60,30,30));
    text_layer_set_font(&work_text, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text(&work_text, "work");
    layer_add_child(root, &work_text.layer);

    text_layer_init(&layers[TIMER_REST], GRect(63,85,53,30));
    text_layer_set_font(&layers[TIMER_REST], fonts_get_system_font(FONT_KEY_GOTHIC_28));
    text_layer_set_text_color(&layers[TIMER_REST], GColorBlack);
    text_layer_set_text(&layers[TIMER_REST], timer_text[TIMER_REST]);
    layer_add_child(root, &layers[TIMER_REST].layer);

    text_layer_init(&rest_text, GRect(28,90,30,30));
    text_layer_set_font(&rest_text, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text(&rest_text, "rest");
    layer_add_child(root, &rest_text.layer);
}

void timer_config_provider(ClickConfig **config, Window *window) {
    config[BUTTON_ID_UP]->click.handler = (ClickHandler) up_click;
    config[BUTTON_ID_UP]->click.repeat_interval_ms = 30;
    config[BUTTON_ID_DOWN]->click.handler = (ClickHandler) down_click;
    config[BUTTON_ID_DOWN]->click.repeat_interval_ms = 30;
    config[BUTTON_ID_SELECT]->click.handler = (ClickHandler) timer_config;
    config[BUTTON_ID_SELECT]->long_click.handler = (ClickHandler) timer_config_stop;
    config[BUTTON_ID_SELECT]->long_click.release_handler = (ClickHandler) timer_config_stop;
    config[BUTTON_ID_SELECT]->long_click.delay_ms = 1000;
}

void timer_config(ClickRecognizerRef recognizer, Window *window) {

    if( timer_state != TIMER_STATE_IDLE )
	return;

    switch(config_mode) {
	case TIMER_NONE:
	    config_mode = TIMER_CYCLE;
	    break;
	case TIMER_CYCLE:
	    config_mode = TIMER_WORK;
	    break;
	case TIMER_WORK:
	    config_mode = TIMER_REST;
	    break;
	case TIMER_REST:
	    config_mode = TIMER_NONE;
	    cycle_length = timer_value[TIMER_WORK] + timer_value[TIMER_REST];
	    break;
    }    

    window_set_view(); 
    reset_display(TIMER_CYCLE);
    reset_display(TIMER_WORK);
    reset_display(TIMER_REST);
}

void timer_config_stop(ClickRecognizerRef recognizer, Window *window) {
    config_mode = TIMER_NONE;
    cycle_length = timer_value[TIMER_WORK] + timer_value[TIMER_REST];

    window_set_view();
    reset_display(TIMER_CYCLE);
    reset_display(TIMER_WORK);
    reset_display(TIMER_REST);
}

void window_set_view() {
    for(int i = 0; i < 3; i++) {
	if( i == config_mode ) {
	    text_layer_set_text_color(&layers[i], GColorWhite);
	    text_layer_set_background_color(&layers[i], GColorBlack);
	}
	else {
	    text_layer_set_text_color(&layers[i], GColorBlack);
	    text_layer_set_background_color(&layers[i], GColorWhite);
	}	
    } 
}
	
void up_click(ClickRecognizerRef recognizer, Window *window) {
    
    switch(config_mode) {
	case TIMER_NONE:
	    toggle_timer();
	    break;
	case TIMER_CYCLE:
	    if( timer_value[TIMER_CYCLE] > 1  ) {
		timer_value[TIMER_CYCLE]--;
		reset_display(config_mode);
	    }
	    break;
	case TIMER_WORK:
	    if( timer_value[TIMER_WORK] > 1 ) {
		timer_value[TIMER_WORK]--;
		reset_display(config_mode);
	    }
	    break;
	case TIMER_REST:
	    if( timer_value[TIMER_REST] > 1 ) {
		timer_value[TIMER_REST]--;
		reset_display(config_mode);
	    }
	    break;
    }
}

void down_click(ClickRecognizerRef recognizer, Window *window) {
    switch(config_mode) {
	case TIMER_NONE:
	    reset_timer();
	    break;
	case TIMER_CYCLE:
	    if( timer_value[TIMER_CYCLE] < 99 ) {
		timer_value[TIMER_CYCLE]++;
		reset_display(config_mode);
	    }
	    break;
	case TIMER_WORK:
	    if( timer_value[TIMER_WORK] < 5999 ) {
		timer_value[TIMER_WORK]++;
		reset_display(config_mode);
	    }
	    break;
	case TIMER_REST:
	    if( timer_value[TIMER_REST] < 5999 ) {
		timer_value[TIMER_REST]++;
		reset_display(config_mode);
	    }
	    break;
    }
}

void toggle_timer() {
    switch(timer_state) {
	case TIMER_STATE_IDLE:
	    previous_mode = TIMER_WORK;
	    completed_cycles = 0;
	    start_timer();
	    break;
	case TIMER_STATE_RUNNING:
	    app_timer_cancel_event(appCtx, timer_handle);
	    pause_offset += timer_elapsed;
	    timer_elapsed = 0; 
	    timer_state = TIMER_STATE_PAUSED;
	    break;
	case TIMER_STATE_PAUSED:
	    start_timer();
	    break;
    }
}

void start_timer() {
    timer_start = get_ticks_now_in_seconds();
    timer_state = TIMER_STATE_RUNNING;
    timer_handle = app_timer_send_event(appCtx, TIMER_TICK, COOKIE_TIMER);
}

void reset_timer() {
    previous_mode = TIMER_WORK;
    completed_cycles = 0;
    timer_state = TIMER_STATE_IDLE;
    app_timer_cancel_event(appCtx, timer_handle);
    timer_elapsed = 0;
    elapsed_time_in_cycle = 0;
    pause_offset = 0;
    reset_display(TIMER_CYCLE);
    reset_display(TIMER_WORK);
    reset_display(TIMER_REST);
}

void timer_handle_timer(AppContextRef ctx, AppTimerHandle handle) {

    int now = get_ticks_now_in_seconds();
    int current_ticks = now - timer_start;
   
    int cycles = current_ticks / cycle_length;
    if( cycles != completed_cycles ) {
	completed_cycles = cycles;
	update_display(TIMER_CYCLE);
    }

    elapsed_time_in_cycle  = current_ticks % cycle_length;
    int current_mode;
    if( elapsed_time_in_cycle < timer_value[TIMER_WORK] ) {
	current_mode = TIMER_WORK;
    }
    else {
	current_mode = TIMER_REST;
    }
    update_display(current_mode);

    if( current_mode != previous_mode ) {
	vibes_short_pulse();
	reset_display(previous_mode);
	previous_mode = current_mode;
    }

    if( completed_cycles >= timer_value[TIMER_CYCLE] )
	return;

    timer_handle = app_timer_send_event(appCtx, TIMER_TICK, COOKIE_TIMER);
}

void update_display(int section) {
    switch(section) {
	case TIMER_CYCLE:
	    convert_cycles_to_text(timer_value[section] - completed_cycles, timer_text[section]);
	    break;
	case TIMER_WORK:
	    convert_seconds_to_text(timer_value[section] - elapsed_time_in_cycle, timer_text[section]);
	    break;
	case TIMER_REST:
	    convert_seconds_to_text(timer_value[section] - (elapsed_time_in_cycle - timer_value[TIMER_WORK]), timer_text[section]);
	    break;
    }

    text_layer_set_text(&layers[section], timer_text[section]);
    layer_mark_dirty(&layers[section].layer);
}

void reset_display(int section) {
    switch(section) {
	case TIMER_CYCLE:
	    convert_cycles_to_text(timer_value[TIMER_CYCLE], timer_text[TIMER_CYCLE]);
	    break;
	case TIMER_WORK:
	case TIMER_REST:
	    convert_seconds_to_text(timer_value[section], timer_text[section]);
	    break;
    }

    text_layer_set_text(&layers[section], timer_text[section]);
    layer_mark_dirty(&layers[section].layer);
}

void convert_seconds_to_text(int ticks, char *text) {

    int seconds = ticks % 60;
    int minutes = ticks / 60 % 60;

    strcpy(text, itoa(minutes,2));
    strcat(text, ":");
    strcat(text, itoa(seconds,2));
}

void convert_cycles_to_text(int cycles, char *text) {
    strcpy(text, itoa(cycles, 2));
}
