#ifndef RAM_TESTER_PINS
#define RAM_TESTER_PINS
#include <stdint.h>

// using pins 7-18 for address, for breadboard wiring, see .fzz file
const uint8_t address_pins[] = {
    12, 11, 10, 9, 8, 7, 13, 14, 15, 16, 17, 18};

enum
{
  reset_prog_pin = 2,
  pass_led_pin = 3,
  fail_led_pin = 4,
  data_input_pin = 5,   // RAM to Arduino
  write_enable_pin = 6, // write = LOW, read = HIGH
  data_output_pin = 19  // Arduino to RAM
};
#endif