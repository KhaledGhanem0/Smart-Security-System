#include "SensorHandler.h"
#include "Config.h" 
#include <Arduino.h>

// Constructor
SensorHandler::SensorHandler() {
    // Populate the internal zones array using constants from Config.h
    for (int i = 0; i < NUM_ZONES; i++) {
        zones[i].name = ZONE_CONFIGS[i].name;

        zones[i].pir_pin = ZONE_CONFIGS[i].pir_pin;
        zones[i].pir_last_state = LOW; 
        zones[i].pir_last_change_time = 0;
        zones[i].pir_last_reading = 0;

        zones[i].reed_1_pin = ZONE_CONFIGS[i].reed_1_pin;
        zones[i].reed_1_last_state = LOW; 
        zones[i].reed_1_last_change_time = 0;
        zones[i].reed_1_last_reading = 0;
        
        zones[i].reed_2_pin = ZONE_CONFIGS[i].reed_2_pin;
        zones[i].reed_2_last_state = LOW; 
        zones[i].reed_2_last_change_time = 0;
        zones[i].reed_2_last_reading = 0;
    }
}

// Initializes pin modes for all sensors in all zones
void SensorHandler::begin() {
    for (int i = 0; i < NUM_ZONES; i++) {
        pinMode(zones[i].pir_pin, INPUT);
        pinMode(zones[i].reed_1_pin, INPUT_PULLUP);
        pinMode(zones[i].reed_2_pin, INPUT_PULLUP);
    }
}


// Handles debouncing logic for a single pin.
int SensorHandler::check_debounce(int pin, int& last_state, int& last_reading, unsigned long& last_change_time) {
    int reading = digitalRead(pin);

    // If the instantaneous reading is different from the last *stable* state, restart the timer
    if (reading != last_reading) {
        last_change_time = millis();
    }

    // If the time elapsed is greater than the debounce period, confirm the state.
    if ((millis() - last_change_time) >= SENSOR_DEBOUNCE_MS) {
        if (reading != last_state) {
            last_state = reading; // Confirm the new stable state
        }
    }

    last_reading = reading;

    return last_state;
}


// Checks all zones (all 3 pins in each) and returns the details of the first breach.
BreachInfo SensorHandler::check_zone(int i) {
    BreachInfo breach = {false, "", "", -1}; 
    int debounced_state;

    // --- 1. PIR Check ---
    debounced_state = check_debounce(zones[i].pir_pin, zones[i].pir_last_state, zones[i].pir_last_reading,  zones[i].pir_last_change_time);
    if (debounced_state == HIGH) {
        breach.is_breached = true;
        breach.zone_name = zones[i].name;
        breach.sensor_name = "Motion sensor";
        breach.zone_index = i;
        return breach; 
    }

    // --- 2. Reed Switch 1 Check ---
    debounced_state = check_debounce(zones[i].reed_1_pin, zones[i].reed_1_last_state, zones[i].reed_1_last_reading, zones[i].reed_1_last_change_time);
    if (debounced_state == HIGH) {
        breach.is_breached = true;
        breach.zone_name = zones[i].name;
        breach.sensor_name = "Door sensor"; 
        breach.zone_index = i;
        return breach; 
    }

    // --- 3. Reed Switch 2 Check ---
    debounced_state = check_debounce(zones[i].reed_2_pin, zones[i].reed_2_last_state, zones[i].reed_2_last_reading, zones[i].reed_2_last_change_time);
    if (debounced_state == HIGH) {
        breach.is_breached = true;
        breach.zone_name = zones[i].name;
        breach.sensor_name = "Window sensor"; 
        breach.zone_index = i;
        return breach; 
    }
    

    return breach; // Return no breach if loop completes
}

const char* SensorHandler::get_zone_name(int index) const {
    if (index >= 0 && index < NUM_ZONES) {
        return zones[index].name;
    }
    return "UNKNOWN ZONE";
}
