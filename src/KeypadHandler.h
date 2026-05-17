#ifndef KEYPAD_HANDLER_H
#define KEYPAD_HANDLER_H

#include <Keypad.h>
#include "Config.h" 
#include "ConfigDataHandler.h" 

// KeypadHandler class for managing 4x4 keypad input
class KeypadHandler {
private:
  char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'} 
  };
  byte row_pins[KEYPAD_ROWS];
  byte col_pins[KEYPAD_COLS];
  Keypad keypad;
  String input_buffer;
  bool submission_pending;
  bool backtrack;
  

public:
  // Constructor: Takes pin definitions 
  KeypadHandler(byte r1, byte r2, byte r3, byte r4, byte c1, byte c2, byte c3, byte c4);

  // Reads a single keypress from the keypad
  char get_key();

  // Collects input until a submission key is pressed.
  bool collect_input();

  // Used by FSM to check if the current buffered input is ready for validation.
  bool is_submission_pending();

  // Returns the current contents of the input buffer.
  String get_current_input() const;

  bool do_backtrack();

  // Clears the input buffer
  void clear_input();
};

#endif