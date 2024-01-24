// SPDX-License-Identifier: GPL-2.0
#include "gpio.h"

void gpio_deinit(gpio_reg* GPIOx)
{
    GPIOx->ODR = GPIO_ODR_RESET_VALUE; // Output Data Register
    GPIOx->DDR = GPIO_DDR_RESET_VALUE; // Data Direction Register
    GPIOx->CR1 = GPIO_CR1_RESET_VALUE; // Control Register 1
    GPIOx->CR2 = GPIO_CR2_RESET_VALUE; // Control Register 2
}

void gpio_init(gpio_reg* GPIOx, gpio_pin pin, gpio_mode mode)
{
    GPIOx->CR2 &= (u8)(~(pin));

    // set: output(hight/low)/input
    if ((((u8)(mode)) & (u8)0x80) != (u8)0x00) {
        if ((((u8)(mode)) & (u8)0x10) != (u8)0x00) {
            GPIOx->ODR |= (u8)pin;        // hight
        } else {
            GPIOx->ODR &= (u8)(~(pin));
        }
        GPIOx->DDR |= (u8)pin;            // Output
    } else {
        GPIOx->DDR &= (u8)(~(pin));
    }

    // set: pull-up,push-pull/float,open-drain
    if ((((u8)(mode)) & (u8)0x40) != (u8)0x00) {
        GPIOx->CR1 |= (u8)pin;            // pull-up/push-pull
    } else {
        GPIOx->CR1 &= (u8)(~(pin));
    }

    // set: interrupt,slow slope/no-external-interrupt,no-slope-control
    if ((((u8)(mode)) & (u8)0x20) != (u8)0x00) {
        GPIOx->CR2 |= (u8)pin;            // interrupt/slow-slope
    } else {
        GPIOx->CR2 &= (u8)(~(pin));
    }
}

void gpio_write_high(gpio_reg* GPIOx, gpio_pin pin)
{
    GPIOx->ODR |= (u8)pin;
}

void gpio_write_low(gpio_reg* GPIOx, gpio_pin pin)
{
    GPIOx->ODR &= (u8)(~pin);
}

BitStatus gpio_read_input_pin(gpio_reg* GPIOx, gpio_pin pin)
{
    return ((BitStatus)(GPIOx->IDR & (u8)pin));
}


void init_gpio(void)
{
    // XM...
    gpio_init(GPIOA, gpio_pin_1, gpio_mode_out_pp_high_fast);   // PIN: MUTE
    gpio_write_low(GPIOA, gpio_pin_1);
    gpio_init(GPIOA, gpio_pin_2, gpio_mode_out_pp_high_fast);	// PIN: NRST_CTRL
    gpio_write_low(GPIOA, gpio_pin_2);
    gpio_init(GPIOC, gpio_pin_7, gpio_mode_out_pp_high_fast);   // PIN: VSYS_EN
    gpio_write_low(GPIOC, gpio_pin_7);

    //
    gpio_init(GPIOA, gpio_pin_3, gpio_mode_out_pp_low_slow);	// PIN: T2V Detect
    gpio_write_low(GPIOA, gpio_pin_3);

    //
    gpio_init(GPIOD, gpio_pin_3, gpio_mode_in_pu_no_it);		// PIN: CPLD_CMD1(T2)
    gpio_init(GPIOD, gpio_pin_4, gpio_mode_in_pu_no_it);		// PIN: CPLD_CMD0(T2)

    // GPIOB4/5 I2C
    gpio_init(GPIOC, gpio_pin_3, gpio_mode_out_pp_high_fast );  // PIN: DAC_I2S_SET0
    gpio_write_low(GPIOC, gpio_pin_3);
    gpio_init(GPIOC, gpio_pin_4, gpio_mode_out_pp_high_fast);   // PIN: DAC_I2S_SET1
    gpio_write_low(GPIOC, gpio_pin_4);

    // KNOBA & KNOBB
    gpio_init(GPIOD, gpio_pin_2, gpio_mode_in_pu_it);          // PIN: FUNC_KEY
    set_interrupt_sensitivity(exti_port_gpio_d, exti_sensitivity_rise_fall);

    gpio_init(GPIOC, gpio_pin_5, gpio_mode_in_pu_it);          // PIN: KNOB_A
    gpio_init(GPIOC, gpio_pin_6, gpio_mode_in_pu_no_it);       // PIN: KNOB_B
    set_interrupt_sensitivity(exti_port_gpio_c, exti_sensitivity_rise_fall);
    // GPIOB4/5 I2C
    // GPIOD5/6 UART
}

void init_gpio_for_i2c(void)
{
    gpio_init(GPIOB, gpio_pin_4, gpio_mode_out_od_hiz_fast);    // PIN: I2C_SCL
    gpio_write_high(GPIOB, gpio_pin_4);
    gpio_init(GPIOB, gpio_pin_5, gpio_mode_out_od_hiz_fast);    // PIN: I2C_SDA
    gpio_write_high(GPIOB, gpio_pin_5);
}

void init_i2c_sda_for_output(void)
{
    gpio_init(GPIOB, gpio_pin_5, gpio_mode_out_od_hiz_fast);    // PIN: I2C_SDA
}

void init_i2c_sda_for_input(void)
{
    gpio_init(GPIOB, gpio_pin_5, gpio_mode_in_pu_no_it);        // PIN: I2C_SDA
}
