
#include "Config.h" 
#include "KeypadHandler.h"
#include "ConfigDataHandler.h" 

// Constructor: Initializes pin arrays and the Keypad object.
KeypadHandler::KeypadHandler(byte r1, byte r2, byte r3, byte r4, byte c1, byte c2, byte c3, byte c4) 
    : keypad(makeKeymap(keys), row_pins, col_pins, KEYPAD_ROWS, KEYPAD_COLS),
      input_buffer(""), 
      submission_pending(false),
      backtrack(false)
{
    // Initialize pin arrays 
    row_pins[0] = r1; row_pins[1] = r2; row_pins[2] = r3; row_pins[3] = r4;
    col_pins[0] = c1; col_pins[1] = c2; col_pins[2] = c3; col_pins[3] = c4;
}


// Reads a single keypress in a non-blocking manner. We will use it to get the input from the user without
// blocking the program (i.e., loop())
char KeypadHandler::get_key() {
    return keypad.getKey();
}


// Collects input from the keypad with some handling properties. Still needs enhancing.
bool KeypadHandler::collect_input() {
    char key = get_key();

    if (key != NO_KEY) {
        backtrack = false;
        if ((key >= '0' && key <= '9') || (key >= 'A' && key <= 'D')) {
            if (input_buffer.length() < PASSWORD_LENGTH) {
                input_buffer += key;
            }
        } 

        else if (input_buffer.length() <= 0 && key == '*') {
            backtrack = true;
        }

        else if (input_buffer.length() > 0) {
            if (key == '*') {
                input_buffer.remove(input_buffer.length() - 1);
            } 
            else if (key == '#') {
                submission_pending = true;
            }     
        }
        return true;
    }
    return false;
}


// Used by FSM to check if the current buffered input is ready for validation.
bool KeypadHandler::is_submission_pending() {
    bool ready = submission_pending;
    if (ready) {
        submission_pending = false; 
    }
    return ready;
}


//Returns the current contents of the input buffer.
String KeypadHandler::get_current_input() const {
    return input_buffer;
}

bool KeypadHandler::do_backtrack() {
    if (backtrack) {
        backtrack = false;
        return true;
    }
    return false;
}

// Clears the input buffer.
void KeypadHandler::clear_input() {
    input_buffer = "";
    submission_pending = false;
}