// Grant of license under MIT License (MIT) stated in license.h.
#include "license.h"
// Template for starting a new Pebble C Project
#include <pebble.h>
static const char* pVersion = "v1.1.007 09/07/2015";  
static Window *s_main_window;
static TextLayer *s_status_layer; // Text msgs on watch face.
static TextLayer *s_time_layer; // Current time on watch face.

// This is a scroll layer
static ScrollLayer *s_scroll_layer; 
static int s_wBounds; 

// Prepare text set in s_status_layer for scrolling.
static void trim_and_scroll_status() {
  GSize max_size = text_layer_get_content_size(s_status_layer);
  text_layer_set_size(s_status_layer, max_size);
  scroll_layer_set_content_size(s_scroll_layer, GSize(s_wBounds, max_size.h + 4));
}


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

// Sends text to phone.
void send_text(const char *text){
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
  dict_write_cstring(iter, 0, "text");
	dict_write_cstring(iter, 1, text);
  dict_write_end(iter);
  app_message_outbox_send();
}

// Sends a click message to phone.
// nButtonId is button id. Use constants BUTTON_ID_SELECT, etc.
// nClickType: 1 for single click, 2 for double click, 3 for long click.
void send_click(uint8_t nButtonId, uint8_t nClickType){
	DictionaryIterator *iter;
	
	app_message_outbox_begin(&iter);
	dict_write_cstring(iter, 0, "click");
  
  dict_write_uint8(iter, 1, nButtonId);
  dict_write_uint8(iter, 2, nClickType);
	
	dict_write_end(iter);
  app_message_outbox_send();
}

// Up button single click.
void show_version_click_handler(ClickRecognizerRef recognizer, void* context) {
  // Set scrolling offset to zero.
  GPoint offsetZero = {0,0};
  scroll_layer_set_content_offset(s_scroll_layer, offsetZero, false);
  text_layer_set_text(s_status_layer, pVersion);
  trim_and_scroll_status();  
  
  // send_text("Text msg from Pebble."); // send_text() just to test, not needed.
}


// Select button single click. Send click message to phone.
void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  // ... called on single click ...
  // Window *window = (Window *)context;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Select button clicked");
  
  text_layer_set_text(s_status_layer, "Getting geolocation...");
  GPoint offsetZero = {0,0};
  scroll_layer_set_content_offset(s_scroll_layer, offsetZero, false);
  
  send_click(BUTTON_ID_SELECT, 1); 
}

// Configure button clicks after configuration for scrolling of UP and DOWN button has been done.
// Note: Do not configure here the UP and DOWN button single clicks because they are used for scrolling.
void config_scroll_click_extension(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
  // long click config:
  window_long_click_subscribe(BUTTON_ID_SELECT, 700, show_version_click_handler, NULL);
}


/* // No longer used. Configure for scrolling layer used instead.
// Configuration for button clicks.
// See Pebble C documentation: User Interface, Clicks.
void config_provider(Window *window) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
  // window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
  

  // Other config examples not used now.
  // Repeating single click:
  // window_single_repeating_click_subscribe(BUTTON_ID_SELECT, 1000, select_single_click_handler);

  // multi click config:
  // window_multi_click_subscribe(BUTTON_ID_SELECT, 2, 10, 0, true, select_multi_click_handler);

  // long click config:
  // window_long_click_subscribe(BUTTON_ID_SELECT, 700, select_long_click_handler, select_long_click_release_handler);
}
*/



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
      if (key == 0) {
        // Set scrolling offset to zero.
        GPoint offsetZero = {0,0};
        scroll_layer_set_content_offset(s_scroll_layer, offsetZero, false);
			  text_layer_set_text(s_status_layer, tuple->value->cstring);
        /* // Update time no longer used.
        // Set update time.
        time_t now = time(NULL);
        struct tm* ptmNow = localtime(&now);
        static char s_time_buffer[] = "upd 00:00:00x ";
        strftime(s_time_buffer, sizeof(s_time_buffer), "%I:%M:%S", ptmNow);
        const char* xm_time = AmOrPm(ptmNow);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "UpdTime: %s%s", s_time_buffer, xm_time);
        strcat(s_time_buffer, xm_time);
        */
        trim_and_scroll_status();  
      }
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
  text_layer_set_text(s_status_layer, "Error, check MyTrail Phone App is running.");
}



static void main_window_load(Window *window) {
  // Todo: Create and add child windows (text layers).
  
  // Get the root layer
  Layer *window_layer = window_get_root_layer(window);
  // Get the bounds of the window for sizing the text layer
  GRect bounds = layer_get_frame(window_layer);  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "get frame bounds: x:%i, y:%i, w:%i, h:%i",  
          bounds.origin.x, bounds.origin.y, bounds.size.w, bounds.size.h);
  GRect max_text_bounds = GRect(0, 0, bounds.size.w, 2000); 
  
  // Initialize the scroll layer  
  const int nRowH = 30;
  s_scroll_layer = scroll_layer_create(GRect(0, nRowH, bounds.size.w, bounds.size.h-nRowH-2)); 

  // This binds the scroll layer to the window so that up and down map to scrolling
  // You may use scroll_layer_set_callbacks to add or override interactivity
  scroll_layer_set_click_config_onto_window(s_scroll_layer, window);
  
  // void scroll_layer_set_callbacks(ScrollLayer * scroll_layer, ScrollLayerCallbacks callbacks)
  // Set click handlers in addition to the default scrolling single click for UP and DOWN.
  struct ScrollLayerCallbacks callbacks;
  callbacks.click_config_provider = config_scroll_click_extension;
  callbacks.content_offset_changed_handler = NULL;
  scroll_layer_set_callbacks(s_scroll_layer, callbacks);
  
  
  // Create status msg TextLayer
  s_status_layer = text_layer_create(max_text_bounds);  
  text_layer_set_background_color(s_status_layer, GColorClear);
  text_layer_set_text_color(s_status_layer, GColorBlack);
  text_layer_set_text(s_status_layer, "MyMsg");
  // Note: Do not call trim_and_scroll_status(). Not sure why, but does not show MyMsg properly.
  
  // Improve the layout to use larger, bolder font.
  text_layer_set_font(s_status_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_status_layer, GTextAlignmentCenter);
  
  // Add the layers for display
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_status_layer));
  layer_add_child(window_layer, scroll_layer_get_layer(s_scroll_layer));
    
  // Create time of day TextLayer
  s_time_layer = text_layer_create(GRect(0, 0, bounds.size.w, nRowH));
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
}

static void main_window_unload(Window *window) {
  // Todo: Destroy child windows created by main_window_load(..).
  // Destroy status msg TextLayer
  text_layer_destroy(s_status_layer);
  text_layer_destroy(s_time_layer);
  
  scroll_layer_destroy(s_scroll_layer); 
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

