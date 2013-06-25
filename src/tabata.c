#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "app.h"
#include "timer.h"
#include "lib.h"

AppContextRef appCtx;
Window window_timer;
TextLayer layers[3];
AppTimerHandle timer_handle;
AppTimerHandle config_handle;
uint32_t config_mode;
int blink_count;
int timer_state;
int timer_start;
int pause_offset;
int timer_elapsed;
int timer_elapsed_in_section;
int previous_mode;
int previous_cycle;
int timer_value[] = {TIMER_DEFAULT_CYCLE, TIMER_DEFAULT_WORK, TIMER_DEFAULT_REST};
char timer_text[][10] = {"", "", ""};
char blinkCycles[] = "__ cycles";
char blinkTime[] = "__:__";

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
    
    config_mode = TIMER_CONFIG_NONE;
    timer_state = TIMER_STATE_IDLE;
    blink_count = 0;
    timer_elapsed = 0;
    pause_offset = 0;
    previous_cycle = 0;
    timer_elapsed_in_section = 0;

    convert_cycles_to_text(timer_value[TIMER_CYCLE], timer_text[TIMER_CYCLE]);
    convert_seconds_to_text(timer_value[TIMER_WORK], timer_text[TIMER_WORK]);
    convert_seconds_to_text(timer_value[TIMER_REST], timer_text[TIMER_REST]);

    text_layer_init(&layers[TIMER_CYCLE], GRect(30,25,144,30));
    text_layer_set_font(&layers[TIMER_CYCLE], fonts_get_system_font(FONT_KEY_GOTHIC_28));
    text_layer_set_text_color(&layers[TIMER_CYCLE], GColorBlack);
    text_layer_set_text(&layers[TIMER_CYCLE], timer_text[TIMER_CYCLE]);
    layer_add_child(root, &layers[TIMER_CYCLE].layer);
 
    text_layer_init(&layers[TIMER_WORK], GRect(45,55,144,30));
    text_layer_set_font(&layers[TIMER_WORK], fonts_get_system_font(FONT_KEY_GOTHIC_28));
    text_layer_set_text_color(&layers[TIMER_WORK], GColorBlack);
    text_layer_set_text(&layers[TIMER_WORK], timer_text[TIMER_WORK]);
    layer_add_child(root, &layers[TIMER_WORK].layer);

    text_layer_init(&layers[TIMER_REST], GRect(45,85,144,30));
    text_layer_set_font(&layers[TIMER_REST], fonts_get_system_font(FONT_KEY_GOTHIC_28));
    text_layer_set_text_color(&layers[TIMER_REST], GColorBlack);
    text_layer_set_text(&layers[TIMER_REST], timer_text[TIMER_REST]);
    layer_add_child(root, &layers[TIMER_REST].layer);

}

void timer_config_provider(ClickConfig **config, Window *window) {
    config[BUTTON_ID_UP]->click.handler = (ClickHandler) up_click;
    config[BUTTON_ID_DOWN]->click.handler = (ClickHandler) down_click;
    config[BUTTON_ID_SELECT]->click.handler = (ClickHandler) timer_config;
    config[BUTTON_ID_SELECT]->long_click.handler = (ClickHandler) timer_config_stop;
    config[BUTTON_ID_SELECT]->long_click.release_handler = (ClickHandler) timer_config_stop;
    config[BUTTON_ID_SELECT]->long_click.delay_ms = 1000;
}

void timer_config(ClickRecognizerRef recognizer, Window *window) {
    app_timer_cancel_event(appCtx, config_handle);

    config_mode++;
    if( config_mode > TIMER_CONFIG_REST_SECOND )
	config_mode = TIMER_CONFIG_NONE;

    switch(config_mode) {
	case TIMER_CONFIG_WORK_MINUTE:
	    update_display(TIMER_CYCLE);
	    break;
	case TIMER_CONFIG_REST_MINUTE:
	    update_display(TIMER_WORK);
	    break;
	case TIMER_CONFIG_NONE:
	    update_display(TIMER_REST);
	    break;
    }    

    if( config_mode != TIMER_CONFIG_NONE )
	config_handle = app_timer_send_event(appCtx, TIMER_BLINK_RATE, COOKIE_TIMER_CONFIG);
}

void timer_config_stop(ClickRecognizerRef recognizer, Window *window) {
    app_timer_cancel_event(appCtx, config_handle);
    config_mode = TIMER_CONFIG_NONE;
    update_display(TIMER_CYCLE);
    update_display(TIMER_WORK);
    update_display(TIMER_REST);
}

void timer_handle_blink(AppContextRef ctx, AppTimerHandle handle) {
    
    switch(config_mode) {
	case TIMER_CONFIG_CYCLE:
	    if( blink_count % 2 == 0 )
		text_layer_set_text(&layers[TIMER_CYCLE], blinkCycles);
	    else
		update_display(TIMER_CYCLE);
	    break;
	case TIMER_CONFIG_WORK_MINUTE:
	    if( blink_count % 2 == 0 )
		text_layer_set_text(&layers[TIMER_WORK], blink_time_text());
	    else
		update_display(TIMER_WORK);
	    break;
	case TIMER_CONFIG_WORK_SECOND:
	    if( blink_count % 2 == 0 )
		text_layer_set_text(&layers[TIMER_WORK], blink_time_text());
	    else
		update_display(TIMER_WORK);
	    break;
	case TIMER_CONFIG_REST_MINUTE:
	    if( blink_count % 2 == 0 )
		text_layer_set_text(&layers[TIMER_REST], blink_time_text());
	    else
		update_display(TIMER_REST);
	    break;
	case TIMER_CONFIG_REST_SECOND:
	    if( blink_count % 2 == 0 )
		text_layer_set_text(&layers[TIMER_REST], blink_time_text());
	    else
		update_display(TIMER_REST);
	    break;
    } 

    blink_count++;
    config_handle = app_timer_send_event(appCtx, TIMER_BLINK_RATE, COOKIE_TIMER_CONFIG);
}

void up_click(ClickRecognizerRef recognizer, Window *window) {
    
    switch(config_mode) {
	case TIMER_CONFIG_NONE:
	    timer_toggle();
	    break;
	case TIMER_CONFIG_CYCLE:
	    if( timer_value[TIMER_CYCLE] < 99 )
		timer_value[TIMER_CYCLE]++;
	    break;
	case TIMER_CONFIG_WORK_MINUTE:
	    if( timer_value[TIMER_WORK] <= 3540 )
		    timer_value[TIMER_WORK] += 60;
	    break;
	case TIMER_CONFIG_WORK_SECOND:
	    if( timer_value[TIMER_WORK] < 3600 )
	    timer_value[TIMER_WORK] += 1;
	    break;
	case TIMER_CONFIG_REST_MINUTE:
	    if( timer_value[TIMER_REST] <= 3540 )
		timer_value[TIMER_REST] += 60;
	    break;
	case TIMER_CONFIG_REST_SECOND:
	    if( timer_value[TIMER_REST] < 3600 )
		timer_value[TIMER_REST] += 1;
	    break;
    }
}

void down_click(ClickRecognizerRef recognizer, Window *window) {
    switch(config_mode) {
	case TIMER_CONFIG_NONE:
	    timer_reset();
	    break;
	case TIMER_CONFIG_CYCLE:
	    if( timer_value[TIMER_CYCLE] > 0 )
		timer_value[TIMER_CYCLE]--;
	    break;
	case TIMER_CONFIG_WORK_MINUTE:
	    if( timer_value[TIMER_WORK] >= 60 )
		timer_value[TIMER_WORK] -= 60;
	    break;
	case TIMER_CONFIG_WORK_SECOND:
	    if( timer_value[TIMER_WORK] > 0 )
		timer_value[TIMER_WORK] -= 1;
	    break;
	case TIMER_CONFIG_REST_MINUTE:
	    if( timer_value[TIMER_REST] >= 60 )
		timer_value[TIMER_REST] -= 60;
	    break;
	case TIMER_CONFIG_REST_SECOND:
	    if( timer_value[TIMER_REST] > 0 )
		timer_value[TIMER_REST] -= 1;
	    break;
    }
}

void timer_toggle() {
    switch(timer_state) {
	case TIMER_STATE_IDLE:
	    previous_mode = TIMER_WORK;
	    timer_start = get_ticks_now_in_seconds();
	    timer_handle = app_timer_send_event(appCtx, TIMER_TICK, COOKIE_TIMER);
	    timer_state = TIMER_STATE_RUNNING;
	    break;
	case TIMER_STATE_RUNNING:
	    app_timer_cancel_event(appCtx, timer_handle);
	    pause_offset += timer_elapsed;
	    timer_elapsed = 0; 
	    timer_state = TIMER_STATE_PAUSED;
	    break;
	case TIMER_STATE_PAUSED:
	    timer_start = get_ticks_now_in_seconds();
	    timer_handle = app_timer_send_event(appCtx, TIMER_TICK, COOKIE_TIMER);
	    timer_state = TIMER_STATE_RUNNING;
	    break;
    }
}

void timer_reset() {
    timer_state = TIMER_STATE_IDLE;
    app_timer_cancel_event(appCtx, timer_handle);
    timer_elapsed = 0;
    timer_elapsed_in_section = 0;
    pause_offset = 0;
    update_display(TIMER_CYCLE);
    update_display(TIMER_WORK);
    update_display(TIMER_REST);
}

void timer_handle_timer(AppContextRef ctx, AppTimerHandle handle) {
    int now = get_ticks_now_in_seconds();
    int current_tick = now - timer_start;
   
    int current_cycle = current_tick / 40;
    int current_time = current_tick % 40;
    
    if( current_cycle != previous_cycle ) {
	vibes_short_pulse();
	previous_cycle = current_cycle;
    }
    
    if( current_cycle > timer_value[TIMER_CYCLE] )
	return;

    int current_mode;
    if( current_time <= timer_value[TIMER_WORK] )
	current_mode = TIMER_WORK;
    else
	current_mode = TIMER_REST;

    if( current_mode != previous_mode ) {
	vibes_short_pulse();
	previous_mode = current_mode;
	update_display(TIMER_CYCLE);
    }
    
    if( current_mode == TIMER_WORK ) {
	timer_elapsed_in_section = current_time;
	update_display(TIMER_WORK);
    }
    else {
	timer_elapsed_in_section = current_time - timer_value[TIMER_WORK];
	update_display(TIMER_REST);
    }

    timer_handle = app_timer_send_event(appCtx, TIMER_TICK, COOKIE_TIMER);
}

void update_display(int section) {
    switch(section) {
	case TIMER_CYCLE:
	    convert_cycles_to_text(timer_value[section] - previous_cycle, timer_text[section]);
	    break;
	case TIMER_WORK:
	case TIMER_REST:
	    convert_seconds_to_text(timer_value[section] - timer_elapsed_in_section, timer_text[section]);
	    break;
    }

    text_layer_set_text(&layers[section], timer_text[section]);
    layer_mark_dirty(&layers[section].layer);
}

char *blink_time_text() {
    
    //TODO save each element separately to not calcualte these twice
    switch(config_mode) {
	case TIMER_CONFIG_WORK_MINUTE:
	    strcpy(blinkTime, "__:");
	    strcat(blinkTime, itoa(timer_value[TIMER_WORK] % 60, 2));
	    break;
	case TIMER_CONFIG_WORK_SECOND:
	    strcpy(blinkTime, itoa(timer_value[TIMER_WORK] / 60 % 60, 2));
	    strcat(blinkTime, ":__");
	    break;
	case TIMER_CONFIG_REST_MINUTE:	
	    strcpy(blinkTime, "__:");
	    strcat(blinkTime, itoa(timer_value[TIMER_REST] % 60, 2));
	    break;
	case TIMER_CONFIG_REST_SECOND:
	    strcpy(blinkTime, itoa(timer_value[TIMER_REST] / 60 % 60, 2));
	    strcat(blinkTime, ":__");
	    break;
    }

    return blinkTime;
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
    strcat(text, " cycles");
}
