#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "string.h"


#define MY_UUID { 0x7C, 0xA0, 0x2F, 0xE1, 0x34, 0x63, 0x4C, 0xB4, 0x98, 0xD5, 0x98, 0x96, 0x3F, 0x82, 0x39, 0xB5 }
PBL_APP_INFO(MY_UUID,
 "Reminder", "Wesley Winder",
             1, 0, /* App version */
 DEFAULT_MENU_ICON,
 APP_INFO_WATCH_FACE);

#define HOURS_IN_A_DAY 24
#define MINS_IN_AN_HOUR 60

Window window;

TextLayer timeLayer;
TextLayer reminderLayer;

char *tasks[HOURS_IN_A_DAY][MINS_IN_AN_HOUR];

 // Vibe pattern: ON for 1000ms, OFF for 500ms, ON for 1000ms:
static const uint32_t const newTaskSegments[] = { 1000, 500, 1000 };
VibePattern newTaskPattern = {
  .durations = newTaskSegments,
  .num_segments = ARRAY_LENGTH(newTaskSegments),
};

void init_task_array() {
  for (int i = 0; i < HOURS_IN_A_DAY; i++) {
    for (int j = 0; j < MINS_IN_AN_HOUR; j++) {
      if (i < 9) {
        tasks[i][j] = "Sleep";
      } else if (i < 10) {
        tasks[i][j] = "Morning Activities";
      } else if (i < 12) {
        tasks[i][j] = "Work on Pebble App";
      } else if (i < 13) {
        tasks[i][j] = "Lunch Time";
      } else if (i < 17) {
        tasks[i][j] = "Work on Blastoff";
      } else if (i < 19) {
        tasks[i][j] = "Dinner Break";
      } else if (i < 22) {
        tasks[i][j] = "Work on Blastoff";
      } else if (i < 23) {
        tasks[i][j] = "Plan Goals for Tomorrow";
      } else {
        tasks[i][j] = "Get Ready for Bed";
      }
    }
  }
}

// Called once per minute
void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {

  static char timeText[] = "00:00"; // Needs to be static because it's used by the system later.

  PblTm currentTime;


  get_time(&currentTime);

  string_format_time(timeText, sizeof(timeText), "%T", &currentTime);

  const char* currentTask = text_layer_get_text(&reminderLayer);
  const char* nextTask = tasks[currentTime.tm_hour][currentTime.tm_min];

  if (strcmp(currentTask, nextTask)) {
    vibes_enqueue_custom_pattern(newTaskPattern);
  }

  text_layer_set_text(&timeLayer, timeText);

  text_layer_set_text(&reminderLayer, nextTask);

  if (currentTime.tm_min == 0) {
    vibes_long_pulse();
  } else if (currentTime.tm_min % 5 == 0) {
    vibes_double_pulse();
  }
  
}

void handle_init(AppContextRef app_ctx) {

  window_init(&window, "Window Name");
  window_stack_push(&window, true /* Animated */);
  window_set_background_color(&window, GColorBlack);

  // Init the text layer used to show the time
  text_layer_init(&timeLayer, GRect(45, 20, 144-45 /* width */, 168-20 /* height */));
  text_layer_set_text_color(&timeLayer, GColorWhite);
  text_layer_set_background_color(&timeLayer, GColorClear);
  text_layer_set_font(&timeLayer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));

  // Init layer for current task to be displayed
  text_layer_init(&reminderLayer, GRect(0, 60, 144, 168-60));
  text_layer_set_text_color(&reminderLayer, GColorWhite);
  text_layer_set_background_color(&reminderLayer, GColorClear);
  text_layer_set_font(&reminderLayer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(&reminderLayer, GTextAlignmentCenter);

  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  handle_minute_tick(app_ctx, NULL);

  layer_add_child(&window.layer, &timeLayer.layer);
  layer_add_child(&window.layer, &reminderLayer.layer);
}


void pbl_main(void *params) {

  init_task_array();

  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,

    .tick_info = {
      .tick_handler = &handle_minute_tick,
      .tick_units = MINUTE_UNIT
    }
  };
  app_event_loop(params, &handlers);
}
