#ifndef SENSOR_HANDLER_H
#define SENSOR_HANDLER_H

#include <Arduino.h>
#include "Config.h"

// Structure to hold information about a single logical zone. It contains all needed information to operate the sensors effictively.
struct Zone {
    const char* name;   // Human-readable name 
    
    // PIR Sensor Configuration
    int pir_pin;         
    int pir_last_state;
    unsigned long pir_last_change_time;
    int pir_last_reading;

    // Reed Switch 1 Configuration
    int reed_1_pin;       
    int reed_1_last_state;
    unsigned long reed_1_last_change_time;
    int reed_1_last_reading;

    // Reed Switch 2 Configuration
    int reed_2_pin;      
    int reed_2_last_state;
    unsigned long reed_2_last_change_time;
    int reed_2_last_reading;
};

// Return structure to collect the needed information in case of a breach.
struct BreachInfo {
    bool is_breached;        // True if a breach occurred
    const char* zone_name;   // Name of the breached logical zone
    const char* sensor_name; // Name of the specific sensor type/location that triggered (e.g., "PIR" or "Reed 1")
    int zone_index;          // Index of the breached zone
};

class SensorHandler {
private:
    Zone zones[NUM_ZONES];  // Array of all zones to monitor

    // Helper function to avoid sensor debouncing
    int check_debounce(int pin, int& last_state, int& last_reading, unsigned long& last_change_time);

public:
    // Constructor: Takes no arguments; relies on Config.h constants
    SensorHandler();

    // Initializes pin modes for all sensors
    void begin();

    // Checks all zones (all 3 pins in each) and returns the details of the first breach.
    BreachInfo check_zone(int i);
    
    const char* get_zone_name(int index) const;
};

#endif