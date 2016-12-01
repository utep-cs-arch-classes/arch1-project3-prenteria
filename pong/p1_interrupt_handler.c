#include <msp430.h>
#include "pongDemo.c"

/* Switch on P1 (S2) */
__interrupt(PORT1_VECTOR) Port_1(){
  
  if (P1IFG & SWITCHES) {	      /* did a button cause this interrupt? */
    P1IFG &= ~SWITCHES;		      /* clear pending sw interrupts */
    wdt_c_handler();	
  }
}

