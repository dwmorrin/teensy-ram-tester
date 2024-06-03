#include <Arduino.h>

class EMT250_RAM
{
public:
  void begin();
  void address(uint16_t);
  void write(uint16_t);
  uint16_t read();
  void zeros() { test_pattern(0); }
  void ones() { test_pattern(1); }
  void aaaa() { test_pattern(0xaaaa); }
  void fives() { test_pattern(0x5555); }

private:
  int address_pin{0};
  int address_enable_pin{1};
  int clock_pin{2};
  int data_in_pin{3};
  int data_in_enable_pin{1};
  int data_out_pin{4};
  int data_out_enable_pin{1}; // enable=low
  int load_pin{5};
  int write_pin{6};
  void clock();
  void serial(int, int, uint16_t);
  void test_pattern(uint16_t data);
};

void EMT250_RAM::clock()
{
  digitalWrite(clock_pin, HIGH);
  delayMicroseconds(2.5);
  digitalWrite(clock_pin, LOW);
  delayMicroseconds(2.5);
}

void EMT250_RAM::begin()
{
  pinMode(address_pin, OUTPUT);
  digitalWrite(address_pin, LOW);
  pinMode(address_enable_pin, OUTPUT);
  digitalWrite(address_enable_pin, LOW);
  pinMode(clock_pin, OUTPUT);
  digitalWrite(clock_pin, LOW);
  pinMode(data_in_pin, OUTPUT);
  digitalWrite(data_in_pin, LOW);
  pinMode(data_in_enable_pin, OUTPUT);
  digitalWrite(data_in_enable_pin, LOW);
  pinMode(data_out_pin, INPUT);
  pinMode(data_out_enable_pin, OUTPUT);
  digitalWrite(data_out_enable_pin, HIGH);
  pinMode(load_pin, OUTPUT);
  digitalWrite(load_pin, HIGH);
  pinMode(write_pin, OUTPUT);
  digitalWrite(load_pin, HIGH);
}

void EMT250_RAM::test_pattern(uint16_t data)
{
  Serial.print("Testing with ");
  Serial.print(data, HEX);

  for (int i = 0; i < 8192; ++i)
  {
    address(i);
    write(data);
  }

  for (int i = 0; i < 8192; ++i)
  {
    address(i);
    uint16_t word = read();
    if (word != data)
    {
      Serial.print("Addr: ");
      Serial.print(i, HEX);
      Serial.print(": Expected: ");
      Serial.print(data, HEX);
      Serial.print(" -- Got: ");
      Serial.println(word, HEX);
    }
  }
}

void EMT250_RAM::address(uint16_t addr)
{
  digitalWrite(address_enable_pin, HIGH);
  serial(13, address_pin, addr);
  digitalWrite(address_enable_pin, LOW);
}

void EMT250_RAM::write(uint16_t data)
{
  digitalWrite(data_in_enable_pin, HIGH);
  serial(16, data_in_pin, data);
  digitalWrite(data_in_enable_pin, LOW);
  digitalWrite(write_pin, LOW); // worst case 450 ns pulse
  delayMicroseconds(1);
  digitalWrite(write_pin, HIGH);
}

uint16_t EMT250_RAM::read()
{
  digitalWrite(load_pin, LOW); // LOW=load; 15 ns min. pulse
  delayMicroseconds(1);
  digitalWrite(load_pin, HIGH);
  digitalWrite(data_out_enable_pin, LOW); // LOW=enable
  uint16_t result = 0;
  for (int i = 15; i >= 0; --i)
  {
    if (digitalRead(data_out_pin))
      result |= 1 << i;
    clock();
  }
  digitalWrite(data_out_enable_pin, HIGH);
  return result;
}

void EMT250_RAM::serial(int n_bits, int pin, uint16_t data)
{
  uint mask = 1 << n_bits;
  for (int i = 0; i < n_bits; ++i)
  {
    digitalWrite(address_pin, data & mask ? HIGH : LOW);
    clock();
    data <<= 1;
  }
}

EMT250_RAM ram;

void setup()
{
  // set pin modes; initialize pins
  ram.begin();

  Serial.begin(9600);
  while (!Serial)
    ;
  Serial.println("EMT-250 RAM Tester");
  Serial.println();
  Serial.println();
  Serial.println("Commands:");
  Serial.println("\ttest");
  Serial.println("\taddr [0-8191]");
  Serial.println("\twrite [0-0xffff]");
  Serial.println("\tread");
}

void loop()
{
  Serial.println(">");

  String rawCommand = Serial.readStringUntil('\n').trim();
  // copy it in case we manipulate it
  String command{rawCommand};

  if (command == "test")
  {
    ram.zeros();
    ram.ones();
    ram.aaaa();
    ram.fives();
  }
  else if (command.startsWith("addr"))
  {
    command.remove(0, 4).trim();
    int addr = command.toInt();
    if (addr >= 0 && addr < 8192)
    {
      ram.address(addr);
      Serial.println("address set");
    }
    else
      Serial.println("Usage: addr n, where n is between 0 and 8191");
  }
  else if (command.startsWith("write"))
  {
    command.remove(0, 4).trim();
    int data = command.toInt();
    if (data >= 0 && data < 0x10000)
    {
      ram.write(data);
      Serial.println("data written");
    }
    else
      Serial.println("Usage: addr n, where n is between 0 and 0xffff");
  }
  else if (command == "read")
  {
    Serial.println(ram.read());
  }
  else
  {
    Serial.print("Sorry, I don't know '");
    Serial.print(command);
    Serial.println("'");
  }
}