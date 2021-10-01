/*
 * Project: Yoghurt Maker
 * Author : LeonH <L.Hiemstra@gmail.com>
 *
 *
 * Config:  ATtiny24; CPU=8000000 Hz; optimize=-Os
 * Fuses:   0xFF, 0xDF, 0xE2
 *
 * Date:    DEC-2012
 *
 */

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>

#include "delay.h"
#include "onewire.h"
#include "ds18x20.h"


uint8_t sysstate;
uint16_t tempsensor;
uint8_t gSensorIDs[OW_ROMCODE_SIZE];


/*
 * State defenitions
 */
#define STATE_COOL   2
#define STATE_BOOST  1
#define STATE_NORMAL 0
#define STATE_ERROR  3

#define LED_RED   PB1
#define LED_GREEN PB0
#define LED_YELLOW PA1
#define BUZZER    PA3


/*
 * Subroutine declarations:
 */
uint8_t search_sensors(void);
uint8_t start_scan_one_channel(void);
uint8_t read_scan_one_channel(void);


/*
 * Main loop
 */
int main() 
{
	uint8_t cmd,ret,beep;

	// init portB
	DDRB = 0x07;
        PORTB = 0x08;
     

        // init portA
	DDRA = 0x0a;     
	PORTA = 0x84;

        sysstate=STATE_NORMAL;

	/* enable interrupts: */	
	sei();		  


	/* Main loop: */
    while (1) {
        if((PINA & (1<<PA7)) == 0) {
            sysstate=STATE_BOOST;
        }

        ret=start_scan_one_channel();

        // have to wait here at least 750ms until scanning is ready
        delay_ms(400);
        delay_ms(400);

        if(ret) {
            if(read_scan_one_channel()) {
                PORTA &= ~(1<<LED_YELLOW);
                PORTB &= ~(1<<LED_RED);
                PORTB &= ~(1<<LED_GREEN);

                if(sysstate==STATE_BOOST) {
                    PORTB |= (1<<LED_RED);
                    if(tempsensor > (86<<4)) {
                        sysstate=STATE_COOL;
                        PORTA |= (1<<BUZZER);
                    } else {
                        PORTB |= (1<<PB2);
                        PORTA &= ~(1<<BUZZER);
                    }
                } else if(sysstate==STATE_COOL) {
                    PORTB |= (1<<LED_GREEN);
                    PORTB &= ~(1<<PB2);
                    PORTA &= ~(1<<BUZZER);
                    if(tempsensor < (43<<4)) {
                        for(beep=0;beep<5;beep++) {
                            PORTA |= (1<<BUZZER);
                            delay_ms(400);
                            PORTA &= ~(1<<BUZZER);
                            delay_ms(400);
                        }
                        sysstate=STATE_NORMAL;
                    }
                } else {
                    PORTA |= (1<<LED_YELLOW);
                    PORTA &= ~(1<<BUZZER);
                    if(tempsensor < (38<<4)) {
                        PORTB |= (1<<PB2);
                    } else if(tempsensor > (39<<4)) {
                        PORTB &= ~(1<<PB2);
                    }
                }
            } else {
                PORTB &= ~(1<<PB2);
            }
        }
    }
    return 0;
}

uint8_t start_scan_one_channel(void)
{
    uint8_t nSensors, i;

    ow_set_bus(&PINA,&PORTA,&DDRA,PA2);

    nSensors = search_sensors();
    if(nSensors == 0) {
        delay_ms(500); // try 1 more time ...
        nSensors = search_sensors();
    }

    if ( nSensors == 1 ) {
        i = gSensorIDs[0]; // family-code for conversion-routine
        if(DS18X20_start_meas( DS18X20_POWER_PARASITE, NULL ) == DS18X20_OK) {

            return nSensors;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}


uint8_t read_scan_one_channel(void)
{
    uint8_t i;
    uint8_t subzero, cel, cel_frac_bits;
    uint8_t ret;

    i = gSensorIDs[0]; // family-code for conversion-routine

    ret=DS18X20_read_meas_single(i,&subzero,&cel,&cel_frac_bits,&tempsensor);

    if(ret==DS18X20_ERROR_CONV || ret==DS18X20_ERROR_CRC) {
        ret=DS18X20_ERROR;
    }

    if(ret==DS18X20_OK) {
        return 1;
    } else {
        return 0;
    }
}


uint8_t search_sensors(void)
{
        uint8_t diff, nSensors;

         // Scanning Bus for DS18X20...

        nSensors = 0;

        for( diff = OW_SEARCH_FIRST;
                diff != OW_LAST_DEVICE && nSensors < 1 ; )
        {
                DS18X20_find_sensor( &diff, &gSensorIDs[0] );

                if( diff == OW_PRESENCE_ERR ) {
                        break;
                }

                if( diff == OW_DATA_ERR ) {
                        break;
                }
                nSensors++;
        }

        return nSensors;
}

