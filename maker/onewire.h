#ifndef _1wire_h_
#define _1wire_h_

#include <inttypes.h>

/*******************************************/
/* Hardware connection                     */
/*******************************************/


#if F_OSC<1843200
 #warning | experimental multi-bus-mode is not tested for 
 #warning | frequencies below 1,84MHz - use OW_ONE_WIRE or
 #warning | faster clock-source (i.e. internal 2MHz R/C-Osc.)
#endif
#define OW_CONF_CYCLESPERACCESS 13
#define OW_CONF_DELAYOFFSET ( (uint16_t)( ((OW_CONF_CYCLESPERACCESS)*1000000L) / F_OSC  ) )

/*******************************************/

// #define OW_SHORT_CIRCUIT  0x01

#define OW_MATCH_ROM	0x55
#define OW_SKIP_ROM	    0xCC
#define	OW_SEARCH_ROM	0xF0

#define	OW_SEARCH_FIRST	0xFF		// start new search
#define	OW_PRESENCE_ERR	0xFF
#define	OW_DATA_ERR	    0xFE
#define OW_LAST_DEVICE	0x00		// last device found
//			0x01 ... 0x40: continue searching

// rom-code size including CRC
#define OW_ROMCODE_SIZE 8

extern uint8_t ow_reset(void);

extern uint8_t ow_bit_io( uint8_t b );
extern uint8_t ow_byte_wr( uint8_t b );
extern uint8_t ow_byte_rd( void );

extern uint8_t ow_rom_search( uint8_t diff, uint8_t *id );

extern void ow_command( uint8_t command, uint8_t *id );

extern void ow_parasite_enable(void);
extern void ow_parasite_disable(void);
extern uint8_t ow_input_pin_state(void);

extern void ow_set_bus(volatile uint8_t* in,
	volatile uint8_t* out,
	volatile uint8_t* ddr,
	uint8_t pin);

#endif

