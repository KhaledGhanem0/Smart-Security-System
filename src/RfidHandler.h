#ifndef RFID_HANDLER_H
#define RFID_HANDLER_H

#include <SPI.h> // Required for MFRC522 communication
#include <MFRC522.h> 
#include "Config.h"


class RfidHandler {
private:
    int ss_pin; // Chip Select pin (SS)
    int rst_pin; // Reset pin (RST)

    MFRC522 mfrc522; // The RFID reader object

    String last_read_uid = ""; // Stores the UID of the last card detected
    unsigned long last_read_time = 0; // Timestamp of the last successful read
    
    // Time (in ms) to ignore the same card to prevent re-triggering the system instantly.
    const unsigned long READ_COOLDOWN_MS = 2000; 

public:
    // Constructor: receives the pin assignment and initialize an object of the library mfrc522
    RfidHandler(int ss, int rst);

    // Initialization
     bool begin();

    // The main function of this class: it checks for a card and reads its UID.
    String read_card_uid();
    
    // Helper function to convert the raw MFRC522 UID bytes into a string. It is used by read_card_uid().
    String uid_to_string(MFRC522::Uid uid) const;
};

#endif