//
//    Terminal functions for ESP32 Arduino
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as
//    published by the Free Software Foundation, either version 3 of the
//    License, or (at your option) any later version.
//

#include <Arduino.h>
#include "terminal.h"

// Global action variable
int ACTION = 0;

// Buffer for serial input
#define SERIAL_BUFFER_SIZE 256
static char serialBuffer[SERIAL_BUFFER_SIZE];
static int bufferIndex = 0;
static int bufferSize = 0;

// Configure terminal for ESP32
void configure_terminal(void) {
    // Serial is already initialized in setup()
    // Clear any pending input
    while (Serial.available()) {
        Serial.read();
    }
    bufferIndex = 0;
    bufferSize = 0;
}

// Check if keyboard/serial data is ready
uint8_t check_keyboard_ready(void) {
    // Check if there's data in our buffer or serial
    if (bufferIndex < bufferSize) {
        return 1; // Data available in buffer
    }

    // Check serial for new data
    int available = Serial.available();
    if (available > 0) {
        // Read available data into buffer
        int toRead = min(available, SERIAL_BUFFER_SIZE - bufferSize);
        for (int i = 0; i < toRead; i++) {
            serialBuffer[bufferSize++] = Serial.read();
        }
        return 1; // Data available
    }

    return 0; // No data available
}

// Read keyboard/serial input
uint8_t read_keyboard(void) {
    if (bufferIndex < bufferSize) {
        uint8_t ch = serialBuffer[bufferIndex++];
        // If we consumed all data, reset buffer
        if (bufferIndex >= bufferSize) {
            bufferIndex = 0;
            bufferSize = 0;
        }
        return ch;
    }

    // If no data in buffer, try to read directly
    if (Serial.available()) {
        return Serial.read();
    }

    return 0; // No data available
}

// Write to terminal/serial output
void write_terminal(uint8_t byte) {
    // Handle special characters
    if (byte == '\n') {
        Serial.println(); // Add CR+LF for terminal compatibility
    } else if (byte >= 32 && byte <= 126) {
        Serial.write(byte); // Printable characters
    } else if (byte == 13) {
        // CR - ignore, let println handle it
    } else {
        // Other control characters - could be handled specially
        Serial.write(byte);
    }
}
