/********************************************
 *
 *  Name: Amman C
 *  Email: chuhan@usc.edu
 *  Section: Friday 12:30
 *  Assignment: Lab 6 - Timers
 *
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "lcd.h"

void debounce(uint8_t);
void timer1_init(void);

enum states { PAUSE, START_RUN, RUN, LAP };
volatile unsigned char tensPlace = '0';
volatile unsigned char unitPlace = '0';
volatile unsigned char tenthsPlace = '0';
volatile unsigned char flag = 0;
volatile char state = PAUSE;

int main(void) {

    char buf1[7];

    // Initialize the LCD and TIMER1

    lcd_init();

    timer1_init();

    // Enable pull-ups for buttons

    PORTC |= (1 << PC2); // Start-Stop
    PORTC |= (1 << PC4); // Lap-Reset

    // Show the splash screen

    lcd_moveto(0,0);

    lcd_stringout("Amman C.");

    lcd_moveto(1,0);

    lcd_stringout("Lab 6: Stopwatch");

    _delay_ms(1000);

    lcd_clear(); // helper function to clear screen

    // Enable global interrupts

    sei();

    lcd_moveto(0,0);

    lcd_stringout(" 0.0");

    while (1) {                 // Loop forever


	// Read the buttons

    unsigned char StartStopBtn = PINC & (1 << PC2);
    unsigned char LapResetBtn = PINC & (1 << PC4);


	if (state == PAUSE)  // PAUSE state
    {      
                
        if(StartStopBtn == 0) // if start stop pressed
        {
            TCCR1B &= ~(1<<CS12);
            TCCR1B |= (1<<CS11);
            TCCR1B |= (1<<CS10);
            state = START_RUN;
        }
        else if (LapResetBtn == 0) // if lap-reset btn pressed
        {
            debounce(1 << PC4);
            tensPlace = '0';
            unitPlace = '0';
            tenthsPlace = '0';
            

            TCCR1B &= ~(1<<CS12);
            TCCR1B &= ~(1<<CS11);
            TCCR1B &= ~(1<<CS10);

            lcd_moveto(0,0);

            snprintf(buf1, 7, " %c.%c", unitPlace, tenthsPlace);
            lcd_stringout(buf1);

        }
	}

    else if (state == START_RUN) // START RUN state
    {            
        if(StartStopBtn == 0) // if start stop pressed
        {
            state = START_RUN;
        }
        else // if start_stop not pressed
        {
            state = RUN; 
        }
	}

	else if (state == RUN)             // RUN state
    {            
        if(StartStopBtn == 0) // if start stop pressed
        {
            debounce(1 << PC2);
            state = PAUSE;
        }
        else if (LapResetBtn == 0) // if lap-reset btn pressed
        {
            debounce(1 << PC4);
            state = LAP; 
        }
	}
	else if (state == LAP)             // LAP state
   {            
        if(StartStopBtn == 0) // if start stop pressed
        {
            debounce(1 << PC2);
            state = RUN;
        }
        else if (LapResetBtn == 0) // if lap-reset btn pressed
        {
            debounce(1 << PC4);
            state = RUN; 
        }
	}

	// If necessary write time to LCD


    if(flag == 1 && (state == START_RUN || state == RUN))
    {
        flag = 0; //reset flag

        if(state == LAP)
        {
            continue; // we dont want to update display while lapped
        }
        else
        {
            lcd_moveto(0,0);
            if(tensPlace != '0')
            {
                
                snprintf(buf1, 7, "%c%c.%c", tensPlace, unitPlace, tenthsPlace);
                lcd_stringout(buf1);
            }
            else
            {
            
                snprintf(buf1, 7, " %c.%c", unitPlace, tenthsPlace);
                lcd_stringout(buf1);
            }


            // lcd_moveto(0,0);
            // if(tensPlace != '0')
            // {
            //     char buf1[5];
            //     buf1[0] = tensPlace;
            //     buf1[1] = unitPlace;
            //     buf1[2] = '.';
            //     buf1[3] = tenthsPlace;

            //     lcd_stringout(buf1);

            //     lcd_stringout("hi");

            // }
            // else
            // {
            //     char buf1[5];
            //     buf1[0] = ' ';
            //     buf1[1] = unitPlace;
            //     buf1[2] = '.';
            //     buf1[3] = tenthsPlace;

            //     lcd_stringout(buf1);
                
            //     lcd_stringout("hi");
            // }

            // lcd_moveto(0,1);
            // char buf2[2];
            // buf2[0] = unitPlace;
            // lcd_stringout(buf2);

            // lcd_moveto(0,2);
            // char buf3[2];
            // buf3[0] = '.';
            // lcd_stringout(buf3);

            // lcd_moveto(0,3);
            // char buf4[10];
            // buf4[0] = tenthsPlace;
            // lcd_stringout(buf4);

        }
    }

    }

    return 0;   /* never reached */
}

/* ----------------------------------------------------------------------- */

void debounce(uint8_t bit)
{
    // Add code to debounce input "bit" of PINC
    // assuming we have sensed the start of a press.

    _delay_ms(5);

    while((PINC & bit) == 0) {}

    _delay_ms(5);

}

/* ----------------------------------------------------------------------- */

void timer1_init(void)
{
  // Set to CTC mode

  TCCR1B |= (1 << WGM12);
  TCCR1B &= ~(1 << WGM13);

  // Enable Timer Interrupt

  TIMSK1 |= (1 << OCIE1A);

  // Load the max count
  // Assuming prescalar = 64
  // counting to 25,000 =
  // 0.1s w/ 16MHz clock

  OCR1A = 25000;

  // Set prescalar to 64 and start counter

  TCCR1B &= ~(1<<CS12);
  TCCR1B |= (1<<CS11);
  TCCR1B |= (1<<CS10);

  
}

ISR(TIMER1_COMPA_vect)
{
    // Increment the time

    if(state == PAUSE)
    {
        // do nothing
    }
    else
    {
        if(tensPlace == '5' && unitPlace == '9' && tenthsPlace == '9') // reset at 1 min mark
        {
            tensPlace = '0';
            unitPlace = '0';
            tenthsPlace = '0';
        }
        else
        {
            if(tenthsPlace == '9')
            {
                tenthsPlace = '0';

                if(unitPlace == '9')
                {
                    unitPlace = '0';
                    tensPlace++;
                }
                else
                {
                    unitPlace++;
                }
            }
            else
            {
                tenthsPlace++;
            }

            flag = 1; // time needs to be updated

        }
    }
}
