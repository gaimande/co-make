#include <msp430g2553.h>

void Send_Char(char chr)
{
	while (!(IFG2&UCA0TXIFG));      	// USCI_A0 TX buffer ready?
  	UCA0TXBUF = chr;            		// TX -> RXed character
	while(UCA0STAT & UCBUSY);			// Wait until the last byte is completely sent
}

void Print_UART(char *ch)
{
	unsigned char i = 0;
	while(ch[i] != '\0')
	{
		Send_Char(ch[i]);
		i++;
	}
}

void ConfigUART(void)
{ 
	  P1SEL |= BIT1 + BIT2 ;                  	// P1.1 = RXD, P1.2=TXD
	  P1SEL2 |= BIT1 + BIT2 ;                  	// P1.1 = RXD, P1.2=TXD
	
	  UCA0CTL1 |= UCSSEL_2;                     // SMCLK
	  UCA0BR0 = 104;                        	// value = SMCLK / 9600
	  UCA0BR1 = 0;                              // value = SMCLK / 9600
	  UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
	  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	  IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
}