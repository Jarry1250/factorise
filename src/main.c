#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_factors_layer;

void factorise(int number, int *factors, int *factor_count){
	int primes[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47 };
	if( number == 1 || number == 0 ) {
		// Reached a prime within the search space
		return;
	}
	for(int i = 0; i < 15; i++) {
		int prime = primes[i];
		if( number%prime == 0 ) {
			factors[*factor_count] = prime;
            *factor_count = *factor_count + 1;
			number = number / prime;
			factorise(number, factors, factor_count);
            return;
		}
	}
	
	// Reached a prime not within the search space
	factors[*factor_count] = number;
	*factor_count = *factor_count + 1;
	return;
}

static void main_window_load(Window *window) {
	// Create time TextLayer
	s_time_layer = text_layer_create(GRect(0, 40, 144, 50));
	text_layer_set_background_color(s_time_layer, GColorClear);
	text_layer_set_text_color(s_time_layer, GColorWhite);

	// Improve the layout to be more like a watchface
	text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

	// Add it as a child layer to the Window's root layer
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
	
	// Create time TextLayer
	s_factors_layer = text_layer_create(GRect(0, 90, 144, 60));
	text_layer_set_background_color( s_factors_layer, GColorClear);
	text_layer_set_text_color( s_factors_layer, GColorWhite);
	
	// Improve the layout to be more like a watchface
	text_layer_set_font( s_factors_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
	text_layer_set_text_alignment( s_factors_layer, GTextAlignmentCenter);
	text_layer_set_overflow_mode( s_factors_layer, GTextOverflowModeWordWrap );
	
	// Add it as a child layer to the Window's root layer
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_factors_layer));
}

static void update_time() {
	// Get a tm structure
	time_t temp = time(NULL); 
	struct tm *tick_time = localtime(&temp);

	// Create a long-lived buffer
	static char buffer[] = "00:00";
	static char text[20];
	char buffer_int[] = "0000";

	// Write the current hours and minutes into the buffer
	if(clock_is_24h_style() == true) {
		// Use 24 hour format
		strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
		strftime(buffer_int, sizeof("0000"), "%H%M", tick_time);
	} else {
		// Use 12 hour format
		strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
		strftime(buffer_int, sizeof("0000"), "%I%M", tick_time);
	}

	// Main time display
	text_layer_set_text(s_time_layer, buffer);
	
	int number = atoi(buffer_int);
	int factors[12];
	int factor_count = 0;
	factorise(number, factors, &factor_count);
	strcpy( text, "" );
	if( factor_count == 1 ) {
		if( number == 1 || number == 0 ) {
			strcpy( text, "It's complicated" );
		} else {
			strcpy( text, "Prime" );
		}
	} else {
		for( int i = 0; i < factor_count; i++ ){
			char factor[100];
			snprintf(factor, sizeof("99999") , "%d", factors[i] );
			if( i > 0 ) strcat(text, "x");
			strcat(text, factor);
		}
	}
	text_layer_set_text(s_factors_layer, text);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
}

static void main_window_unload(Window *window) {
	text_layer_destroy(s_time_layer);
	text_layer_destroy(s_factors_layer);
}

void handle_init(void) {
	// Create main Window element and assign to pointer
	s_main_window = window_create();
	window_set_background_color(s_main_window, GColorBlack );

	// Set handlers to manage the elements inside the Window
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});

	// Register with TickTimerService
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	
	// Show the Window on the watch, with animated=true
	window_stack_push(s_main_window, true);
}

void handle_deinit(void) {
	window_destroy(s_main_window);
}
int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}
