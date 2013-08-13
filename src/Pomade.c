// TBD: app is the controller, owns timer and click handlers (which are registered
// via the view), view is the window/layer/etc, model is pomodoro
// controller makes decisions based on model and input coming called back from
// the view, then updates the model and updates the view with the model

#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>

#include "pomodoro.h"

#define MY_UUID { 0x78, 0x1D, 0x21, 0x66, 0x09, 0x09, 0x4F, 0x9C, 0x88, 0xFD, 0x89, 0x9B, 0x04, 0xBF, 0x5E, 0x32 }

PBL_APP_INFO(MY_UUID,
             "Pomade", "Jon Speicher",
             0, 1, /* App version */
             RESOURCE_ID_MENU_ICON_POMADE,
             APP_INFO_STANDARD_APP);

Window window;
TextLayer timer_layer;
ActionBarLayer action_bar;

HeapBitmap bmp_action_bar_icon_start;
HeapBitmap bmp_action_bar_icon_abort;
HeapBitmap bmp_action_bar_icon_restart;

AppContextRef application_context;
AppTimerHandle timer;

#define POMODORO_COOKIE 1
#define POMODORO_TICK_PERIOD_SEC 1
#define POMODORO_TICK_PERIOD_MS  POMODORO_TICK_PERIOD_SEC * 1000

static Pomodoro pomodoro;

void window_load(Window* window) {
  action_bar_layer_add_to_window(&action_bar, window);
  action_bar_layer_set_icon(&action_bar, BUTTON_ID_SELECT, &bmp_action_bar_icon_start.bmp);
}

void window_unload(Window* window) {
  action_bar_layer_remove_from_window(&action_bar);
}

void select_clicked(ClickRecognizerRef recognizer, void* context) {
  if (pomodoro.running) {
    app_timer_cancel_event(application_context, timer);
    pomodoro_abort(&pomodoro);
    vibes_double_pulse();
    action_bar_layer_set_icon(&action_bar, BUTTON_ID_SELECT, &bmp_action_bar_icon_restart.bmp);
  } else {
    timer = app_timer_send_event(application_context, POMODORO_TICK_PERIOD_MS, POMODORO_COOKIE);
    pomodoro_init(&pomodoro);
    text_layer_set_text(&timer_layer, pomodoro.time_left_string);
    pomodoro_start(&pomodoro);
    action_bar_layer_set_icon(&action_bar, BUTTON_ID_SELECT, &bmp_action_bar_icon_abort.bmp);
  }
}

void click_config_provider(ClickConfig **config, void* context) {
  config[BUTTON_ID_SELECT]->click.handler = select_clicked;
}

void handle_timer(AppContextRef ctx, AppTimerHandle handle, uint32_t cookie) {
  if (cookie == POMODORO_COOKIE) {
    pomodoro_decrement_by_seconds(&pomodoro, POMODORO_TICK_PERIOD_SEC);
    text_layer_set_text(&timer_layer, pomodoro.time_left_string);

    if (pomodoro.complete) {
      vibes_long_pulse();
    } else {
      timer = app_timer_send_event(application_context, POMODORO_TICK_PERIOD_MS, POMODORO_COOKIE);
    }
  }
}

void handle_init(AppContextRef ctx) {
  application_context = ctx;
  pomodoro_init(&pomodoro);

  resource_init_current_app(&APP_RESOURCES);
  heap_bitmap_init(&bmp_action_bar_icon_start, RESOURCE_ID_ACTION_BAR_ICON_START);
  heap_bitmap_init(&bmp_action_bar_icon_abort, RESOURCE_ID_ACTION_BAR_ICON_ABORT);
  heap_bitmap_init(&bmp_action_bar_icon_restart, RESOURCE_ID_ACTION_BAR_ICON_RESTART);

  window_init(&window, "Pomade");
  window_set_window_handlers(&window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  action_bar_layer_init(&action_bar);
  action_bar_layer_set_click_config_provider(&action_bar, click_config_provider);

  text_layer_init(&timer_layer,
    GRect(0, 10, window.layer.frame.size.w - ACTION_BAR_WIDTH, window.layer.frame.size.h));
  text_layer_set_text_alignment(&timer_layer, GTextAlignmentCenter);
  text_layer_set_font(&timer_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  text_layer_set_text(&timer_layer, pomodoro.time_left_string);
  layer_add_child(&window.layer, &timer_layer.layer);

  window_stack_push(&window, true /* Animated */);
}

void handle_deinit(AppContextRef ctx) {
  heap_bitmap_deinit(&bmp_action_bar_icon_start);
  heap_bitmap_deinit(&bmp_action_bar_icon_abort);
  heap_bitmap_deinit(&bmp_action_bar_icon_restart);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,
    .timer_handler = &handle_timer
  };
  app_event_loop(params, &handlers);
}
