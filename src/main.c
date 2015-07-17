// Grant of license under MIT License (MIT) stated in license.h.
#include "license.h"
// Template for starting a new Pebble C Project
#include <pebble.h>
static Window *s_main_window;
static TextLayer *s_status_layer; // Text msgs on watch face.
static TextLayer *s_time_layer; // Current time on watch face.


// Returns am or pm of a tick_time.
static const char* AmOrPm(struct tm* tick_time) {
  static const char* xm = "p";
  if (tick_time->tm_hour < 12)
    xm = "a";
  return xm;
};

// Handler to display time every minute.
static void tick_handler_minute(struct tm* tick_time, TimeUnits units_changed) {
  // Use 12 hour format
  // Create a long-lived buffer
  static char s_time_buffer[] = "00:00";
  strftime(s_time_buffer, sizeof("00:00"), "%I:%M", tick_time);
  const char* xm_time = AmOrPm(tick_time);
  
  static char s_status_buffer[64];
  snprintf(s_status_buffer, sizeof(s_status_buffer), "%s%s", 
           s_time_buffer, xm_time);
  
  text_layer_set_text(s_time_layer, s_status_buffer);
};



// http://stackoverflow.com/questions/21150193/logging-enums-on-the-pebble-watch/21172222#21172222
char *translate_error(AppMessageResult result) {
  switch (result) {
    case APP_MSG_OK: return "APP_MSG_OK";
    case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
    case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
    case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
    case APP_MSG_BUSY: return "APP_MSG_BUSY";
    case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
    case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
    case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
    default: return "UNKNOWN ERROR";
  }
}

// Return positive integer including 0 for string of digits in pText.
// Returns -1 for string not starting with a digit.
// Quits if a nondigit is encountered in the pText
// returning the value before the nondigit.
static int Text2Ord(char* pText) {
  int ord  = 0;
  int count = strlen(pText);
  int place = 1;// Multiplier for digits place. Set for most significant digit
  for (int iPlace = 1; iPlace < count; iPlace++) {
    place *= 10;
  } 
  
  int digit = 0; // Value of digit in pText.
  for (int i=0; i < count; i++) {
    digit = (int)(pText[i] - '0');
    if (digit >= 0 && digit <= 9) {
      ord += digit*place;
      place /= 10; // Reduce for next less significant digit.
    } else {
      if (i==0)
        ord=-1;
      break; // quit on invalid digit.
    }
  } 
  return ord;
}


// Called when a message is received from phone-app.
static void in_received_handler(DictionaryIterator *received, void *context) {
	Tuple *tuple;

	int key; // Key.
	for ( key = 0; key < 2; key++ ) {
		tuple = dict_find(received, key);
		if(tuple) {
			APP_LOG(APP_LOG_LEVEL_DEBUG, "received text: %s", tuple->value->cstring);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "received key, %u", (unsigned int)tuple->key);
      if (key == 0)
			  text_layer_set_text(s_status_layer, tuple->value->cstring);
      else if (key == 1)  {
        int vibes = Text2Ord(tuple->value->cstring);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "vibes: %i", vibes);
        for (int i=0; i < vibes; i++ ) {
          vibes_long_pulse();
          psleep(1000);
        }
      }
		}
	}
}

// Called when an incoming message from phone-app is dropped
static void in_dropped_handler(AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "dropped: %s", translate_error(reason));
}

// Called when phone-app does not acknowledge receipt of a message
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "failed: %s", translate_error(reason));
}



static void main_window_load(Window *window) {
  // Todo: Create and add child windows (text layers).
  // Create status msg TextLayer
  ////20150716 s_status_layer = text_layer_create(GRect(0, 20, 144, 150));
  s_status_layer = text_layer_create(GRect(0, 40, 144, 150));
   
  text_layer_set_background_color(s_status_layer, GColorClear);
  text_layer_set_text_color(s_status_layer, GColorBlack);
  ////20150716 text_layer_set_text(s_status_layer, "00:00");
  text_layer_set_text(s_status_layer, "MyMsg");
  
  // Improve the layout to use larger, bolder font.
  text_layer_set_font(s_status_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  ////20150715WayBig text_layer_set_font(s_status_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  ////20150715TadBig text_layer_set_font(s_status_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(s_status_layer, GTextAlignmentCenter);
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_status_layer));

  // Create time of day TextLayer
  s_time_layer = text_layer_create(GRect(0, 0, 144, 40));
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
}

static void main_window_unload(Window *window) {
  // Todo: Destroy child windows created by main_window_load(..).
  // Destroy status msg TextLayer
  text_layer_destroy(s_status_layer);
  text_layer_destroy(s_time_layer);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // Subcribe to tick minute handler to show time of day.
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler_minute);
  
  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Todo: more initialization. Register ui handlers, etc.
  // Register app message handlers
	app_message_register_inbox_received(in_received_handler); 
	app_message_register_inbox_dropped(in_dropped_handler); 
	app_message_register_outbox_failed(out_failed_handler);

  // Open message channel with phone.
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

}

static void deinit() {
  // Deregister handler for app message.	
  app_message_deregister_callbacks();
  
  // Destroy Window created by init(..).
   window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

