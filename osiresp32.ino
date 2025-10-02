//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as
//    published by the Free Software Foundation, either version 3 of the
//    License, or (at your option) any later version.
//


#include "cpu6502.h"
#include "motherboard.h"
#include "terminal.h"


void setup() {
    // Inicializar Serial para terminal - PRIMERA PRIORIDAD
    Serial.begin(115200);
    
    // Wait for serial to initialize - CRITICAL for ESP32
    delay(3000); // 3 segundos para asegurar conexión
    
    // BASIC TEST - Should appear immediately
    Serial.println("*** SERIAL WORKING - ESP32 ALIVE ***");
    Serial.flush();
    
    Serial.println();
    Serial.println("====================================");
    Serial.println("OSI Superboard II Emulator for ESP32");  
    Serial.println("     Based on UK101RE by ea4eoz     ");
    Serial.println("====================================");
    Serial.println();
    Serial.flush();

    // Set terminal into raw mode
    // among other things
    configure_terminal();
    
    // We are ready. Let's start the emulation!

    // Initializes hardware
    motherboard_init();
    
    // Reset all devices
    motherboard_reset();
}

void loop() {
    unsigned long start_time, end_time, elapsed_us;
    int cycles = 0;
    
    start_time = micros();

    // Run for 20000 cycles
    while (cycles < 20000){
        cycles += cpu_execute();
    }

    if (ACTION){
        if (ACTION == ACTION_RESET){
            Serial.println("\n*** CPU Reset ***");
            cpu_reset();
        }
        // Process other user actions here
        ACTION = ACTION_NONE;
    }

    end_time = micros();

    // Calculate the time spent in microseconds
    elapsed_us = end_time - start_time;

    // 20000 cycles at 1.000 MHz should take 20 ms = 20000 us
    long target_time_us = 20000L;
    long sleep_time_us = target_time_us - elapsed_us;
    
    if(sleep_time_us > 0){
        delayMicroseconds(sleep_time_us);
    }
}