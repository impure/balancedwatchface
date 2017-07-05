#include <pebble.h>
#include "main.h"

// Note: must enable 'health' in settings
// Apparently finding the size of health text is bugged so we use
// the date text * 2 as an approximator. Possibly due to using the
// same file as the date font.

// [Fixed] Vibration test on settings change only trigger if settings actually change
// [Fixed] Bug: Time is weird. Mod 12 is not sufficient for adequately computing time.
// [Fixed] Bug: Text appearing too far up is caused by text being too long causing it to create an extra line.
//                          This messes up the height formula and causes the math to be off.

// Layer and font for center time
static TextLayer *s_time_layer;
static GFont s_time_font;
static GFont centerFontSmall;
static GFont dateFont;
static bool twoDigitHour; // Do we have a 2 hour digit?

// Total screen realistate
static GRect bounds;

// The textsize, position, and text box for the center text.
// As well as the size of a single character (we use zero)
static TextLayer *dateLayer;
static GSize centerTextSize;
static GRect centerTextBox;
static GPoint centerBoxPoint;
static GSize centerZeroSize;

// The textsize, position, and text box for the date text.
static GRect dateTextBox;
static GPoint dateBoxPoint;
static GSize dateZeroSize;

// Parameters for the health bar
static TextLayer *healthBar;
static GRect bottomTextBox;
static GPoint bottomTextPoint;
// static GSize bottomZeroSize;
static GFont healthFont;
static int prevChars = 1;
static bool initializedHealth = false;
static bool initializedHealthIcon = false;

// Steps icon
static BitmapLayer *stepsLayer;
static GBitmap *stepsBitmap;
//static GBitmap *stepsBitmapBlack;
  
// If this is the first iteration or not
bool initializedTimebox;

// Information about main window and layer
static Layer *window_layer;
static Window *s_main_window;

// Bluetoth icon/layer/font
static BitmapLayer *s_bt_icon_layer;
static GBitmap *s_bt_icon_bitmap;
//static GFont bluetoothFont;

// User customizable settings
ClaySettings settings;

// Only for debugging. Remove after.
// This is to artificially increase step count as I don't know how to do this in emulator
// static int debugSteps = 0;


// Handle bluetooth disconnect. Note: takes a long time to notice we've lost
// bluetooth connection.
static void bluetooth_callback(bool connected) {
  
  // Show icon if disconnected and showing bluetooth icon is enabled
  layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected || !settings.showBluetoothIcon);
  
  // Lost connection
  if (!connected) {
    switch (settings.bluetoothVibrationType) {
      case 0: // No vibration
        break;
      case 1: // Short vibration
        vibes_short_pulse();
        break;
      case 2: // Long vibration
        vibes_long_pulse();
      case 3: // Double vibration
        vibes_double_pulse();
      default:
        break;
    }
    
    // Got connection
  } else {
    switch (settings.bluetoothReconnectionVibrationType) {
      case 0: // No vibration
        break;
      case 1: // Short vibration
        vibes_short_pulse();
        break;
      case 2: // Long vibration
        vibes_long_pulse();
      case 3: // Double vibration
        vibes_double_pulse();
      default:
        break;
    }
  }
}

// Handle recieved messages
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  bool updateDate = false;
  bool updateHealth = false;
  
  // Change settings for various options
  // Note: we must call the changes immediately or they won't show up
  // until redrawing
  // If we want a dark mode
  Tuple *darkModeMessage = dict_find(iter, MESSAGE_KEY_darkMode);
  if (darkModeMessage && settings.darkMode != darkModeMessage->value->int8) {
    
    settings.darkMode = darkModeMessage->value->int8;
    
    if (settings.darkMode) {
      text_layer_set_text_color(s_time_layer, GColorWhite);
      if (!settings.hideDate) {
        text_layer_set_text_color(dateLayer, GColorWhite);
      }
      window_set_background_color(s_main_window, GColorBlack);
    } else {
      if (!settings.hideDate) {
        text_layer_set_text_color(dateLayer, GColorBlack);
      }
      text_layer_set_text_color(s_time_layer, GColorBlack);
      window_set_background_color(s_main_window, GColorWhite);
    }
		
		
		if (settings.darkMode) {
			DESTROY_STEPS
			MAKE_STEPS
		} else {
			DESTROY_STEPS
			MAKE_STEPS_BLACK
		}
    
  }
  
  // Trim leading zero
  Tuple *trimLeadingZeroMessage = dict_find(iter, MESSAGE_KEY_trimLeadingZero);
  if (trimLeadingZeroMessage) {
    settings.trimLeadingZero = trimLeadingZeroMessage->value->int8;
    
    updateDate = true;
    
  }
  
  // Abridged Month
  Tuple *abridgedMonth = dict_find(iter, MESSAGE_KEY_abridgedMonth);
  if (abridgedMonth) {
    settings.abridgedMonth = abridgedMonth->value->int8;
    
    updateDate = true;
    
  }
  
  // Hide date
  Tuple *hideDate = dict_find(iter, MESSAGE_KEY_hideDate);
  if (hideDate) {
    settings.hideDate = hideDate->value->int8;
    
		// First destroy the date layer
    text_layer_destroy(dateLayer);
		
    updateDate = true;
    
  }
  
  // Set type of vibration
  Tuple *bluetoothVibrationType = dict_find(iter, MESSAGE_KEY_bluetoothVibrationType);
  if (bluetoothVibrationType) {
    
    // Test out the vibration we set
    if (settings.bluetoothVibrationType != bluetoothVibrationType->value->int8 - 48) {
    
      // - 48 because for some reason pebble is giving us the ascii value of our integers
      // not the integer themselves
      settings.bluetoothVibrationType = bluetoothVibrationType->value->int8 - 48;
      
      switch (settings.bluetoothVibrationType) {
        case 0: // No vibration
          break;
        case 1: // Short vibration
          vibes_short_pulse();
          break;
        case 2: // Long vibration
          vibes_long_pulse();
        case 3: // Double vibration
          vibes_double_pulse();
        default:
          break;
      }
    }
  }
  
  // Set type of vibration on reconnect
  Tuple *bluetoothReconnectionVibrationType = dict_find(iter, MESSAGE_KEY_bluetoothReconnectionVibrationType);
  if (bluetoothReconnectionVibrationType) {
    
    // Test out the vibration we set
    if (settings.bluetoothReconnectionVibrationType != bluetoothReconnectionVibrationType->value->int8 - 48) {
  
      // - 48 because for some reason pebble is giving us the ascii value of our integers
      // not the integer themselves
      settings.bluetoothReconnectionVibrationType = bluetoothReconnectionVibrationType->value->int8 - 48;
      
      switch (settings.bluetoothReconnectionVibrationType) {
        case 0: // No vibration
          break;
        case 1: // Short vibration
          vibes_short_pulse();
          break;
        case 2: // Long vibration
          vibes_long_pulse();
        case 3: // Double vibration
          vibes_double_pulse();
        default:
          break;
      }
    }
  }
  
  // Set allignment of date
  Tuple *dateAlignment = dict_find(iter, MESSAGE_KEY_dateAlignment);
  if (dateAlignment) {
    
    // - 48 because for some reason pebble is giving us the ascii value of our integers
    // not the integer themselves
    settings.dateAlignment = dateAlignment->value->int8 - 48;
    
    updateDate = true;
  }
  
  // Enable health
  Tuple *healthStatus = dict_find(iter, MESSAGE_KEY_healthEnabled);
  if (healthStatus && settings.healthEnabled != healthStatus->value->int8) {
    
    settings.healthEnabled = healthStatus->value->int8;
    
    if (settings.healthEnabled) {
      health_service_events_subscribe(health_handler, NULL);
      drawHealthBar();
      updateSteps();
    } else {
      // Health subscriptions may be computationally expensive so if we don't need it unsubscribe.
      text_layer_destroy(healthBar);
      bitmap_layer_destroy(stepsLayer);
      health_service_events_unsubscribe();
      initializedHealth = false;
      initializedHealthIcon = false;
    }
       
  }

  
  // Do we want a bluetooth icon?
  Tuple *showBluetoothIcon = dict_find(iter, MESSAGE_KEY_showBluetoothIcon);
  if (showBluetoothIcon) {
    settings.showBluetoothIcon = showBluetoothIcon->value->int8;
    // Direct call rather than calling handler. Because calling handler would cause vibration.
    layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), 
                     connection_service_peek_pebble_app_connection() || !settings.showBluetoothIcon);
  }
  
  // Do we want the month above or under the time?
  Tuple *dateAboveTime = dict_find(iter, MESSAGE_KEY_dateAboveTime);
  if (dateAboveTime) {
    
    settings.dateAboveTime = dateAboveTime->value->int8;
    
    updateHealth = true;
  }
  
  // Save data to settings
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
  
  
  
  // If requested draw date
  if (updateDate) {
    // Redraw the clock 
    if (!settings.hideDate) {
      text_layer_destroy(dateLayer);
    }
    text_layer_destroy(s_time_layer);
    initializedTimebox = false;
    update_time();
  }

  
  // Update all objects
  if (updateHealth) { 
    update_time();
    drawHealthBar();
    updateSteps();
  }
}

// Handle failed messages
static void messageFailed(AppMessageResult reason, void *context) {
  // A message was received, but had to be dropped
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped. Reason: %d", (int)reason);
}

// Update the step counter with the number of steps
static void updateSteps() {
      static char numSteps[8];
  
      // Save num steps to string
      snprintf(numSteps, 7, "%d", getSteps());
  
      // If the number of characters changed we need to draw everything again. This has to be
      // != and not less than because the number goes down at the end of every day
      if ((int)strlen(numSteps) != prevChars) {
        prevChars = strlen(numSteps);
        drawHealthBar();
      }
  
      // Update steps
      text_layer_set_text(healthBar, numSteps); 
}

// Get the number of steps since last midnight
static int getSteps() {
      HealthMetric metric;
      HealthServiceAccessibilityMask mask;
  
      metric = HealthMetricStepCount;
      
      // Time interval is from start of day (midnight) to now
      time_t start = time_start_of_today();
      time_t end = time(NULL);

      // Check the metric has data available for today
      mask = health_service_metric_accessible(metric, start, end);
      if (mask & HealthServiceAccessibilityMaskAvailable) {
          return (int)health_service_sum_today(metric);
      } else {
        return 0;
      }
}

// Called when health subscription sends an update
static void health_handler(HealthEventType event, void *context) {
  
  // Which type of event occurred?
  switch(event) {
    case HealthEventSignificantUpdate:
    
       updateSteps();
      break;
    case HealthEventMovementUpdate:
    
      updateSteps();
      break;
    case HealthEventSleepUpdate:
      break;
    case HealthEventHeartRateUpdate:
      break;
    case HealthEventMetricAlert:
      break;
  }
}

// Loads the main window and configures contents
static void main_window_load(Window *window) {
  
  // Initialize things that need memory (except bitmaps)
  s_time_font = FONT;
  centerFontSmall = FONT2;
  window_layer = window_get_root_layer(window);
  s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BLUETOOTH_ICON);
  //stepsBitmapBlack = gbitmap_create_with_resource(RESOURCE_ID_STEPS_ICON_BLACK);
  dateFont = DATE_FONT;
  healthFont = HEALTH_FONT;
  
  // Initialize global variables
  initializedTimebox = false;
  bounds = layer_get_bounds(window_layer);
  settings.darkMode = true;
  settings.trimLeadingZero = true;
  settings.bluetoothVibrationType = 0;
  settings.bluetoothReconnectionVibrationType = 0;
  settings.showBluetoothIcon = true;
  settings.dateAboveTime = true;
  settings.abridgedMonth = false;
  settings.hideDate = false;
  settings.healthEnabled = healthCapable;
  settings.dateAlignment = 1;

  // Create the Bluetooth bitmap layer
  // If round pebble draw bluetooth lost in middle of display or it'll be clipped by corners
  PBL_IF_ROUND_ELSE(s_bt_icon_layer = bitmap_layer_create(GRect(bounds.size.w/2 - 20, 10, 40, 40)), 
                    s_bt_icon_layer = bitmap_layer_create(GRect(0, 0, 40, 40)));
  bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_layer));
  
  // Try reloading settings from data
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
  
  // Check if health is enabled, user wants health, and health subscribe works. If it isn't disable health.
  if(healthCapable && settings.healthEnabled && !health_service_events_subscribe(health_handler, NULL)) {
    settings.healthEnabled = false;
  }
	
	// Initialize bitmaps
	if (settings.darkMode) {
  	MAKE_STEPS
	} else {
		MAKE_STEPS_BLACK	
	}
  
  // Initialize bluetooth_callback with whether or not we're connected
  // Temporarily hide switch vibration to no vibration to avoid vibrating at start
  layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), 
                     connection_service_peek_pebble_app_connection() || !settings.showBluetoothIcon);
  
  // Open AppMessage to send and recieve data
  // Register all of the things
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_register_inbox_dropped(messageFailed);
  app_message_open(128, 0);
  
  // Subscribe to minute interval and bluetooth
  // Note: putting this in init() causes it to silently fail
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });
  
  
  // Calculate the size of zeros
  dateZeroSize = graphics_text_layout_get_content_size("0", dateFont, 
                                                      GRect(0, 0, bounds.size.w, bounds.size.h), 
                                                      GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft);
 
  
  
  // Make sure the time is displayed from the start. This must be the last line or text will look off.
  update_time();
  
  
}

// Destroys excess memory
static void main_window_unload(Window *window) {
  
  // Unload layers
  text_layer_destroy(s_time_layer);
  if (!settings.hideDate) {
    text_layer_destroy(dateLayer);
  }
	if (initializedHealth) {
    text_layer_destroy(healthBar);
	}
	if (initializedHealthIcon) {
  	bitmap_layer_destroy(stepsLayer);
	}
  bitmap_layer_destroy(s_bt_icon_layer);
  
  // Unload fonts
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(centerFontSmall);
  fonts_unload_custom_font(dateFont);
  fonts_unload_custom_font(healthFont);
  
  // Unload bitmaps
  gbitmap_destroy(s_bt_icon_bitmap);
	DESTROY_STEPS
	//gbitmap_destroy(stepsBitmapBlack);
}

// Initialize components. Only called once where load is called multiple times. Things that don't change
// we may wish to put here.
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Set background colour for the whole window
  if (settings.darkMode) {
    window_set_background_color(s_main_window, GColorBlack);
  }
  // This sets Locale to Pebbles's Locale Settings
  setlocale(LC_TIME, "");
}

// Destroy window on closing
static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

// Don't know why we have this but it works
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

// Sets up drawing
static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  bool nowTwoDigitHour;
  static char s_buffer[8]; // This has to be static. Why? ¯\_(ツ)_/¯
  static char dateString[22];
  static char day[3];
  
  // Write the current hours and minutes into a buffer
  if /* The first character is a '0' indicating leading zero */ (!settings.trimLeadingZero) {
    strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);
    nowTwoDigitHour = true;
  } else {
    strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%k:%M" : "%l:%M", tick_time);
    // Convert tm_hour from 24 hour to 12 hour
    if (clock_is_24h_style()) {
      nowTwoDigitHour = tick_time->tm_hour >= 10;
    } else {
      // 12 hour time makes no sense
      nowTwoDigitHour = (tick_time->tm_hour % 12 == 0 || tick_time->tm_hour % 12 >= 10);
    }
  }
  
  // Get the month and shorten it if there is a leading 0
  // Day has to be length 3 to hold the terminating character
  strftime(day, sizeof(day), "%d", tick_time);

  // get the system language configuration
  const char * sys_locale = i18n_get_system_locale();
  const char * strftimestring;  
  if (strcmp(sys_locale,"es_ES")==0) {
    if   (settings.abridgedMonth) {
      if (day[0] == '0'){
        strftimestring = "%a%e de %b";
      }else{
        strftimestring = "%a %e de %b";
      }
    }else{
      strftimestring = "%e de %B";
    }
  }else if (strcmp(sys_locale,"fr_FR")==0) {
    if (settings.abridgedMonth) {
      if (day[0] == '0'){
        strftimestring = "%a%e %b";
      }else{
        strftimestring = "%a %e %b";
      }
    }else{
      strftimestring = "%e %B";
    }
  }else if (strcmp(sys_locale,"de_DE")==0) {
    if (settings.abridgedMonth) {
      if (day[0] == '0'){
        strftimestring = "%a,%e. %b";
      }else{
        strftimestring = "%a, %e. %b";
      }
    }else{
      strftimestring = "%e. %B";
    }
  }else if (strcmp(sys_locale,"it_IT")==0) {
    if (settings.abridgedMonth) {
      if (day[0] == '0'){
        strftimestring = "%a%e %b";
      }else{
        strftimestring = "%a %e %b";
      }
    }else{
      strftimestring = "%e %B";
    }
  }else if (strcmp(sys_locale,"pt_PT")==0) {
    if (settings.abridgedMonth) {
      if (day[0] == '0'){      
        strftimestring = "%a,%e de %b";
      }else{
         strftimestring = "%a, %e de %b";
      }
    }else{
      strftimestring = "%e de %B";
    } 
  }else{
    if (settings.abridgedMonth){
      if (day[0] == '0'){      
        strftimestring = "%a, %b%e";
      }else{
        strftimestring = "%a, %b %e";
      }      
    }else{
      if (day[0] == '0'){      
        strftimestring = "%a, %B %e";
      }else{
        strftimestring = "%a, %B%e";
      }
    }
  }
  
  // Get the Date String of the pebble system Language with the srftime function
  strftime(dateString, sizeof(dateString), strftimestring, tick_time);
  
  // Calculate the size of the text and use that to set the point (middle center). 
  // Note: graphics_text_layout_get_content_size calculates size for single character not maximum size
  // We multiply bounds.size.w by 2 because this does some funny stuff involving overflowing text sooner
  // then it should resulting in unnecessary new lines and incorrect height
  if (nowTwoDigitHour) {
    centerTextSize = graphics_text_layout_get_content_size(s_buffer, centerFontSmall, 
                                                      GRect(0, 0, bounds.size.w * 2, bounds.size.h), 
                                                      GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft);
    
    centerZeroSize = graphics_text_layout_get_content_size("0", centerFontSmall, 
                                                      GRect(0, 0, bounds.size.w, bounds.size.h), 
                                                      GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter);
  } else {
    centerTextSize = graphics_text_layout_get_content_size(s_buffer, s_time_font, 
                                                      GRect(0, 0, bounds.size.w * 2, bounds.size.h), 
                                                      GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft);
    centerZeroSize = graphics_text_layout_get_content_size("0", s_time_font, 
                                                      GRect(0, 0, bounds.size.w, bounds.size.h), 
                                                      GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter);
  }
  
  if /* we haven't yet made a text layer */ (!initializedTimebox) {
    
    drawCenterBox(nowTwoDigitHour);
    if (!settings.hideDate) {
      drawDate();
    }
    if (settings.healthEnabled) {
      drawHealthBar();
      updateSteps();
    }
    
    // Set variables with the current state
    initializedTimebox = true;
    twoDigitHour = nowTwoDigitHour;
    
  } else if /* The state of the leading zero has changed */ (twoDigitHour != nowTwoDigitHour) {
    
    // Destroy and redraw layers
    text_layer_destroy(s_time_layer);
    drawCenterBox(nowTwoDigitHour);
    if (!settings.hideDate) {
      text_layer_destroy(dateLayer);
      drawDate();
    }
    if (settings.healthEnabled) {
      drawHealthBar();
      updateSteps();
    }
    
    // Set state
    twoDigitHour = nowTwoDigitHour;
    
  } else if (!settings.hideDate) /* Redraw date */ {
    
    text_layer_destroy(dateLayer);
    drawDate();
        
  }
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
  if (!settings.hideDate) {
    text_layer_set_text(dateLayer, dateString);
  }
  
}


// Draws time box
static void drawCenterBox(bool nowTwoDigitHour) {
  
		/* I wanted to allow Pebble to handle centering text on it's own but I guess that doesn't work
		
		// Font and alignment
    switch(settings.dateAlignment) {
      
      // Right aligned
      case 0:

				// Configure boxes that will be the position of our layers
				if (nowTwoDigitHour) {
					centerBoxPoint.x = 0;//bounds.size.w/2 - centerTextSize.w/2 + LEADING_SMALL_ZERO_OFFSET;
					centerBoxPoint.y = 0;
				} else {
					centerBoxPoint.x = LEADING_LARGE_ZERO_OFFSET;//bounds.size.w/2 - centerTextSize.w/2 + LEADING_LARGE_ZERO_OFFSET;
					centerBoxPoint.y = 0;
					//APP_LOG(APP_LOG_LEVEL_DEBUG, "Large offset");
				}
				centerBoxPoint.y += bounds.size.h/2 - 2 * centerTextSize.h/3;
				centerTextBox.origin = centerBoxPoint;
				centerTextBox.size = GSize(bounds.size.w, centerTextSize.h * 2); // A little extra height and width
      
      // Center aligned
      case 1:

				// Configure boxes that will be the position of our layers
				centerBoxPoint.x = 0;
				centerBoxPoint.y = 0;
			
				centerBoxPoint.y += bounds.size.h/2 - 2 * centerTextSize.h/3;
					
				centerTextBox.origin = centerBoxPoint;
				centerTextBox.size = GSize(bounds.size.w, centerTextSize.h * 2); // A little extra height and width
        break;
      default:
        break;
    }*/
			
			
		// Configure boxes that will be the position of our layers
		if (nowTwoDigitHour) /* We don't need as much padding */ {
			centerBoxPoint.x = 0;//bounds.size.w/2 - centerTextSize.w/2 + LEADING_SMALL_ZERO_OFFSET;
			centerBoxPoint.y = 0;
		} else {
			centerBoxPoint.x = LEADING_LARGE_ZERO_OFFSET;//bounds.size.w/2 - centerTextSize.w/2 + LEADING_LARGE_ZERO_OFFSET;
			centerBoxPoint.y = 0;
			//APP_LOG(APP_LOG_LEVEL_DEBUG, "Large offset");
		}
		centerBoxPoint.y += bounds.size.h/2 - 2 * centerTextSize.h/3;
		centerTextBox.origin = centerBoxPoint;
		centerTextBox.size = GSize(bounds.size.w, centerTextSize.h * 2); // A little extra height and width
	
    s_time_layer = text_layer_create(centerTextBox);
  
    // Set the font colour
    if (!settings.darkMode) {
      text_layer_set_text_color(s_time_layer, GColorBlack);
    } else {
      text_layer_set_text_color(s_time_layer, GColorWhite);
    }
  
    // Set the background colour to clear so we can actually see things
    text_layer_set_background_color(s_time_layer, GColorClear);
    // Insert my font
    if (nowTwoDigitHour) {
      text_layer_set_font(s_time_layer, centerFontSmall);
    } else {
      text_layer_set_font(s_time_layer, s_time_font);
    }
    // Old font pebble was using:
    //text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter); // Left align is easier to center

    // Add it as a child layer to the Window's root layer
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
}


// Draws the date and day
static void drawDate() {
  
  
    // Create the date layer
    dateBoxPoint.x = 0;
    if (settings.dateAboveTime) {
       dateBoxPoint.y = centerBoxPoint.y - 2 * (dateZeroSize.h/3); // Account for margins above characters
    } else {
       // The position of the text after this condition looks different in the emulator for some reason
       dateBoxPoint.y = centerBoxPoint.y + centerTextSize.h;// - dateZeroSize.h;
    }
    dateTextBox.origin = dateBoxPoint;
  
   	// Font and alignment
    switch(settings.dateAlignment) {
      
      // Right aligned
      case 0:      
        // Create the date layer
        dateTextBox.size = GSize(bounds.size.w/2 + centerTextSize.w/2 + centerBoxPoint.x
                                 - (centerZeroSize.w - dateZeroSize.w)/8, dateZeroSize.h * 2);
        //dateTextBox.size = GSize(bounds.size.w/2 + centerTextSize.w/2, dateZeroSize.h * 2); 
                      // We used to have - (centerZeroSize.w - dateZeroSize.w)/4 to take into account for 
                      // difference in left/right margins in font. But I guess we don't need that now.
        dateLayer = text_layer_create(dateTextBox); 
      
        // Set the font colour
        if (!settings.darkMode) {
          text_layer_set_text_color(dateLayer, GColorBlack);
        } else {
          text_layer_set_text_color(dateLayer, GColorWhite);
        }
      
        text_layer_set_text_alignment(dateLayer, GTextAlignmentRight); // Left align is easier to center
        break;
      
      // Center aligned
      case 1:
        dateTextBox.size = GSize(bounds.size.w, dateZeroSize.h * 2);
        //dateTextBox.size = GSize(bounds.size.w/2 + centerTextSize.w/2, dateZeroSize.h * 2); 
                      // We used to have - (centerZeroSize.w - dateZeroSize.w)/4 to take into account for 
                      // difference in left/right margins in font. But I guess we don't need that now.
        dateLayer = text_layer_create(dateTextBox); 
      
        // Set the font colour
        if (!settings.darkMode) {
          text_layer_set_text_color(dateLayer, GColorBlack);
        } else {
          text_layer_set_text_color(dateLayer, GColorWhite);
        }
        text_layer_set_text_alignment(dateLayer, GTextAlignmentCenter); 
        break;
      default:
        break;
    }
  
    text_layer_set_background_color(dateLayer, GColorClear);
    text_layer_set_font(dateLayer, dateFont);
    layer_add_child(window_layer, text_layer_get_layer(dateLayer));
  
}

// Draws Step Counter
static void drawHealthBar() {
    GSize stepSize;
    static char numSteps[8];
  
    // Skip if health disabled
    if (!settings.healthEnabled) {
      return;
    }
  
    // Get the size of the text
    snprintf(numSteps, 7, "%d", getSteps());
    stepSize = graphics_text_layout_get_content_size(numSteps, healthFont, 
                                                      GRect(0, 0, bounds.size.w, bounds.size.h), 
                                                      GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter);
  
  
    // Create the date layer. The box starts from the left + half size of icon and spans the entire distance
    bottomTextPoint.x = 10;
    // Health metrics should be on the other side of the date
    if (!settings.dateAboveTime) {
      bottomTextPoint.y = centerBoxPoint.y - 2 * (dateZeroSize.h); // Account for margins above characters
    } else {
      // The position of the text after this condition looks different in the emulator for some reason
      bottomTextPoint.y = centerBoxPoint.y + centerTextSize.h + dateZeroSize.h/3;
    }
    // Bug: this should use stepSize instead of dateZeroSize bu when I do that it won't show the icon
    bottomTextBox.size = GSize(bounds.size.w - 10, dateZeroSize.h * 3);
    bottomTextBox.origin = bottomTextPoint;
  
    // If we have already drawn the health bar destroy it here
    if (initializedHealth) {
      text_layer_destroy(healthBar);
    } else {
      initializedHealth = true;
    }
    healthBar = text_layer_create(bottomTextBox); 
  
  
    // Set the font colour
    if (!settings.darkMode) {
      text_layer_set_text_color(healthBar, GColorBlack);
    } else {
      text_layer_set_text_color(healthBar, GColorWhite);
    }
  
    // Font and alignment
    text_layer_set_background_color(healthBar, GColorClear);
    text_layer_set_font(healthBar, healthFont);
    text_layer_set_text_alignment(healthBar, GTextAlignmentCenter); // Left align is easier to center
    layer_add_child(window_layer, text_layer_get_layer(healthBar));
    
    bottomTextBox.origin.x = 0;
    // Hard coded value as icon is larger than text
    bottomTextBox.origin.y -= 5;
    bottomTextBox.size.w -= stepSize.w + 10;

	
    // Update the icon
    if (initializedHealthIcon) {
      bitmap_layer_destroy(stepsLayer);
    } else {
      initializedHealthIcon = true;
    }
    stepsLayer = bitmap_layer_create(bottomTextBox);
	
    //if (settings.darkMode) {
    bitmap_layer_set_bitmap(stepsLayer, stepsBitmap);
    //} else {
    //  bitmap_layer_set_bitmap(stepsLayer, stepsBitmapBlack);
    //}
    layer_add_child(window_layer, bitmap_layer_get_layer(stepsLayer));
}


// The main loop
int main(void) {
  init();
  app_event_loop();
  deinit();
}