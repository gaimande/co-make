/***********************************************************************
 *             TI Controller for sBox
 *        (c) May 2015 gaimande@gmail.com
 ***********************************************************************/

#include <msp430g2553.h>
#include "uart_simple.h"
#include <string.h>

void FaultRoutine(void);
void ConfigWDT(void);
void ConfigClocks(void);
void ConfigIOs(void);

void main(void)
{
    ConfigWDT();
    ConfigClocks();
    ConfigIOs();
    ConfigUART();               
        
    //_BIS_SR(LPM4_bits + GIE);                      // Go in standby mode to minimum consumption
    while (1);
}

void ConfigWDT(void)
{
    WDTCTL = WDTPW + WDTHOLD;                      // Disable Watchdog
}

void FaultRoutine(void)
{
    P1OUT = BIT0;                                  // P1.0 on (red LED)
    while(1);                                      // TRAP
}

void ConfigClocks(void)
{
    if (CALBC1_1MHZ ==0xFF ||
        CALDCO_1MHZ == 0xFF)
    {
        FaultRoutine();
    }
    
    BCSCTL1 = CALBC1_1MHZ;                         // Set range
    DCOCTL = CALDCO_1MHZ;                          // Set DCO step + modulation
    BCSCTL3 |= LFXT1S_2;                           // LFXT1 = VLO
    IFG1 &= ~OFIFG;                                // Clear OSCFault flag
    BCSCTL2 |= SELM_0 + DIVM_3 + DIVS_0;           // MCLK = DCO/8, SMCLK = DCO/1
}

void ConfigIOs(void)
{
    P1DIR = 0xFF & ~(BIT3);                               // Set P1.3 as inputs, other outputs
    P1OUT &= ~BIT0;
    P1REN |= BIT3;                                 // Pull-up resistor enable
    P1IE |= BIT3;                                  // Enable port interrupt
    P1IES &= ~BIT3;                                // Low to High transition
    P1IFG &= ~BIT3;    
        
    __enable_interrupt();                          // enable all interrupts
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	static int i = 0;
    static char buf[32];
	
    P1OUT ^= BIT0;                                 // Toggle LED
    
    if (13 != UCA0RXBUF)
    {
        buf[i] = UCA0RXBUF;
        i++;
        return;
    }
    
    buf[i] = 10;    
    i = 0;
    
    Print_UART(buf);    
    memset(buf, 0, sizeof(buf));
}

#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{    
    P1OUT ^= BIT0;                                 // Toggle LED
    Print_UART ("make\n");
        
    __delay_cycles(40000);
    P1IFG &= ~BIT3;                                // Clear interrupt Flag for next interrupt
}

