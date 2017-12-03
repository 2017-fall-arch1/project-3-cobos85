#include <msp430.h>
#include <libTimer.h>
#include "noise.h"


void set_pd(short cycles)
{
  CCR0 = cycles;
  CCR1 = cycles >> 1;
}

void init_noise()
{
  timerAUpmode();
  P2SEL &= ~(BIT6 | BIT7);
  P2SEL &= ~BIT7;
  P2SEL |= BIT6;
  P2DIR = BIT6;
}
