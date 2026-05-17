#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H
#include <WiFiEspAT.h>
#include "Config.h"
#include <Arduino.h>

// NOTE: All extern declarations for WiFi/IFTTT
// are now located in Config.h

class WiFiHandler {
private:
    WiFiClient client; // The client object for making web requests
    int status = WL_IDLE_STATUS; // Holds the WiFi connection status
    bool is_connected = false; // Internal flag for connection state

    unsigned long last_connect_attempt = 0; // Timestamp for non-blocking retries
    const unsigned long CONNECT_RETRY_MS = 30000; // Wait 30 seconds between retries

public:
    // Constructor
    WiFiHandler();

    // Initialization: Connects to the WiFi network.
    // Call this in setup(). Returns true on success, false on failure.
    bool begin();

    // Main task: To be called in the main loop().
    // This checks the connection and attempts to reconnect if it was lost.
    void maintain_connection();

    // Returns the current connection state
    bool is_wifi_connected();

    // Sends a notification to the IFTTT server.
    // You can send up to three string values.
    // value1 is mandatory, value2 and value3 are optional.
    bool send_notification(const String& value1, const String& value2 = "", const String& value3 = "");
};

#endif