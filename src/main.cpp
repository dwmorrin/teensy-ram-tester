#include <Arduino.h>

// using pins 0-11 for address
// pin 12 is write enable
// pin 13 is data from arudino to RAM
// pin 14 is data from RAM to arudino (input)
enum
{
  write_enable_pin = 12,
  data_output_pin = 13,
  data_input_pin = 14
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
  for (int i = 0; i < 12; i++)
    digitalWrite(i, address & (1 << i));
}

bool read(int address)
{
  set_address(address);
  return digitalRead(data_input_pin);
}

void setup()
{
  // collect errors into this int and display at end
  int error = NO_ERROR;

  // set pin modes
  for (int i = 0; i < data_input_pin; ++i)
    pinMode(i, OUTPUT);
  pinMode(data_input_pin, INPUT);

  // write 0 to all addresses
  set_address(0);
  digitalWrite(data_output_pin, 0);
  set_write_mode();
  for (int i = 1; i < 4096; ++i)
    set_address(i);

  // check that all addresses are 0
  set_read_mode();
  for (int i = 0; i < 4096; ++i)
    if (read(i) != 0)
      error |= ZERO_READ_ERROR;

  // write 1 to all addresses
  set_address(0);
  digitalWrite(data_output_pin, 1);
  set_write_mode();
  for (int i = 0; i < 4096; ++i)
    set_address(i);

  // check that all addresses are 1
  set_read_mode();
  // check that all addresses are 1
  for (int i = 0; i < 4096; ++i)
    if (read(i) != 1)
      error |= ONE_READ_ERROR;

  // display error
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
}

void loop()
{
  // blink led to indicate done
  static bool led_is_on = false;
  led_is_on = !led_is_on;
  digitalWrite(13, led_is_on);
  delay(500);
}