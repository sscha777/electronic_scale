/*
@file    EVE_target.h
@brief   target specific includes, definitions and functions
@version 4.0
@date    2020-05-01
@author  Rudolph Riedel

@section LICENSE

MIT License

Copyright (c) 2016-2020 Rudolph Riedel

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

@section History

4.0
- still 4.0 for EVE itself, switched to hardware-SPI on SAMC21
- minor maintenance
- added DMA to SAMC21 branch
- started testing things with a BT816
- added a block for the SAME51J18A
- started to add support for Imagecraft AVR
- moved all target specific lines from EVE_config.h to EVE_target.h
- cleaned up history
- added support for MSP432 - it compiles with Code Composer Studio but is for the most part untested...
- wrote a couple lines of explanation on how DMA is to be used
- replaced the dummy read of the SPI data register with a var for ATSAMC21 and ATSAME51 with "(void) REG_SERCOM0_SPI_DATA;"
- added support for RISC-V, more specifically the GD32VF103 that is on the Sipeed Longan Nano - not tested with a display yet but it looks very good with the Logic-Analyzer
- added support for STM32F407 by adding code supplied by User "mokka" on MikroController.net and modifying it by replacing the HAL functions with direct register accesses
- added comment lines to separate the various targets visually
- reworked ATSAMC21 support code to use defines for ports, pins and SERCOM, plus changed the "legacy register definitions" to more current ones
- changed ATSAME51 support code to the new "template" as well
- bugifx: STM32F407 support was neither working or compiling, also changed it to STM32F4 as it should support the whole family now
- bugifx: second attempt to fix STM32F4 support, thanks again to user "mokka" on MikroController.net
- combined ATSAMC21 and ATSAME51 targets into one block since these were using the same code anyways
- moved the very basic DELAY_MS() function for ATSAM to EVE_target.c and therefore removed the unneceesary inlining for this function
- expanded the STM32F4 section with lines for STM32L073, STM32F1, STM32F207 and STM32F3
- forgot to add the "#include <Arduino.h>" line I found to be necessary for ESP32/Arduino
- started to implement DMA support for STM32
- added a few more controllers as examples from the ATSAMC2x and ATSAMx5x family trees
- measured the delay for ATSAME51 again and changed EVE_DELAY_1MS to 20000 for use with 120MHz core-clock and activated cache

*/

#ifndef EVE_TARGET_H_
#define EVE_TARGET_H_


/* While the following lines make things a lot easier like automatically compiling the code for the target you are compiling for, */
/* a few things are expected to be taken care of beforehand. */
/* - setting the Chip-Select and Power-Down pins to Output, Chip-Select = 1 and Power-Down = 0 */
/* - setting up the SPI which may or not include things like
       - setting the pins for the SPI to output or some alternate I/O function or mapping that functionality to that pin
	   - if that is an option with the controller your are using you probably should set the drive-strength for the SPI pins to high
	   - setting the SS pin on AVRs to output in case it is not used for Chip-Select or Power-Down
	   - setting SPI to mode 0
	   - setting SPI to 8 bit with MSB first
	   - setting SPI clock to no more than 11 MHz for the init - if the display-module works as high

  For the SPI transfers single 8-Bit transfers are used with busy-wait for completion.
  While this is okay for AVRs that run at 16MHz with the SPI at 8 MHz and therefore do one transfer in 16 clock-cycles,
  this is wasteful for any 32 bit controller even at higher SPI speeds.

  Check out the section for SAMC21E18A as it has code to transparently add DMA.

  If the define "EVE_DMA" is set the spi_transmit_async() is changed at compile time to write in a buffer instead directly to SPI.
  EVE_init() calls EVE_init_dma() which sets up the DMA channel and enables an IRQ for end of DMA.
  EVE_start_cmd_burst() resets the DMA buffer instead of transferring the first bytes by SPI.
  EVE_end_cmd_burst() just calls EVE_start_dma_transfer() which triggers the transfer of the SPI buffer by DMA.
  EVE_cmd_start() just instantly returns if there is an active DMA transfer.
  EVE_busy() does nothing but to report that EVE is busy if there is an active DMA transfer.
  At the end of the DMA transfer an IRQ is executed which clears the DMA active state, calls EVE_cs_clear() and EVE_cmd_start().

*/


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define EVE_CS 			5
//#define EVE_PDN			GPIO_NUM_25
#define EVE_SPI_MOSI	23
#define EVE_SPI_MISO	19
#define EVE_SPI_CLK		18

#define EVE_SPI_HOST	VSPI_HOST

typedef enum _spi_send_flag_t
{
	SPI_SEND_QUEUED			= 0x00000000,
	SPI_SEND_POLLING		= 0x00000001,
	SPI_SEND_SYNCHRONOUS	= 0x00000002,
	SPI_RESERVED			= 0x00000004,		// reserved
	SPI_RECEIVE				= 0x00000008,
	SPI_CMD_8				= 0x00000010,		// reserved - not yet implemented
	SPI_CMD_16				= 0x00000020,		// reserved - not yet implemented
	SPI_ADDRESS_8			= 0x00000040,		// reserved - not yet implemented
	SPI_ADDRESS_16			= 0x00000080,		// reserved - not yet implemented
	SPI_ADDRESS_24			= 0x00000100,
	SPI_ADDRESS_32			= 0x00000200,		// reserved - not yet implemented
	SPI_MODE_DIO			= 0x00000400,		// reserved - not yet implemented
	SPI_MODE_QIO			= 0x00000800,		// reserved - not yet implemented
	SPI_MODE_DIOQIO_ADDR	= 0x00001000,		// reserved - not yet implemented
} spi_send_flag_t;

typedef struct _spi_read_data
{
	uint8_t _dummy_byte;
	union
	{
		uint8_t 	byte;
		uint16_t	word;
		uint32_t	dword;
	} __attribute__((packed));					// Note: this packing and alignment ensures that the data will be DMA-able
} spi_read_data __attribute__((aligned(4)));

// receive data helpers
#define member_size(type, member) sizeof(((type *)0)->member)

#define SPI_READ_DUMMY_LEN 	member_size(spi_read_data, _dummy_byte)
#define SPI_READ_BYTE_LEN 	(SPI_READ_DUMMY_LEN + member_size(spi_read_data, byte))
#define SPI_READ_WORD_LEN 	(SPI_READ_DUMMY_LEN + member_size(spi_read_data, word))
#define SPI_READ_DWORD_LEN 	(SPI_READ_DUMMY_LEN + member_size(spi_read_data, dword))

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp32/rom/ets_sys.h"

static inline void DELAY_MS(uint32_t ms)
{
	if (ms == 0) {
		return;
	}
	TickType_t ticks = pdMS_TO_TICKS(ms);
	if (ticks == 0) {
		ets_delay_us(ms * 1000);
		return;
	}
	vTaskDelay(ticks);
}


// static inline void EVE_pdn_set(void)
// {
// 	gpio_set_level(EVE_PDN, 0);	/* Power-Down low */
// }

// static inline void EVE_pdn_clear(void)
// {
// 	gpio_set_level(EVE_PDN, 1);	/* Power-Down high */
// }

void spi_init(void);

void spi_wait_for_pending_transactions();

void spi_transaction(const uint8_t* data, uint16_t length, spi_send_flag_t flags, spi_read_data* out, uint64_t addr);




#endif /* EVE_TARGET_H_ */
