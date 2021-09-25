#include <Arduino.h>
#include "pins.h"
#include "tms4044.h"
using std::size_t;

// uncomment for serial console
// #define USING_SERIAL_CONSOLE

// using simple error codes
enum
{
  NO_ERROR,
  ZERO_READ_ERROR,
  ONE_READ_ERROR
};

// collect errors into this int and display at end
int error = NO_ERROR;

void setup()
{
  // set pin modes; initialize pins
  for (size_t i = 0; i <= fail_led_pin; ++i)
    pinMode(i, OUTPUT);
  pinMode(data_input_pin, INPUT);
  digitalWrite(pass_led_pin, LOW);
  digitalWrite(fail_led_pin, LOW);

  // write 0 to all addresses
  for (size_t i = 0; i < 4096; ++i)
    write(i, 0);

  // check that all addresses are 0
  for (size_t i = 0; i < 4096; ++i)
    if (read(i))
      error |= ZERO_READ_ERROR;

  // write 1 to all addresses
  for (size_t i = 0; i < 4096; ++i)
    write(i, 1);

  // check that all addresses are 1
  for (size_t i = 0; i < 4096; ++i)
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