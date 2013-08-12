#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#define MY_UUID { 0x78, 0x1D, 0x21, 0x66, 0x09, 0x09, 0x4F, 0x9C, 0x88, 0xFD, 0x89, 0x9B, 0x04, 0xBF, 0x5E, 0x32 }

PBL_APP_INFO(MY_UUID,
             "Pomade", "Jon Speicher",
             0, 1, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_STANDARD_APP);

Window window;
TextLayer timerLayer;

AppTimerHandle timer_handle;
#define COOKIE_MY_TIMER 1

void handle_timer(AppContextRef ctx, AppTimerHandle handle, uint32_t cookie) {

  if (cookie == COOKIE_MY_TIMER) {
      text_layer_set_text(&timerLayer, "Timer happened!");
  }

  // If you want the timer to run again you need to call `app_timer_send_event()`
  // again here.
}

void handle_init(AppContextRef ctx) {
  window_init(&window, "Pomade");
  window_stack_push(&window, true /* Animated */);

  text_layer_init(&timerLayer, window.layer.frame);
  text_layer_set_text(&timerLayer, "Waiting for timer...");
  layer_add_child(&window.layer, &timerLayer.layer);

  timer_handle = app_timer_send_event(ctx, 1500 /* milliseconds */, COOKIE_MY_TIMER);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .timer_handler = &handle_timer
  };
  app_event_loop(params, &handlers);
}
