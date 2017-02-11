//#define BLUETOOTH_FONT fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FUTURA_MEDIUM_20))
#include <pebble.h>
// Pragma once makes the emulator respond more similarly to the phone.
// Without this the watch may have bugs not visible in the emulator.
#pragma once
// Largest font size for futura bold
#define FONT fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FUTURA_BOLD_47))
#define FONT2 fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FUTURA_BOLD_42))
#define DATE_FONT fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FUTURA_MEDIUM_13))
#define HEALTH_FONT fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FUTURA_MEDIUM_22))
#define ASCII_ZERO 48

// Persistent storage key
#define SETTINGS_KEY 1

// There's a difference between what looks good in the emulator and what looks good on the watch 
#define LEADING_LARGE_ZERO_OFFSET -(centerZeroSize.w/5)
//-(centerZeroSize.w/3)
#define LEADING_SMALL_ZERO_OFFSET centerZeroSmallSize.w/4

#define SETTINGS_KEY 1

// A structure containing our settings
typedef struct ClaySettings {
  bool darkMode;
  bool trimLeadingZero;
  int8_t bluetoothVibrationType;
  int8_t bluetoothReconnectionVibrationType;
  bool showBluetoothIcon;
  bool dateAboveTime;
  bool abridgedMonth;
  bool hideDate;
  bool healthEnabled;
  
  // Bool should be good enough now but is int because we may want to add more options later
  // 0 = right
  // 1 = center
  int8_t dateAlignment;
} __attribute__((__packed__)) ClaySettings;

// See if we have Pebble health (healthCapable = false iff running on OG Pebble and Pebble Steel)
#if defined(PBL_HEALTH)
#define healthCapable true
#define MAKE_STEPS stepsBitmap=gbitmap_create_with_resource(RESOURCE_ID_STEPS_ICON);
#define MAKE_STEPS_BLACK stepsBitmap=gbitmap_create_with_resource(RESOURCE_ID_STEPS_ICON_BLACK);
#define DESTROY_STEPS gbitmap_destroy(stepsBitmap);
#else
#define healthCapable false
#define MAKE_STEPS
#define MAKE_STEPS_BLACK
#define DESTROY_STEPS
#endif

static void main_window_load(Window *window);
static void main_window_unload(Window *window);
static void init();
static void deinit();
static void update_time();
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
static void drawCenterBox(bool leadidngZero);
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context);
static void bluetooth_callback(bool connected);
static void messageFailed(AppMessageResult reason, void *context);
static void drawDate();
static void drawHealthBar();
static void updateSteps();
static int getSteps();
static void health_handler(HealthEventType event, void *context);