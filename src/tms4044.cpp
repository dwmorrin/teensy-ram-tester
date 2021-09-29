#include <Arduino.h>
#include "pins.h"
#include "tms4044.h"

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
  for (size_t i = 0; i < 12; i++)
    digitalWrite(address_pins[i], address & (1 << i) ? HIGH : LOW);
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