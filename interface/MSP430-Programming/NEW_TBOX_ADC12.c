/******************************************************************************
 *  TEABOX: 24 input (8 analog), 12-bit Sensor Interface
 *	2004.05.05
 *
 *  Code by Tim Place & Jesse Allison
 *  Copyright © 2004 Electrotap
 *  All Rights Reserved.
 *
 *  Built with IAR KickStart
 ******************************************************************************/

#include  "msp430x14x.h"
#define TEABOX_VERSION_MAJOR 0x01;
#define TEABOX_VERSION_MINOR 0x00;

// ********* GLOBALS **********
short adc1[11], adc2[11], adc3[11];
char low_byte[12], high_byte[12]; // Store the bytes from the ADC Conversion, 8 channels
                                  // byte 0 is the start flag
                                  // byte 1 is the hardware version number                                  
                                  // bytes 2-9 are the ADC input pins
                                  // byte 10 is the GPIO (digital) inputs from ports 4 & 5
                                  // byte 11 is the internal temperature sensor                                  

char word_out_counter = 0;        // Store the number of the converter, whose data should be sent next
unsigned int i;                   // General purpose counter
unsigned int led_counter = 0;
char sr_switch_state = 0;         // The state of the sample-rate switch
char adc_toggle = 0;

// ********* PROTOTYPES **********
void teabox_gpioconfig(void);
void teabox_clockconfig(void);
void teabox_adcconfig(void);
int teabox_spiconfig(void);



/******************************************************************************/
// ********* MAIN() **********
void main(void)
{ 
  WDTCTL = WDTPW + WDTHOLD;   // STOP THE WATCH DOG TIMER
  
  teabox_gpioconfig();  
  teabox_clockconfig();
  teabox_adcconfig();
  if(teabox_spiconfig() == -1){       // FAILED!!!! RESTART!!!!
    WDTCTL = WDTPW + WDTSSEL;
    while(1);        
  }	
 
    
  P1OUT |= 0x10;              // Set Pin 1.4 HIGH to start the CS8405A
  _EINT();                    // Enable interrupts

  WDTCTL = WDTPW + WDTSSEL;   // TURN THE WATCHDOG COUNTER BACK ON

  /****************************************************
   * Main Loop
   * This loop copies samples from the ADC's output register continuously.
   * It is interrupted for SPI transmission by the flag set by Timer_A.
   ****************************************************/
  
  // Init the loop end flags
  low_byte[0] = high_byte[0] =0xFF;
  
  // Init the version number flags
  high_byte[1] = TEABOX_VERSION_MAJOR;
  low_byte[1] = TEABOX_VERSION_MINOR;

    
  // Copy the analog conversion data into globals...
  while(1){ 
    // Sensor Inputs 
    low_byte[2] = ADC12MEM0 & 255;    high_byte[2] = ADC12MEM0 >> 8;
    low_byte[3] = ADC12MEM1 & 255;    high_byte[3] = ADC12MEM1 >> 8;     
    low_byte[4] = ADC12MEM2 & 255;    high_byte[4] = ADC12MEM2 >> 8;     
    low_byte[5] = ADC12MEM3 & 255;    high_byte[5] = ADC12MEM3 >> 8;     
    low_byte[6] = ADC12MEM4 & 255;    high_byte[6] = ADC12MEM4 >> 8;     
    low_byte[7] = ADC12MEM5 & 255;    high_byte[7] = ADC12MEM5 >> 8;     
    low_byte[8] = ADC12MEM6 & 255;    high_byte[8] = ADC12MEM6 >> 8;     
    low_byte[9] = ADC12MEM7 & 255;    high_byte[9] = ADC12MEM7 >> 8;
   // low_byte[10] = P4IN;              high_byte[10] = P5IN;
    // Internal Temperature Sensor...     
    //low_byte[11] = ADC12MEM8 & 255;  high_byte[11] = ADC12MEM8 >> 8;

     // TOGGLE THE STATUS LED
    led_counter++;
    if(led_counter == 256){
      P1OUT ^= 0x01;		// invert pin 1.0
      led_counter = 0;
    }
   
    WDTCTL = WDTPW + WDTCNTCL + WDTSSEL;      // clear the WDT counter
   }  
}





// ********* SPI INTERRUPT SERVICE ROUTINE **********
interrupt[TIMERA0_VECTOR] void spi_tx(void)
{
  //RIGHT CHANNEL ***********************************
  // while((IFG1 & UTXIFG0)==0);   // USART0 TX buffer ready? - Don't seem to need the first time
  TXBUF0 = 0x00;
  
  while((IFG1 & UTXIFG0)==0);   // USART0 TX buffer ready?
  TXBUF0 = 0x00;

//  for(i=0; i<1; i++)    // DELAY
//   _NOP();

  low_byte[10] = P4IN;  high_byte[10] = P5IN;
  //low_byte[10] = 0xAB; high_byte[10] = 0xCD;
  _NOP();  _NOP();  _NOP();  _NOP();  _NOP();  _NOP();  _NOP();  _NOP();  _NOP();  _NOP();

  // LEFT CHANNEL ***********************************
  while((IFG1 & UTXIFG0)==0);   // USART0 TX buffer ready?
  TXBUF0 = high_byte[word_out_counter];

  while((IFG1 & UTXIFG0)==0);   // USART0 TX buffer ready?
  TXBUF0 = low_byte[word_out_counter];

  word_out_counter++;       // increment the word counter
  if(word_out_counter > 10)  // in tests, this seems to work better than modulo
    word_out_counter = 0;
    
  
  // !!!!!!!!!!!!!!! MOVE THE PORT4/5 INPUTS DOWN TO HERE !!!!!!
   //     low_byte[10] = P4IN;              high_byte[10] = P5IN;
}




// ********* GPIO PIN CONFIGURATION ROUTINE **********
void teabox_gpioconfig(void)
{
    // SETUP OUR PINS FOR THE LED INDICATOR AND THE EXTERNAL OSCILLATOR SWITCH
    P1DIR |= 0xF3;      // port 1, all ports are output except pin2(switch_in), pin3(nc) 
    P1OUT = 0x00;	// set all output pins to LOW in port 1

    P2DIR = 0xEB;       // Init port 2, all pins OUTPUT except pins 2 and 4 (no-connect)
    P2OUT = 0x00;       // set all output pins to LOW in port 2
                         // ... additional setup is done in SPI Config
                         
    P3DIR = 0x6A;       // Init port 3, all pins OUTPUT except pins 0, 2, 4, and 7 (nc)
    P3OUT = 0x00;       // set all output pins to LOW in port 3
                        // ... additional setup is done in SPI Config
    
      P4DIR = 0x00;     // Init port 4 pins to INPUT
      //P4OUT = 0x00;     //  Set any output pins on port 4 to low (may not need this)
      P5DIR = 0x00;     // Init port 5 pins to INPUT
      //P5OUT = 0x00;     //  Set any output pins on port 5 to low (may not need this)
      //P5SEL = 0x00;
                        // ... Port 6 is handled in the ADC config
}



// ********* EXTERNAL CLOCK CONFIGURATION ROUTINE **********
void teabox_clockconfig(void)
{
  // pin 1.1 controls the switch
  // pin 1.2 is input from the electrical switch
/*
    sr_switch_state = (P1IN ^ 0x04);

    if(sr_switch_state)// If pin 1.2 is high... (LOW)
      P1OUT |= 0x02;   // Set the pin 1.1 high (48KHz)
                       // because it is initialized low, we don't need an else clause here.  
*/
   // DELAY TO ALLOW THE CRYSTAL OSCILLATORS TO POWER UP  
    for(i=0; i<0xFFFF; i++)
      _NOP();
    for(i=0; i<0xFFFF; i++)
      _NOP(); 
    for(i=0; i<0xFFFF; i++)
      _NOP(); 
}




// ********* INTERNAL ADC CONFIGURATION ROUTINE **********
void teabox_adcconfig(void)
{
  P6SEL |= 0xFF;                        // Enable A/D channels A0-A7
//  ADC12CTL0 = ADC12ON + MSC + SHT0_12/* + SHT1_4*/;   // MSC: Multiple Sample and Conversion
  ADC12CTL0 = ADC12ON + MSC + SHT0_4/* + SHT1_4*/;   // MSC: Multiple Sample and Conversion
                                        // SHT0_2: sample-and-hold time - sample-timer divisor: 16 clock cycles
                                        // SHT0_3: 32 cycles, SHT0_4: 64 cycles, etc.  
  ADC12CTL1 = CONSEQ_3                  // conseq_3: repeated conversion of sequence of channels
            + SHP                       //  source hold select
            + ADC12SSEL_2;              //  Sets the ADC clock to the master clock                 
  ADC12MCTL0 = INCH_0;
  ADC12MCTL1 = INCH_1;
  ADC12MCTL2 = INCH_2;
  ADC12MCTL3 = INCH_3;
  ADC12MCTL4 = INCH_4;
  ADC12MCTL5 = INCH_5;
  ADC12MCTL6 = INCH_6;
  ADC12MCTL7 = INCH_7; 
  ADC12MCTL8 = INCH_10 + EOS;
  
  ADC12CTL0 |= ADC12SC + ENC;       // Start Conversions + Enable Conversions
  
}





// ********* SPI CONFIGURATION ROUTINE **********
int teabox_spiconfig(void)
{
  int test = 0;
  
  BCSCTL1 |= XTS;                       // BasicClockSystem Control Register
                                        //    ACLK = LFXT1 = HF XTAL
  //  BCSCTL1 |= DIVA_3;                    // [JTA] Divide ACLK frequency by 8
  
  do{
    IFG1 &= ~OFIFG;                     // Clears OSC Fault Flag
    for(i = 0xFF; i > 0; i--)           // Burn some cycles to give flag time to set
      _NOP();
      
    test++;
    if(test > 0xFF)
      return(-1);      

  }while((IFG1 & OFIFG) != 0);          // OSCFault Flag Still Set?                                      
                                        
  BCSCTL2 |= SELM1+SELM0;               // MCLK = LFXT1 (safe) 
  TACTL = TASSEL0 + TACLR;              // Timer Clock = ACLK, TAR Cleared

  CCTL1 = OUTMOD_2+OUTMOD_1+OUTMOD_0;      // () CCR1 Reset/Set       *******************
                                                 // May need to add underscores before numbers on the right side   


  CCR0 = 31;                            // CCR0 Clock Period (in upmode, counts to this number from 0 cyclically)
                                        // CCIFG is CC Interrupt FlaG, it is set automagically when the timer counts to the CCR0 value 
                                        // TAIFG is higher priority

  CCR1 = 13;                             // CCR1 PWM Duty Cycle
                                              // THE CUT-POINT OF THE COUNTER: ABOVE WHICH THE OUTPUT IS 1, BELOW WHICH THE OUTPUT IS ZERO
                                              // THIS DRIVES PIN 2.3-TAOUT (our ILRCK)
                                              // THIS WAS "15" BUT THE LR CLK LOOKED **UNEVEN** so we changed to 16 [TAP]
  CCTL0 = CCIE;                         // CCR0 (capture/compare) Interrupt Enabled for TACCTL0 (control register for TimerA)
  
  P3SEL |= 0x0A;                        // P3.1,3 SPI option select
  P3DIR |= 0x0A;                        // P3.1,3 output direction
  P2SEL |= 0x08;                        // P2.3 CCR1 Direction
  P2DIR |= 0x08;                        // P2.3 Output
  
  ME1 = USPIE0;                        // Enable USART0 SPI mode
  //  UBR00 = 0x04; UBR10 = 0x00;           // UCLK/4 [see 115k UART demo]
  UBR00 = 0x02; UBR10 = 0x00;           // UCLK/2 [see 115k UART demo]
                                        //    Crystal Speed (6.144MHz) / Baud Rate (8bits * 48KHz * 2 bytes * 2channels = 1536000)
  
  UMCTL0 = 0x00;                        // no modulation / Clear Modulation (not used for SPI mode - UART mode only)

  UTCTL0 = /*CKPH+CKPL+*/SSEL0+STC;    // CKPH = delay by 1/2 cycle
                                        // CKPL = polarity, setting high inverts pin polarity
                                        // ACLK, 3-pin (was SMCLK, 3-pin mode), SSEL0 = ACLK, 
                                        // STC = 3-pin mode
                                            
  UCTL0 = CHAR+SYNC+MM;                 // 8-bit + SPI + Master  [**SWRST** disabled]

  TACTL |= MC_1;                         // Start Timer_A in upmode

  // TACTL |= ID_3;                         // [JTA] Divides timer freq by 8
  TACTL |= ID_2;                         // [JTA] Divides timer freq by 4
  // TACTL |= ID_1;                         // [JTA] Divides timer freq by 2

return(0);
}
