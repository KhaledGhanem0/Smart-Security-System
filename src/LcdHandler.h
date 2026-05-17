#ifndef LCD_HANDLER_H
#define LCD_HANDLER_H

#include <LiquidCrystal.h> 
#include "Config.h"

// LCD Handler class for managing the 2x16 display
class LcdHandler {
private:
  int rs_pin;
  int en_pin;
  int d4_pin;
  int d5_pin;
  int d6_pin;
  int d7_pin;
  
  LiquidCrystal lcd;
  
  unsigned long message_end_time = 0; // Used with millis() to define the duration the message should be displayed on the screen
  bool is_message_active = false;   // Flag to check if the screen is busy
  String last_message;

public:
  // Constructor
  LcdHandler(int rs, int en, int d4, int d5, int d6, int d7);

  // Initialization
  void begin();
  
  // Helper function to display a temporary message. 
  void temp_message(unsigned long duration_ms, const String& line1, const String& line2 = "");

  void base_message(const String& line1, const String& line2 = "");

  void clear();

  // Helper function to check if a message's time run out and the screen should be cleared
  bool check_message_timeout();

  // Helper function to hide (mask) the input when being displayed
  String mask_input(const String& input) const;


  void display_input(const String& input);
};

#endif