/***********************************************************************
 *             TI Controller for sBox
 *        (c) May 2015 gaimande@gmail.com
 ***********************************************************************/

#include <string.h>
#include <msp430g2553.h>
#include "uart_simple.h"

#define CHAR_NEWLINE          10
#define CHAR_CARR_RETURN      13
#define STR_RUNNING           "ACK200"
#define STR_ERROR             "ERROR"
#define STR_SUCCESS           "SUCCESS"
#define STR_FIN_STAGE         "FINISH_STAGE"
#define STR_MAKE              "make"
#define IS_MATCH              0
#define NORMAL_SPEED          6000
#define FAST_SPEED            NORMAL_SPEED/4

#define TURN_ON_ERR_LED()     P1OUT |= BIT0
#define TURN_OFF_ERR_LED()    P1OUT &= ~ BIT0

#define TURN_ON_OK_LED()      P1OUT |= BIT6
#define TURN_OFF_OK_LED()     P1OUT &= ~ BIT6
#define TOGGLE_OK_LED()       P1OUT ^= BIT6;

#ifndef TRUE
#define TRUE                  1
#define FALSE                 0
#endif

void FaultRoutine(void);
void ClearWDT(void);
void StopWDT(void);
void ConfigClocks(void);
void ConfigIOs(void);
void ConfigTimerA2(void);

int is_running = FALSE;
int blink_speed = NORMAL_SPEED;

void main(void)
{
    ClearWDT();    
    ConfigClocks();
    ConfigIOs();
    ConfigTimerA2();
    ConfigUART();               
        
    __delay_cycles(100);
    Print_UART ("Welcome to sBox!\n");
    TURN_ON_OK_LED();

    while (1)
    {
        if (FALSE == is_running)
        {
            StopWDT();  
            _BIS_SR(LPM4_bits + GIE);              // If in standby mode, choose LPM4
                                                   // to minimum consumption
        }
        else /* Running */
        {
            _BIS_SR(LPM3_bits + GIE);		       // In process mode, choose LPM3 to
                                                   // enable ACLK clock for timer
        }
    }
}

void ClearWDT(void)
{
    WDTCTL = WDTPW + WDTCNTCL + WDTSSEL;           // Clear watchdog timer
                                                   // Clock Source from VLO in this project
                                                   // so WDT will reset MCU after 2^16 * 1/12000 ~ 5s
}

void StopWDT(void)
{
    WDTCTL = WDTPW + WDTHOLD;                      // Stop watchdog timer
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
    P1DIR = 0xFF & ~(BIT3);                        // Set P1.3 as inputs, other outputs
    P1OUT &= ~(BIT0 + BIT6);
    
    P1REN |= BIT3;                                 // Pull-up resistor enable
    P1IE |= BIT3;                                  // Enable port interrupt
    P1IES &= ~BIT3;                                // Low to High transition
    P1IFG &= ~BIT3;    
        
    __enable_interrupt();                          // enable all interrupts
}

void ConfigTimerA2(void)
{
    CCTL0 &= ~CCIE;                                // Disable capture/compare mode   
    TACTL = TASSEL_1 + MC_1;                       // Change timer to 500ms period with CCR0 = 12000 x (time in second)
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
    CCR0 = blink_speed;
    TOGGLE_OK_LED();
    
    ClearWDT();   
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    static int i = 0;
    static char buf[32];
    
    if (CHAR_CARR_RETURN != UCA0RXBUF)
    {
        buf[i] = UCA0RXBUF;
        i++;
        return;
    }
    
    buf[i] = CHAR_NEWLINE;
    i = 0;
    
    if (IS_MATCH == strncmp (buf, STR_RUNNING, strlen(STR_RUNNING)))
    {
        /* Support make command by user input */
        _BIC_SR_IRQ(LPM4_bits);                    // LPM off
        is_running = TRUE;
        CCTL0 |= CCIE;                             // Enable capture/compare mode
        CCR0 = blink_speed = NORMAL_SPEED;
        TURN_OFF_ERR_LED();        
    }
    else if (IS_MATCH == strncmp (buf, STR_ERROR, strlen(STR_ERROR)))
    {
        is_running = FALSE;
        CCTL0 &= ~CCIE;
        TURN_ON_ERR_LED();
        TURN_OFF_OK_LED();
        _BIC_SR_IRQ(LPM3_bits);					   // LPM off
    }
    else if (IS_MATCH == strncmp (buf, STR_SUCCESS, strlen(STR_SUCCESS)))
    {
        is_running = FALSE;
        CCTL0 &= ~CCIE;
        TURN_ON_OK_LED();
        TURN_OFF_ERR_LED();
        _BIC_SR_IRQ(LPM3_bits);					   // LPM off
    }
    else if (IS_MATCH == strncmp (buf, STR_FIN_STAGE, strlen(STR_FIN_STAGE)))
    {
        CCR0 = blink_speed = FAST_SPEED;
    }
    
    Print_UART(buf);    
    memset(buf, 0, sizeof(buf));
}

#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{    
    if (FALSE == is_running)
    {
        is_running = TRUE;
        Print_UART ("make\n");
        _BIC_SR_IRQ(LPM4_bits);                    // LPM off
        TURN_OFF_OK_LED();
        ClearWDT();
    }
        
    //__delay_cycles(10000);
    P1IFG &= ~BIT3;                                // Reset interrupt flag for next time
}

