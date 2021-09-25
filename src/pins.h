#ifndef RAM_TESTER_PINS
#define RAM_TESTER_PINS
// using pins 0-11 for address
// pin 12 is write enable
// pin 13 is data from Arduino to RAM
// pin 14 is data from RAM to Arduino (input)
// pin 15 is PASS LED
// pin 16 is FAIL LED
enum
{
  write_enable_pin = 12,
  data_output_pin = 13,
  data_input_pin = 14,
  pass_led_pin = 15,
  fail_led_pin = 16
};
#endif