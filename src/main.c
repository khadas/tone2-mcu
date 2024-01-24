// SPDX-License-Identifier: GPL-2.0
#include "main.h"
#define KHADAS_TEST 0
#define XMOS_OTA_EN 1

int main(void)
{
	mcu_init();

	audio_init();

	while (1) {
		if (sys_status == SYS_STATUS_OFF) {
			standby_thread();

		} else {
			work_thread();
		}
	}
}


void mcu_init(void)
{
	// Internal 16M clock
	hsi_prescaler_set();
	set_irq_priority();
	init_gpio();
	init_tim4_for_timer();
	init_uart();
	enableInterrupts();

}

void init_vol_gently(void)
{
	if (sys_vol_data > 0) {
		sys_vol_data_temp = sys_vol_data;
		sys_vol_data = 0;
		sys_vol_init_flag = VOL_INIT_ING;

	} else {
		knob_change = TRUE;
	}
}

void sys_pwron_thread(void)
{
	// set UART to XMOS to init XMOS
	DAC_I2S_SET0_H();
	DAC_I2S_SET1_L();

	//
	SYS_POWER_ON();
	tim4_delay_ms = 45;
	while(tim4_delay_ms){};
	SYS_NRST_ON();

	// return T2 SN if XMOS ask
	tim4_delay_ms = 500;
	while (tim4_delay_ms) {
		if (uart_msg_ready == TRUE) {
			uart_cmd_respone(uart_data_buff);
			uart_msg_ready = FALSE;
			break;
		}
	}
#if KHADAS_TEST
		// Not Check Upgrade While Test
#else

#if XMOS_OTA_EN
	// check MCU version in XMOS(check upgrade)
	uart_sent_cmd(UC_VER_XMOS, UD_VER_QUERY);
	tim4_delay_ms = 500;
	while (tim4_delay_ms) {
		if (uart_msg_ready == TRUE) {
			uart_cmd_respone(uart_data_buff);
			uart_msg_ready = FALSE;
			break;
		}
	}
#endif

#endif

	// Detect T2V Module Insert
	if (FALSE == i2c_detect(I2C_T2V_ADDR)) {
		T2V_NG_L();

	} else {
		T2V_OK_H();
	}

}

void audio_init(void)
{
	// disable KEY & KNOB
	sys_key_en 	= FALSE;
	sys_knob_en = FALSE;
	key_knob_lock_ms = 0;

	sys_pwron_thread();

	sys_led_init();

	dac_init();

	// load sys config
	sys_dac_filter = eeprom_read_byte(E2P_ADD_FILTER);
	i2c_write(I2C_DAC_ADDR, DAC_REG_FILTER, dac_filter[sys_dac_filter - 1]);

	sys_input_actual = 0;
	sys_input_channel = eeprom_read_byte(E2P_ADD_INPUT);
	hist_input_channel = sys_input_channel;

#if KHADAS_TEST
	sys_vol_data = 0x20;
	knob_change = TRUE;

#else
	sys_vol_data = eeprom_read_byte(E2P_ADD_VOL);
	init_vol_gently();

#endif

	// Choose One Operation Which Not Mask
	sys_op_mode = OP_MODE_VOL;
	sys_mode_en = eeprom_read_byte(E2P_ADD_MODE_EN);
	sys_op_mode_mask();

	// Reload sys_time_stop & sys_time_off
	//sys_time_stop = eeprom_read_byte(E2P_ADD_TIME_OFF);
	//sys_time_off = 0xFF;

	// Detect Key Press to Clear BT Pair
	if (FUN_KEY_IN() == RESET) {
		bt_pair_clear = KEY_PRESS_TIME_PWROFF;
	}

	SYS_MUTE_OFF();

#if KHADAS_TEST
	// Flash 3 Times White LED
	lock_flash_count = 8;
	while(lock_flash_count >0){
		switch(lock_flash_count){
		case 2:
			lock_flash_count = 1;
		case 8:
		case 6:
		case 4:
			led_display_set(LED_ALL_OFF, LED_ALL_OFF);
			break;

		case 7:
		case 5:
		case 3:
			led_display_set(LED_ALL_ON, LED_ALL_ON);
			break;

		case 1:
			// reset history led status
			if(sys_hid_vol_chang_en == 1){
				sys_hid_vol_chang_en = 2;
			}
			operation_mode_change();
			break;

		default:
			break;
		}
	}

#endif

}

bool dac_init(void)
{
	u8 i  = 0;
	u8 status = FALSE;

	// Init DAC: input source, vol, THD+N parameter...
	for (i = 0; i < 18; i++) {
		if (i > 12 && i < 17) {
			DAC_REG[i][1] = eeprom_read_byte((E2P_ADD_DAC16 + i - 13));
		}

		status = i2c_write(I2C_DAC_ADDR, DAC_REG[i][0], DAC_REG[i][1]);
		if (0x01 != status) {
			return FALSE;
		}
	}

	dac_filter_init();

	return TRUE;

}

void dac_filter_init(void)
{
	u8 i = 0;
	u8 j = 0;
	u8 temp = 0;

	for (i = 0; i < 144; i++) {
		for (j = 40; j < 45; j++) {
			switch (j) {
			case 40:
				temp = i;
				break;

			case 41:
			case 42:
			case 43:
				if (i < 25) {
					temp = 0;
				}
				if (i > 24 && i < 32) {
					temp = foeffs_stage[i - 25][j - 41];
				}
				if (i > 31 && i < 140) {
					temp = 0;
				}
				if (i > 139 && i < 142) {
					temp=foeffs_stage[i - 133][j - 41];
				}
				if (i > 141) {
					temp = 0;
				}
				break;

			case 44:
				temp = 2;
				break;

			default:
				break;
			}

			i2c_write(I2C_DAC_ADDR, j, temp);
			}
	}

}
#if 0
void mqa_rendering_en(void)
{
	i2c_write(I2C_DAC_ADDR, DAC_REG_FILTER,  0x00);	// LINEAR FAST
	i2c_write(I2C_DAC_ADDR, DAC_REG_FIR_CFG, 0x05);	// enable custom filter with stage 2 even

}

void mqa_rendering_disable(void)
{
	i2c_write(I2C_DAC_ADDR, DAC_REG_FIR_CFG, 0x00);	// enable custom filter with stage 2 even
	i2c_write(I2C_DAC_ADDR, DAC_REG_FILTER, dac_filter[(sys_dac_filter-1)]);	// Reload DAC Filter
}
#endif
void sys_volume_ctrl(void)
{
	uart_sent_cmd(UC_VOL, (sys_vol_data + 1));
	i2c_write( I2C_DAC_ADDR, DAC_REG_VOL_L, VOL_REG[sys_vol_data]);
	i2c_write( I2C_DAC_ADDR, DAC_REG_VOL_R, VOL_REG[sys_vol_data]);
	led_analog_vol_level(sys_vol_data);
}

void operation_mode_change(void)
{
	switch(sys_op_mode){
	case OP_MODE_VOL:
		led_analog_vol_level(sys_vol_data);
		break;

	case OP_MODE_NP:
		LED_NEXT_PREV();
		break;

	case OP_MODE_INPUT:
		led_input_source(sys_input_channel);
		break;

	case OP_MODE_FILTER:
		led_filter_mode(sys_dac_filter);
		break;

	default:
		break;
	}
}

void input_source_select(void)
{
	u8 channel = 0;

	// [1] get actual input channel
	if (sys_input_channel == INPUT_AUTO_CH) {
		if (VIM_I2S_DETECT()  != RESET) {
			channel = INPUT_VIM_I2S;
#if KHADAS_TEST
		} else if (EXT_I2S_DETECT()  != RESET) {
			channel = INPUT_EXT_I2S;
#endif

		} else if (sys_xmos_status != UD_XMOS_STOP) {
			channel = INPUT_XMOS_I2S;
		} else {
			channel = INPUT_DAC_SPDIF;
		}
	} else {
		channel = sys_input_channel;
	}

	// [2] not modify at the same mode
	if (sys_input_actual != channel) {
		sys_input_actual = channel;

		switch (channel) {
		case INPUT_XMOS_I2S:
			i2c_write(I2C_DAC_ADDR, DAC_REG_INPUT, DAC_INPUT_I2S_DSD);
			DAC_I2S_SET0_H();
			DAC_I2S_SET1_L();
			break;

		case INPUT_DAC_SPDIF:
			i2c_write(I2C_DAC_ADDR, DAC_REG_INPUT, DAC_INPUT_SPDIF);
			DAC_I2S_SET0_H();
			DAC_I2S_SET1_L();
			break;

		case INPUT_EXT_I2S:
			i2c_write(I2C_DAC_ADDR, DAC_REG_INPUT, DAC_INPUT_I2S_DSD);
			DAC_I2S_SET0_L();
			DAC_I2S_SET1_L();
			break;

		case INPUT_VIM_I2S:
			i2c_write(I2C_DAC_ADDR, DAC_REG_INPUT, DAC_INPUT_I2S_DSD);
			DAC_I2S_SET0_H();
			DAC_I2S_SET1_H();
			break;

		default:
			break;
		}
		uart_sent_cmd(UC_INPUT, sys_input_channel);     // notify to all device
		uart_sent_cmd(UC_MCU_AUTOI, sys_input_actual);	// notify to XMOS not mute while not XMOS input source
	}

	// [3] save input change
	if (hist_input_channel != sys_input_channel) {
		hist_input_channel = sys_input_channel;
		eeprom_write_byte(E2P_ADD_INPUT, sys_input_channel);

		if (sys_op_mode == OP_MODE_INPUT) {
			led_input_source(sys_input_channel);
		}
	}
}

void work_thread(void)
{
	u8 hist_sys_status = sys_status;
	u8 hist_sys_op_mode = sys_op_mode;
	u8 hist_filter_mode = sys_dac_filter;
	u8 hist_lock_flash_count = lock_flash_count;
	u8 led_data_temp1, led_data_temp2;

	sys_knob_en = TRUE;

	while (hist_sys_status == sys_status) {

		// send pairing clean cmd to BTMagic
		if (bt_pair_clear == 1) {
			bt_pair_clear = 0;
			if (FUN_KEY_IN() == RESET) {
				uart_sent_cmd(UC_PAIR_CLEAN, UD_PAIR_CLEAN);
			}
		}

		// show knob mode change by led
		if ((hist_sys_op_mode != sys_op_mode) || (uart_hist_resend_delay==1)) {
			hist_sys_op_mode = sys_op_mode;

			// redisplay sys led status, after pcm/dsd music source display
			if (uart_hist_resend_delay == 1) {
				uart_hist_resend_delay = 0;
			}
			operation_mode_change();
		}

                // knob operation respone
		switch (sys_op_mode) {
		case OP_MODE_VOL:
			if (knob_change == TRUE) {
				knob_change = FALSE;
				sys_vol_init_flag = VOL_INIT_REL_KEY;

				if (sys_vol_data < VOL_STEP_COUNT) {
					sys_hid_vol_chang_en = 0;
					sys_volume_ctrl();
					eeprom_write_byte(E2P_ADD_VOL, sys_vol_data);
					lock_flash_count = 1;

				} else {
					// flash LED 3 times, and then vol up HID vol
					if (sys_hid_vol_chang_en == 0) {
						sys_hid_vol_chang_en = 1;
						lock_flash_count = 8;

					// flash 3 time led finshed
					}else if(sys_hid_vol_chang_en == 2){

						if (sys_input_actual == INPUT_XMOS_I2S) {
							uart_sent_cmd(UC_VOL, UD_XDVOL_UP);	// for xmos

						} else if (sys_input_actual == INPUT_EXT_I2S) {
							uart_sent_cmd(UC_VOL, UD_DVOL_UP);	// for btmagic
						}
					}
				}
			}
			break;

		case OP_MODE_NP:
			if ((sys_np_config != UD_NP_NG) && (np_cont_ms == 0)) {

				if (sys_input_actual == INPUT_XMOS_I2S) {
					uart_sent_cmd(UC_NP, (sys_np_config + 2));

				} else if (sys_input_actual == INPUT_EXT_I2S) {
					uart_sent_cmd(UC_NP, sys_np_config);
				}

				sys_np_config = UD_NP_NG;
				np_cont_ms = NP_CONTINUE_TIMEOUT_MS;
			}
			break;

		case OP_MODE_FILTER:
			if (hist_filter_mode != sys_dac_filter) {
				uart_sent_cmd(UC_FILTER, sys_dac_filter);	// for multichannel-audio-module
				hist_filter_mode = sys_dac_filter;
				eeprom_write_byte(E2P_ADD_FILTER, sys_dac_filter);
				i2c_write(I2C_DAC_ADDR, 0x07, dac_filter[sys_dac_filter-1]);
				led_filter_mode(sys_dac_filter);
			}
			break;


		default:
			break;
        }

		// input source change
		if (xmos_send_delay == 1) {
			sys_xmos_status = sys_xmos_status_temp;
			xmos_send_delay = 0;
		}
		input_source_select();


		// init volume value gentlely
		if ((sys_vol_init_flag > VOL_INIT_REL_KEY) && (sys_vol_change != FALSE)) {
			sys_vol_change = FALSE;
			if (sys_vol_data < VOL_STEP_COUNT) {
				sys_volume_ctrl();
			} else {
				operation_mode_change();
			}
		}

		// release key after init finished
		if (sys_vol_init_flag == VOL_INIT_REL_KEY) {
			user_key_en = TRUE;
			sys_key_en 	= TRUE;
			sys_vol_init_flag = VOL_INIT_FINISHED;
		}

		// audio format tobe handled
		if (uart_send_delay == 1) {
			if (sys_input_actual == INPUT_XMOS_I2S) {
#if 0	// not support mqa format, and the mcu flash was no space, so reduce this code
				uart_sent_cmd(uart_cmd_temp, uart_data_temp);	// For Multichannel sync rendering setting

				// diable/enable mqa rendering depen on music format sent from XMOS
				if(uart_cmd_temp== UC_DATA){
					mqa_rendering_disable();

				}else if(uart_cmd_temp== UC_DATA_MQA){
					mqa_rendering_en();
				}

				// display led show the last music format
				//uart_sent_cmd(uart_cmd_temp, uart_data_temp);
#endif
				led_music_quality(uart_cmd_temp, uart_data_temp);
			}
			uart_send_delay=0;

			// delay to reload LED display
			uart_hist_resend_delay = UART_HIST_RESEND_DELAY;
		}

		// uart data tobe handle
		if (uart_msg_ready == TRUE) {
			uart_cmd_respone(uart_data_buff);
			uart_msg_ready = FALSE;
		}

		// flash led 3 times, all led white
		if (hist_lock_flash_count != lock_flash_count) {
			hist_lock_flash_count = lock_flash_count;

			switch (lock_flash_count) {
			case 2:
				lock_flash_count = 1;
			case 8:
			case 6:
			case 4:
				led_display_set(LED_ALL_OFF, LED_ALL_OFF);
				break;

			case 7:
			case 5:
			case 3:
				if (sys_hid_vol_chang_en == 1) {
					led_data_temp1 = LED_VOL_L5_2;
					led_data_temp2 = LED_VOL_L5_1;

				} else if (user_key_en != TRUE) {
					led_data_temp1 = LED_ALL_ON;
					led_data_temp2 = LED_ALL_ON;
				}

				led_display_set(led_data_temp1, led_data_temp2);
				break;

			case 1:
				// reset led history status
				if (sys_hid_vol_chang_en == 1) {
					sys_hid_vol_chang_en = 2;
				}
				operation_mode_change();
				break;

			default:
				break;
			}
		}

    }//////////////////////////////////////////////////////

	if (sys_status == SYS_STATUS_PLAY) {
		if (sys_input_actual == INPUT_XMOS_I2S) {
			uart_sent_cmd(UC_PLAY, UD_XPLAY);

		} else if (sys_input_actual == INPUT_EXT_I2S) {
			uart_sent_cmd(UC_PLAY, UD_PLAY);
		}

	} else if(sys_status == SYS_STATUS_STOP) {
		if (sys_input_actual == INPUT_XMOS_I2S) {
			uart_sent_cmd(UC_PLAY, UD_XSTOP);

		} else if (sys_input_actual == INPUT_EXT_I2S) {
			uart_sent_cmd(UC_PLAY, UD_STOP);
		}
	}
}

void standby_thread(void)
{
#if 0	// for multichannel-audio-module test
	while(1){
		tim4_delay_ms = 5000;	while(tim4_delay_ms){};
		uart_sent_cmd(UC_MUL_GPIO, UD_PCM_DSD_L);
		uart_sent_cmd(UC_MUL_GPIO, UD_MCLK_SEL_H);
		tim4_delay_ms = 5000;	while(tim4_delay_ms){};
		uart_sent_cmd(UC_MUL_GPIO, UD_PCM_DSD_H);
		uart_sent_cmd(UC_MUL_GPIO, UD_MCLK_SEL_L);
		tim4_delay_ms = 5000;	while(tim4_delay_ms){};
		uart_sent_cmd(UC_MODULE, UD_MODULE_QUERY);
		tim4_delay_ms = 50;	while(tim4_delay_ms){};
		uart_sent_cmd(UC_VER_LOCAL, UC_VER_LOCAL);
		tim4_delay_ms = 50;	while(tim4_delay_ms){};
		uart_sent_cmd(UC_VOL, UD_AVOL_QUERY);
		tim4_delay_ms = 50;	while(tim4_delay_ms){};
		uart_sent_cmd(UC_FILTER, UD_FILTER_QUERY);
	}
#endif

	// disable key & knob
	sys_key_en 	= FALSE;
	sys_knob_en = FALSE;

	// notify power off for all device
	SYS_MUTE_ON();

	// shutdown xmos power
	led_display_set(LED_ALL_OFF, LED_ALL_OFF);
	SYS_POWER_OFF();
	SYS_NRST_OFF();

	// wait key release
	while (1) {
		if (FUN_KEY_IN() != RESET) {
			tim4_delay_ms = 500;
			while(tim4_delay_ms){};

			if (FUN_KEY_IN() != RESET) {
				sys_key_press = FALSE;
				sys_key_en = TRUE;
				break;
			}
		}
	}

	// ####### wait for status change, key event #####################//
	while (sys_status != SYS_STATUS_PLAY) {

	}

	// reinit audio
	audio_init();

}



void eeprom_write_byte(u16 dLocal_Addr,u8 dLocal_Data)
{
	flash_unlock();
	while (flash_get_flag_status() == RESET);

	flash_program_byte(FLASH_DATA_START_PHYSICAL_ADDRESS + dLocal_Addr,dLocal_Data);
	flash_lock();

	return;
}

u8 eeprom_read_byte(u16 dLocal_Addr)
{
	u8 dLocal_1;
	dLocal_1 = flash_read_byte(FLASH_DATA_START_PHYSICAL_ADDRESS + dLocal_Addr);

	return dLocal_1;
}

void sys_knob_process_thread(void)
{
	// [1] ignore while knob disable
	if (sys_knob_en == FALSE) {
		sys_knob_shake_ms = 0;
		return;
	}

	// [2] knob shake ignore
	if (sys_knob_shake_ms != 1) {
		return;
	}

	// [3] detect key change
	knob_pin6 = KNOB_B_IN();
	knob_pin5 = KNOB_A_IN();
	if (knob_pin5 == knob_pin6) {
		knob_data = KNOB_PREV;

	}else if ((knob_pin5 != RESET) &&  (knob_pin6 != RESET) ) {
		knob_data = KNOB_PREV;

	} else {
		knob_data = KNOB_NEXT;
	}

	// [4] start next int detect
	sys_knob_shake_ms = 0;

	// [5] ignore opposite shake & first touch shake
	if (hist_knob_data == KNOB_NG) {
		hist_knob_data = knob_data;
	}

	if (hist_knob_data != knob_data) {
		if (sys_knob_opposite_ms > 0) {
			shake_time_count++;

			if (shake_time_count == 1) {
				knob_data = hist_knob_data;

			} else if (shake_time_count == 2) {
				knob_data = KNOB_NG;

			} else {
				shake_time_count = 0;
			}
		// first touch shake
		} else {
			knob_data = KNOB_NG;
		}

		hist_knob_data = knob_data;
	} else {
		shake_time_count = 0;
	}
	sys_knob_opposite_ms = KNOB_OPPOSITE_TIME;

#if 0	// [6] test log
	if(knob_data == KNOB_PREV){
		uart_send_data8(0x00);

	}else if(knob_data == KNOB_NEXT){
		uart_send_data8(0x11);

	}else{
		uart_send_data8(0x22);
	}
	return;
#endif

	// [7] ignore some operation
	if (sys_op_mode != OP_MODE_VOL) {
		if (knob_operate_ignore_ms != 0) {
			return;
		}

		if (knob_data != KNOB_NG) {
			knob_operate_ignore_ms = KNOB_OPERATE_IGNORE_MS;
		}
	}

	// [8] process knob msg
	switch (sys_op_mode) {
	case OP_MODE_VOL:
		if (knob_data == KNOB_PREV) {
			if (sys_vol_data > 0) {
				sys_vol_data--;
			}
			knob_change = TRUE;

		} else if (knob_data == KNOB_NEXT) {
			if (sys_vol_data < VOL_STEP_COUNT ) {
				sys_vol_data++;
			}
			knob_change = TRUE;
		}
		break;

	case OP_MODE_NP:
		if (knob_data == KNOB_PREV) {
			sys_np_config = UD_PREV;

		} else if (knob_data == KNOB_NEXT) {
			sys_np_config = UD_NEXT;
		}
		break;

	case OP_MODE_INPUT:
		if (knob_data == KNOB_PREV) {
			if (sys_input_channel > INPUT_XMOS_I2S) {
				sys_input_channel--;
			}

		} else if (knob_data == KNOB_NEXT) {
			if (sys_input_channel < INPUT_AUTO_CH) {
				sys_input_channel++;
			}
		}
		break;

	case OP_MODE_FILTER:
		if (knob_data == KNOB_PREV) {
			if (sys_dac_filter > DAC_FILTER_A) {
				sys_dac_filter--;
			}

			if (sys_dac_filter == DAC_FILTER_F) {
				sys_dac_filter--;
			}

		} else if (knob_data == KNOB_NEXT) {
			if (sys_dac_filter < DAC_FILTER_H) {
				sys_dac_filter++;
			}

			if (sys_dac_filter == DAC_FILTER_F) {
				sys_dac_filter++;
			}
		}
		break;

	default:
		break;
	}
}

void sys_key_detect_thread()
{
	// [1] key shake ignore
	if (sys_key_shake_ms != 1) {
		return;
	}

	// [2] detect key change
	if (FUN_KEY_IN() == RESET) {
		sys_key_press  = TRUE;

	} else {
		sys_key_press  = FALSE;
	}

	// [3] start next interrupt detect
	sys_key_shake_ms = 0;
}

void sys_key_process_thread()
{
	if (sys_key_en == FALSE) {
		sys_key_press_ms = 0;
		key_press_count = 0;
		sys_key_release_ms = 0;
		sys_key_press = FALSE;
		return;
	}

	sys_key_detect_thread();

	// ####### key status: long press, short press ####################//
	if (sys_key_press == TRUE) {

		sys_key_press_ms++;
		sys_key_release_ms = 0;

		// [1] status off & short press: pwr on
		if (sys_status == SYS_STATUS_OFF) {
			sys_status = SYS_STATUS_PLAY;
			sys_key_press_ms = 0;
			key_press_count  = 0;
		}

		// [2] status on & long press: pwr off
		if (sys_key_press_ms > KEY_PRESS_TIME_PWROFF) {
			// key lock, flash led 3 times
			if (user_key_en == FALSE) {
				sys_key_press_ms = 0;
				key_press_count = 0;
				sys_key_release_ms = 0;

				if (lock_flash_count == 0) {
					lock_flash_count = 8;
				}
				return;
			}

			sys_status = SYS_STATUS_OFF;
			sys_key_en = FALSE;
			sys_knob_en = FALSE;
			sys_key_press_ms = 0;
			key_press_count = 0;
		}

	// ####### key status: single press, double press, 3 press  ####//
	} else {
		sys_key_release_ms++;
		if (sys_key_press_ms > KEY_PRESS_TIME_SHORT) {
			key_press_count++;
		}
		sys_key_press_ms = 0;

		// process short press counter
		if (sys_key_release_ms > KEY_PRESS_TIME_DOUBLE) {

			switch (key_press_count) {
			// [3] single press: play/stop
			case 1:
				if (user_key_en == TRUE) {
					if (sys_status == SYS_STATUS_PLAY) {
						sys_status = SYS_STATUS_STOP;

					} else if (sys_status == SYS_STATUS_STOP) {
						sys_status = SYS_STATUS_PLAY;
					}

				} else if (lock_flash_count == 0) {
					lock_flash_count = 8;
				}
				break;

			// [4] double press: mode switch
			case 2:
				knob_change = FALSE;

				if (user_key_en == TRUE) {
					sys_op_mode++;
					sys_op_mode_mask();

				} else if (lock_flash_count == 0) {
					lock_flash_count = 8;
				}
				break;

			// [5] 3 or more press: lock/unlock
			default:
				if (key_press_count > 2) {
					if (user_key_en == FALSE) {
						user_key_en = TRUE;
					} else {
						user_key_en = FALSE;
						lock_flash_count = 8;
					}
				}
				break;
			}

			key_press_count	 = 0;
		}

	}

}

void sys_op_mode_mask(void)
{
	// choose one operation which not mask
	while (1) {
		if (sys_op_mode == OP_MODE_VOL) {
			break;
		}
		if (sys_op_mode == OP_MODE_NP) {
			if (!(sys_mode_en & MODE_NP_MASK)) {
				sys_op_mode++;
			} else {
				break;
			}
		}
		if (sys_op_mode == OP_MODE_INPUT) {
			if (!(sys_mode_en & MODE_INPUT_MASK)) {
				sys_op_mode++;
			} else {
				break;
			}
		}
		if (sys_op_mode == OP_MODE_FILTER) {
			if (!(sys_mode_en & MODE_FILTER_MASK)) {
				sys_op_mode = OP_MODE_VOL;
			} else {
				break;
			}
		}
		if (sys_op_mode > OP_MODE_FILTER) {
			sys_op_mode = OP_MODE_VOL;
			break;
		}
	}
}

void uart_cmd_respone(u8 cmd[4])
{
	u8 i = 0;

	// write sn
	if (uart_cmd_sn_flag > 6) {
		for (i = 0; i < 4; i++) {
			eeprom_write_byte((E2P_ADD_SN12 + i), cmd[i]);	// E2P_ADD_SN12(0x10)
			uart_sent_cmd((UC_SN12 + i), cmd[i]);
		}
		uart_cmd_sn_flag = 0;
		return;

	// read sn
	} else if (uart_cmd_sn_flag == 3) {
		for (i = 0; i < 4; i++) {
			tim4_uart_delay_ms = 0;
			cmd[i] = eeprom_read_byte((E2P_ADD_SN12 + i));	// E2P_ADD_SN12(0x10)
			uart_sent_cmd((UC_SN12 + i), cmd[i]);
		}
		uart_cmd_sn_flag = 0;
		return;
	}

	switch (cmd[1]) {

	case UC_PLAY:
		switch(cmd[2]){
		// get btm msg and forword uart protocol to xmos
		case UD_PLAY:
		case UD_STOP:
			if (sys_input_actual == INPUT_XMOS_I2S) {
				uart_sent_cmd(UC_PLAY, (cmd[2] + 4));
			} else if (sys_input_actual == INPUT_EXT_I2S) {
				uart_sent_cmd(UC_PLAY, cmd[2]);
			}
			break;

		// get xmos status and change input source while auto mode
		case UD_XMOS_PLAY:
		case UD_XMOS_STOP:
			xmos_send_delay = 0xFF;
			//sys_time_stop = eeprom_read_byte(E2P_ADD_TIME_OFF);	// reload timeout parameter
			sys_xmos_status_temp = cmd[2];
			break;

		default:
			break;
		}
		break;

	case UC_VOL:
		if (UD_AVOL_QUERY == cmd[2]) {
			uart_sent_cmd(UC_VOL, (sys_vol_data + 1));

		} else if (UD_AVOL_QUERY < cmd[2] && cmd[2] <= UD_AVOL_MAX) {
			sys_vol_data = (cmd[2] - 1);
			sys_op_mode = OP_MODE_VOL;
			knob_change = TRUE;
		}
	break;

	case UC_NP:
		switch (cmd[2]) {
		//	get btmagic msg and send uart protocol to xmos
		case UD_NEXT:
		case UD_PREV:
			if (sys_input_actual == INPUT_XMOS_I2S) {
				uart_sent_cmd(UC_NP, (cmd[2]) + 2);
			} else if (sys_input_actual == INPUT_EXT_I2S) {
				uart_sent_cmd(UC_NP, cmd[2]);
			}
			sys_op_mode = OP_MODE_NP;
			break;
		default:
			break;
		}
	break;

	case UC_INPUT:
		switch (cmd[2]) {
		//	get btmagic msg and send uart protocol to xmos
		case UD_INPUT_QUERY:
			uart_sent_cmd(UC_INPUT, sys_input_channel);
			break;
		case UD_INPUT_XMOS:
		case UD_INPUT_SPDIF:
		case UD_INPUT_EXT:
		case UD_INPUT_VIM:
		case UD_INPUT_AUTO:
			sys_input_channel = cmd[2];
			sys_op_mode = OP_MODE_INPUT;
			break;
		default:
			break;
		}
	break;

	case UC_FILTER:
		switch (cmd[2]) {
		case UD_FILTER_QUERY:
			uart_sent_cmd(UC_FILTER, sys_dac_filter);
			break;

		case UD_FILTER_A:
		case UD_FILTER_B:
		case UD_FILTER_C:
		case UD_FILTER_D:
		case UD_FILTER_E:
		case UD_FILTER_G:
		case UD_FILTER_H:
			sys_dac_filter = cmd[2];
			sys_op_mode = OP_MODE_FILTER;
			break;

		default:
			break;
		}
		break;

	case UC_DATA:
	case UC_DATA_MQA:
		// delay 1.5s to receive audio format msg, cause xmos will send it serval times, and the last one is right.
		uart_send_delay = UART_SEND_DELAY;
		uart_cmd_temp = cmd[1];
		uart_data_temp = cmd[2];
		break;

	case UC_DAC16:
	case UC_DAC17:
	case UC_DAC18:
	case UC_DAC19:
        eeprom_write_byte((cmd[1]-0x31), cmd[2]);		// 0x31 = UC_DAC19(0x55) - E2P_ADD_DAC19(0x24)
		i2c_write(I2C_DAC_ADDR, (cmd[1]-0x3C), cmd[2]);	// 0x3C = UC_DAC19(0x55) - DAC_REG_THD_C32(0x19)
		break;

	case UC_MODULE:
		if (UD_MODULE_QUERY == cmd[2]) {
			uart_sent_cmd(UC_MODULE, UD_MODULE_T2);
		}
		break;

	case UC_MODE_EN:
		if (UD_MODE_QUERY == cmd[2]) {
			uart_sent_cmd(UC_MODE_EN, sys_mode_en);
		} else if ((cmd[2] & 0x01) != 0) {
			sys_mode_en = cmd[2];
			eeprom_write_byte(E2P_ADD_MODE_EN, sys_mode_en);
			sys_op_mode_mask();
		}
		break;

#if 0	// remove this function, for no more mcu flash space to add
	case UC_VER_LOCAL:
		uart_sent_cmd(UC_VER_LOCAL, UD_VER_LOCAL);
	break;
#endif

	case UC_SN12:
	case UC_SN34:
	case UC_SN56:
	case UC_SN78:
		uart_sent_cmd(cmd[1], eeprom_read_byte((cmd[1] - 0x61)));// 0x61 = UC_SN78(0x74) - E2P_ADD_SN78(0x13)
		break;

#if XMOS_OTA_EN
	// check upgrade
	case UC_VER_XMOS:
		if (cmd[2] != UD_VER_LOCAL) {
			eeprom_write_byte(E2P_ADD_UPGRADE, DATA_UPGRADE);

			SYS_POWER_OFF();
			tim4_delay_ms =50; while(tim4_delay_ms){};
			WWDG->CR= 0x80;	// soft rest

		} else if (DATA_NOTUPGRADE != eeprom_read_byte(E2P_ADD_UPGRADE)) {
			eeprom_write_byte(E2P_ADD_UPGRADE, DATA_NOTUPGRADE);
		}
		break;
#endif

	default:
		break;
	}

}

///////////////////////////////////////////////////////////////////////////////////////////
void led_display_set(u8 data_ls0, u8 data_ls1)
{
	i2c_write(LED_9535BS_ADDR, LED_9535BS_REG_OUT0, data_ls0);
	i2c_write(LED_9535BS_ADDR, LED_9535BS_REG_OUT1, data_ls1);
}

void sys_led_init()
{
	i2c_write(LED_9535BS_ADDR, LED_9535BS_REG_OUT0, LED_ALL_OFF);
	i2c_write(LED_9535BS_ADDR, LED_9535BS_REG_OUT1, LED_ALL_OFF);
	i2c_write(LED_9535BS_ADDR, LED_9535BS_REG_POL0, LED_9535BS_POL);
	i2c_write(LED_9535BS_ADDR, LED_9535BS_REG_POL1, LED_9535BS_POL);
	i2c_write(LED_9535BS_ADDR, LED_9535BS_REG_CFG0, LED_9535BS_CFG);
	i2c_write(LED_9535BS_ADDR, LED_9535BS_REG_CFG1, LED_9535BS_CFG);
}

void led_analog_vol_level(u8 sys_vol_data)
{
	if (sys_vol_data > VOL_LEVEL_LED4) {
		led_display_set(LED_VOL_L5_2, LED_VOL_L5_1);

	} else if(sys_vol_data > VOL_LEVEL_LED3) {
		led_display_set(LED_VOL_L4_2, LED_VOL_L4_1);

	} else if (sys_vol_data > VOL_LEVEL_LED2) {
		led_display_set(LED_VOL_L3_2, LED_VOL_L3_1);

	} else if (sys_vol_data > VOL_LEVEL_LED1) {
		led_display_set(LED_ALL_OFF, LED_VOL_L2);

	} else if (sys_vol_data > VOL_LEVEL_LED0) {
		led_display_set(LED_ALL_OFF, LED_VOL_L1);

	} else if (sys_vol_data == VOL_LEVEL_LED0) {
		led_display_set(LED_ALL_OFF, LED_VOL_MUTE);
	}
}

void led_input_source(u8 sys_input_channel)
{
	switch (sys_input_channel) {
	case INPUT_XMOS_I2S:
		LED_INPUT_XMOS_I2S();
		break;

	case INPUT_DAC_SPDIF:
		LED_INPUT_DAC_SPDIF();
		break;

	case INPUT_EXT_I2S:
		LED_INPUT_EXT_I2S();
		break;

	case INPUT_VIM_I2S:
		LED_INPUT_VIM_I2S();
		break;

	case INPUT_AUTO_CH:
		LED_INPUT_AUTO();
		break;

	default:
		break;
	}
}

void led_filter_mode(u8 sys_filter_mode)
{
	switch (sys_filter_mode) {
	case DAC_FILTER_A:
		LED_FILTER_MODE_A();
		break;

	case DAC_FILTER_B:
		LED_FILTER_MODE_B();
		break;

	case DAC_FILTER_C:
		LED_FILTER_MODE_C();
		break;

	case DAC_FILTER_D:
		LED_FILTER_MODE_D();
		break;

	case DAC_FILTER_E:
		LED_FILTER_MODE_E();
		break;

	case DAC_FILTER_G:
		LED_FILTER_MODE_G();
		break;

	case DAC_FILTER_H:
		LED_FILTER_MODE_H();
		break;

	default:
		break;
	}

}

void led_music_quality(u8 uart_cmd_temp, u8 uart_data_temp)
{
	switch (uart_data_temp) {
	case 0x1:
		LED_PCM_X1_441_48();
		break;

	case 0x2:
		LED_PCM_X2_882_96();
		break;

	case 0x3:
		LED_PCM_X3_1764_192();
		break;

	case 0x4:
		LED_PCM_X4_3524_384();
		break;

	case 0x5:
		LED_PCM_X5_7056_768();
		break;

	case 0x11:
		LED_DSD_64();
		break;

	case 0x12:
		LED_DSD_128();
		break;

	case 0x13:
		LED_DSD_256();
		break;

	case 0x14:
		LED_DSD_512();
		break;

	default:
		break;
	}

}


///////////////////////////////////////////////////////////////////////////////////////////
// interrupt respone
#pragma vector=0x2
__interrupt void TLI_IRQHandler(void)
{

}

#pragma vector=0x3
__interrupt void AWU_IRQHandler(void)
{

}
#pragma vector=0x4
__interrupt void CLK_IRQHandler(void)
{

}

#pragma vector=0x5
__interrupt void EXTI_PORTA_IRQHandler(void)
{

}

#pragma vector=0x6
__interrupt void EXTI_PORTB_IRQHandler(void)
{

}

#pragma vector=0x7
__interrupt void EXTI_PORTC_IRQHandler(void)
{
	// [1] lock before init finished
	if (sys_knob_en == FALSE) {
		return;
	}

	// [2] flash led while user key lock
	if (user_key_en == FALSE) {
		if (lock_flash_count == 0) {
			lock_flash_count = 8;
		}
		sys_knob_shake_ms = 0;
		return;
	}

	// [3] lock before last change detect finished
	if (sys_knob_shake_ms > 0) {
		return;
	}

	// [4] ignore shake signal
	knob_pin6 = KNOB_B_IN();
	if (hist_knob_pin6 == knob_pin6) {
		return;
	} else if ((hist_knob_pin6 != RESET) && (knob_pin6 != RESET)) {
		return;
	}
	hist_knob_pin6 = knob_pin6;

	// [4] start to ignore shake signal
	sys_knob_shake_ms = KNOB_SHAKE_TIME;

	// [5] lock key press
	sys_key_en = FALSE;
	key_knob_lock_ms = KEY_KNOB_LOCK_MS;

	// [6] detect at time4 thread


}

#pragma vector=0x8
__interrupt void EXTI_PORTD_IRQHandler(void)
{
	// [1] lock before init finished
	if (sys_key_en == FALSE) {
		return;
	}

	// [2] lock before last change detect finished
	if (sys_key_shake_ms > 0) {
		return;
	}

	// [3] start to ignore shake signal
	sys_key_shake_ms = KEY_SHAKE_TIME;

	// [4] lock key press
	sys_knob_en = FALSE;
	key_knob_lock_ms = KEY_KNOB_LOCK_MS;

	// [5] detect at time4 thread

}

#pragma vector=0x9
__interrupt void EXTI_PORTE_IRQHandler(void)
{

}

#pragma vector=0xC
__interrupt void SPI_IRQHandler(void)
{

}

#pragma vector=0xD
__interrupt void TIM1_UPD_OVF_TRG_BRK_IRQHandler(void)
{

}

#pragma vector=0xE
__interrupt void TIM1_CAP_COM_IRQHandler(void)
{

}

#pragma vector=0xF
__interrupt void TIM2_UPD_OVF_BRK_IRQHandler(void)
{

}

#pragma vector=0x10
__interrupt void TIM2_CAP_COM_IRQHandler(void)
{

}

#pragma vector=0x13
__interrupt void UART_TX_IRQHandler(void)
{

}

#pragma vector=0x14
__interrupt void UART_RX_IRQHandler(void)
{
    u8 receive;
    if (uart_get_it_status() != RESET) {
        receive = uart_receive_data8();

	    if (uart_msg_ready == FALSE) {

			// general protocol match
			if ((receive == UD_START) && (uart_cmd_sn_flag < 3)) {
				uart_cmd_flag = 1;
			}

			if (uart_cmd_flag > 0) {
				uart_data_buff[(uart_cmd_flag - 1)] = receive;
				uart_cmd_flag++;

				if (uart_cmd_flag == 5) {
					if(uart_data_buff[3] == UD_END){
						uart_msg_ready = TRUE;
					}
					uart_cmd_flag = 0;
				}
			}

			// special sn protocol match
			if (uart_cmd_sn_flag > 2) {
				uart_data_buff[uart_cmd_sn_flag - 3] = receive;
				uart_cmd_sn_flag++;
				if (uart_cmd_sn_flag > 6) {
					uart_msg_ready = TRUE;
				}
			} else if (uart_cmd_sn_flag > 1) {
				if (receive == 0xFF) {
					uart_cmd_sn_flag = 3;
				} else if (receive == 0x3D) {
					uart_cmd_sn_flag = 3;
					uart_msg_ready = TRUE;
				}
			} else if (uart_cmd_sn_flag > 0) {
				if(receive == 0x4E){
					uart_cmd_sn_flag = 2;
				}
			} else if (uart_cmd_sn_flag == 0) {
				if (receive == 0x53) {
					uart_cmd_sn_flag = 1;
				}
			}
	    }
    }
}

#pragma vector=0x15
__interrupt void I2C_IRQHandler(void)
{

}

#pragma vector=0x18
__interrupt void ADC_IRQHandler(void)
{

}

#pragma vector=0x19
__interrupt void TIM4_UPD_OVF_IRQHandler(void)
{
	tim4_ms_cnt++;

	if (tim4_ms_cnt > 16) {
		tim4_ms_cnt = 0;

		if (tim4_delay_ms > 0) {
			tim4_delay_ms--;
		}

		if (tim4_uart_delay_ms > 0) {
			tim4_uart_delay_ms--;
		}

		if (sys_key_shake_ms > 1) {
			sys_key_shake_ms--;
		}

		if (sys_knob_shake_ms > 1) {
			sys_knob_shake_ms--;
		}

		if (key_knob_lock_ms > 1) {
			key_knob_lock_ms--;
		} else if (key_knob_lock_ms == 1) {
			sys_key_en = TRUE;
			sys_knob_en = TRUE;
			key_knob_lock_ms = 0;
		}

		if (sys_knob_opposite_ms > 0) {
			sys_knob_opposite_ms--;
		}

		if (np_cont_ms > 0) {
			np_cont_ms--;
		}

		if (knob_operate_ignore_ms > 0) {
			knob_operate_ignore_ms --;
		}

		if (bt_pair_clear>1) {
			bt_pair_clear --;
		}
		sys_key_process_thread();
		sys_knob_process_thread();

		if (xmos_send_delay > 1) {
			xmos_send_delay--;
		}

		if (uart_send_delay > 1) {
			uart_send_delay--;
		}
		if (uart_hist_resend_delay > 1) {
			uart_hist_resend_delay--;
		}


		if (sys_vol_init_flag > VOL_INIT_REL_KEY) {
			sys_vol_init_flag++;
			if (sys_vol_init_flag > 250) {
				sys_vol_init_flag = VOL_INIT_ING;
				if (sys_vol_data < sys_vol_data_temp) {
					sys_vol_data++;
					sys_vol_change = TRUE;
				} else {
					sys_vol_init_flag = VOL_INIT_REL_KEY;
					sys_vol_data_temp = 0;
				}
			}
		}

		if(lock_flash_count > 0){
			lock_flash_ms--;
			if(lock_flash_ms == 0){
				lock_flash_ms = LED_LOCK_FLASH_MS;
				lock_flash_count--;
			}
		}
	}

    clear_tim4_pending_bit();
}

#pragma vector=0x1A
__interrupt void EEPROM_EEC_IRQHandler(void)
{

}
///////////////////////////////////////////////////////////////////////////////////////////


