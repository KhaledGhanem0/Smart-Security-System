// ------------------ HEADERS ------------------
#include <Arduino.h>
#include "Config.h" 
#include "WiFiHandler.h"
#include "KeypadHandler.h"
#include "ConfigDataHandler.h" 
#include "LcdHandler.h"
#include "RfidHandler.h"
#include "SensorHandler.h"




// ------------------ GlOBAL VARIABLES ------------------
SystemState current_state;  // Finite State Machine

String input;    // Global variable to store the input from the keypad
String rfidUID;  // Global variable to store the tag from the RFID reader
String prompt = "";   // Global variable to store a message

BreachInfo breach; // Struct to store information related to a breach

int authentication_step = 1; // Counter used for 2-step authentication
int config_s;                // Counter used for CONFIG_MODE state logic 
bool is_submission_pending = false; // Flag used for CONFIG_MODE state logic

int current_zone_index;
int zones_state[NUM_ZONES] = {0}; // An array to show the state of all zones. 0:disarmed, 1:armed, 2:triggered

bool check = true;

// Countdown trackers
bool rfid_start_countdown;
bool msg_start_countdown;
unsigned long rfid_start_time;
unsigned long msg_start_time;

// For displaying multiples messages on the LCD screen at the same time
String triggered_messages[NUM_ZONES][2];
int triggered_messages_index = 0;


// ------------------ INSTANTIATING THE HANDLERS ------------------
KeypadHandler keypad(KEYPAD_ROW_1, KEYPAD_ROW_2, KEYPAD_ROW_3, KEYPAD_ROW_4, KEYPAD_COL_1, KEYPAD_COL_2, KEYPAD_COL_3, KEYPAD_COL_4); 
LcdHandler lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7); 
RfidHandler rfid(RFID_SS, RFID_RST); 
ConfigDataHandler configData; 
SensorHandler sensor; 
WiFiHandler wifi;


// ------------------ HELPER FUNCTIONS ------------------

// Helper function to check if the password is correct; returns the zones index if true, return -1 otherwise
int check_password(String input) {
    const char* master_pass = configData.get_master_password();
    if (input.equals(master_pass)) {
        return 0; // 0 refer to master zone: controlling all zones
    }
    for (int i = 0; i < NUM_ZONES; i++) {
        const char* zone_pass = configData.get_zone_password(i);

        if (input.equals(zone_pass)) {
            return i+1;            
        }
    }
    return -1;
}


int check_RFID(String uid) {
    if (configData.check_master_rfid(uid)){
        return 0;
    }
    for (int i = 0; i < NUM_ZONES; i++) {
        if (configData.check_RFID(i, uid)) {
            return i+1;
        } 
    }
    return -1;
}

bool is_zone_armed(int zone_index) {
    if (zones_state[zone_index-1] == 1) {
        return true;
    }
    return false;
}

void raise_alarm(int zone_index) {
    if (zone_index == 1) {
        digitalWrite(Zone_1_alarm_pin, HIGH);
    } else if (zone_index == 2) {
        digitalWrite(Zone_2_alarm_pin, HIGH);
    } else if (zone_index == 3) {
        digitalWrite(Zone_3_alarm_pin, HIGH);
    }
}

void cancel_alarm(int zone_index) {
    if (zone_index == 1) {
        digitalWrite(Zone_1_alarm_pin, LOW);
    } else if (zone_index == 2) {
        digitalWrite(Zone_2_alarm_pin, LOW);
    } else if (zone_index == 3) {
        digitalWrite(Zone_3_alarm_pin, LOW);
    }
}

// Builds the ARMED state status prompt based on which zones are currently armed (state == 1).
// Returns "All zones armed" when every zone is armed, otherwise lists each armed zone number.
String build_armed_prompt() {
    int count = 0;
    for (int i = 0; i < NUM_ZONES; i++) {
        if (zones_state[i] == 1) count++;
    }
    if (count == NUM_ZONES) return "All zones armed";

    String p = "";
    int listed = 0;
    for (int i = 0; i < NUM_ZONES; i++) {
        if (zones_state[i] == 1) {
            if (listed == 0) p += "Zone " + String(i+1);
            else             p += " & " + String(i+1);
            listed++;
        }
    }
    p += " armed";
    return p;
}


// Helper function to display prompts received from other files on the LCD screen properly
void display_prompt(String prompt) {
  const int LCD_WIDTH = 16;
  String line1, line2;

  if (prompt.length() <= LCD_WIDTH) {
    line1 = prompt;
    line2 = "";
  } else {
    int splitIndex = prompt.lastIndexOf(' ', LCD_WIDTH);
    if (splitIndex == -1) splitIndex = LCD_WIDTH;  

    line1 = prompt.substring(0, splitIndex);
    line2 = prompt.substring(splitIndex + 1);  
  }

  lcd.temp_message(1500, line1, line2);
}

// Helper function to display a countdown on the LCD screen; returns true when the countdown reaches zero.
bool countdown(unsigned long start_time, const unsigned long m_seconds) {
    return (millis() - start_time) >= m_seconds;
}

// Sends a WiFi notification and prints the outcome to the Serial monitor.
void wifi_notify(const String& msg, const String& v2 = "", const String& v3 = "") {
    bool ok = wifi.send_notification(msg, v2, v3);
    Serial.print("[WiFi] ");
    if (ok) {
        Serial.print("Sent: "); Serial.println(msg);
    } else {
        Serial.print("Failed: "); Serial.println(msg);
    }
}



// ------------------ STATE LOGIC FUNCTION ------------------

// Handles logic for the initialization when the system power up
void handleInitialization() {    
    if (wifi.begin()) {
        Serial.println("[WiFi] Init success.");
    } else {
        Serial.println("[WiFi] Init failed. Check wiring/credentials.");
    }

    bool rfid_ok = rfid.begin();      // Returns bool for success/failure

    if (rfid_ok) {
        lcd.temp_message(1500, "INITIALIZED", "SUCCESSFULLY");
        current_state = DISARMED;
    } else {
        // Build specific error message
        String errorMsg = "";
        if (!rfid_ok) errorMsg += "RFID FAIL";
        
        lcd.temp_message(1500, "CRITICAL ERROR!", errorMsg);
        current_state = ERROR_MODE;
    }
    delay(1500);

    pinMode(Zone_1_alarm_pin, OUTPUT);
    pinMode(Zone_2_alarm_pin, OUTPUT);
    pinMode(Zone_3_alarm_pin, OUTPUT);

    // If no master password and/or RFID tag is saved, the system must first ask for them before activating
    if(!configData.is_master_credentials_initialized()) {
        lcd.temp_message(1500, "NO CREDENTIALS");
        
        while(true) {
            lcd.base_message("Enter new pass:");
            if (keypad.collect_input()) {
                lcd.display_input(keypad.get_current_input());
            }

            if (keypad.is_submission_pending()) {
                input = keypad.get_current_input();

                const char* new_password = input.c_str();
                if (configData.set_master_password(new_password, &prompt)) {
                    display_prompt(prompt);
                    break;
                }
                display_prompt(prompt);
            }
        }

        while(true) {
            lcd.base_message("Scan new UID:");
            rfidUID = rfid.read_card_uid();

            if (rfidUID.length() > 0) {
                if (configData.set_master_rfid(rfidUID, &prompt)) {
                    display_prompt(prompt);
                    break;
                }
                display_prompt(prompt);
            }

        }
    }
}


// Handles logic for the DISARMED state.
void handleDisarmedState() {

    // Handle LCD prompt when the program switches to DISARMED. Also, reset parameters
    if (check) {
        lcd.temp_message(1500, "SYSTEM DISARMED");
        check = false;
        keypad.clear_input();
        rfid_start_countdown = true;
        authentication_step = 1;

        for (int i = 1; i < NUM_ZONES+1; i++) {
            cancel_alarm(i);
        }

        wifi_notify("System disarmed");
    }


    // Collecting inputs from the keypad and the RFID reader
    switch (authentication_step){
        case 1: // Check for password or configuration mode 
            lcd.base_message("Arm / Config?");
            if (keypad.collect_input()) {
                lcd.display_input(keypad.get_current_input());
            }

            if (keypad.is_submission_pending()) {
                input = keypad.get_current_input();

                if (input.equals(CONFIG_CODE)) {
                    current_state = CONFIG_MODE;
                    check = true;
                    return;
                }

                current_zone_index = check_password(input);
                if (current_zone_index == -1) {
                    lcd.temp_message(1500, "ACCESS DENIED", "Invalid code");
                    wifi_notify("Invalid password attempt");
                } else {
                    if (current_zone_index == 0) {
                        lcd.temp_message(1500, "Master password", "was entered");
                    } else{
                        lcd.temp_message(1500, "Zone " + String(current_zone_index) + " ...");
                    }
                    authentication_step = 2;
                }

                keypad.clear_input();
            }
            break;
        
        case 2: // Check for RFID UID
            lcd.base_message("Scan RFID tag");
            rfidUID = rfid.read_card_uid();
            if (rfidUID.length() > 0) {
                Serial.println(current_zone_index);
                Serial.println(check_RFID(rfidUID));
                if (check_RFID(rfidUID) != current_zone_index) {
                    lcd.temp_message(1500, "RFID REJECTED", "Invalid tag");
                    wifi_notify("Invalid RFID tag scanned");
                } else {
                    current_state = ARMED;
                    check = true;
                }
                authentication_step = 1;
                rfid_start_countdown = true;
            } else {
                // Only run timer logic when no tag was scanned this iteration,
                // so rfid_start_countdown is not immediately consumed after being set.
                if (rfid_start_countdown) {
                    rfid_start_time = millis();
                    rfid_start_countdown = false;
                }
                if (countdown(rfid_start_time, RFID_TAG_MS)) {
                    lcd.temp_message(1500, "No tag scanned");
                    authentication_step = 1;
                    rfid_start_countdown = true;
                }
            }
            break;
    }
}



// Handles logic for the ARMED state.
void handleArmedState() {

    if (check) {
        if (current_zone_index == 0) { //i.e., master credentials. Arm all zones.
            for (int i = 0; i < NUM_ZONES; i++) {
                zones_state[i] = 1;
            }
        } else {
            zones_state[current_zone_index-1] = 1;
        }
        prompt = build_armed_prompt();
        keypad.clear_input();
        check = false;
        authentication_step = 1;
        rfid_start_countdown = true;

        wifi_notify(prompt);
    }

    // Sensor implemention
    for (int i = 0; i < NUM_ZONES; i++) {
        if (zones_state[i] == 1) {
            breach = sensor.check_zone(i);

            if (breach.is_breached) {
            zones_state[i] = 2;
            current_zone_index = i + 1;
            current_state = TRIGGERED;
            check = true;
            return;
            }
        }

    }

    // Collecting inputs from the keypad and the RFID reader
    switch (authentication_step){
        case 1: // Check for password 
            lcd.base_message(prompt);
            if (keypad.collect_input()) {
                lcd.display_input(keypad.get_current_input());
            }

            if (keypad.is_submission_pending()) {
                input = keypad.get_current_input();

                current_zone_index = check_password(input);
                if (current_zone_index == -1) {
                    lcd.temp_message(1500, "ACCESS DENIED", "Invalid code");
                    wifi_notify("Invalid password attempt");
                } else {
                    if (current_zone_index == 0) {
                        lcd.temp_message(1500, "Master password", "was entered");
                    } else{
                        lcd.temp_message(1500, "Zone " + String(current_zone_index) + " ...");
                    }
                    authentication_step = 2;
                }

                keypad.clear_input();
            }
            break;
        
        case 2: // Check for RFID UID
            lcd.base_message("Scan RFID tag");

            rfidUID = rfid.read_card_uid();
            if (rfidUID.length() > 0) { 
                if (check_RFID(rfidUID) != current_zone_index) {
                    lcd.temp_message(1500, "RFID REJECTED", " Invalid tag");
                    wifi_notify("Invalid RFID tag scanned");
                } else {
                    if (current_zone_index == 0) { // If master, disarm all zones
                            current_state = DISARMED;
                            check = true;
                            return;
                    }
                    
                    if (is_zone_armed(current_zone_index)) { //If not master, check the zone index
                        lcd.temp_message(1500, "Zone " + String(current_zone_index) + " disarmed");
                        zones_state[current_zone_index-1] = 0;
                        wifi_notify("Zone " + String(current_zone_index) + " disarmed");

                        // Check if any zones remain armed
                        bool any_armed = false;
                        for (int i = 0; i < NUM_ZONES; i++) {
                            if (zones_state[i] == 1) { any_armed = true; break; }
                        }

                        if (!any_armed) {
                            current_state = DISARMED;
                            check = true;
                        } else {
                            prompt = build_armed_prompt(); // Reflect the remaining armed zones
                        }
                    } else {
                        check = true;
                    }
                }
                authentication_step = 1;
                rfid_start_countdown = true;
            } else {
                // Only run timer logic when no tag was scanned this iteration,
                // so rfid_start_countdown is not immediately consumed after being set.
                if (rfid_start_countdown) {
                    rfid_start_time = millis();
                    rfid_start_countdown = false;
                }
                if (countdown(rfid_start_time, RFID_TAG_MS)) {
                    lcd.temp_message(1500, "No tag scanned");
                    authentication_step = 1;
                    rfid_start_countdown = true;
                }
            }
            break;
    }




}


// Handles logic for the TRIGGERED state.
void handleTriggeredState() {

    // Handle LCD prompt when the program switches to TRIGGERED
    if (check) {
        lcd.temp_message(1500, "!! BREACH !!");
        triggered_messages[current_zone_index-1][0] = String(breach.zone_name);
        triggered_messages[current_zone_index-1][1] = String(breach.sensor_name);
        raise_alarm(current_zone_index);
        wifi_notify("SECURITY ALERT AT", breach.zone_name, breach.sensor_name);

        keypad.clear_input();
        check = false;
        triggered_messages_index = 0; // Always start cycling from zone 1 on a new breach event
        msg_start_countdown = true;
        rfid_start_countdown = true;
        authentication_step = 1;
    }


    // Keep checking the other armed zones while raising the alarm for the triggered zone.
    for (int i = 0; i < NUM_ZONES; i++) {
        if (zones_state[i] == 1) {
            breach = sensor.check_zone(i);

            if (breach.is_breached) {
                zones_state[i] = 2;
                current_zone_index = i+1;
                check = true;
                return;
            }
        }
    }


    switch (authentication_step){
        case 1: // Check password to cancel the alarm
            // This part creates a dynamic message on the LCD screen to show all triggered zones
            if (zones_state[triggered_messages_index] == 2) {
                lcd.base_message(triggered_messages[triggered_messages_index][0], triggered_messages[triggered_messages_index][1]);
                if (msg_start_countdown) {
                    msg_start_time = millis();
                    msg_start_countdown = false;
                }

                if (countdown(msg_start_time, 3000)) {
                    triggered_messages_index++;
                    msg_start_countdown = true;
                }
            } else {
                triggered_messages_index++;
            }

            if (triggered_messages_index >= NUM_ZONES) {
                    triggered_messages_index = 0;
                }

            // Collect password while freezing the screen for 5 seconds to not confuse the user
            if (keypad.collect_input()) {
                lcd.temp_message(5000, "Password", "");
                lcd.display_input(keypad.get_current_input());
            }

            if (keypad.is_submission_pending()) {
                input = keypad.get_current_input();

                current_zone_index = check_password(input);
                if (current_zone_index == -1) {
                    lcd.temp_message(1500, "ACCESS DENIED", "Invalid code");
                    wifi_notify("Invalid password attempt");
                } else {
                    if (current_zone_index == 0) {
                        lcd.temp_message(1500, "Master password", "was entered");
                        authentication_step = 2;
                    } else{
                        if (zones_state[current_zone_index-1] == 2) {
                            lcd.temp_message(1500, "Zone " + String(current_zone_index) + " ...");
                            authentication_step = 2;
                        } else {
                            lcd.temp_message(1500, "ACCESS DENIED", "Invalid code");
                        }
                    }
        
                }

                keypad.clear_input();
            }
            break;
        
        case 2: // Check for RFID UID
            lcd.base_message("Scan RFID tag");
            rfidUID = rfid.read_card_uid();
            if (rfidUID.length() > 0) { 
                if (check_RFID(rfidUID) != current_zone_index) {
                    lcd.temp_message(1500, "RFID REJECTED", " Invalid tag");
                    wifi_notify("Invalid RFID tag scanned");
                } else {
                    if (current_zone_index == 0) {
                        current_state = DISARMED;
                        check = true;
                    } else {
                        cancel_alarm(current_zone_index);
                        zones_state[current_zone_index-1] = 0;
                        wifi_notify("Alarm cleared: Zone " + String(current_zone_index));

                        bool is_system_triggered = false;
                        for (int i = 0; i < NUM_ZONES; i++) {
                            if (zones_state[i] == 2) {
                                is_system_triggered = true;
                                break;
                            }
                        }
                        if (!is_system_triggered) {
                            current_state = DISARMED;
                            check = true;
                        }
                    }
                }
                authentication_step = 1;
                rfid_start_countdown = true;
                msg_start_countdown = true;
            } else {
                // Only run timer logic when no tag was scanned this iteration,
                // so rfid_start_countdown is not immediately consumed after being set.
                if (rfid_start_countdown) {
                    rfid_start_time = millis();
                    rfid_start_countdown = false;
                }
                if (countdown(rfid_start_time, RFID_TAG_MS)) {
                    lcd.temp_message(1500, "No tag scanned");
                    authentication_step = 1;
                    rfid_start_countdown = true;
                    msg_start_countdown = true;
                }
            }
            break;
    }
}


// Handles logic for the CONFIG_MODE state.
void handleConfigMode() {

    // Handle LCD prompt when the program switches to CONFIG_MODE
    if (check) {
        lcd.temp_message(1500, "Configuration", "Mode");
        keypad.clear_input();
        authentication_step = 1;
        config_s = 0;
        check = false;
        is_submission_pending = false;
    }
    
    // Collects input from the keypad. Also, it handles backtracking when the user presses '*'
    if (keypad.collect_input()) {
        if (keypad.do_backtrack()){
            // Menu backtracking logic
            if (config_s == 0 || config_s == 1) {
                current_state = DISARMED;
                check = true;
            } else if (config_s == 2) {
                config_s = 1;
            } else if (config_s == 3 || config_s == 4) {
                config_s = 2;
            }
        }

        lcd.display_input(keypad.get_current_input());
    } 

    if (keypad.is_submission_pending()) {
        input = keypad.get_current_input();
        keypad.clear_input();
        is_submission_pending = true;
    }

    // Establishing a 'menu' logic to design the menu for CONFIG_MODE
    switch (config_s){        
        case 0: // Ask for cridentials
            //////////////////////////////////////////
            switch (authentication_step){
                case 1: // Check for password or configuration mode 
                    lcd.base_message("Enter password:");

                    if (is_submission_pending) {
                        current_zone_index = check_password(input);
                        if (current_zone_index != 0) { // master password
                            lcd.temp_message(1500, "ACCESS DENIED", "Invalid code");
                        } else {
                            lcd.temp_message(1500, "Master password", "was entered");
                            authentication_step = 2;
                        }
                        is_submission_pending = false;
                    }
                    break;
                
                case 2: // Check for RFID UID
                    lcd.base_message("Scan RFID tag");
                    rfidUID = rfid.read_card_uid();
                    if (rfidUID.length() > 0) { 
                        if (check_RFID(rfidUID) != current_zone_index) {
                            lcd.temp_message(1500, "RFID REJECTED", " Invalid tag");
                        } else {
                            lcd.temp_message(1500, "Access granted...");
                            config_s = 1;
                        }
                        authentication_step = 1;
                    }
                    break;
            }
            break;
            //////////////////////////////////////////

        case 1:
            lcd.base_message("Enter zone number");
            
            if (is_submission_pending) {
                if (input == "0") {
                    current_zone_index = 0;
                    lcd.temp_message(1500, "Master zone");
                    config_s = 2;
                } else if (input.toInt() >= 1 && input.toInt() <= NUM_ZONES) {
                    current_zone_index = input.toInt();
                    lcd.temp_message(1500, "Zone " + String(current_zone_index));
                    config_s = 2;
                } else {
                    lcd.temp_message(1500, "Invalid input");
                }
                is_submission_pending = false;
            }
            break;
        
        case 2:
            lcd.base_message("1:Pass  2:UID");

            if (is_submission_pending) {
                if (input == "1") {
                    config_s = 3;
                } else if (input == "2") {
                    config_s = 4;
                } else {
                    lcd.temp_message(1500, "Invalid input");
                }
                is_submission_pending = false;
            }
            break;
        
        case 3: // Update password
            lcd.base_message("Enter new password");
        
            if (is_submission_pending) {
                const char* new_password = input.c_str();
                if (current_zone_index == 0) {
                    if (configData.set_master_password(new_password, &prompt)) {
                        current_state = DISARMED;
                        check = true;
                    } 
                } else {
                    if (configData.set_zone_password(current_zone_index-1, new_password, &prompt)) {
                        current_state = DISARMED;
                        check = true;
                    }
                }
            
                display_prompt(prompt);
                is_submission_pending = false;
            }
            break;
        
        case 4: // Update RFID UID: adding a new UID
            lcd.base_message("Scan new UID");
            rfidUID = rfid.read_card_uid();

            if (rfidUID.length() > 0) {
                if (current_zone_index == 0) {
                    if (configData.set_master_rfid(rfidUID, &prompt)) {
                        current_state = DISARMED;
                        check = true;
                    }
                } else {
                    if (configData.set_zone_RFID(current_zone_index-1, rfidUID, &prompt)) {
                        current_state = DISARMED;
                        check = true;
                    } 
                }
                display_prompt(prompt);
            }
            break;
        
    }
}


// Handles logic for the ERROR_MODE state.
void handleErrorMode() {
    // No way out. The system must be restarted manually.
    lcd.base_message("Initialization", "Failed"); 
}



// ------------------ SETUP() ------------------
void setup() 
{
    Serial.begin(115200); // For debugging
    
    configData.begin(); 
    lcd.begin();        
    sensor.begin();
    
    // Handle initializing components that need to be checked if initialized successfully.
    handleInitialization();
}



// ------------------ LOOP() ------------------
void loop() 
{
    wifi.maintain_connection();

    switch (current_state) {
        case DISARMED:
            handleDisarmedState();
            break;
        
        case ARMED:
            handleArmedState();
            break;

        case TRIGGERED:
            handleTriggeredState();
            break;
        
        case CONFIG_MODE:
            handleConfigMode();
            break;
        
        case ERROR_MODE: // Once here, there is no way out but to restart the system
            handleErrorMode();
            break;
    }
}
