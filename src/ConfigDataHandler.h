#ifndef CONFIG_DATA_HANDLER_H
#define CONFIG_DATA_HANDLER_H

#include <EEPROM.h>
#include <Arduino.h>
#include "Config.h"


// Forward declaration of assumed external constants from Config.h for clarity.
// #define NUM_ZONES 4
// #define PASSWORD_LENGTH 4
// #define RFID_UID_LENGTH 8 
// #define EEPROM_MASTER_PASS_ADDR (needs definition)
// #define EEPROM_MASTER_RFID_ADDR (needs definition)

class ConfigDataHandler {
public:
    void begin();
    
    // --- Master Credentials Management (Global Access) ---
    bool set_master_password(const char* newPass, String *prompt = nullptr);
    const char* get_master_password();

    bool set_master_rfid(String uid, String *prompt = nullptr);
    bool check_master_rfid(String uid);
    
    // --- Zone Credentials Management ---
    bool set_zone_password(int zoneIndex, const char* newPass, String *prompt = nullptr);
    const char* get_zone_password(int zoneIndex);
    
    // --- RFID Management (Zone-Specific - check_RFID now checks Master and Zone UID) ---
    bool check_RFID(int zoneIndex, String uid);
    bool set_zone_RFID(int zoneIndex, String uid, String *prompt = nullptr); // Replaces the current UID

    // --- System Management (Loops through all credentials) ---
    bool is_master_credentials_initialized();

private:
    // Internal loading functions
    void load_master_password();
    void load_master_rfid();
    void load_password(int zoneIndex);
    void load_RFID(int zoneIndex);
    void save_RFID(int zoneIndex);

    // Writes factory-default credentials to RAM and EEPROM when the system is first powered on.
    void seed_defaults();

    // EEPROM Address Calculation Helpers
    int get_password_addr(int zoneIndex);
    int get_flag_addr(int zoneIndex);
    int get_rfid_start_addr(int zoneIndex);
};

#endif // CONFIG_DATA_HANDLER_H

