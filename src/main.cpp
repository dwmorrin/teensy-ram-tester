#include <Arduino.h>
#include "pins.h"
#include "tms4044.h"

#define USING_SERIAL

// using simple error codes
enum
{
  NO_ERROR,
  ZERO_READ_ERROR,
  ONE_READ_ERROR
};

// collect errors into this int and display at end
uint8_t error = NO_ERROR;
uint16_t read_zero_error_count = 0;
uint16_t read_one_error_count = 0;

void setup()
{
  // set pin modes; initialize pins
  for (size_t i = 0; i < 12; ++i)
    pinMode(address_pins[i], OUTPUT);
  pinMode(pass_led_pin, OUTPUT);
  pinMode(fail_led_pin, OUTPUT);
  pinMode(write_enable_pin, OUTPUT);
  pinMode(data_output_pin, OUTPUT);
  pinMode(data_input_pin, INPUT);
  pinMode(reset_prog_pin, INPUT);
  digitalWrite(pass_led_pin, LOW);
  digitalWrite(fail_led_pin, LOW);

#ifdef USING_SERIAL
  digitalWrite(fail_led_pin, HIGH);
  Serial.begin(9600);
  while (!Serial)
    ;
  digitalWrite(fail_led_pin, LOW);
#endif
}

void test_ram()
{
  // reset errors
  error = NO_ERROR;
  read_zero_error_count = 0;
  read_one_error_count = 0;
  // reset LEDs
  digitalWrite(pass_led_pin, LOW);
  digitalWrite(fail_led_pin, LOW);

  // write 0 to all addresses
  for (size_t i = 0; i < 4096; ++i)
    write(i, 0);

  // check that all addresses are 0
  for (size_t i = 0; i < 4096; ++i)
    if (read(i))
    {
      error |= ZERO_READ_ERROR;
      ++read_zero_error_count;
    }

  // write 1 to all addresses
  for (size_t i = 0; i < 4096; ++i)
    write(i, 1);

  // check that all addresses are 1
  for (size_t i = 0; i < 4096; ++i)
    if (!read(i))
    {
      error |= ONE_READ_ERROR;
      ++read_one_error_count;
    }

#ifdef USING_SERIAL
  Serial.print("Error: ");
  Serial.print(error);
  Serial.println();
  if (error & ZERO_READ_ERROR)
  {
    Serial.print(read_zero_error_count);
    Serial.println(" 1s when expecting 0s");
  }
  if (error & ONE_READ_ERROR)
  {
    Serial.print(read_one_error_count);
    Serial.println(" 0s when expecting 1s");
  }
  Serial.println();
#endif

  digitalWrite(error ? fail_led_pin : pass_led_pin, HIGH);
}

void loop()
{
  static unsigned long last_time = 0;
  static uint8_t last_state = LOW;
  uint8_t reset_prog_state = digitalRead(reset_prog_pin);
  if (reset_prog_state != last_state)
  {
    last_time = millis();
  }
  if ((millis() - last_time) > 100)
  {
    if (reset_prog_state == HIGH)
      test_ram();
  }
  last_state = reset_prog_state;
}