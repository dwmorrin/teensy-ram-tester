#include <Arduino.h>

void print_bad_bits(uint16_t a, uint16_t b)
{
  uint16_t mask = 1 << 15;
  for (int i = 15; i >= 0; --i)
  {
    if ((a & mask) != (b & mask))
    {
      Serial.print(i);
      Serial.print(" ");
    }
    a <<= 1;
    b <<= 1;
  }
}

class EMT250_RAM
{
public:
  void begin();
  void address(uint16_t);
  void write(uint16_t);
  uint16_t read(bool);
  void zeros() { test_pattern(0); }
  void ones() { test_pattern(0xffff); }
  void aaaa() { test_pattern(0xaaaa); }
  void fives() { test_pattern(0x5555); }
  void test_by_chip(uint16_t);

private:
  int address_pin{15};
  int address_enable_pin{14};
  int address_clock_pin{17};
  int clock_pin{16};
  int data_in_pin{2};
  int data_in_enable_pin{3};
  int data_out_pin{10};
  int data_out_enable_pin{9}; // enable=low
  int data_out_clock_pin{18};
  int load_pin{7};
  int write_pin{6};
  void clock(int);
  void serial(int, int, int, uint16_t);
  void test_pattern(uint16_t data);
  bool is_bank_a(uint16_t address);
  int decode_row(uint16_t address);
};

// 1st version of RAM board is divided into 8 rows by 16 bits, returns row number
int EMT250_RAM::decode_row(uint16_t address)
{
  // 74LS138 decodes top 3 bits directly into ~{CE} lines (active low)
  return address >> 10;
}

// 2nd version of board is divided into banks A and B; this checks which bank an address is in
bool EMT250_RAM::is_bank_a(uint16_t address)
{
  const uint16_t bits = address >> 10;
  switch (bits)
  {
  case 0:
    return true;
  case 1:
    return false;
  case 2:
    return false;
  case 3:
    return true;
  case 4:
    return false;
  case 5:
    return true;
  case 6:
    return false;
  case 7:
    return true;
  }
  return false; // this line should not be reached
}

void EMT250_RAM::clock(int pin)
{
  digitalWrite(pin, HIGH);
  delayMicroseconds(50);
  digitalWrite(pin, LOW);
  delayMicroseconds(50);
}

void EMT250_RAM::begin()
{
  pinMode(address_pin, OUTPUT);
  digitalWrite(address_pin, LOW);
  pinMode(address_enable_pin, OUTPUT);
  digitalWrite(address_enable_pin, LOW);
  pinMode(address_clock_pin, OUTPUT);
  digitalWrite(address_clock_pin, LOW);
  pinMode(clock_pin, OUTPUT);
  digitalWrite(clock_pin, LOW);
  pinMode(data_in_pin, OUTPUT);
  digitalWrite(data_in_pin, LOW);
  pinMode(data_in_enable_pin, OUTPUT);
  digitalWrite(data_in_enable_pin, LOW);
  pinMode(data_out_pin, INPUT);
  pinMode(data_out_enable_pin, OUTPUT);
  digitalWrite(data_out_enable_pin, HIGH);
  pinMode(data_out_clock_pin, OUTPUT);
  digitalWrite(data_out_clock_pin, HIGH);
  pinMode(load_pin, OUTPUT);
  digitalWrite(load_pin, HIGH);
  pinMode(write_pin, OUTPUT);
  digitalWrite(write_pin, HIGH);
}

void EMT250_RAM::test_by_chip(uint16_t data)
{
  for (int addr = 0; addr < 8192; ++addr)
  {
    address(addr);
    write(data);
    uint16_t result = read(false);
    uint16_t mask = 1;
    for (int bit = 0; bit < 16; ++bit)
    {
      if ((mask & data) != (mask & result))
      {
        Serial.print("V1: Row ");
        Serial.print(decode_row(addr));
        Serial.print(", V2: Bank ");
        Serial.print(is_bank_a(addr) ? "A" : "B");
        Serial.print(", Bit ");
        Serial.print(bit);
        Serial.print(", address: ");
        Serial.println(addr);
      }
      mask <<= 1;
    }
  }
}

void EMT250_RAM::test_pattern(uint16_t data)
{
  Serial.print("Testing with ");
  Serial.println(data, HEX);

  for (int i = 0; i < 8192; ++i)
  {
    address(i);
    write(data);
  }

  int count = 0;

  for (int i = 0; i < 8192; ++i)
  {
    address(i);
    uint16_t word = read(false);
    if (word != data)
    {
      ++count;
      Serial.print("Addr: ");
      Serial.print(i, HEX);
      Serial.print(" (");
      Serial.print(i, BIN);
      Serial.print("): Expected: ");
      Serial.print(data, HEX);
      Serial.print(" -- Got: ");
      Serial.print(word, HEX);
      Serial.print(" ");
      print_bad_bits(word, data);
      Serial.println();
    }
  }

  Serial.print("Bad count: ");
  Serial.println(count);
}

void EMT250_RAM::address(uint16_t addr)
{
  digitalWrite(address_enable_pin, HIGH);
  delayMicroseconds(20);
  serial(13, address_clock_pin, address_pin, addr);
  digitalWrite(address_pin, LOW);
  digitalWrite(address_enable_pin, LOW);
  delayMicroseconds(20);
}

void EMT250_RAM::write(uint16_t data)
{
  digitalWrite(data_in_enable_pin, HIGH);
  delayMicroseconds(20);
  serial(16, clock_pin, data_in_pin, data);
  digitalWrite(data_in_pin, LOW); // default to low, not necessary, for debugging
  digitalWrite(data_in_enable_pin, LOW);
  delayMicroseconds(20);
  digitalWrite(write_pin, LOW); // worst case 450 ns pulse
  delayMicroseconds(50);
  digitalWrite(write_pin, HIGH);
  delayMicroseconds(50);
}

uint16_t EMT250_RAM::read(bool verbose)
{
  digitalWrite(load_pin, LOW); // LOW=load; 15 ns min. pulse
  delayMicroseconds(20);
  digitalWrite(load_pin, HIGH); // HIGH=shift enabled
  delayMicroseconds(20);
  digitalWrite(data_out_enable_pin, LOW); // LOW=clock enabled
  delayMicroseconds(20);
  uint16_t result = 0;
  if (verbose)
    Serial.print("Reading:\n\t");
  for (int i = 15; i >= 0; --i)
  {
    int bit = digitalRead(data_out_pin);
    if (verbose)
    {
      Serial.print(bit);
      if (i % 4 == 0)
        Serial.print(" ");
    }
    if (bit)
      result |= 1 << i;
    clock(data_out_clock_pin);
  }
  if (verbose)
  {
    Serial.println();
    Serial.println(result, BIN);
  }
  digitalWrite(data_out_enable_pin, HIGH); // clock disabled
  delayMicroseconds(10);
  return result;
}

void EMT250_RAM::serial(int n_bits, int c_pin, int pin, uint16_t data)
{
  uint mask = 1 << (n_bits - 1);
  // Serial.print("n_bits: ");
  // Serial.print(n_bits);
  // Serial.print(" pin: ");
  // Serial.print(pin);
  // Serial.print(" data: ");
  // Serial.print(data, BIN);
  // Serial.print(" mask: ");
  // Serial.println(mask, BIN);
  for (int i = 0; i < n_bits; ++i)
  {
    digitalWrite(pin, (data & mask) ? HIGH : LOW);
    delayMicroseconds(5);
    clock(c_pin);
    data <<= 1;
    // Serial.print("\tdata shifted: ");
    // Serial.println(data, BIN);
  }
}

EMT250_RAM ram;

void setup()
{
  // set pin modes; initialize pins
  ram.begin();

  Serial.begin(9600);
  Serial.setTimeout(-1); // no timeout! please wait indefinitely for input.
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
  Serial.print(">");

  String rawCommand = Serial.readStringUntil('\n').trim();
  // copy it in case we manipulate it
  String command{rawCommand};

  if (command == "test")
  {
    ram.test_by_chip(0);
    ram.test_by_chip(0xffff);
    // ram.zeros();
    // ram.ones();
    // ram.aaaa();
    // ram.fives();
  }
  else if (command.startsWith("addr"))
  {
    command.remove(0, 4).trim();
    int addr = command.toInt();
    if (addr >= 0 && addr < 8192)
    {
      ram.address(addr);
      Serial.print(addr);
      Serial.println(" - address set");
    }
    else
      Serial.println("Usage: addr n, where n is between 0 and 8191");
  }
  else if (command.startsWith("write"))
  {
    command.remove(0, 5).trim();
    int data = command.toInt();
    if (data >= 0 && data < 0x10000)
    {
      ram.write(data);
      Serial.print(data);
      Serial.println(" data written");
    }
    else
      Serial.println("Usage: addr n, where n is between 0 and 0xffff");
  }
  else if (command.startsWith("read"))
  {
    bool verbose = command.length() > 4;
    uint16_t result = ram.read(verbose);
    Serial.print(result, BIN);
    Serial.print(" ");
    Serial.print(result, HEX);
    Serial.print(" ");
    Serial.println(result, DEC);
  }
  else
  {
    Serial.print("Sorry, I don't know '");
    Serial.print(command);
    Serial.println("'");
  }
}