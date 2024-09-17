/*
*************************	PINS LAYOUT  **********************
*************	 HC-SR05: Trig-> PF4, Echo -> PB4 *************
*************	 UART: Tx-> A1 , Rx-> A0          *************
*************  Motor: DATA-> PF0, PF1, PF2, PF3 *************
*************************************************************
*/
#include <stdio.h>
//Register definitions for UART Clock Enable
#define 		SYSCTLRCGCUARTR			(*((volatile unsigned long*)0x400FE618))

/* GPIO Clock*/
#define		SYSCTL_RCGCGPIO_R			(*((volatile unsigned long*)0x400FE608))

/* Timer Clock*/
#define		SYSCTL_RCGCTIMER_R	(*((volatile unsigned long*)0x400FE604))

/* GPIO registers PortF for Pulse*/
#define		GPIO_PORTF_DATA_R			(*((volatile unsigned long*)0x400253FC))
#define		GPIO_PORTF_DIR_R			(*((volatile unsigned long*)0x40025400))
#define		GPIO_PORTF_DEN_R			(*((volatile unsigned long*)0x4002551C))

/* GPIO registers PortC for Input Capture*/
#define		GPIO_PORTB_DATA_R			(*((volatile unsigned long*)0x400053FC))
#define		GPIO_PORTB_DIR_R			(*((volatile unsigned long*)0x40005400))
#define		GPIO_PORTB_DEN_R			(*((volatile unsigned long*)0x4000551C))
#define		GPIO_PORTB_AFSEL_R	  (*((volatile unsigned long*)0x40005420))
#define		GPIO_PORTB_PCTL_R			(*((volatile unsigned long*)0x4000552C))

/* Delay Timer registers T0CCP0*/
#define		TIMER0_CTL_R				(*((volatile unsigned long*)0x4003000C))
#define		TIMER0_CFG_R				(*((volatile unsigned long*)0x40030000))
#define		TIMER0_TAMR_R				(*((volatile unsigned long*)0x40030004))
#define		TIMER0_TAILR_R			(*((volatile unsigned long*)0x40030028))
#define		TIMER0_ICR_R				(*((volatile unsigned long*)0x40030024))
#define		TIMER0_TA_R				  (*((volatile unsigned long*)0x40030048))

/* Input Capture Timer registers T1CCP0 @ Pin PB4*/
#define		TIMER1_CTL_R				(*((volatile unsigned long*)0x4003100C))
#define		TIMER1_CFG_R				(*((volatile unsigned long*)0x40031000))
#define	  TIMER1_TAMR_R			  (*((volatile unsigned long*)0x40031004))
#define		TIMER1_TAILR_R			(*((volatile unsigned long*)0x40031028))
#define		TIMER1_ICR_R				(*((volatile unsigned long*)0x40031024))
#define		TIMER1_TA_R				  (*((volatile unsigned long*)0x40031048))
#define		TIMER1_TAV_R				(*((volatile unsigned long*)0x40031050))
#define		TIMER1_RIS_R				(*((volatile unsigned long*)0x4003101C))
#define		TIMER1_PR_R				  (*((volatile unsigned long*)0x40031038))

//Register definitions for UART0 module
#define 	UART0CTLR						(*((volatile unsigned long*)0x4000C030))
#define  	UART0IBRDR					(*((volatile unsigned long*)0x4000C024))
#define  	UART0FBRDR					(*((volatile unsigned long*)0x4000C028))
#define 	UART0LCRHR					(*((volatile unsigned long*)0x4000C02C))
#define 	UART0CCR						(*((volatile unsigned long*)0x4000CFC8))
#define 	UART0FRR						(*((volatile unsigned long*)0x4000C018))
#define 	UART0DRR						(*((volatile unsigned long*)0x4000C000))

//Register definitions for GPIOPortA
#define 	GPIOPORTAAFSELR			(*((volatile unsigned long*)0x40004420))
#define 	GPIOPORTAPCTLR			(*((volatile unsigned long*)0x4000452C))
#define 	GPIOPORTADENR				(*((volatile unsigned long*)0x4000451C))
#define 	GPIOPORTADIRR				(*((volatile unsigned long*)0x40004400))
#define 	GPIOPORTAAMSELR			(*((volatile unsigned long*)0x40004528))

#define UART_FR_TXFF            0x00000020  // UART Transmit FIFO Full
#define UART_FR_RXFE            0x00000010  // UART Receive FIFO Empty
#define UART_LCRH_WLEN_8        0x00000060  // 8 bit word length
#define UART_LCRH_FEN           0x00000010  // UART Enable FIFOs
#define UART_CTL_UARTEN         0x00000001  // UART Enable
#define CR											13          // Carriage Return Move to the Start of the line
#define LF											10					// Line Feed moves the cursor to the next line

#define Directions  						4



void Delay_us (void);
void Delay_Timer_Init (void);
void GPIOF_Init(void);
void InputCapture_Timer_Init (void);
void GPIOB_Init(void);
void UART_OutString(char *pt);
void UART_OutChar(char data);
void OutCRLF(void);
void UART_Init(void);
void rotate(void);
void Delay_ms();



int main (void)
{
    int Rising, Falling, Diff, Width,i, Objects, Check;
		char str [20];
		int Distances[Directions];
    char ch;

    // Initializing GPIO, TIMER0A, WTIMERA0
    Delay_Timer_Init ();
    GPIOF_Init ();
    GPIOB_Init ();
    InputCapture_Timer_Init ();
    UART_Init();

    while(1)
    {
        Objects = 0;
        for (i = 0; i<Directions; i++)
        {
            // Generating a Pulse of 10us Approx.
            GPIO_PORTF_DATA_R &= 0x00;
            Delay_us();
            GPIO_PORTF_DATA_R |= 0x10;
            Delay_us();
            GPIO_PORTF_DATA_R &= 0x00;

            // Initalizes the timer to starting value
            TIMER1_TAILR_R |= 700;
            TIMER1_TAMR_R  &= 0xFFFFFEFF;

            // Clearing Any Interrupt
            TIMER1_ICR_R |= 0x00000004 ;

            // Waiting for Pulse to get Captured
            while ((TIMER1_RIS_R & (0x04)) == 0)
            {
                if ((TIMER1_TAV_R & 0x0000FFFF) <= 60)
                {
                    Check = 1;
										break;
                }
            }

            Check = 1;
            // Checking if it was Rising Edge
            if (GPIO_PORTB_DATA_R & 0x00000010)
            {
                Rising = 	TIMER1_TA_R & 0x0000FFFF;
                Check = 0;

            }

            // If the timer hasn't timed out enter here
            if (Check == 0)
            {
                // Clearing Previous Interrupt
                TIMER1_ICR_R |= 0x00000004 ;

                // Waiting for Pulse to get Captured
                while ((TIMER1_RIS_R & (0x04)) == 0)
                {
                    if ((TIMER1_TAV_R & 0x0000FFFF) <= 60)
                    {
                        Check = 1;
												break;
                    }
                }

                // If the Timer Hasn't timed out enter here
                if (Check == 0)
                {
                    Falling = TIMER1_TA_R & 0x0000FFFF;
                    Diff = Rising - Falling;
                    Width = (int) ((Diff * 343) / 32000 ); // S = (V * t) / 2

                    if (Width <= 405)
                    {
                        Distances[Objects] = Width;
											  Objects +=1;
                    }
                }

            }
            else
            {
                OutCRLF();
                UART_OutString("Nothing");
            }
            Check = 0;
            rotate();
        }
				
				OutCRLF();
        UART_OutString("Total Objects: ");
				sprintf(str,"%d",Objects);
        UART_OutString(str);
				
				// Sending the aquired data to Hyperterminal
				for(i = 0; i < Objects ; i++)
				{
						OutCRLF();
						UART_OutString("Distance ");
						sprintf(str,"%d",i+1);
						UART_OutString(str);
						sprintf(str,": %dcm",Distances[i]);
						UART_OutString(str);
				}				
			
    }

}

void Delay_us (void)
{
    TIMER0_TAILR_R |= 200; // 16 MHz * (10 us) = 160	-> 0xA0
	
		TIMER0_ICR_R |= 0x00000001 ;

    /* Enable Timer_0A */
    TIMER0_CTL_R |= 0x00000001;
    /* Checking Value */
    while ( (TIMER0_TA_R ) >= 40 )
    {

    }
}

void Delay_Timer_Init (void)
{
    SYSCTL_RCGCTIMER_R |= 0x00000001; // Timer T0CCP0

    /* Disable Timer0 before setup */
    TIMER0_CTL_R &= 0x00000000;
    /* Configure 16-bit timer mode */
    TIMER0_CFG_R |= 0x00000004;
    /* Configure one shot mode */
    TIMER0_TAMR_R |= 0x00000001;
    /* Set initial load value */
    TIMER0_TAILR_R |= 320; // 16 MHz * (10 us) = 160	-> 0xA0

    /* Clear timeout interrupt */
    TIMER0_ICR_R |= 0x00000001 ;
}


void GPIOF_Init()
{
    unsigned long int dummy;
    /* Enable clock for PortF
    * enable digital IO for PF0
    * and set its direction */
    SYSCTL_RCGCGPIO_R |= 0x00000020;

    dummy = SYSCTL_RCGCGPIO_R;

    GPIO_PORTF_DIR_R |= 0x1F;
    GPIO_PORTF_DEN_R |= 0x1F;
}

void rotate(void)
{
    int i;
    for(i=0; i<208; i++) // take 208 steps to complete 90 revolution
    {
        // Wave Drive Mode
        GPIO_PORTF_DATA_R = 0x08;
        Delay_ms();
        GPIO_PORTF_DATA_R = 0x04;
        Delay_ms();
        GPIO_PORTF_DATA_R = 0x02;
        Delay_ms();
        GPIO_PORTF_DATA_R = 0x01;
        Delay_ms();
    }

}

void Delay_ms()
{		
		TIMER0_TAILR_R |= 64040; // 16 MHz * (4 ms) = 64000
					
		TIMER0_ICR_R |= 0x00000001 ;

    /* Enable Timer_0A */
    TIMER0_CTL_R |= 0x00000001;
    /* Checking Value */
    while ( (TIMER0_TA_R ) >= 40 )
    {

    }
}

void InputCapture_Timer_Init (void)
{
    SYSCTL_RCGCTIMER_R |= 0x00000002; // Timer T1CCP0

    /* Disable Timer1 before setup */
    TIMER1_CTL_R &= ~ 0x00000001;

    /* Configure 16-bit timer mode */
    TIMER1_CFG_R |= 0x00000004;

    /* Configure Input Capture & Capture Time*/
    TIMER1_TAMR_R |= 0x00000007;

    /* Pre-Scaler Value */
    TIMER1_PR_R = 999; // ( (16e^6) / (16e^3) )-1

    /* Set initial load value */
    TIMER1_TAILR_R |= 700; // 16 KHz * (40 ms) = 640	-> 0x280

    /* Capture Rising and Falling Edges */
    TIMER1_CTL_R &= ~ 0x0000000C;

    /* Clear timeout interrupt @ CAMRIS*/
    TIMER1_ICR_R |= 0x00000004 ;

    /* Enabling Timer0 before setup */
    TIMER1_CTL_R |=  0x00000001;
}

void GPIOB_Init()
{
    unsigned long int dummy;
    /* Enable clock for PortB
    * enable digital IO for PB4
    * and set its direction */
    SYSCTL_RCGCGPIO_R  |= 0x00000002;

    dummy = SYSCTL_RCGCGPIO_R;

    GPIO_PORTB_DIR_R   &= ~0x10;
    GPIO_PORTB_DEN_R   |=  0x10;
    GPIO_PORTB_AFSEL_R |=  0x10;
    GPIO_PORTB_PCTL_R  &= ~0x000F0000;
    GPIO_PORTB_PCTL_R  |=  0x00070000;
}

void UART_Init(void)
{
    SYSCTLRCGCUARTR |= 0x00000001;            		   // activate UART0
    SYSCTL_RCGCGPIO_R |= 0x00000001;            		 // activate port A

    UART0CTLR &= ~UART_CTL_UARTEN;      						 // disable UART
    UART0IBRDR = 8;                    							 // IBRD = int(16,000,000 / (16 * 115,200)) = int(8.6805)
    UART0FBRDR = 44;                     						 // FBRD = int(0.6805 * 64 + 0.5) = 44
    UART0LCRHR = (UART_LCRH_WLEN_8|UART_LCRH_FEN);   // 8 bit word length (no parity bits, one stop bit, FIFOs)
    UART0CTLR |= UART_CTL_UARTEN;       						 // enable UART
    GPIOPORTAAFSELR |= 0x00000003;           				 // enable alt funct on PA1-0
    GPIOPORTADENR   |= 0x00000003;             			 // enable digital I/O on PA1-0
    
    GPIOPORTAPCTLR = (GPIOPORTAPCTLR & 0xFFFFFF00)+0x00000011;
    GPIOPORTAAMSELR &= ~0x00000003;          				 // disable analog functionality on PA
}


void UART_OutChar(char data)
{
    while((UART0FRR & UART_FR_TXFF) != 0);
    UART0DRR = data;
}


void UART_OutString(char *pt)
{
    while(*pt)
    {
        UART_OutChar(*pt);
        pt++;
    }
}


void OutCRLF(void)
{
    UART_OutChar(CR);
    UART_OutChar(LF);
}
