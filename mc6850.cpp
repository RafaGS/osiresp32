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

#include <stdint.h>
#include <stdio.h> // printf
#include <Arduino.h>
#include "terminal.h"
#include "mc6850.h"

// https://www.cpcwiki.eu/imgs/3/3f/MC6850.pdf

// MC6850 ACIA registers
static uint8_t TDR; // Transmit Data Register
static uint8_t RDR; // Receive Data Register
static uint8_t CR;  // Control Register
static uint8_t SR;  // Status Register



void mc6850_reset(void){
    TDR = 0x00;
    RDR = 0x00;
    CR = 0x00;
    SR = 0x0E;
}



// Read a byte from the ACIA
uint8_t mc6850_readbyte(uint16_t address){
    uint8_t data;
    switch (address & 0x0001){
        case 0: // SR (Status Register)
            // Check if there's a character available from terminal
            if (check_keyboard_ready()){
                SR |= 0x01; // Set RDRF (Receive Data Register Full)
            } else {
                SR &= 0xFE; // Clear RDRF if no key
            }
            data = SR;
            break;
            
        case 1: // RDR (Receive Data Register)
            // Read character from terminal
            RDR = read_keyboard();
            data = RDR;
            SR &= 0xFE; // Clear RDRF after reading
            break;
    }
    return data;
}



// Writes a byte to the ACIA
void mc6850_writebyte(uint16_t address, uint8_t data){
    switch (address & 0x0001){
        case 0: // CR
            CR = data;
            break;
            
        case 1: // TDR
            write_terminal(data);
            SR |= 0x02;
            break;
    }
}
