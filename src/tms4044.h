#ifndef RAM_TESTER_TMS4044
#define RAM_TESTER_TMS4044
/**
 * TI TMS4044 RAM
 * 12-bit address, 1 bit data
 * Fully static
 *       ___ ___
 *   A0 |1  U 18| Vcc
 *   A1 |       | A6
 *   A2 |       | A7
 *   A3 |       | A8
 *   A4 |       | A9
 *   A5 |       | A10
 *    Q |       | A11
 *   !W |       | D
 *  Vss |9    10| !S
 *       -------
 * !W = write: 0 = write, 1 = read
 * !S = select: 0 = enabled, 1 = disabled
 * D = data in
 * Q = data out
 * A0-11 = address
 * Vcc = +5V
 * Vss = GND
 */

/**
 * Timing info      (nanoseconds)
 * TMS4044-XX:  Read max  Write min
 * 12           120       120
 * 20           200       200
 * 25           250       250
 * 40           450       450
 */
const unsigned int MAX_DELAY_NS = 500;

void set_read_mode();
void set_write_mode();
void set_address(int address);
bool read(int address);
void write(int address, bool value);
#endif