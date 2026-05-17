#include "Config.h"
#include "LcdHandler.h"

// Constructor: receives the pin assignment
LcdHandler::LcdHandler(int rs, int en, int d4, int d5, int d6, int d7)
    : rs_pin(rs), en_pin(en), d4_pin(d4), d5_pin(d5), d6_pin(d6), d7_pin(d7),
      lcd(rs_pin, en_pin, d4_pin, d5_pin, d6_pin, d7_pin) 
{}


// Initialization
void LcdHandler::begin() {
    lcd.begin(16, 2);
    lcd.print("System Ready..."); 
    is_message_active = false;
}


// Helper function to hide (mask) the input when being displayed
String LcdHandler::mask_input(const String& input) const {
    String masked = "";
    for (size_t i = 0; i < input.length(); i++) {
        masked += '*';
    }
    masked[input.length()-1] = input[input.length()-1]; // Display the last character, mask the others
    return masked;
}


// Helper function to display a temporary message. The function receives two lines of string and the desired
// duration for the message to be displayed
void LcdHandler::temp_message(unsigned long duration_ms, const String& line1, const String& line2) {
    
    is_message_active = true;
    message_end_time = millis() + duration_ms;

    lcd.clear();

    lcd.setCursor(0, 0);  
    lcd.print(line1);
    lcd.setCursor(0, 1);  
    lcd.print(line2);

    last_message = "";
}

void LcdHandler::base_message(const String& line1, const String& line2) {
    if (!check_message_timeout()) {
        return;
    }

    if ((line1 + line2) == last_message) {
        return;
    }

    lcd.clear();

    lcd.setCursor(0, 0);  
    lcd.print(line1);
    lcd.setCursor(0, 1);  
    lcd.print(line2);

    last_message = line1 + line2;

}


// Helper function to check if a message's time run out and the screen should be cleared
bool LcdHandler::check_message_timeout() {
    if (!is_message_active) {
        return true;
    }

    if (millis() >= message_end_time) {
        is_message_active = false;
        return true; 
    }
    return false;
}

void LcdHandler::clear() {
    lcd.clear();
    last_message = "";
}


void LcdHandler::display_input(const String& input) {
    String maskedInput = mask_input(input);
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print(maskedInput);

}


