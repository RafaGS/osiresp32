//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as
//    published by the Free Software Foundation, either version 3 of the
//    License, or (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful, but
//    WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this program.  If not, see
//    <http://www.gnu.org/licenses/>
//
//#define OHIO
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <Arduino.h>

#include "cpu6502.h"
#include "mc6850.h"
//#include "pia6820.h"
#include "osi.rom.h"


// 32 kB ROM
#define ROMSIZE 0x8000
#define ROMMASK 0x7FFF
static uint8_t ROM[ROMSIZE];


// 32k kB RAM
#define RAMSIZE 0x8000
#define RAMMASK 0x7FFF
static uint8_t RAM[RAMSIZE];

// OSI video RAM simulation (1KB for 32x32 screen)
static uint8_t VIDEO_RAM[1024];
static uint16_t video_ram_base = 0xD000;  // Default OSI video RAM address

// OSI Polled Keyboard (0xDF00) - 8x8 matrix
static uint8_t keyboard_row_select = 0;
static uint8_t keyboard_matrix[8] = {0};  // 8 rows, each byte holds 8 column bits
static int keyboard_read_counter = 0;  // Count reads to auto-release key
static int key_detected = 0;  // Flag: key has been detected at least once

// Forward declarations
static void check_keyboard_input(void);
static void update_keyboard_matrix(char c);

// ROM read: returns the byte requested
static uint8_t rom_readbyte(uint16_t address){
    //printf("Addr: %x / %x\n", address, address & ROMMASK);

    return ROM[address & ROMMASK];
}



//ROM write: writting to the ROM does nothing
static void rom_writebyte(uint16_t address, uint8_t data){
    // Do nothing
}



// RAM read: returns the byte requested
static uint8_t ram_readbyte(uint16_t address){
    return RAM[address & RAMMASK];
}



// RAM write: write the byte requested with the new value
static void ram_writebyte(uint16_t address, uint8_t data){
    RAM[address & RAMMASK] = data;
}

// Check for keyboard input (ESP32 Serial version)
static void check_keyboard_input(void) {
    // Check if there's data available from Serial
    if (Serial.available() > 0) {
        char c = Serial.read();
        
        // Convert to uppercase for OSI
        if (c >= 'a' && c <= 'z') {
            c = c - 'a' + 'A';
        }
        // Handle special keys
        if (c == '\n' || c == '\r') {
            c = 0x0D;  // CR
        } else if (c == 0x7F || c == 0x08) {
            c = 0x5F;  // Backspace -> underscore (OSI convention)
        }
        update_keyboard_matrix(c);
    }
}

// Update keyboard matrix based on pressed key
// OSI keyboard matrix layout - based on osiemu/src/keyboard.c
// matrix[row][col]:
// Row 0: - rshift lshift - - ESC ctrl rpt
// Row 1: - P ; / SPACE Z A Q
// Row 2: - , M N B V C X
// Row 3: - K J H G F D S
// Row 4: - I U Y T R E W
// Row 5: - - - RETURN LF O L .
// Row 6: - - BKSP - : 0 9 8
// Row 7: - 7 6 5 4 3 2 1
static void update_keyboard_matrix(char c) {
    // Don't clear matrix - let key persist
    // Matrix will be cleared after a timeout
    
    // Map character to row/col (inverted logic - clear bit when key pressed)
    int row = -1, col = -1;
    
    // Row 1: P ; / SPACE Z A Q
    if (c == 'P') { row = 1; col = 1; }
    else if (c == ';') { row = 1; col = 2; }
    else if (c == '/') { row = 1; col = 3; }
    else if (c == ' ') { row = 1; col = 4; }
    else if (c == 'Z') { row = 1; col = 5; }
    else if (c == 'A') { row = 1; col = 6; }
    else if (c == 'Q') { row = 1; col = 7; }
    
    // Row 2: , M N B V C X
    else if (c == ',') { row = 2; col = 1; }
    else if (c == 'M') { row = 2; col = 2; }
    else if (c == 'N') { row = 2; col = 3; }
    else if (c == 'B') { row = 2; col = 4; }
    else if (c == 'V') { row = 2; col = 5; }
    else if (c == 'C') { row = 2; col = 6; }
    else if (c == 'X') { row = 2; col = 7; }
    
    // Row 3: K J H G F D S
    else if (c == 'K') { row = 3; col = 1; }
    else if (c == 'J') { row = 3; col = 2; }
    else if (c == 'H') { row = 3; col = 3; }
    else if (c == 'G') { row = 3; col = 4; }
    else if (c == 'F') { row = 3; col = 5; }
    else if (c == 'D') { row = 3; col = 6; }
    else if (c == 'S') { row = 3; col = 7; }
    
    // Row 4: I U Y T R E W
    else if (c == 'I') { row = 4; col = 1; }
    else if (c == 'U') { row = 4; col = 2; }
    else if (c == 'Y') { row = 4; col = 3; }
    else if (c == 'T') { row = 4; col = 4; }
    else if (c == 'R') { row = 4; col = 5; }
    else if (c == 'E') { row = 4; col = 6; }
    else if (c == 'W') { row = 4; col = 7; }
    
    // Row 5: RETURN O L .
    else if (c == 0x0D || c == '\n') { row = 5; col = 3; }
    else if (c == 'O') { row = 5; col = 5; }
    else if (c == 'L') { row = 5; col = 6; }
    else if (c == '.') { row = 5; col = 7; }
    
    // Row 6: BKSP : 0 9 8
    else if (c == 0x08 || c == 0x7F) { row = 6; col = 2; }
    else if (c == ':') { row = 6; col = 4; }
    else if (c == '0') { row = 6; col = 5; }
    else if (c == '9') { row = 6; col = 6; }
    else if (c == '8') { row = 6; col = 7; }
    
    // Row 7: 7 6 5 4 3 2 1
    else if (c == '7') { row = 7; col = 1; }
    else if (c == '6') { row = 7; col = 2; }
    else if (c == '5') { row = 7; col = 3; }
    else if (c == '4') { row = 7; col = 4; }
    else if (c == '3') { row = 7; col = 5; }
    else if (c == '2') { row = 7; col = 6; }
    else if (c == '1') { row = 7; col = 7; }
    
    if (row >= 0 && col >= 0) {
        keyboard_matrix[row] = 0xFF & ~(1 << col);  // Clear bit for pressed key (inverted logic)
        keyboard_read_counter = 0;  // Reset auto-release counter
    }
}

void motherboard_init(void){
    // Load rom from embedded data
    if (osi_rom_len != ROMSIZE) {
        Serial.print("Warning: ROM size mismatch! Embedded ROM is ");
        Serial.print(osi_rom_len);
        Serial.print(" bytes, expected ");
        Serial.print(ROMSIZE);
        Serial.println(" bytes.");
    }
    memcpy(ROM, osi_rom, (osi_rom_len < ROMSIZE) ? osi_rom_len : ROMSIZE);

    // Initialize keyboard matrix to 0xFF (all keys released)
    for (int i = 0; i < 8; i++) {
        keyboard_matrix[i] = 0xFF;
    }
    
    // Start with SHIFTLOCK pressed (row 0, bit 0) like UK101 Java emulator
    keyboard_matrix[0] = 0xFE;  // Clear bit 0 (SHIFTLOCK)
}



void motherboard_reset(void){
    // This function simulates the reset of the whole system, just
    // like the real reset signal in a motherboard
    // Neither ROM or RAM have reset
    //
    // Resets ACIA
    mc6850_reset();
    
    // Resets CPU
    cpu_reset();
    
    // Done
}



// NOTE: This kind of switch-case with built-in
// ranges is an extension of GCC and Clang/LLVM
uint8_t motherboard_readbyte(uint16_t address){ 
    // This switch-case implements the address decoding
    switch (address){
        case 0x0000 ... 0x7FFF :
            return ram_readbyte(address);
            break;

        case 0x8000 ... 0xBFFF :
            return rom_readbyte(address);
            break;
        
        // OSI compatibility: ACIA at 0xC010-0xC011 (mapped to mc6850)
        case 0xC010 ... 0xC011 :
            return mc6850_readbyte(0xF000 + (address & 0x01));
            break;
        
        // OSI Video RAM (0xD000-0xD3FF typically)
        case 0xD000 ... 0xD3FF :
            return VIDEO_RAM[address - video_ram_base];
            break;
        
        // OSI middle range (0xD400-0xDEFF)
        case 0xD400 ... 0xDEFF :
            return rom_readbyte(address);
            break;
        
        // OSI Polled Keyboard (entire page 0xDF00-0xDFFF reads from keyboard)
        // This matches osiemu: all addresses in this page return keyboard_read()
        case 0xDF00 ... 0xDFFF :
            check_keyboard_input();
            {
                // UK101 keyboard logic: check bits that are 0 in row_select
                // NO inversion needed - check directly for 0 bits
                uint8_t value = 0xFF;
                
                for (int i = 0; i < 8; i++) {
                    if ((keyboard_row_select & (1 << i)) == 0) {  // If bit is 0
                        value &= keyboard_matrix[i];
                    }
                }
                
                // Debug: show ALL reads for debugging
                static int total_reads = 0;
                total_reads++;
                if (keyboard_read_counter < 10 && value != 0xFF) {
                    key_detected++;
                    // Release key after 3 detections
                    if (key_detected >= 3) {
                        for (int i = 0; i < 8; i++) {
                            keyboard_matrix[i] = 0xFF;
                        }
                        key_detected = 0;
                    }
                }
                
                // Auto-release key after enough reads (backup mechanism)
                keyboard_read_counter++;
                if (keyboard_read_counter > 100) {
                    for (int i = 0; i < 8; i++) {
                        keyboard_matrix[i] = 0xFF;
                    }
                    keyboard_read_counter = 0;
                    key_detected = 0;
                }
                
                return value;
            }
            break;
        
        case 0xF000 ... 0xF7FF :
            return mc6850_readbyte(address);
            break;
            
        // Kernel ROM at 0xF800-0xFBFF
        case 0xF800 ... 0xFBFF :
            return rom_readbyte(address);
            break;
        
        // Monitor ACIA at 0xFC00-0xFC01 (OSI C3 Serial monitor uses this address)
        case 0xFC00 ... 0xFC01 :
            return mc6850_readbyte(address);
            break;
        
        // Kernel ROM continues at 0xFC02-0xFFFF
        case 0xFC02 ... 0xFFFF :
            return rom_readbyte(address);
            break;
            
        default: 
            // Reading a not used address 
            // returns 0xFF in real hardware
            return 0xFF;
            break;
    }
}



void motherboard_writebyte(uint16_t address, uint8_t data){
    // This switch-case implements the address decoding
    switch (address){
        case 0x0000 ... 0x7FFF : 
            ram_writebyte(address, data);
            break;
                                
        case 0x8000 ... 0xBFFF :
            return rom_writebyte(address, data);
            break;
        
        // OSI compatibility: ACIA at 0xC010-0xC011 (mapped to mc6850)
        case 0xC010 ... 0xC011 :
            mc6850_writebyte(0xF000 + (address & 0x01), data);
            break;
        
        // OSI Video RAM (0xD000-0xD3FF typically)
        case 0xD000 ... 0xD3FF :
            VIDEO_RAM[address - video_ram_base] = data;
            break;
        
        // OSI middle range (0xD400-0xDEFF)
        case 0xD400 ... 0xDEFF :
            return rom_writebyte(address, data);
            break;
        
        // OSI Polled Keyboard (0xDF00) - row select on write
        case 0xDF00 :
            check_keyboard_input();  // Check for new input on every row select
            keyboard_row_select = data;
            break;
        
        // OSI unused (0xDF01)
        case 0xDF01 :
            // Nothing
            break;
        
        // Rest (0xDF02-0xEFFF)
        case 0xDF02 ... 0xEFFF :
            return rom_writebyte(address, data);
            break;
                                 
        case 0xF000 ... 0xF7FF : 
            return mc6850_writebyte(address, data);
            break;
            
        // Kernel ROM at 0xF800-0xFBFF
        case 0xF800 ... 0xFBFF :
            return rom_writebyte(address, data);
            break;
        
        // Monitor ACIA at 0xFC00-0xFC01 (OSI C3 Serial monitor uses this address)
        case 0xFC00 ... 0xFC01 :
            return mc6850_writebyte(address, data);
            break;
            
        // Kernel ROM continues at 0xFC02-0xFFFF
        case 0xFC02 ... 0xFFFF :
            return rom_writebyte(address, data);
            break;
            
        default: 
            // Writting to a not used address 
            // should do nothing
            break;
    }
}
            

