
#include "ConfigDataHandler.h"
#include "Config.h" // Includes all necessary constants like NUM_ZONES, EEPROM addresses, etc.

// Multi-Zone RAM Storage for Passwords:
char zone_passwords[NUM_ZONES][PASSWORD_LENGTH + 1]; 
// Multi-Zone RAM Storage for Single RFID UID per Zone:
String zone_valid_RFIDs[NUM_ZONES]; // Only one UID string per zone

// --- New Global Master Credentials RAM Storage ---
char master_password[PASSWORD_LENGTH + 1] = "0000"; // Default Master Pass
String master_rfid_uid; // Global Master UID


bool is_master_pass_initialized;
bool is_master_rfid_initialized;
 ///// EEPROM Helper Functions (Utility) /////

// Write 'data' into EEPROM with a starting address of 'address'
void EEPROM_write_string(int address, const char* data, size_t length) {
    size_t i;
    for (i = 0; i < length && data[i] != '\0'; i++) {
        EEPROM.write(address + i, data[i]);
    }
    EEPROM.write(address + i, '\0');  // guarantee terminator
}

// Reads a string from 'address' with 'length' and save into 'buffer'
void EEPROM_read_string(int address, char* buffer, size_t length) {
    size_t i;
    for (i = 0; i < length; i++) {
        buffer[i] = EEPROM.read(address + i);
        if (buffer[i] == '\0') break;
    }
    for (; i <= length; i++) buffer[i] = '\0';  // pad remainder safely
}

///// EEPROM Address Calculation Helpers (Multi-Zone Logic) /////

// Calculates the EEPROM address for a specific zone's password.
int ConfigDataHandler::get_password_addr(int zoneIndex) {
    // Each password block is PASSWORD_LENGTH + 1 bytes.
    return EEPROM_ZONE_PASS_START_ADDR + (zoneIndex * (PASSWORD_LENGTH + 1));
}

// Calculates the EEPROM address for the initialization flag (one flag used for all zones).
int ConfigDataHandler::get_flag_addr(int zoneIndex) {
    return EEPROM_PASS_FLAG_ADDR; 
}

// Calculates the EEPROM address where a specific zone's SINGLE RFID UID starts.
int ConfigDataHandler::get_rfid_start_addr(int zoneIndex) {
    // Each zone's RFID takes (RFID_UID_LENGTH + 1) bytes.
    return EEPROM_RFID_START_ADDR + (zoneIndex * (RFID_UID_LENGTH + 1));
}


///// Class Implementation /////

// Initialize EEPROM and loads data for ALL zones, including Master.
void ConfigDataHandler::begin() {
    EEPROM.begin(); 

    load_master_password();
    load_master_rfid();
    
    for (int i = 0; i < NUM_ZONES; i++) {
        load_password(i);
        load_RFID(i); 
    }

    // On first boot (no credentials in EEPROM), seed factory defaults.
    if (!is_master_credentials_initialized()) {
        seed_defaults();
    }

    Serial.println("ConfigDataHandler initialized. Multi-Zone Data loaded.");
}

// =========================================================================
// MASTER PASSWORD FUNCTIONS (GLOBAL)
// =========================================================================

// Loads the global master password.
void ConfigDataHandler::load_master_password() {
    // Uses the same global flag to check if EEPROM has been initialized before
    long read_flag;
    EEPROM.get(EEPROM_PASS_FLAG_ADDR, read_flag); 

    char storedPass[PASSWORD_LENGTH + 1];
    EEPROM_read_string(EEPROM_MASTER_PASS_ADDR, storedPass, PASSWORD_LENGTH);

    if (read_flag != PASS_FLAG_NUMBER) {     
        // set_master_password("0000"); // Use the dedicated setter which saves to EEPROM
        Serial.println("Master password is not initialized");
        is_master_pass_initialized = false;
    } else {
        strncpy(master_password, storedPass, PASSWORD_LENGTH);
        master_password[PASSWORD_LENGTH] = '\0';
        Serial.print("Master Password is: "); Serial.println(storedPass);
        is_master_pass_initialized = true;
    }
    Serial.println(read_flag, HEX);
}

// Takes a new password and updates the global master password.
bool ConfigDataHandler::set_master_password(const char* newPass, String *prompt) {
    if (strlen(newPass) > PASSWORD_LENGTH) {
        *prompt = "Password too long."; 
        Serial.println(*prompt); 
        return false;
    }
    if (strlen(newPass) < MIN_PASSWORD_LENGTH) {
        *prompt = "Password too short."; 
        Serial.println(*prompt); 
        return false;
    }
    
    EEPROM.put(EEPROM_MASTER_PASS_ADDR, PASS_FLAG_NUMBER);

    strncpy(master_password, newPass, PASSWORD_LENGTH);
    master_password[PASSWORD_LENGTH] = '\0';
    
    EEPROM_write_string(EEPROM_MASTER_PASS_ADDR, master_password, PASSWORD_LENGTH + 1);
    
    *prompt = "Master Password updated";
    Serial.print("Master Password saved to EEPROM: "); 
    Serial.println(master_password);
    is_master_pass_initialized = true;
    return true; 
}

// Returns a pointer to the current global master password
const char* ConfigDataHandler::get_master_password() {
    return master_password;
}


// =========================================================================
// MASTER RFID FUNCTIONS (GLOBAL)
// =========================================================================

// Loads the global master RFID UID.
void ConfigDataHandler::load_master_rfid() {
    long read_flag;
    EEPROM.get(EEPROM_RFID_FLAG_ADDR, read_flag);

    char uidBuffer[RFID_UID_LENGTH + 1];
    EEPROM_read_string(EEPROM_MASTER_RFID_ADDR, uidBuffer, RFID_UID_LENGTH);

    if (read_flag != RFID_FLAG_NUMBER) {
        Serial.println("Master RFID is not initialized.");
        is_master_rfid_initialized = false;
    } else {
        master_rfid_uid = String(uidBuffer);
        Serial.print("Master RFID is: "); Serial.println(master_rfid_uid);
        is_master_rfid_initialized = true;
    }
}



// Sets (replaces) the global master RFID UID.
bool ConfigDataHandler::set_master_rfid(String uid, String *prompt) {
    if (uid.length() != RFID_UID_LENGTH) {
        *prompt = "Invalid RFID format"; 
        Serial.println(*prompt); 
        return false;
    }

    EEPROM.put(EEPROM_RFID_FLAG_ADDR, RFID_FLAG_NUMBER);
    
    master_rfid_uid = uid;
    is_master_rfid_initialized = true;
    
    int currentAddr = EEPROM_MASTER_RFID_ADDR;
    const char* uidCstr = master_rfid_uid.c_str();
    EEPROM_write_string(currentAddr, uidCstr, RFID_UID_LENGTH + 1);

    *prompt = "Master RFID set successfully";
    Serial.print("Set Master RFID: "); Serial.println(uid); 
    return true;
}


bool ConfigDataHandler::check_master_rfid(String uid) {
    if (uid.length() != RFID_UID_LENGTH) return false;

    if (master_rfid_uid.length() > 0 && master_rfid_uid.equalsIgnoreCase(uid)) {
        Serial.println("Master RFID Authorized.");
        return true; 
    }
}


// =========================================================================
// ZONE PASSWORD FUNCTIONS
// =========================================================================

// Loads the password for a specific zone.
void ConfigDataHandler::load_password(int zoneIndex) {
    if (zoneIndex < 0 || zoneIndex >= NUM_ZONES) return;
    
    long read_flag;
    EEPROM.get(get_flag_addr(zoneIndex), read_flag); 

    if (read_flag == PASS_FLAG_NUMBER) {
        char storedPass[PASSWORD_LENGTH + 1];
        EEPROM_read_string(get_password_addr(zoneIndex), storedPass, PASSWORD_LENGTH);
        strncpy(zone_passwords[zoneIndex], storedPass, PASSWORD_LENGTH);
        zone_passwords[zoneIndex][PASSWORD_LENGTH] = '\0';
        Serial.print("Zone "); Serial.print(zoneIndex);
        Serial.print(" Password is: "); Serial.println(storedPass);
    } else {
        Serial.print("Zone "); Serial.print(zoneIndex);
        Serial.println(" password not initialized.");
    }
}


// Takes a new password and updates the global password for a specific zone.
bool ConfigDataHandler::set_zone_password(int zoneIndex, const char* newPass, String *prompt) {
    if (zoneIndex < 0 || zoneIndex >= NUM_ZONES) {
        *prompt = "Invalid zone index."; 
        Serial.println(*prompt); 
        return false;
    }
    if (strlen(newPass) > PASSWORD_LENGTH) {
        *prompt = "Password too long."; 
        Serial.println(*prompt); 
        return false;
    }
    if (strlen(newPass) < MIN_PASSWORD_LENGTH) {
        *prompt = "Password too short."; 
        Serial.println(*prompt); 
        return false;
    }
    
    strncpy(zone_passwords[zoneIndex], newPass, PASSWORD_LENGTH);
    zone_passwords[zoneIndex][PASSWORD_LENGTH] = '\0';
    
    EEPROM_write_string(get_password_addr(zoneIndex), zone_passwords[zoneIndex], PASSWORD_LENGTH + 1);
    
    *prompt = "Password updated";
    Serial.print("Zone "); Serial.print(zoneIndex); 
    Serial.print(" Password saved to EEPROM: "); 
    Serial.println(zone_passwords[zoneIndex]);
    return true; 
}


// Returns a pointer to the current password for a specific zone.
const char* ConfigDataHandler::get_zone_password(int zoneIndex) {
    if (zoneIndex < 0 || zoneIndex >= NUM_ZONES) return nullptr;
    return zone_passwords[zoneIndex];
}


// =========================================================================
// ZONE RFID FUNCTIONS
// =========================================================================

// Loads the single RFID for a specific zone.
void ConfigDataHandler::load_RFID(int zoneIndex) {
    if (zoneIndex < 0 || zoneIndex >= NUM_ZONES) return;

    int currentAddr = get_rfid_start_addr(zoneIndex);
    char uidBuffer[RFID_UID_LENGTH + 1];

    EEPROM_read_string(currentAddr, uidBuffer, RFID_UID_LENGTH);
    zone_valid_RFIDs[zoneIndex] = String(uidBuffer);
    
    Serial.print("Zone "); Serial.print(zoneIndex);
    Serial.print(" Loaded RFID: "); 
    Serial.println(zone_valid_RFIDs[zoneIndex].length() == 0 ? "None" : zone_valid_RFIDs[zoneIndex]); 
}

// Saves the single RFID UID to EEPROM for a specific zone.
void ConfigDataHandler::save_RFID(int zoneIndex) {
    if (zoneIndex < 0 || zoneIndex >= NUM_ZONES) return;

    int currentAddr = get_rfid_start_addr(zoneIndex);
    const char* uidCstr = zone_valid_RFIDs[zoneIndex].c_str();
    
    // Writes the stored UID (or an empty string/null terminator if cleared)
    EEPROM_write_string(currentAddr, uidCstr, RFID_UID_LENGTH + 1);
}


// Checks if a given UID is authorized for a specific zone OR if it is the master UID.
bool ConfigDataHandler::check_RFID(int zoneIndex, String uid) {
    if (zoneIndex < 0 || zoneIndex >= NUM_ZONES) return false;
    
    // Only need to check against the single stored UID for the zone
    return zone_valid_RFIDs[zoneIndex].equalsIgnoreCase(uid);
}


// Sets (replaces) the single RFID UID for a specific zone.
bool ConfigDataHandler::set_zone_RFID(int zoneIndex, String uid, String *prompt) {
    if (zoneIndex < 0 || zoneIndex >= NUM_ZONES) {
        *prompt = "Invalid zone index"; 
        Serial.println(*prompt); 
        return false;
    }
    if (uid.length() != RFID_UID_LENGTH) {
        *prompt = "Invalid RFID format"; 
        Serial.println(*prompt); 
        return false;
    }
    
    zone_valid_RFIDs[zoneIndex] = uid;
    save_RFID(zoneIndex);

    *prompt = "RFID set successfully";
    Serial.print("Zone "); Serial.print(zoneIndex);
    Serial.print(" Set new RFID: "); Serial.println(uid); 
    return true;
}

bool ConfigDataHandler::is_master_credentials_initialized() {
    if (is_master_pass_initialized && is_master_rfid_initialized) {
        return true;
    }
    return false;
}


// =========================================================================
// DEFAULT CREDENTIALS SEEDING
// =========================================================================

void ConfigDataHandler::seed_defaults() {
    Serial.println("First boot detected. Seeding factory-default credentials...");

    // --- Master password: "0000" ---
    strncpy(master_password, "0000", PASSWORD_LENGTH);
    master_password[PASSWORD_LENGTH] = '\0';
    EEPROM.put(EEPROM_PASS_FLAG_ADDR, PASS_FLAG_NUMBER);
    EEPROM_write_string(EEPROM_MASTER_PASS_ADDR, master_password, PASSWORD_LENGTH + 1);
    is_master_pass_initialized = true;

    // --- Master RFID: "01020304" ---
    master_rfid_uid = "01020304";
    EEPROM.put(EEPROM_RFID_FLAG_ADDR, RFID_FLAG_NUMBER);
    EEPROM_write_string(EEPROM_MASTER_RFID_ADDR, master_rfid_uid.c_str(), RFID_UID_LENGTH + 1);
    is_master_rfid_initialized = true;

    // --- Zone credentials ---
    const char* default_passwords[NUM_ZONES] = {"1111", "2222", "3333"};
    const char* default_rfids[NUM_ZONES]     = {"11223344", "55667788", "AABBCCDD"};

    for (int i = 0; i < NUM_ZONES; i++) {
        // Password
        strncpy(zone_passwords[i], default_passwords[i], PASSWORD_LENGTH);
        zone_passwords[i][PASSWORD_LENGTH] = '\0';
        EEPROM.put(get_flag_addr(i), PASS_FLAG_NUMBER);
        EEPROM_write_string(get_password_addr(i), zone_passwords[i], PASSWORD_LENGTH + 1);

        // RFID
        zone_valid_RFIDs[i] = String(default_rfids[i]);
        EEPROM_write_string(get_rfid_start_addr(i), zone_valid_RFIDs[i].c_str(), RFID_UID_LENGTH + 1);

        Serial.print("Zone "); Serial.print(i);
        Serial.print(" seeded -> pass: "); Serial.print(zone_passwords[i]);
        Serial.print("  RFID: "); Serial.println(zone_valid_RFIDs[i]);
    }

    Serial.println("Factory defaults seeded successfully.");
}