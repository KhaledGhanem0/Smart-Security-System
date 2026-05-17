#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ---------- PIN ASSIGNMENT ---------- //

// Pin assignments for PIR Motion Sensors (assuming one for each zone)
#define PIR_SENSOR_1 4
#define PIR_SENSOR_2 5
#define PIR_SENSOR_3 6
#define PIR_SENSOR_4 7

// Pin assignments for ESP8266 WiFi Module (Serial1)
#define WIFI_TX 18
#define WIFI_RX 19

// Pin assignments for DS3231 RTC (I2C)
#define RTC_SDA 20
#define RTC_SCL 21

// Pin assignments for Magnetic Reed Switches (assuming two for each zone)
#define REED_SWITCH_1 22
#define REED_SWITCH_2 23
#define REED_SWITCH_3 24
#define REED_SWITCH_4 25
#define REED_SWITCH_5 26
#define REED_SWITCH_6 27
#define REED_SWITCH_7 28
#define REED_SWITCH_8 29

// Pin assignments for Alert System
#define Zone_1_alarm_pin 8
#define Zone_2_alarm_pin 9
#define Zone_3_alarm_pin 10

// Pin assignments for 2x16 LCD (4-bit parallel mode)
#define LCD_RS 34
#define LCD_EN 35
#define LCD_D4 36
#define LCD_D5 37
#define LCD_D6 38
#define LCD_D7 39

// Pin assignments for 4x4 Keypad
#define KEYPAD_ROW_1 40
#define KEYPAD_ROW_2 41
#define KEYPAD_ROW_3 42
#define KEYPAD_ROW_4 43
#define KEYPAD_COL_1 44
#define KEYPAD_COL_2 45
#define KEYPAD_COL_3 46
#define KEYPAD_COL_4 47

// Pin assignments for SD Card Module (SPI)
#define SD_MOSI 51
#define SD_MISO 50
#define SD_SCK 52
#define SD_CS 48

// Pin assignments for RC522 RFID Reader (SPI)
#define RFID_MOSI 51
#define RFID_MISO 50
#define RFID_SCK 52
#define RFID_SS 53
#define RFID_RST 49
// Note: both RC522 and the SD card module use SPI communcation. Therefore, they are 
//       utilizing the same pin with different 'select pins'



// ---------- MODULES CONFIGURATION PARAMETERS ---------- //

// Keypad
const byte KEYPAD_ROWS = 4;
const byte KEYPAD_COLS = 4;

// LCD screen
const int LCD_COLS = 16;
const int LCD_ROWS = 2;

// Sensors
const int NUM_PIR_SENSORS = 3;
const int NUM_REED_SWITCHES = 6;



// ---------- TIME CONSTANTS ---------- //

const unsigned long RFID_TAG_MS = 7000UL; // The time allowed for a user to scan the RFID tag
const unsigned long SENSOR_DEBOUNCE_MS = 200UL; // A short delay to prevent sensor false alarms from momentary glitches.
const unsigned long NOTIFICATION_COOLDOWN_MS = 60000UL; // A minimum time between sending remote notifications to prevent spamming the user.



// ---------- ZONE CONFIGURATION ---------- //

const int NUM_ZONES = 3; // Global constant for the total number of zones

// Struct to define different zones and their corresponding pins. Assume every zone has 1 PIR sensor and 2 reed switches
struct ZoneConfig {
    const char* name;   // Human-readable name (e.g., "Main Hall")
    int pir_pin;         // Digital Pin for PIR Motion Sensor
    int reed_1_pin;       // Digital Pin for Reed Switch 1
    int reed_2_pin;       // Digital Pin for Reed Switch 2
};


// Array containing the configuration for all logical zones.
// This array defines the pins used by the SensorHandler::begin() constructor.
const ZoneConfig ZONE_CONFIGS[NUM_ZONES] = {
    // ZONE 0
    {"Zone 1", PIR_SENSOR_1, REED_SWITCH_1, REED_SWITCH_2}, 
    
    // ZONE 1
    {"Zone 2", PIR_SENSOR_2, REED_SWITCH_3, REED_SWITCH_4},

    // ZONE 2
    {"Zone 3", PIR_SENSOR_3, REED_SWITCH_5, REED_SWITCH_6} 
};



// ---------- FINITE STATE MACHINE ---------- //

enum SystemState {DISARMED, ARMED, TRIGGERED, ERROR_MODE, CONFIG_MODE};
const String CONFIG_CODE = "D";



// ---------- EEPROM & CREDENTIALS ---------- //

const int EEPROM_SIZE = 4096; // Size of Arduino's EEPROM

// Password
const size_t PASSWORD_LENGTH = 8;       // Max 8 characters
const size_t MIN_PASSWORD_LENGTH = 4;
const long PASS_FLAG_NUMBER = 0xAABBCC11; // A unique, non-zero value
const int EEPROM_PASS_FLAG_ADDR = 0;      // Starting address for flag
const int EEPROM_MASTER_PASS_ADDR = 20;
const int EEPROM_ZONE_PASS_START_ADDR = 40;      // Starting address for password
// extern char master_password[PASSWORD_LENGTH + 1];

// RFID 
const int RFID_UID_LENGTH = 8;         // MFRC522 4-byte UID formatted as XXXXXXXX (8 hex chars)
const long RFID_FLAG_NUMBER = 0xF1A2B3C4;
const int EEPROM_RFID_FLAG_ADDR = 100;
const int EEPROM_MASTER_RFID_ADDR = 120;
const int EEPROM_RFID_START_ADDR = 140; // Start address for the list of RFIDs

// WiFi Credentials
extern const char* WIFI_SSID;
extern const char* WIFI_PASS;
extern const char* IFTTT_SERVER;
extern const char* IFTTT_EVENT_NAME;
extern const char* IFTTT_KEY;


#endif