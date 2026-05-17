#include "Config.h"
#include "RfidHandler.h"


// Constructor: receives the pin assignment and initialize an object of the library mfrc522
RfidHandler::RfidHandler(int ss, int rst) 
    : ss_pin(ss), rst_pin(rst), 
      mfrc522(ss_pin, rst_pin) 
{}


// Helper function to convert the raw MFRC522 UID bytes into a string. It is used by read_card_uid().
String RfidHandler::uid_to_string(MFRC522::Uid uid) const {
    String str = "";
    for (byte i = 0; i < uid.size; i++) {
        // Concatenate each byte as a two-digit hexadecimal number
        if (uid.uidByte[i] < 0x10) {
            str += "0";
        }
        str += String(uid.uidByte[i], HEX);
    }
    // Convert the entire string to uppercase for consistent comparison
    str.toUpperCase();
    return str;
}


// Initialization
bool RfidHandler::begin() {
    SPI.begin();
    mfrc522.PCD_Init();

    byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);

    if ((v == 0x00) || (v == 0xFF)) {
        // Serial for debugging
        Serial.print("RFID ERROR: Could not find MFRC522 chip (Version: 0x");
        Serial.print(v, HEX);
        Serial.println(")");
        return false; 
    }

    // Serial for debugging
    Serial.print("RFID Handler Initialized (Chip Version: 0x");
    Serial.print(v, HEX);
    Serial.println(")");
    return true; 
}



// The main function of this class: it checks for a card and reads its UID. It returns the detected UID or 
// an empty string in case of error.
String RfidHandler::read_card_uid() {
    // Check if the reader is already processing a card or is in a cooldown period
    if (!mfrc522.PICC_IsNewCardPresent()) {
        return "";
    }
    
    // Check if the card has been successfully read
    if (!mfrc522.PICC_ReadCardSerial()) {
        return "";
    }

    // 1. Get the UID of the card
    String currentUID = uid_to_string(mfrc522.uid);

    // 2. Check the Cooldown Timer: Prevents re-triggering from the same card
    if (currentUID.equals(last_read_uid) && (millis() - last_read_time < READ_COOLDOWN_MS)) {
        // If the same card is read too quickly, reset the PICC to stop reading it
        mfrc522.PICC_HaltA(); 
        return "";
    }

    // 3. Successful Read - Update State
    last_read_uid = currentUID;
    last_read_time = millis();
    
    // Halt PICC to prevent re-reading the same card instantly
    mfrc522.PICC_HaltA(); 

    // Return the new UID 
    return currentUID;
}
