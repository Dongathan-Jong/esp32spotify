

#include "lcd2004.h"

#include <inttypes.h>

LCD2004::LCD2004(uint8_t rs) {
    rs_pin = rs;
}

void LCD2004::begin() {
    delay(DELAY_MS_POR);
    pinMode(rs_pin, OUTPUT);
    pinMode(16, OUTPUT);

    pinMode(15, OUTPUT);
    pinMode(14, OUTPUT);
    pinMode(13, OUTPUT);
    pinMode(12, OUTPUT);

    // Initialization sequence
    GP16O = 1;
    GPOC = 0xF000 | (1 << rs_pin);
    GPOS = 0x3000;
    for(uint8_t i=0; i<3; i++) {
        delayMicroseconds(DELAY_US_SETUP_HOLD);
        GP16O = 0;
        delayMicroseconds(DELAY_US_SETUP_HOLD);
        GP16O = 1;
        delayMicroseconds(DELAY_US_INIT);
    }
    
    // Set 4-bit mode
    GPOC = 0x1000;
    delayMicroseconds(DELAY_US_SETUP_HOLD);
    GP16O = 0;
    delayMicroseconds(DELAY_US_SETUP_HOLD);
    GP16O = 1;
    delayMicroseconds(DELAY_US_DATA_COMMAND);

    // Function Set
    cmd(0x28); // 4-bit mode, 2 lines, 5x8 font
    delayMicroseconds(DELAY_US_DATA_COMMAND);

    cmd(0x0C); // Display on
    delayMicroseconds(DELAY_US_DATA_COMMAND);

    cmd(0x06); // Entry mode auto-increment
    delayMicroseconds(DELAY_US_DATA_COMMAND);
}

void LCD2004::clear() {
    cmd(0x01);
    delayMicroseconds(DELAY_US_LONG_COMMAND);
}

void LCD2004::cmd(uint8_t val) {
    GPOC = (1 << rs_pin);
    delayMicroseconds(DELAY_US_SETUP_HOLD);
    write(val);
    GPOS = (1 << rs_pin);
    delayMicroseconds(DELAY_US_DATA_COMMAND);
}

size_t IRAM_ATTR LCD2004::write(uint8_t val) {
    GPOC = 0xF000;
    GPOS = ((uint32_t)(val & 0xF0)) << 8;
    delayMicroseconds(DELAY_US_SETUP_HOLD);
    GP16O = 0;
    GP16O = 1;
    GPOC = 0xF000;
    GPOS = ((uint32_t)(val & 0x0F)) << 12;
    delayMicroseconds(DELAY_US_SETUP_HOLD);
    GP16O = 0;
    GP16O = 1;
    delayMicroseconds(DELAY_US_DATA_COMMAND);
    return 1;
}

void LCD2004::setCursor(uint8_t col, uint8_t row) {
    const uint8_t row_addrs[] = { 0x00, 0x40, 0x14, 0x54 }; // For regular LCDs
    cmd(0x80 | (col + row_addrs[row]));
}
