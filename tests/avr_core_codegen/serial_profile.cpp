#include <HardwareSerial.h>
#ifndef EXPECT_RX
#error EXPECT_RX required
#endif
#ifndef EXPECT_TX
#error EXPECT_TX required
#endif
static_assert(SERIAL_RX_BUFFER_SIZE == EXPECT_RX, "RX profile mismatch");
static_assert(SERIAL_TX_BUFFER_SIZE == EXPECT_TX, "TX profile mismatch");
uint16_t serial_profile_size_probe(){ return (uint16_t)sizeof(HardwareSerial); }
