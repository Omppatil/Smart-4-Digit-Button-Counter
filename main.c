#include <REGX51.H>

// Segment codes for 0–9 (Common Cathode)
unsigned char seg_code[10] = {
    0x3F, 0x06, 0x5B, 0x4F,
    0x66, 0x6D, 0x7D, 0x07,
    0x7F, 0x6F
};

// Digit select pins (Active LOW)
sbit DIG1 = P0^0;
sbit DIG2 = P0^1;
sbit DIG3 = P0^2;
sbit DIG4 = P0^3;

// Buttons
sbit MANUAL_BTN = P3^3;
sbit RESET_BTN  = P3^4;

// Global variables
unsigned int number = 0;
bit is_auto_mode = 0;
bit manual_flag = 0;
bit reset_flag = 0;

unsigned int timer_count = 0;
unsigned char digit_index = 0;
unsigned char digits[4];   // ? Declare globally

// INT0 toggles auto/manual mode
void external0_ISR(void) interrupt 0 {
    is_auto_mode = !is_auto_mode;
}

// Timer0 ISR – every 2ms
void timer0_ISR(void) interrupt 1 {
    TH0 = 0xF8;   // Reload for ~2ms (11.0592 MHz)
    TL0 = 0x30;

    // Update digits array
    digits[0] = number / 1000;
    digits[1] = (number / 100) % 10;
    digits[2] = (number / 10) % 10;
    digits[3] = number % 10;

    // Turn off all digits
    DIG1 = DIG2 = DIG3 = DIG4 = 1;

    // Output segment for current digit
    P2 = seg_code[digits[digit_index]];

    switch (digit_index) {
        case 0: DIG1 = 0; break;
        case 1: DIG2 = 0; break;
        case 2: DIG3 = 0; break;
        case 3: DIG4 = 0; break;
    }

    digit_index = (digit_index + 1) % 4;

    // Count 2ms intervals to reach 1 second
    if (is_auto_mode) {
        timer_count++;
        if (timer_count >= 500) {  // 500 × 2ms = 1s
            timer_count = 0;
            number++;
            if (number > 9999) number = 0;
        }
    }
}

// Short delay (~2–3 us per count)
void delay_us(unsigned int us) {
    while (us--) {
        unsigned char i;
        for (i = 0; i < 5; i++);
    }
}

void main() {
    // Port setup
    P0 = 0xFF;  // Digit control output (active LOW)
    P2 = 0x00;  // Segment output
    P3 = 0xFF;  // Enable pull-ups

    // INT0 setup (toggle mode)
    IT0 = 1;    // Edge triggered
    EX0 = 1;
    EA = 1;

    // Timer0 setup for ~2ms
    TMOD = 0x01;    // Timer0 Mode 1
    TH0 = 0xF8;
    TL0 = 0x30;
    ET0 = 1;        // Enable Timer0 interrupt
    TR0 = 1;        // Start Timer0

    while (1) {
        // Manual mode counting
        if (!is_auto_mode) {
            if (MANUAL_BTN == 0 && manual_flag == 0) {
                manual_flag = 1;
                number++;
                if (number > 9999) number = 0;
            } else if (MANUAL_BTN == 1) {
                manual_flag = 0;
            }
        }

        // Reset button logic
        if (RESET_BTN == 0 && reset_flag == 0) {
            reset_flag = 1;
            number = 0;
        } else if (RESET_BTN == 1) {
            reset_flag = 0;
        }
    }
}