#include <pebble.h>
static Window *s_main_window;
// static TextLayer *s_time_layer;

// Window *window;
static TextLayer *time_layer;
TextLayer *date_layer;
Layer* motherLayer;
Layer *myCanvas;


struct hexTm {
    int hex_sec;
    int hex_min;
    int hex_hr;
};

char* getMonthName(int month) {
    char* month_name = "";
    APP_LOG(APP_LOG_LEVEL_DEBUG, "month = %d", month);

    switch(month) {
        case 0:
            month_name = "January";
            break;
        case 1:
            month_name = "Febuary";
            break;
        case 2:
            month_name = "March";
            break;
        case 3:
            month_name = "April";
            break;
        case 4:
            month_name = "May";
            break;
        case 5:
            month_name = "June";
            break;
        case 6:
            month_name = "July";
            break;
        case 7:
            month_name = "August";
            break;
        case 8:
            month_name = "September";
            break;
        case 9:
            month_name = "October";
            break;
        case 10:
            month_name = "November";
            break;
        case 11:
            month_name = "December";
            break;
    }
    return month_name;
}



static void updateGame(Layer *layer, GContext *ctx)
{
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_line(ctx, GPoint(10, 99), GPoint(131,99));
}

void create_layout() {
     GFont time_font =  fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT);
     GFont date_font =  fonts_get_system_font(FONT_KEY_GOTHIC_18);  
  
     myCanvas = layer_create(GRect(0, 0, 143, 160));
     motherLayer = window_get_root_layer(s_main_window);
     layer_add_child(motherLayer, myCanvas);
     layer_set_update_proc(myCanvas, updateGame);

     date_layer = text_layer_create(GRect(10,78,122,20));
     text_layer_set_background_color(date_layer, GColorClear);
     text_layer_set_text_color(date_layer, GColorWhite);
     layer_add_child(motherLayer, text_layer_get_layer(date_layer));
     text_layer_set_font(date_layer, date_font);
      
      time_layer = text_layer_create(GRect(10,95,122,50));
      text_layer_set_background_color(time_layer, GColorClear);
      text_layer_set_text_color(time_layer, GColorWhite);
      layer_add_child(motherLayer, text_layer_get_layer(time_layer));
      text_layer_set_text_alignment(time_layer, GTextAlignmentLeft);
      text_layer_set_font(time_layer, time_font);
}

char *itoa(int num)
{
  static char buff[20] = {};
  int i = 0, temp_num = num, length = 0;
  char *string = buff;
  
  if(num >= 0) { //See NOTE
    // count how many characters in the number
    while(temp_num) {
      temp_num /= 10;
      length++;
    }
    // assign the number to the buffer starting at the end of the 
    // number and going to the begining since we are doing the
    // integer to character conversion on the last number in the
    // sequence
    for(i = 0; i < length; i++) {
      buff[(length-1)-i] = '0' + (num % 10);
      num /= 10;
    }
    buff[i] = '\0'; // can't forget the null byte to properly end our string
  }
  else {
    return "Unsupported Number";
  }
  
  return string;
}

void getHexTime(struct hexTm* hexTime, struct tm* timeData) {
    
    // instantiate useful values;
    int seconds_in_hour = 60*60;
    int seconds_in_min = 60;
    float seconds_in_hex = 1.318359;
    int hex_seconds_in_hex_hour = 1*256*16;
    int hex_seconds_in_hex_min = 16;
    int offset = 0;
    int hr = timeData->tm_hour + offset;
    int min = timeData->tm_min;
    int sec = timeData->tm_sec;
    int sinceStart = (hr*seconds_in_hour) + (min*seconds_in_min) + (sec);

    //get total number of elapsed hex seconds in a day.
    hexTime->hex_sec =  sinceStart;
    hexTime->hex_sec /= seconds_in_hex;
    
    //convert hex seconds into hex hours
    hexTime->hex_hr = hexTime->hex_sec/hex_seconds_in_hex_hour;
    
    //divide the remaining seconds after converting to hex hours hex mins
    hexTime->hex_min = (hexTime->hex_sec%(hexTime->hex_hr*hex_seconds_in_hex_hour));
    hexTime->hex_min /= hex_seconds_in_hex_min;

}


static void update_time() {
  int month = 0;
  static char month_day[40] = "";
  static char full_str[5] = "";
  static char* month_name = "";
  static char* day_name = "";

  memset(&full_str[0], 0, sizeof(full_str));
  memset(&month_day[0], 0, sizeof(month_day));

  
 // Get a tm structure
  time_t currentTime;
  time(&currentTime);
    
  //time struct for local time (important for getting local day/date).
  struct tm* timeDataLocal = localtime(&currentTime);

  int day = timeDataLocal-> tm_mday;
  month = timeDataLocal->tm_mon;
  month_name = getMonthName(month);  
  
// try to figure out why on earth you need to initalize here and not in emulator...
  struct hexTm hexTest;
  struct hexTm* hexTime = &hexTest;
  hexTime->hex_hr=0;
  hexTime->hex_min=0;
  hexTime->hex_sec=0;
  getHexTime(hexTime, timeDataLocal);


  
     day_name = itoa(day);
     strcat(month_day, month_name);
     strcat(month_day, "  ");
     strcat(month_day, day_name);
  
  // Write the current hours and minutes into a buffer
     static char* s_buffer = "";
     APP_LOG(APP_LOG_LEVEL_DEBUG, "%X_%X", hexTime->hex_hr,hexTime->hex_min);
  
      if (hexTime->hex_min < 16) {
          APP_LOG(APP_LOG_LEVEL_DEBUG, "its smaller!");
          snprintf(s_buffer, 6, "%X_0%X\n", hexTime->hex_hr, hexTime->hex_min); 
      }
      if (hexTime->hex_min >= 16) {
         APP_LOG(APP_LOG_LEVEL_DEBUG, "its bigger!");
         snprintf(s_buffer, 6, "%X_%X\n", hexTime->hex_hr, hexTime->hex_min);        
      }


  
  // Display this time on the TextLayer
     text_layer_set_text(time_layer, s_buffer);
     text_layer_set_text(date_layer, month_day);
}

static void tick_handler(struct tm *timeData, TimeUnits units_changed) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Heap Available: %d", (int)heap_bytes_free());
  update_time();
}


static void main_window_load(Window *window) {
   create_layout();
}  

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(time_layer);
  text_layer_destroy(date_layer);
  layer_destroy(myCanvas);
}



static void init() {
  
  // Register with TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  
  s_main_window = window_create();
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_set_background_color(s_main_window, GColorBlack);
  window_stack_push(s_main_window, true);
  
  // Make sure the time is displayed from the start
  update_time();
}

static void deinit() {
   // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}