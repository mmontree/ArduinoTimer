#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "Timer5.h"

uint8_t __timer5Control;
uint16_t __timer5CounterValue;
// 0 < microsecondsInterval <= 1000000
// On 16 MHz Arduino boards, this function has a resolution of 4us, for intervals <= 260000, and a resolution of 16us for other intervals.
// On 8 MHz Arduino boards, this function has a resolution of 8us, for intervals <= 520000, and a resolution of 32us for other intervals.
void startTimer5(uint32_t microsecondsInterval) {
  pauseTimer5();
  // 18. Timer/Counter 0, 1, 3, 4, and 5 Prescaler (page 169)
  // 17.9.1 Normal Mode (page 149)
  TCCR5A = 0;
  TCCR5C = 0;
  // 17.11.5 TCCR5B (page 160)
  // 0 0 0 No clock source (Timer/Counter stopped)
  // 0 0 1 clkIO/1 (No prescaling)
  // 0 1 0 clkIO/8 (From prescaler)
  // 0 1 1 clkIO/64 (From prescaler)
  // 1 0 0 clkIO/256 (From prescaler)
  // 1 0 1 clkIO/1024 (From prescaler)
#if (F_CPU == 16000000L)
  if (microsecondsInterval <= 260000) {
    __timer5Control = B00000011;
    // The proper way of doing this would be:
    // 65536 - (microsecondsInterval / 4)
    // But, in order to save one 32-bit operation, this "- 1" is necessary...
    __timer5CounterValue = 65535 - ((uint16_t)(microsecondsInterval >> 2) - 1);
  } else {
    __timer5Control = B00000100;
    __timer5CounterValue = 65535 - ((uint16_t)(microsecondsInterval >> 4) - 1);
  }
#elif (F_CPU == 8000000L)
  if (microsecondsInterval <= 520000) {
    __timer5Control = B00000011;
    __timer5CounterValue = 65535 - ((uint16_t)(microsecondsInterval >> 3) - 1);
  } else {
    __timer5Control = B00000100;
    __timer5CounterValue = 65535 - ((uint16_t)(microsecondsInterval >> 5) - 1);
  }
#else
  #error("Unsupported CPU frequency")
#endif
  resetTimer5();
  // 17.11.37 TIFR1 � Timer/Counter5 Interrupt Flag Register (page 167)
  TIFR5 = 0;
  TIMSK5 = 1;
  resumeTimer5();
}
uint16_t readTimer5(void) {
  // 17.3 Accessing 16-bit Registers (page 138)
  uint8_t sreg;
  uint16_t i;
  // Save global interrupt flag
  // 7.4.1 SREG � AVR Status Register (page 14)
  sreg = SREG;
  // Disable interrupts
  cli();
  // Read TCNTn
  i = TCNT5;
  // Restore global interrupt flag
  SREG = sreg;
  return i;
}