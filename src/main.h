// SPDX-License-Identifier: GPL-2.0
#ifndef __MAIN_H__
#define __MAIN_H__

#include "stm8s.h"
#include "clk.h"
#include "int.h"
#include "tim4.h"
#include "i2c.h"
#include "uart.h"
#include "flash.h"
#include "gpio.h"

#define KHADAS_TEST 0

#if KHADAS_TEST
#define UD_VER_LOCAL		0xF3   // MCU firmware version

#else
#define UD_VER_LOCAL		0x07   // MCU firmware version

#endif

// I2C ADDR
#define I2C_T2V_ADDR		0xA0	// Chip: AT24C01D
#define I2C_DAC_ADDR		0x90	// DAC I2C Address

// GPIO GET/SET
#define SYS_MUTE_OFF()		gpio_write_high(GPIOA, gpio_pin_1)
#define SYS_MUTE_ON()		gpio_write_low(GPIOA, gpio_pin_1)
#define SYS_NRST_ON()		gpio_write_high(GPIOA, gpio_pin_2)
#define SYS_NRST_OFF()		gpio_write_low(GPIOA, gpio_pin_2)
#define SYS_POWER_ON()		gpio_write_high(GPIOC, gpio_pin_7)
#define SYS_POWER_OFF()		gpio_write_low(GPIOC, gpio_pin_7)
#define DAC_I2S_SET0_H()	gpio_write_high(GPIOC, gpio_pin_3)
#define DAC_I2S_SET0_L()    gpio_write_low(GPIOC, gpio_pin_3)
#define DAC_I2S_SET1_H()    gpio_write_high(GPIOC, gpio_pin_4)
#define DAC_I2S_SET1_L()    gpio_write_low(GPIOC, gpio_pin_4)
#define FUN_KEY_IN()        gpio_read_input_pin(GPIOD, gpio_pin_2)
#define KNOB_A_IN()      	gpio_read_input_pin(GPIOC, gpio_pin_5)
#define KNOB_B_IN()      	gpio_read_input_pin(GPIOC, gpio_pin_6)
#define T2V_NG_L()          gpio_write_low(GPIOA, gpio_pin_3)
#define T2V_OK_H()          gpio_write_high(GPIOA, gpio_pin_3)
#define VIM_I2S_DETECT()   	gpio_read_input_pin(GPIOD, gpio_pin_4)
#define EXT_I2S_DETECT()    gpio_read_input_pin(GPIOD, gpio_pin_3)

// DAC REG
#define DAC_REG_SYS			0x00	// OSC DRV, CLK GEAR, SOFT RST
#define DAC_REG_INPUT		0x01	// Input Selection
#define DAC_REG_MIX			0x02	// Mixing, Serial Data and Automute
#define DAC_REG_SPDIF_C		0x03	// S/PDIF Configuration
#define DAC_REG_MUTE_T		0x04	// Automute Time
#define DAC_REG_MUTE_L		0x05	// Automute Level
#define DAC_REG_DEEMPH		0x06	// De-emphasis, DoP and Volume Ramp Rate
#define DAC_REG_FILTER		0x07	// Filter Bandwidth and System Mute
#define DAC_REG_GPIO_CFG1	0x08	// GPIO1-2 Configuration
#define DAC_REG_MODE		0x0A	// Master Mode and Sync Configuration
#define DAC_REG_SPDIF_S		0x0B	// SPDIF Select
#define DAC_REG_DPLL		0x0C	// ASRC/DPLL Bandwidth
#define DAC_REG_THD			0x0D	// THD Bypass
#define DAC_REG_SOFT		0x0E	// Soft Start Configuration
#define DAC_REG_VOL_L		0x0F	// Left Volume Control
#define DAC_REG_VOL_R		0x10	// Right Volume Control
#define DAC_REG_TRIM1		0x11	// Master Trim
#define DAC_REG_TRIM2		0x12	// Master Trim
#define DAC_REG_TRIM3		0x13	// Master Trim
#define DAC_REG_TRIM4		0x14	// Master Trim
#define DAC_REG_GPIO_CFG2	0x15	// GPIO Input Selection
#define DAC_REG_THD_C21		0x16	// THD Compensation C2
#define DAC_REG_THD_C22		0x17	// THD Compensation C2
#define DAC_REG_THD_C31		0x18	// THD Compensation C3
#define DAC_REG_THD_C32		0x19	// THD Compensation C3
#define DAC_REG_GEN_CFG1	0x1B	// General Configuration
#define DAC_REG_GPIO_CFG3	0x1D	// GPIO Configuration
#define DAC_REG_CHARG1		0x1E	// Charge Pump Clock
#define DAC_REG_CHARG2		0x1F	// Charge Pump Clock
#define DAC_REG_INT_MASK	0x21	// Interrupt Mask
#define DAC_REG_NCO1		0x22	// Programmable NCO
#define DAC_REG_NCO2		0x23	// Programmable NCO
#define DAC_REG_NCO3		0x24	// Programmable NCO
#define DAC_REG_NCO4		0x25	// Programmable NCO
#define DAC_REG_RESERVED5	0x26	// Reserved
#define DAC_REG_GEN_CFG2	0x27	// General Configuration 2
#define DAC_REG_FIR_ADD 	0x28	// Programmable FIR RAM Address
#define DAC_REG_FIR_DATA1 	0x29	// Programmable FIR RAM Data
#define DAC_REG_FIR_DATA2 	0x2A	// Programmable FIR RAM Data
#define DAC_REG_FIR_DATA3 	0x2B	// Programmable FIR RAM Data
#define DAC_REG_FIR_CFG 	0x2C	// Programmable FIR Configuration
#define DAC_REG_LP_CALI		0x2D	// Low Power and Auto Calibration
#define DAC_REG_ADC_CFG		0x2E	// ADC Configuration
#define DAC_REG_ADC_FILTER1	0x2F	// ADC Filter Configuration (ftr_scale)
#define DAC_REG_ADC_FILTER2	0x30	// ADC Filter Configuration (ftr_scale)
#define DAC_REG_ADC_FILTER3	0x31	// ADC Filter Configuration (fbq_scale)
#define DAC_REG_ADC_FILTER4	0x32	// ADC Filter Configuration (fbq_scale)
#define DAC_REG_ADC_FILTER5	0x33	// ADC Filter Configuration (fbq_scale)
#define DAC_REG_ADC_FILTER6	0x34	// ADC Filter Configuration (fbq_scale)
#define DAC_REG_CHIP_ID		0x40	// Chip ID and Status(Read-Only)
#define DAC_REG_GPIO_RD		0x41	// GPIO Readback(Read-Only)
#define DAC_REG_DPLL_R1		0x42	// DPLL Number(Read-Only)
#define DAC_REG_DPLL_R2		0x43	// DPLL Number(Read-Only)
#define DAC_REG_DPLL_R3		0x44	// DPLL Number(Read-Only)
#define DAC_REG_DPLL_R4		0x45	// DPLL Number(Read-Only)
#define DAC_REG_SPDIF_R1	0x46	// SPDIF Channel Status/User Status(Read-Only)
// ... n=24 //
#define DAC_REG_SPDIF_R24	0x5D	// SPDIF Channel Status/User Status(Read-Only)
#define DAC_REG_IN_SELECT	0x60	// Input Selection and Automute Status(Read-Only)
#define DAC_REG_ADC_R1		0x64	// ADC Readback(Read-Only)
#define DAC_REG_ADC_R2		0x65	// ADC Readback(Read-Only)
#define DAC_REG_ADC_R3		0x66	// ADC Readback(Read-Only)

#define DAC_INPUT_SPDIF		0xC1	// S/PDIF Input
#define DAC_INPUT_I2S_DSD	0xC4	// I2S/DSD Input

// EEPROM ADDR
#define E2P_ADD_UPGRADE		0x00	// 50
#define E2P_ADD_SN12		0x10	// 00
#define E2P_ADD_SN34		0x11	// 00
#define E2P_ADD_SN56		0x12	// 00
#define E2P_ADD_SN78		0x13	// 00
#define E2P_ADD_VOL			0x14	// 21
#define E2P_ADD_GAIN		0x15	// T2P OPTION, T2 NG
#define E2P_ADD_INPUT		0x16	// 05
#define E2P_ADD_FILTER 		0x17	// 05
#define E2P_ADD_MODE_EN		0x18	// VOL+NP+GAIN+INPUT+FILTER=00011111
#define E2P_ADD_TIME_OFF	0x19	// DEF 10 MINUTE OFF WHILE NO PLAY
#define E2P_ADD_DAC16		0x21	// 02
#define E2P_ADD_DAC17		0x22	// 00
#define E2P_ADD_DAC18		0x23	// D6
#define E2P_ADD_DAC19		0x24	// FF

#define DATA_NOTUPGRADE 	0x50
#define DATA_UPGRADE		0x51

#define DATA_VOL_INIT		0x11	// 50%
#define DATA_VOL_MIN		0x01	// MUTE
#define DATA_VOL_MAX		0x21	// MUTE(1),2~33

#define INPUT_XMOS_I2S      0x01
#define INPUT_DAC_SPDIF     0x02
#define INPUT_EXT_I2S       0x03
#define INPUT_VIM_I2S       0x04
#define INPUT_AUTO_CH       0x05

#define DAC_FILTER_A		0x01	// LINEAR FAST
#define DAC_FILTER_B		0x02	// LINEAR SLOW
#define DAC_FILTER_C		0x03	// MINI FAST
#define DAC_FILTER_D		0x04	// MINI SLOW
#define DAC_FILTER_E		0x05	// APODIZING FAST(DEFAULT)
#define DAC_FILTER_F		0x06	// RESERVE
#define DAC_FILTER_G		0x07	// HYBRID FAST
#define DAC_FILTER_H		0x08	// BRICK WALL

#define DEF_THDN_16			0x02
#define DEF_THDN_17			0x00
#define DEF_THDN_18			0xD6
#define DEF_THDN_19			0xFF

// UART PROTOCOL [START BYTE] [CMD BYTE] [Data BYTE] [STOP BYTE]
#define UB_HEAD			0x00
#define UB_CMD			0x01
#define UB_DATA			0x02
#define UB_END			0x03
#define UD_START		0x4B
#define UD_END			0x73

// UART CMD
#define UC_LOCK			0x10
#define UC_PLAY			0x20
#define UC_STATUS		0x30
#define UC_VOL			0x31
#define UC_NP			0x32
#define UC_GAIN			0x33
#define UC_INPUT		0x34
#define UC_FILTER		0x35
#define UC_DATA			0x40
#define UC_DATA_MQA		0x41
#define UC_DAC16		0x52
#define UC_DAC17		0x53
#define UC_DAC18		0x54
#define UC_DAC19		0x55
#define UC_MODULE		0x60
#define UC_MODE_EN		0x61
//#define UC_TIME_STOP	0x62	// Power off while no play and timeout
#define UC_TIME_OFF		0x63	// Power off while timeout
#define UC_PAIR_CLEAN	0x64
#define UC_MCU_OFF		0x65
#define UC_MCU_AUTOI	0x66
#define UC_VER_LOCAL	0x70
#define UC_SN12			0x71
#define UC_SN34			0x72
#define UC_SN56			0x73
#define UC_SN78			0x74
#define UC_UPGRADE		0x80
#define UC_VER_XMOS		0x81
#define UC_MUL_GPIO		0x90

// UART DATA
#define UD_LOCK			0x01
#define UD_LOCK_TIPS	0x02
#define UD_UNLOCK		0x03

#define UD_PLAY			0x01
#define UD_STOP			0x02
#define UD_XMOS_PLAY	0x03
#define UD_XMOS_STOP	0x04
#define UD_XPLAY		0x05
#define UD_XSTOP		0x06

#define UD_MODE_VOL		0x01
#define UD_MODE_NP		0x02
#define UD_MODE_GAIN	0x03	// T2P OPTION, T2 NG
#define UD_MODE_INPUT	0x04
#define UD_MODE_FILTER	0x05

#define UD_AVOL_QUERY	0x00	// CHANGE !!
#define UD_AVOL_MAX		0x21
#define UD_XDVOL_DOWN	0xFC
#define UD_XDVOL_UP		0xFD
#define UD_DVOL_DOWN	0xFE
#define UD_DVOL_UP		0xFF

#define UD_NP_NG		0x00
#define UD_NEXT			0x01
#define UD_PREV			0x02
#define UD_XNEXT		0x03
#define UD_XPREV		0x04


#define UD_GAIN_QUERY	0x00
#define UD_GAIN_LOW		0x01
#define UD_GAIN_HIGH	0x02
#define UD_GAIN_PREAMP	0x03

#define UD_INPUT_QUERY	0x00
#define UD_INPUT_XMOS	0x01
#define UD_INPUT_SPDIF	0x02
#define UD_INPUT_EXT    0x03
#define UD_INPUT_VIM	0x04
#define UD_INPUT_AUTO	0x05

#define UD_FILTER_QUERY	0x00
#define UD_FILTER_A		0x01
#define UD_FILTER_B		0x02
#define UD_FILTER_C		0x03
#define UD_FILTER_D		0x04
#define UD_FILTER_E		0x05
#define UD_FILTER_F		0x06
#define UD_FILTER_G		0x07
#define UD_FILTER_H		0x08

#define UD_PCM_48		0x01	// PCM44.1/48
#define UD_PCM_96		0x02	// PCM88.2/96
#define UD_PCM_192		0x03	// PCM176.4/192
#define UD_PCM_384		0x04	// PCM352.8/384
#define UD_PCM_768		0x05	// PCM705.6/768
#define UD_DSD_64		0x11	// DSD64
#define UD_DSD_128		0x12	// DSD128
#define UD_DSD_256		0x13	// DSD256
#define UD_DSD_512		0x14	// DSD512
#define UD_BT_SBC		0x81	// BT-SBC
#define UD_BT_AAC		0x82	// BT-AAC
#define UD_BT_ATPX		0x83	// BT-aptX
#define UD_BT_ATPXLL	0x84	// BT-aptX LL
#define UD_BT_ATPXHD	0x85	// BT-aptX HD
#define UD_BT_ATPXA		0x86	// BT-aptX ADAPTIVE
#define UD_BT_LDAC		0x87	// BT-LDAC
#define UD_BT_LHDC		0x88	// BT-LHDC
#define UD_MQA_OFS_48	0x01	// MQA OFS 44.1/48
#define UD_MQA_OFS_96	0x02	// MQA OFS 88.2/96
#define UD_MQA_OFS_192	0x03	// MQA OFS 176.4/192
#define UD_MQA_OFS_384	0x04	// MQA OFS 352.8/384
#define UD_MQA_OFS_768	0x05	// MQA OFS 705.6/768
#define UD_MQA_STU_48	0x11	// MQA STUDIO 44.1/48
#define UD_MQA_STU_96	0x12	// MQA STUDIO 88.2/96
#define UD_MQA_STU_192	0x13	// MQA STUDIO 176.4/192
#define UD_MQA_STU_384	0x14	// MQA STUDIO 352.8/384
#define UD_MQA_STU_768	0x15	// MQA STUDIO 705.6/768
#define UD_MQA_48		0x21	// MQA 44.1/48
#define UD_MQA_96		0x22	// MQA 88.2/96
#define UD_MQA_192		0x23	// MQA 176.4/192
#define UD_MQA_384		0x24	// MQA 352.8/384
#define UD_MQA_768		0x25	// MQA 705.6/768

#define UD_MODULE_QUERY	0x00	// CHECK WHICH DEVICE BTM CONNECTED
#define UD_MODULE_T2P	0x01	// TONE2 PRO
#define UD_MODULE_T2	0x02	// TONE2
#define UD_MODULE_MUL	0x03	// MULTICHANNEL

#define UD_MODE_QUERY	0x00	// CHECK MODE EN LIST
#define UD_MODE_DEF		0x1F	// 00011111: *-*-*-FILTER-INPUT-GAIN-NP-VOL //
//#define UD_TIME_QUERY	0x00	// CHECK TIMEOUT PARAMETER(PWR OFF WHILE NO PLAY)(UNIT: MINUTE)
#define UD_TIME_DEF		0x0A	// 10 MINUTE OFF WHILE NO PLAY
#define UD_PAIR_CLEAN	0x01	// CLEAR BT PAIR INFO
#define UD_MCU_OFF		0x01	// MCU PWR OFF
#define UD_VER_QUERY	0x00	// CHECK MCU FIRMWARE VERSION
#define UD_SN_QUERY		0x00	// CHECK DEVICE SN

#define UD_PCM_DSD_L	0x01	// PCM_DSD SET L
#define UD_PCM_DSD_H	0x02	// PCM_DSD SET H
#define UD_MCLK_SEL_L	0x03	// MCLK_SEL SET L
#define UD_MCLK_SEL_H	0x04	// MCLK_SEL SET H

#define MODE_VOL_MASK		0x01
#define MODE_NP_MASK		0x02
#define MODE_GAIN_MASK		0x04
#define MODE_INPUT_MASK		0x08
#define MODE_FILTER_MASK	0x10

// LED2 PCA9535BS
#define LED_LOCK_FLASH_MS		500
#define LED_9535BS_ADDR			0x42
#define LED_9535BS_REG_IN0		0x00
#define LED_9535BS_REG_IN1		0x01
#define LED_9535BS_REG_OUT0		0x02
#define LED_9535BS_REG_OUT1		0x03
#define LED_9535BS_REG_POL0		0x04
#define LED_9535BS_REG_POL1		0x05
#define LED_9535BS_REG_CFG0		0x06
#define LED_9535BS_REG_CFG1		0x07
#define LED_9535BS_CFG		    0x00
#define LED_9535BS_POL		    0x00

#define LED_ALL_OFF	    	0xFF
#define LED_ALL_ON			0x00

#define LED_VOL_MUTE		0xEF
#define LED_VOL_L1			0x9F
#define LED_VOL_L2			0x93

#define LED_VOL_L3_1		0x92
#define LED_VOL_L3_2		0x7F
#define LED_VOL_L4_1		0x92
#define LED_VOL_L4_2		0x4F
#define LED_VOL_L5_1		0x92
#define LED_VOL_L5_2		0x49

#define LED_NEXT_PRE		0xED

#define LED_IN_XMOS1		0xCD
#define LED_IN_XMOS2		0xB6
#define LED_IN_SPDIF1		0xE9
#define LED_IN_SPDIF2		0xB6
#define LED_IN_EXT_I2S1		0xED
#define LED_IN_EXT_I2S2		0x36
#define LED_IN_VIM_I2S1		0xED
#define LED_IN_VIM_I2S2		0xA6
#define LED_IN_AUTO1		0xED
#define LED_IN_AUTO2		0xB4

#define LED_FILTER_A1		0xBD
#define LED_FILTER_A2		0xB6
#define LED_FILTER_B1		0xE7
#define LED_FILTER_B2		0xB6
#define LED_FILTER_C1		0xEC
#define LED_FILTER_C2		0xF6
#define LED_FILTER_D1		0xED
#define LED_FILTER_D2		0x9E
#define LED_FILTER_E1		0xED
#define LED_FILTER_E2		0xB3
#define LED_FILTER_G1		0xBD
#define LED_FILTER_G2		0xB3
#define LED_FILTER_H1		0xE7
#define LED_FILTER_H2		0xB3

#define LED_DSD64			0x8F
#define LED_DSD128			0xF1
#define LED_DSD256_1		0xFE
#define LED_DSD256_2		0x3F
#define LED_DSD512			0xC7

#define LED_PCM_X1			0xCF
#define LED_PCM_X2			0xF9
#define LED_PCM_X3			0x3F
#define LED_PCM_X4			0xE7
#define LED_PCM_X5			0xFC

#define LED_NEXT_PREV()			led_display_set(LED_ALL_OFF, LED_NEXT_PRE)
#define LED_INPUT_XMOS_I2S()	led_display_set(LED_IN_XMOS2, LED_IN_XMOS1)
#define LED_INPUT_DAC_SPDIF()	led_display_set(LED_IN_SPDIF2, LED_IN_SPDIF1)
#define LED_INPUT_EXT_I2S()		led_display_set(LED_IN_EXT_I2S2, LED_IN_EXT_I2S1)
#define LED_INPUT_VIM_I2S()		led_display_set(LED_IN_VIM_I2S2, LED_IN_VIM_I2S1)
#define LED_INPUT_AUTO()		led_display_set(LED_IN_AUTO2, LED_IN_AUTO1)
#define LED_FILTER_MODE_A()		led_display_set(LED_FILTER_A2, LED_FILTER_A1)
#define LED_FILTER_MODE_B()		led_display_set(LED_FILTER_B2, LED_FILTER_B1)
#define LED_FILTER_MODE_C()		led_display_set(LED_FILTER_C2, LED_FILTER_C1)
#define LED_FILTER_MODE_D()		led_display_set(LED_FILTER_D2, LED_FILTER_D1)
#define LED_FILTER_MODE_E()		led_display_set(LED_FILTER_E2, LED_FILTER_E1)
#define LED_FILTER_MODE_G()		led_display_set(LED_FILTER_G2, LED_FILTER_G1)
#define LED_FILTER_MODE_H()		led_display_set(LED_FILTER_H2, LED_FILTER_H1)
#define LED_DSD_64()			led_display_set(LED_ALL_OFF, LED_DSD64)
#define LED_DSD_128()			led_display_set(LED_ALL_OFF, LED_DSD128)
#define LED_DSD_256()			led_display_set(LED_DSD256_2, LED_DSD256_1)
#define LED_DSD_512()			led_display_set(LED_DSD512, LED_ALL_OFF)
#define LED_PCM_X1_441_48()		led_display_set(LED_ALL_OFF, LED_PCM_X1)
#define LED_PCM_X2_882_96()		led_display_set(LED_ALL_OFF, LED_PCM_X2)
#define LED_PCM_X3_1764_192()	led_display_set(LED_PCM_X3, LED_ALL_OFF)
#define LED_PCM_X4_3524_384()	led_display_set(LED_PCM_X4, LED_ALL_OFF)
#define LED_PCM_X5_7056_768()	led_display_set(LED_PCM_X5, LED_ALL_OFF)

// LED VOL DISPLAY LEVEL
#define VOL_LEVEL_LED0			0
#define VOL_LEVEL_LED1			6
#define VOL_LEVEL_LED2			12
#define VOL_LEVEL_LED3			18
#define VOL_LEVEL_LED4			25
#define VOL_LEVEL_LED5			32

#define VOL_INIT_FINISHED		0
#define VOL_INIT_REL_KEY		1
#define VOL_INIT_ING			2

// SYS STATUS
#define SYS_STATUS_PLAY			0x10
#define SYS_STATUS_STOP 		0x20
#define SYS_STATUS_LOCK 		0x30
#define SYS_STATUS_OFF			0x40

// OPERATION MODE
#define OP_MODE_SELECT	    	0x50
#define OP_MODE_VOL	    		0x51
#define OP_MODE_NP		    	0x52
#define OP_MODE_INPUT 	    	0x53
#define OP_MODE_FILTER			0x54

// KNOB STATUS
#define KNOB_PREV				0x00
#define KNOB_NEXT				0x01
#define KNOB_NG               	0x02

// OPTIMIZATION  PARAMETER
#define KNOB_EVENT_TIMEOUT      500
#define KNOB_IGNORE_TIMEOUT_MS  500
#define NP_CONTINUE_TIMEOUT_MS 	200
#define KNOB_EVENT_COUNT	    0x02
#define UART_DATA_SEND_TIMEOUT 	255
#define UART_SEND_DELAY 		1500
#define UART_HIST_RESEND_DELAY 	3000
#define SENDING_LOCK_STATUS_DELAY 	2500
#define KNOB_OPERATE_IGNORE_MS 	200


///////////////////////////////////////////////////////////////////////////////////////////
// UART PARAMETER
bool uart_msg_ready = FALSE;
u8 uart_cmd_temp    = 0x00;
u8 uart_data_temp   = 0x00;
u16 uart_send_delay = 0x00;
u8 xmos_send_delay  = 0x00;
u16 uart_hist_resend_delay = 0x00;
u8 uart_cmd_flag    = 0;
u8 uart_cmd_sn_flag = 0;
u8 uart_data_buff[4]={0x00, 0x00, 0x00, 0x00};

// KEY & KNOB PARAMETER
#define KEY_SHAKE_TIME 			10
#define KNOB_SHAKE_TIME 		5
#define KNOB_OPPOSITE_TIME		50	// TOBE OPT
#define KEY_PRESS_TIME_PWROFF	4000
#define KEY_PRESS_TIME_SHORT  	20
#define KEY_PRESS_TIME_DOUBLE	300
#define KEY_KNOB_LOCK_MS		500

BitStatus knob_pin5 = RESET;
BitStatus knob_pin6 = RESET;
BitStatus hist_knob_pin6 = RESET;
bool sys_key_en 	= FALSE;
bool user_key_en 	= TRUE;
bool sys_knob_en 	= FALSE;
bool sys_key_press 	= FALSE;
bool knob_change 	= FALSE;
volatile u16 sys_key_shake_ms = 0;
volatile u16 sys_knob_shake_ms = 0;
volatile u16 sys_knob_opposite_ms = 0;
volatile u16 sys_key_press_ms = 0;
volatile u16 sys_key_release_ms = 0;
volatile u8 key_press_count = 0;
volatile u16 bt_pair_clear = 0;
volatile u8 knob_data  = KNOB_NG;
u8 hist_knob_data  = KNOB_NG;
volatile u8 shake_time_count = 0;
volatile u8 np_cont_ms = 0;
volatile u8 lock_flash_count = 0;
volatile u16 lock_flash_ms = LED_LOCK_FLASH_MS;
volatile u16 key_knob_lock_ms = 0;
volatile u8 knob_operate_ignore_ms = 0;

// VOL PARAMETER
bool sys_vol_change = FALSE;
u8 sys_vol_data = 0;
u8 sys_vol_init_flag = VOL_INIT_FINISHED;
u8 sys_vol_data_temp = 0;
u8 sys_dac_filter = DAC_FILTER_E;
u8 dac_filter[8]={0x00, 0x20, 0x40, 0x60, 0x80, 0xA0, 0xC0, 0xE0};
u8 sys_hid_vol_chang_en = 0;

#define VOL_STEP_COUNT	33
u8 VOL_REG[VOL_STEP_COUNT]={
0xFF, 0x82, 0x76, 0x6A, 0x60, 0x56, 0x4E, 0x46, 0x40, 0x3A, 0x34, 0x30, 0x2C, 0x28, 0x24, 0x21, 0x1E, 0x1B, 0x18, 0x15, 0x12, 0x10, 0x0E, 0x0C, 0x0A, 0x08, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
};

// TIME4 PARAMETER
u16 tim4_ms_cnt    = 0;
volatile u16 tim4_delay_ms  = 0;
volatile u8 tim4_uart_delay_ms  = 0;

// SYS STATUS PARAMETER
u8 sys_status = SYS_STATUS_PLAY;
u8 sys_op_mode = OP_MODE_VOL;
u8 sys_status_np_flag    = UD_NP_NG;
u8 sys_input_channel = INPUT_XMOS_I2S;
u8 sys_input_actual = 0;
u8 hist_input_channel = 0;
u8 sys_np_config = UD_NP_NG;
volatile u8 sys_xmos_status = UD_XMOS_STOP;
volatile u8 sys_xmos_status_temp = UD_XMOS_STOP;
u8 sys_mode_en = UD_MODE_DEF;

// DAC:ES9038Q2M INIT REG
u8 DAC_REG[18][2]={
{0x0F,0xFF},	// volume1 // Mute
{0x10,0xFF},	// volume2 // Mute
{0x00,0xF0},
{0x01,0xC1},    // SPDIF mode
{0x08,0x98},
{0x0A,0x02},
{0x0B,0x30},
{0x15,0x50},
{0x09,0x22},
{0x0C,0x6F},	//5F
{0x0D,0x00},
{0x0E,0x8A},
{0x14,0x70},
{0x16,0x02},    // TH+N
{0x17,0x00},    // TH+N
{0x18,0xD6},    // TH+N
{0x19,0xFF},    // TH+N
{0x01,0xC4},    // I2S DSD mode
};

// dac filter reg init
static const u8 foeffs_stage[9][3] = {
{0x1B, 0x1C, 0x4E},
{0x53, 0x78, 0x43},
{0x5B, 0x17, 0x4C},
{0x2A, 0xF7, 0x5D},
{0x0A, 0x64, 0x62},
{0x01, 0x90, 0x60},
{0xFF, 0x68, 0x04},
{0x77, 0xCA, 0x8D},
{0x77, 0xCA, 0x8D},
};


void mcu_init(void);
void init_vol_gently();
void audio_init(void);
bool dac_init(void);
void dac_filter_init(void);
//void mqa_rendering_en(void);
//void mqa_rendering_disable(void);
void sys_volume_ctrl(void);
void operation_mode_change(void);
void input_source_select();
void work_thread(void);
void standby_thread(void);
void sys_knob_detect_thread(void);
void sys_knob_process_thread(void);
void sys_key_detect_thread(void);
void sys_key_process_thread(void);
void sys_op_mode_mask(void);
void uart_cmd_respone(u8 cmd[4]);
void eeprom_write_byte(u16 dLocal_Addr,u8 dLocal_Data);
u8 eeprom_read_byte(u16 dLocal_Addr);
void sys_led_init(void);
void led_display_set(u8 data_ls0, u8 data_ls1);
void led_analog_vol_level(u8 sys_vol_data);
void led_input_source(u8 sys_input_channel);
void led_filter_mode(u8 sys_filter_mode);
void led_music_quality(u8 uart_cmd_temp, u8 uart_data_temp);


#endif /* __MAIN_H__ */

