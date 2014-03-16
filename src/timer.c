#include <pebble.h>
#include "timer.h"
#include "lib.h"

Window *window_timer;
TextLayer *layers[3];
TextLayer *cycle_text;
TextLayer *work_text;
TextLayer *rest_text;
TextLayer *reset_text;
TextLayer *pause_text;
TextLayer *start_text;
TextLayer *countdown_text;
Layer *timer_layer;
Layer *countdown_layer;

AppTimer *timer_handle;

int config_mode;
int timer_state;
int timer_start;
int timer_countdown_elapsed;
int pause_offset;
int elapsed_time_in_cycle;
int previous_mode;
int completed_cycles;
int timer_value[] = {TIMER_DEFAULT_CYCLE, TIMER_DEFAULT_WORK, TIMER_DEFAULT_REST};
int cycle_length;
char timer_text[][15] = {"", "", ""};

void window_timer_init() {
    window_timer = window_create();
    window_set_click_config_provider(window_timer, timer_config_provider);
    WindowHandlers window_handlers = {
	.load = &window_timer_load
    };
    window_set_window_handlers(window_timer, window_handlers);
    window_stack_push(window_timer, true);
}

void window_timer_deinit() {
    for(int i = 0; i < 3; i++) {
	text_layer_destroy(layers[i]);
    }
 
    text_layer_destroy(cycle_text);
    text_layer_destroy(work_text);
    text_layer_destroy(rest_text);
    text_layer_destroy(reset_text);
    text_layer_destroy(pause_text);
    text_layer_destroy(start_text);
    text_layer_destroy(countdown_text);
    layer_destroy(timer_layer);
    layer_destroy(countdown_layer);

    window_destroy(window_timer);
}

void window_timer_load(Window *window) {
    Layer *root = window_get_root_layer(window);
    
    config_mode = TIMER_NONE;
    timer_state = TIMER_STATE_IDLE;
    pause_offset = 0;
    completed_cycles = 0;
    elapsed_time_in_cycle = 0;
    cycle_length = timer_value[TIMER_WORK] + timer_value[TIMER_REST];

    convert_cycles_to_text(timer_value[TIMER_CYCLE], timer_text[TIMER_CYCLE]);
    convert_seconds_to_text(timer_value[TIMER_WORK], timer_text[TIMER_WORK]);
    convert_seconds_to_text(timer_value[TIMER_REST], timer_text[TIMER_REST]);

    timer_layer = layer_create(GRect(0,0,144,167));
    layer_add_child(root, timer_layer);
    countdown_layer = layer_create(GRect(0,0,144,167));
    layer_add_child(root, countdown_layer);
    layer_set_hidden(countdown_layer, true);

    countdown_text = text_layer_create(GRect(20,45,120,55));
    text_layer_set_font(countdown_text, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
    text_layer_set_text(countdown_text, "ready");
    layer_add_child(countdown_layer, text_layer_get_layer(countdown_text));

    layers[TIMER_CYCLE] = text_layer_create(GRect(28,25,27,30));
    text_layer_set_font(layers[TIMER_CYCLE], fonts_get_system_font(FONT_KEY_GOTHIC_28));
    text_layer_set_text_color(layers[TIMER_CYCLE], GColorBlack);
    text_layer_set_text(layers[TIMER_CYCLE], timer_text[TIMER_CYCLE]);
    layer_add_child(timer_layer, text_layer_get_layer(layers[TIMER_CYCLE]));

    cycle_text = text_layer_create(GRect(55,25,80,30));
    text_layer_set_font(cycle_text, fonts_get_system_font(FONT_KEY_GOTHIC_24));
    text_layer_set_text(cycle_text, "cycles");
    layer_add_child(timer_layer, text_layer_get_layer(cycle_text));
 
    layers[TIMER_WORK] = text_layer_create(GRect(63,55,53,30));
    text_layer_set_font(layers[TIMER_WORK], fonts_get_system_font(FONT_KEY_GOTHIC_28));
    text_layer_set_text_color(layers[TIMER_WORK], GColorBlack);
    text_layer_set_text(layers[TIMER_WORK], timer_text[TIMER_WORK]);
    layer_add_child(timer_layer, text_layer_get_layer(layers[TIMER_WORK]));

    work_text = text_layer_create(GRect(28,60,30,30));
    text_layer_set_font(work_text, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text(work_text, "work");
    layer_add_child(timer_layer, text_layer_get_layer(work_text));

    layers[TIMER_REST] = text_layer_create(GRect(63,85,53,30));
    text_layer_set_font(layers[TIMER_REST], fonts_get_system_font(FONT_KEY_GOTHIC_28));
    text_layer_set_text_color(layers[TIMER_REST], GColorBlack);
    text_layer_set_text(layers[TIMER_REST], timer_text[TIMER_REST]);
    layer_add_child(timer_layer, text_layer_get_layer(layers[TIMER_REST]));

    rest_text = text_layer_create(GRect(28,90,30,30));
    text_layer_set_font(rest_text, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text(rest_text, "rest");
    layer_add_child(timer_layer, text_layer_get_layer(rest_text));

    reset_text = text_layer_create(GRect(35,130,109,52));
    text_layer_set_font(reset_text, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text(reset_text, "1 more round -->");
    layer_add_child(timer_layer, text_layer_get_layer(reset_text)); 
    layer_set_hidden(text_layer_get_layer(reset_text), true);

    pause_text = text_layer_create(GRect(35,0,109,25));
    text_layer_set_font(pause_text, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text(pause_text, "timer paused -->");
    layer_add_child(timer_layer, text_layer_get_layer(pause_text));
    layer_set_hidden(text_layer_get_layer(pause_text), true);

    start_text = text_layer_create(GRect(45,0,99,25));
    text_layer_set_font(start_text, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text(start_text, "start timer -->");
    layer_add_child(timer_layer, text_layer_get_layer(start_text));
    layer_set_hidden(text_layer_get_layer(start_text), false);
}

void timer_config_provider(void *context) {
    window_single_repeating_click_subscribe(BUTTON_ID_UP, 30, (ClickHandler)up_click);
    window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 30, (ClickHandler)down_click);
    window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler)timer_config);
    window_long_click_subscribe(BUTTON_ID_SELECT, 1000, (ClickHandler)timer_config_stop, (ClickHandler)timer_config_stop);
}

void timer_config(ClickRecognizerRef recognizer, void *context) {

    if( timer_state != TIMER_STATE_IDLE )
	return;

    layer_set_hidden(text_layer_get_layer(start_text), true);
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
	    layer_set_hidden(text_layer_get_layer(start_text), false);
	    break;
    }    

    window_set_view(); 
    reset_display(TIMER_CYCLE);
    reset_display(TIMER_WORK);
    reset_display(TIMER_REST);
}

void timer_config_stop(ClickRecognizerRef recognizer, void *context) {
    config_mode = TIMER_NONE;
    cycle_length = timer_value[TIMER_WORK] + timer_value[TIMER_REST];
    layer_set_hidden(text_layer_get_layer(start_text), false);
    
    window_set_view();
    reset_display(TIMER_CYCLE);
    reset_display(TIMER_WORK);
    reset_display(TIMER_REST);
}

void window_set_view() {
    for(int i = 0; i < 3; i++) {
	if( i == config_mode ) {
	    text_layer_set_text_color(layers[i], GColorWhite);
	    text_layer_set_background_color(layers[i], GColorBlack);
	}
	else {
	    text_layer_set_text_color(layers[i], GColorBlack);
	    text_layer_set_background_color(layers[i], GColorWhite);
	}	
    } 
}
	
void up_click(ClickRecognizerRef recognizer, void *context) {
    
    switch(config_mode) {
	case TIMER_NONE:
	    if( timer_state != TIMER_STATE_COUNTDOWN ) 
		toggle_timer();
	    break;
	case TIMER_CYCLE:
	    if( timer_state == TIMER_STATE_IDLE && timer_value[TIMER_CYCLE] > 1  ) {
		timer_value[TIMER_CYCLE]--;
		reset_display(config_mode);
	    }
	    break;
	case TIMER_WORK:
	    if( timer_state == TIMER_STATE_IDLE && timer_value[TIMER_WORK] > 1 ) {
		timer_value[TIMER_WORK]--;
		reset_display(config_mode);
	    }
	    break;
	case TIMER_REST:
	    if( timer_state == TIMER_STATE_IDLE && timer_value[TIMER_REST] > 1 ) {
		timer_value[TIMER_REST]--;
		reset_display(config_mode);
	    }
	    break;
    }
}

void down_click(ClickRecognizerRef recognizer, void *context) {
    switch(config_mode) {
	case TIMER_NONE:
	    if( timer_state != TIMER_STATE_COUNTDOWN )
		reset_timer();
	    break;
	case TIMER_CYCLE:
	    if( timer_state == TIMER_STATE_IDLE && timer_value[TIMER_CYCLE] < 99 ) {
		timer_value[TIMER_CYCLE]++;
		reset_display(config_mode);
	    }
	    break;
	case TIMER_WORK:
	    if( timer_state == TIMER_STATE_IDLE && timer_value[TIMER_WORK] < 5999 ) {
		timer_value[TIMER_WORK]++;
		reset_display(config_mode);
	    }
	    break;
	case TIMER_REST:
	    if( timer_state == TIMER_STATE_IDLE && timer_value[TIMER_REST] < 5999 ) {
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
	    layer_set_hidden(text_layer_get_layer(start_text), true);
	    start_countdown();
	    break;
	case TIMER_STATE_RUNNING:
	    app_timer_cancel(timer_handle);
	    pause_offset += get_ticks_now_in_seconds() - timer_start;
	    timer_state = TIMER_STATE_PAUSED;
	    layer_set_hidden(text_layer_get_layer(pause_text), false);
	    vibes_short_pulse();
	    break;
	case TIMER_STATE_PAUSED:
	    start_countdown();
	    break;
    }
}

void start_timer() {
    vibes_long_pulse();
    timer_start = get_ticks_now_in_seconds();
    timer_state = TIMER_STATE_RUNNING;
    layer_set_hidden(text_layer_get_layer(pause_text), true);
    layer_set_hidden(timer_layer, false);
    layer_set_hidden(countdown_layer, true);
    timer_handle = app_timer_register(TIMER_TICK, timer_handle_timer, NULL);
}

void start_countdown() {
    timer_countdown_elapsed = 0;
    timer_state = TIMER_STATE_COUNTDOWN;
    layer_set_hidden(timer_layer, true);
    layer_set_hidden(countdown_layer, false);
    timer_handle = app_timer_register(1000, countdown_handle_timer, NULL);
}

void reset_timer() {
    previous_mode = TIMER_WORK;
    completed_cycles = 0;
    timer_state = TIMER_STATE_IDLE;
    app_timer_cancel(timer_handle);
    elapsed_time_in_cycle = 0;
    pause_offset = 0;
    timer_countdown_elapsed = 0;
    reset_display(TIMER_CYCLE);
    reset_display(TIMER_WORK);
    reset_display(TIMER_REST);
    layer_set_hidden(text_layer_get_layer(reset_text), true);
    layer_set_hidden(text_layer_get_layer(start_text), false);
    layer_set_hidden(text_layer_get_layer(pause_text), true);
}

void countdown_handle_timer() {
    switch(timer_countdown_elapsed) {
	case 0:
	case 1:
	case 2:
	    update_countdown(timer_countdown_elapsed++);
	    vibes_double_pulse();
	    timer_handle = app_timer_register(1000, countdown_handle_timer, NULL); 	    
	    break;	
	case 3:
	    timer_countdown_elapsed = 0;
	    update_countdown(0);
	    start_timer();
	    break;
    }
}

void timer_handle_timer() {

    int now = get_ticks_now_in_seconds();
    int current_ticks = now - timer_start + pause_offset;
   
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

    if( current_mode != previous_mode && completed_cycles < timer_value[TIMER_CYCLE]) {
	if( current_ticks < (timer_value[TIMER_WORK] + timer_value[TIMER_REST]) * timer_value[TIMER_CYCLE] - timer_value[TIMER_REST] )
	    vibes_long_pulse();
	reset_display(previous_mode);
	previous_mode = current_mode;
    }

    if( completed_cycles == timer_value[TIMER_CYCLE] - 1 && current_mode == TIMER_REST ) {
	layer_set_hidden(text_layer_get_layer(reset_text), false);
	timer_state = TIMER_STATE_DONE;

	const uint32_t segments[] = {100, 100, 100, 100, 100};
	VibePattern pat = {
	    .durations = segments,
	    .num_segments = ARRAY_LENGTH(segments)
	};
	vibes_enqueue_custom_pattern(pat);	

	return;
    }

    timer_handle = app_timer_register(TIMER_TICK, timer_handle_timer, NULL);
}

void update_countdown(int second) {
    switch(second) {
	case 0:
	    text_layer_set_text(countdown_text, "ready");
	    break;
	case 1:
	    text_layer_set_text(countdown_text, "  set");
	    break;
	case 2:
	    text_layer_set_text(countdown_text, "  go!");
	    break;
    }
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

    text_layer_set_text(layers[section], timer_text[section]);
    layer_mark_dirty(text_layer_get_layer(layers[section]));
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

    text_layer_set_text(layers[section], timer_text[section]);
    layer_mark_dirty(text_layer_get_layer(layers[section]));
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
