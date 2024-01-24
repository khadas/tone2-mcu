// SPDX-License-Identifier: GPL-2.0
#include "iostm8s003f3.h"

__root const long reintvec[]@".intvec"={
0x82008080,0x82008404,0x82008408,0x8200840c,
0x82008410,0x82008414,0x82008418,0x8200841c,
0x82008420,0x82008424,0x82008428,0x8200842c,
0x82008430,0x82008434,0x82008438,0x8200843c,
0x82008440,0x82008444,0x82008448,0x8200844c,
0x82008450,0x82008454,0x82008458,0x8200845c,
0x82008460,0x82008464,0x82008468,0x8200846c,
0x82008470,0x82008474,0x82008478,0x8200847c,
};

/******************************************************************
RAM:		 0x0000~0x03FF
EEPROM : 	 0x4000~0x407F
OPTION BYTE: 0x4800~0x480A
Flash :
	Interrupt vectors(32):0x8000~0x807F
	BOOT 0x807F~0x83FF
	APP	 0x8400~0x9FFF
*******************************************************************/

#define FLASH_START_ADDR    0x8400
#define BOOT_APP_DONE       0x50
#define BOOT_UPDATE         0x51
#define FLASH_RASS_KEY1     ((unsigned char)0x56)
#define FLASH_RASS_KEY2     ((unsigned char)0xAE)
#define BLOCK_BYTES         64
#define ENTER_UPDATA        0x00
#define PIN_1           0x02
#define PIN_2           0x04
#define PIN_3           0x08
#define PIN_4           0x10
#define PIN_7           0x80
#define PAGE_START	    0x10
#define PAGE_COUNT	    0x80
#define CHECK_ERR	    0xFF
#define RUN_APP		    0xFE
#define BOOT_VERSION	0x11

unsigned char eeprom_updata_reg @0x4000;
unsigned char ucAppCodeData[64] = {0};

void sys_clock_init(void)
{
    CLK_CKDIVR = 0x00;
    while(!(CLK_ICKR & 0x02));
}

void sys_uart_init(void)
{
    // close interrupt
    UART1_CR1 = 0x00;
    UART1_CR2 = 0x00;
    UART1_CR3 = 0x00;

    // set 115200
    UART1_BRR2 = 0x0A;
    UART1_BRR1 = 0x08;

    // enable write & read
    UART1_CR2 =0x0C;
}

void uart_write_byte(unsigned char data)
{
    while (!(UART1_SR & 0x80));
    UART1_DR = data;
}

unsigned char uart_read_byte(void)
{
    while(!(UART1_SR & 0x20));
    return ((unsigned char)UART1_DR);
}

__ramfunc void sys_block_write(unsigned char* addr,unsigned char* buf)
{
    unsigned char index=0;

    FLASH_PUKR = FLASH_RASS_KEY1;
    FLASH_PUKR = FLASH_RASS_KEY2;

    FLASH_CR2   |= 0x01;
    FLASH_NCR2  &= ~0x01;

    for(index = 0; index < BLOCK_BYTES; index++)
    {
        *((__near unsigned char*)((unsigned short)addr+index))=(unsigned char)buf[index];
    }

    FLASH_IAPSR &= 0xFD;
}


unsigned char unlock_eeprom(void)
{
    FLASH_DUKR = FLASH_RASS_KEY2;
    FLASH_DUKR = FLASH_RASS_KEY1;

    if (FLASH_IAPSR&0x08) {
        return 0;
    } else {
        return 1;
    }
}

void eeprom_write(unsigned char data)
{
    while(unlock_eeprom());
    eeprom_updata_reg = data;
    FLASH_IAPSR &=0xF7;
}

unsigned char eeprom_updata_flag_read(void)
{
    return eeprom_updata_reg;
}

void sys_updata_cmd(unsigned char page_data)
{
    uart_write_byte(0x4B);
    uart_write_byte(0x80);
    uart_write_byte(page_data);
    uart_write_byte(0x73);
}

void delay_ms(unsigned short Count)
{
  unsigned char i, j;
  while (Count--) {
    for (i=0;i<60;i++) {
        for (j=0;j<64;j++);
    }
  }
}

void sys_xmos_init(void)
{
    // rest xmos
    PA_CR2 &= (unsigned char)(~(PIN_1 | PIN_2));    // no interrupt
    PA_ODR &= (unsigned char)(~(PIN_1 | PIN_2));    // PA1, PA2  L
    PA_DDR |= (unsigned char)(PIN_1 | PIN_2);       // PA1, PA2 output
    PA_CR1 |= (unsigned char)(PIN_1 | PIN_2);       // pull up
    PA_CR2 &= (unsigned char)(~(PIN_1 | PIN_2));    // no interrupt
    PA_ODR &= (unsigned char)(~(PIN_1 | PIN_2));    // PA1, PA2  L


    // pwr on and set uart to xmos
    PC_CR2 &= (unsigned char)(~(PIN_7 | PIN_3 | PIN_4));    // no interrupt
    PC_ODR &= (unsigned char)(~PIN_4);                      // PC4 output L
    PC_ODR |= (unsigned char)(PIN_7 | PIN_3);               // PC7/PC3 output H
    PC_DDR |= (unsigned char)(PIN_7 | PIN_3 | PIN_4);       // output
    PC_CR1 |= (unsigned char)(PIN_7 | PIN_3 | PIN_4);       // pull up
    PC_CR2 &= (unsigned char)(~(PIN_7 | PIN_3 | PIN_4));    // no interrupt


    // rest xmos
    delay_ms(40);
    PA_ODR |= (unsigned char)(PIN_2);                       // PA2 output H

    PC_ODR |= (unsigned char)(PIN_3);                       // PC3 output H
    PC_ODR &= (unsigned char)(~PIN_4);                      // PC4 output L

    // sync update signal
    delay_ms(213);
    if (uart_read_byte() !=0x73) {
        sys_updata_cmd(ENTER_UPDATA);
    }

    // wait for xmos ready & enter mcu upgrade mode // xmos ask for SN
    while (uart_read_byte() !=0x73) {

    }
    delay_ms(85);
}

int main( void )
{
    unsigned char index = 0;
    unsigned char data_read = 0;
    unsigned char page_count = 0;
    unsigned char* app_code_addr = (unsigned char*)FLASH_START_ADDR;

    sys_clock_init();
    sys_uart_init();

    data_read = eeprom_updata_flag_read();

    if (data_read != BOOT_UPDATE) {
        // no need to updata, jump to app code to run
        asm("JP $8400");
    }

    sys_xmos_init();
    delay_ms(85);

    // write 1~112page, STM8S003F3 have 8K,boot use 1K, leave 7K*1024/64Byte = 112 Page
    while (1) {
GotoReWrite:
        for (page_count = PAGE_START; page_count < PAGE_COUNT; page_count++) {
            app_code_addr = (unsigned char*)(FLASH_START_ADDR + (page_count-0x10) * BLOCK_BYTES);

            // request 1 page
            sys_updata_cmd(page_count);
            for (index = 0; index < BLOCK_BYTES; index++) {
                ucAppCodeData[index] = uart_read_byte();
            }

            // write 1 page
            sys_block_write(app_code_addr, ucAppCodeData);

            // check 1 page
            for (index = 0; index < BLOCK_BYTES; index++) {
                if (app_code_addr[index] != ucAppCodeData[index]) {
                    sys_updata_cmd(CHECK_ERR);
                    goto GotoReWrite;
                }
            }
        }

        // set success flag and run to app
        sys_updata_cmd(0x00);

        // power off |PC7
        PC_ODR &= (unsigned char)(~PIN_7);  // output L
        delay_ms(850);

        // update success, jump to app code to run
        asm("JP $8400");
    }

}
