#include <Arduino.h>

// see data sheet for read/write timing info
// setting a delay that should give the RAM time to settle before read/write
#define MAX_DELAY_NS 500

// uncomment for serial console
// #define USING_SERIAL_CONSOLE

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

// using simple error codes
enum
{
  NO_ERROR,
  ZERO_READ_ERROR,
  ONE_READ_ERROR
};

void set_read_mode()
{
  digitalWrite(write_enable_pin, HIGH);
}

void set_write_mode()
{
  digitalWrite(write_enable_pin, LOW);
}

void set_address(int address)
{
  set_read_mode();
  for (int i = 0; i < 12; i++)
    digitalWrite(i, address & (1 << i));
  // wait for data to be stable
  delayNanoseconds(MAX_DELAY_NS);
}

bool read(int address)
{
  set_address(address);
  return digitalRead(data_input_pin);
}

void write(int address, bool value)
{
  set_address(address);
  digitalWrite(data_output_pin, value);
  set_write_mode();
  // wait for write to complete
  delayNanoseconds(MAX_DELAY_NS);
}

// collect errors into this int and display at end
int error = NO_ERROR;

void setup()
{
  // set pin modes; initialize pins
  for (int i = 0; i <= fail_led_pin; ++i)
    pinMode(i, OUTPUT);
  pinMode(data_input_pin, INPUT);
  digitalWrite(pass_led_pin, LOW);
  digitalWrite(fail_led_pin, LOW);

  // write 0 to all addresses
  for (int i = 0; i < 4096; ++i)
    write(i, 0);

  // check that all addresses are 0
  set_read_mode();
  for (int i = 0; i < 4096; ++i)
    if (read(i))
      error |= ZERO_READ_ERROR;

  // write 1 to all addresses
  for (int i = 0; i < 4096; ++i)
    write(i, 1);

  // check that all addresses are 1
  for (int i = 0; i < 4096; ++i)
    if (!read(i))
      error |= ONE_READ_ERROR;

  // display pass/fail
  digitalWrite(error ? fail_led_pin : pass_led_pin, HIGH);

  // send more info to serial console
#ifdef USING_SERIAL_CONSOLE
  Serial.begin(9600);
  Serial.println("error code:");
  Serial.println(error);
  Serial.println("error message:");
  if (error == NO_ERROR)
    Serial.println("tests OK");
  if (error & ZERO_READ_ERROR)
    Serial.println("zero read error");
  if (error & ONE_READ_ERROR)
    Serial.println("one read error");
  Serial.println("done");
#endif
}

void loop()
{
  // blink led to indicate done
  static bool led_is_on = false;
  led_is_on = !led_is_on;
  digitalWrite(data_output_pin, led_is_on);
  delay(500);
}