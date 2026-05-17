#include "WiFiHandler.h"

// ---------- WIFI & IFTTT CONFIGURATION ---------- //
// (You must fill in these 4 values!)

// Your WiFi network name
const char* WIFI_SSID = "Wifi name";
// Your WiFi network password
const char* WIFI_PASS = "Wifi password";

// The "Event Name" you create on IFTTT (e.g., "security_breach")
const char* IFTTT_EVENT_NAME = "security_breach";
// Your personal key from the IFTTT Webhooks documentation page
const char* IFTTT_KEY = "*********************";

// IFTTT Server (this is constant)
const char* IFTTT_SERVER = "maker.ifttt.com";


// Constructor
WiFiHandler::WiFiHandler() {
    is_connected = false;
    last_connect_attempt = 0;
}

// Initialization
bool WiFiHandler::begin() {
    // Initialize serial communication with the ESP8266
    // We use Serial1 (Pins 18, 19) on the Arduino Mega
    Serial1.begin(115200); 
    WiFi.init(&Serial1);

    // Check if the ESP8266 module is present
    if (WiFi.status() == WL_NO_SHIELD) {
        Serial.println("WiFi ERROR: ESP8266 module not detected!");
        is_connected = false;
        return false;
    }

    // Attempt to connect to the WiFi network
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(WIFI_SSID);

    status = WiFi.begin(WIFI_SSID, WIFI_PASS);

    if (status == WL_CONNECTED) {
        Serial.println("WiFi Connected.");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        is_connected = true;
        last_connect_attempt = millis(); // Set timestamp
        return true;
    } else {
        Serial.println("WiFi Connection Failed.");
        is_connected = false;
        last_connect_attempt = millis(); // Set timestamp for retry
        return false;
    }
}

// Returns the current connection state
bool WiFiHandler::is_wifi_connected() {
    return is_connected;
}

// Main connection maintenance function (call in loop())
void WiFiHandler::maintain_connection() {
    // If we are connected, check the status.
    if (is_connected) {
        status = WiFi.status();
        if (status == WL_CONNECTED) {
            return; // All good
        }
        
        // If we *were* connected, but now are not
        Serial.println("WiFi connection lost!");
        is_connected = false;
        last_connect_attempt = millis();
    }

    // If we are not connected, and the retry timer has expired
    if (!is_connected && (millis() - last_connect_attempt > CONNECT_RETRY_MS)) {
        Serial.println("Attempting to reconnect WiFi...");
        begin(); // Re-run the begin logic to connect
        // 'begin' will update is_connected and last_connect_attempt
    }
}

// Sends a notification to the IFTTT server
bool WiFiHandler::send_notification(const String& value1, const String& value2, const String& value3) {
    
    // Check if we are connected before trying to send
    if (!is_wifi_connected()) {
        Serial.println("IFTTT Error: WiFi not connected.");
        return false;
    }

    Serial.println("Connecting to IFTTT server...");

    // Attempt to connect to the IFTTT server
    if (client.connect(IFTTT_SERVER, 80)) {
        Serial.println("IFTTT connected.");

        // Construct the URL for the GET request
        // Format: /trigger/{event}/with/key/{key}?value1=...&value2=...&value3=...
        String url = "/trigger/";
        url += IFTTT_EVENT_NAME;
        url += "/with/key/";
        url += IFTTT_KEY;
        
        // Add values as query parameters
        url += "?value1=" + value1;
        
        if (value2.length() > 0) {
            url += "&value2=" + value2;
        }
        if (value3.length() > 0) {
            url += "&value3=" + value3;
        }

        // Send the HTTP GET request
        client.print("GET " + url + " HTTP/1.1\r\n");
        client.print("Host: " + String(IFTTT_SERVER) + "\r\n");
        client.print("Connection: close\r\n");
        client.print("\r\n");

        Serial.println("IFTTT request sent.");
        
        // You can add code here to read the response from the server if needed
        // For now, we just close the connection
        client.stop();
        return true;

    } else {
        // Connection to IFTTT server failed
        Serial.println("IFTTT connection failed!");
        return false;
    }
}