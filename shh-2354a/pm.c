// pm.c

#include <stdio.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pm.h"

#define USBC_I2C				i2c1
#define USBC_SDA				26
#define USBC_SCL				27
#define TUSB320_ADDR			0x47

#define TUSB320_REG_CURRENT 	0x08
#define TUSB320_REG_STATUS		0x09
#define TUSB320_REG_RESET		0x0A
#define TUSB320_CURRENT_MASK	0x30
#define TUSB320_CURRENT_DEFAULT	0x00
#define TUSB320_CURRENT_MEDIUM	0x10
#define TUSB320_CURRENT_CTA		0x20
#define TUSB320_CURRENT_HIGH	0x30
#define TUSB320_ATTACH_MASK		0xC0
#define TUSB320_ATTACH_SINK		0x80
#define TUSB320_RESET 			0x08

#define BQ25628E_ADDR			0x6A

#define BQ25628E_REG_CHARGE_CURRENT_LIMIT	0x02
#define BQ25628E_REG_INPUT_CURRENT_LIMIT	0x06
#define BQ25628E_REG_CHARGER_CONTROL_0		0x16
#define BQ25628E_REG_CHARGER_CONTROL_1		0x17
#define BQ25628E_REG_CHARGER_CONTROL_3		0x19
#define BQ25628E_EN_EXTILIM					0x04

// 1000mA
#define BQ25628E_ICHG_0		0b00100000
#define BQ25628E_ICHG_1		0b00000011

#define I2C_TIMEOUT_US		3000
#define STARTUP_GRACE_MS	500

static absolute_time_t next_handle;

static pm_state_t pm_state;
static pm_current_t pm_current;

static uint8_t task = 0;


static bool set_charge_current(void)
{
	int ret = 0;
	static uint8_t data[] = {
		BQ25628E_REG_CHARGE_CURRENT_LIMIT,
		BQ25628E_ICHG_0,
		BQ25628E_ICHG_1
	};
	ret = i2c_write_timeout_us(
		USBC_I2C, BQ25628E_ADDR,
		data, 3,
		false, I2C_TIMEOUT_US);
	return (ret == 3);
}

void init_pm(void)
{
	i2c_init(USBC_I2C, 100000);

	gpio_set_function(USBC_SDA, GPIO_FUNC_I2C);
	gpio_set_function(USBC_SCL, GPIO_FUNC_I2C);

	gpio_disable_pulls(USBC_SDA);
	gpio_disable_pulls(USBC_SCL);

	next_handle = make_timeout_time_ms(STARTUP_GRACE_MS);
	pm_current = USB_500MA;
	pm_state = USBC_UNDEF;

	return;
}

pm_state_t pm_get_state(void)
{
	return pm_state;
}

pm_current_t pm_get_current(void)
{
	return pm_current;
}

static bool tusb320_reset(void)
{
	uint8_t reg = TUSB320_REG_RESET;
	uint8_t control;
	
	int ret = i2c_write_timeout_us(
		USBC_I2C, TUSB320_ADDR, &reg, 1,
		true, I2C_TIMEOUT_US);

	if (ret != 1)
		return false;

	ret = i2c_read_timeout_us(
		USBC_I2C, TUSB320_ADDR, &control, 1,
		false, I2C_TIMEOUT_US);

	if (ret != 1)
		return false;

	uint8_t reset[] = {
		TUSB320_REG_RESET, control |
		TUSB320_RESET
	};

	ret = i2c_write_timeout_us(
		USBC_I2C, TUSB320_ADDR, reset, sizeof(reset),
		false, I2C_TIMEOUT_US);

	return (ret == (int)sizeof(reset));
}

static void pm_set_state(void)
{
	static uint8_t reg = TUSB320_REG_STATUS;
	static uint8_t status;

	int ret = i2c_write_timeout_us(
		USBC_I2C, TUSB320_ADDR, &reg, 1,
		true, I2C_TIMEOUT_US);

	if (ret != 1)
	{
		pm_state = USBC_UNKNOWN;
		return;
	}

	ret = i2c_read_timeout_us(
		USBC_I2C, TUSB320_ADDR, &status, 1,
		false, I2C_TIMEOUT_US);

	if (ret != 1)
	{
		pm_state = USBC_UNKNOWN;
		return;
	}

	switch (status & TUSB320_ATTACH_MASK)
	{
	case 0x00 :
		pm_state = USBC_DETACHED;
		break;
	case TUSB320_ATTACH_SINK :
		pm_state = USBC_ATTACHED;
		break;
	default :
		pm_state = USBC_UNKNOWN;
		break;
	}
	return;
}

static void pm_set_current(void)
{
	static uint8_t reg = TUSB320_REG_CURRENT;
	static uint8_t current;

	if (pm_get_state() == USBC_ATTACHED)
	{
		int ret = i2c_write_timeout_us(
			USBC_I2C, TUSB320_ADDR, &reg, 1,
			true, I2C_TIMEOUT_US);

		if (ret != 1)
		{
			pm_current = USB_500MA;
			return;
		}

		ret = i2c_read_timeout_us(
			USBC_I2C, TUSB320_ADDR, &current, 1,
			false, I2C_TIMEOUT_US);

		if (ret != 1)
		{
			pm_current = USB_500MA;
			return;
		}

		switch (current & TUSB320_CURRENT_MASK)
		{
		case TUSB320_CURRENT_DEFAULT:
			pm_current = USB_500MA;
			break;
		case TUSB320_CURRENT_MEDIUM:
			pm_current = USB_1500MA;
			break;
		case TUSB320_CURRENT_CTA:
			pm_current = USB_500MA;
			break;
		case TUSB320_CURRENT_HIGH:
			pm_current = USB_3000MA;
			break;
		}
	}
	else
	{
		pm_current = USB_500MA;
	}

	return;
}

static void reset_bq25628e(void)
{
	static uint8_t data[] = {
		BQ25628E_REG_CHARGER_CONTROL_1,
		0x80
	};
	i2c_write_timeout_us(
		USBC_I2C, BQ25628E_ADDR,
		data, 2,
		false, I2C_TIMEOUT_US);
}

static bool pm_apply_current(void)
{
	static uint8_t reg_input_current_limit =
		BQ25628E_REG_INPUT_CURRENT_LIMIT;
	static uint8_t reg_charger_control_0 =
		BQ25628E_REG_CHARGER_CONTROL_0;
	static uint8_t reg_charger_control_1 =
		BQ25628E_REG_CHARGER_CONTROL_1;
	static uint8_t reg_charger_control_3 =
		BQ25628E_REG_CHARGER_CONTROL_3;
	static pm_current_t last_current = USB_500MA;
	static bool failed = false;

	uint8_t data[4];

	if (last_current != pm_current ||
		failed)
	{
		int ret;
		switch (pm_current)
		{
		case USB_500MA:
			ret = i2c_write_timeout_us(
				USBC_I2C, BQ25628E_ADDR,
				&reg_charger_control_3, 1,
				true, I2C_TIMEOUT_US);
			if (ret != 1) goto apply_current_fail;
			ret = i2c_read_timeout_us(
				USBC_I2C, BQ25628E_ADDR,
				(data + 1), 1,
				false, I2C_TIMEOUT_US);
			if (ret != 1) goto apply_current_fail;
			data[0] = BQ25628E_REG_CHARGER_CONTROL_3;
			data[1] |= BQ25628E_EN_EXTILIM;
			ret = i2c_write_timeout_us(
				USBC_I2C, BQ25628E_ADDR,
				data, 2,
				false, I2C_TIMEOUT_US);
			if (ret != 2) goto apply_current_fail;
			break;
		case USB_1500MA:
			data[0] = BQ25628E_REG_INPUT_CURRENT_LIMIT;
			data[1] = 0xB0;
			data[2] = 0x04;
			ret = i2c_write_timeout_us(
				USBC_I2C, BQ25628E_ADDR,
				data, 3,
				false, I2C_TIMEOUT_US);
			if (ret != 3) goto apply_current_fail;

			ret = i2c_write_timeout_us(
				USBC_I2C, BQ25628E_ADDR,
				&reg_charger_control_3, 1,
				true, I2C_TIMEOUT_US);
			if (ret != 1) goto apply_current_fail;
			ret = i2c_read_timeout_us(
				USBC_I2C, BQ25628E_ADDR,
				(data + 1), 1,
				false, I2C_TIMEOUT_US);
			if (ret != 1) goto apply_current_fail;
			data[0] = BQ25628E_REG_CHARGER_CONTROL_3;
			data[1] &= ~(BQ25628E_EN_EXTILIM);
			ret = i2c_write_timeout_us(
				USBC_I2C, BQ25628E_ADDR,
				data, 2,
				false, I2C_TIMEOUT_US);
			if (ret != 2) goto apply_current_fail;
			break;
		case USB_3000MA:
			data[0] = BQ25628E_REG_INPUT_CURRENT_LIMIT;
			data[1] = 0x60;
			data[2] = 0x09;
			ret = i2c_write_timeout_us(
				USBC_I2C, BQ25628E_ADDR,
				data, 3,
				false, I2C_TIMEOUT_US);
			if (ret != 3) goto apply_current_fail;

			ret = i2c_write_timeout_us(
				USBC_I2C, BQ25628E_ADDR,
				&reg_charger_control_3, 1,
				true, I2C_TIMEOUT_US);
			if (ret != 1) goto apply_current_fail;
			ret = i2c_read_timeout_us(
				USBC_I2C, BQ25628E_ADDR,
				(data + 1), 1,
				false, I2C_TIMEOUT_US);
			if (ret != 1) goto apply_current_fail;
			data[0] = BQ25628E_REG_CHARGER_CONTROL_3;
			data[1] &= ~(BQ25628E_EN_EXTILIM);
			ret = i2c_write_timeout_us(
				USBC_I2C, BQ25628E_ADDR,
				data, 2,
				false, I2C_TIMEOUT_US);
			if (ret != 2) goto apply_current_fail;
			break;
		}
		failed = false;
	}

	last_current = pm_current;

	int ret = 0;
	ret = i2c_write_timeout_us(
		USBC_I2C, BQ25628E_ADDR,
		&reg_charger_control_0, 1,
		true, I2C_TIMEOUT_US);
	if (ret != 1) return false;
	ret = i2c_read_timeout_us(
		USBC_I2C, BQ25628E_ADDR,
		(data + 1), 1,
		false, I2C_TIMEOUT_US);
	if (ret != 1) return false;
	data[0] = BQ25628E_REG_CHARGER_CONTROL_0;
	data[1] |= 0x04;
	ret = i2c_write_timeout_us(
		USBC_I2C, BQ25628E_ADDR,
		data, 2,
		false, I2C_TIMEOUT_US);
	if (ret != 2) return false;

	return true;

apply_current_fail:
	reset_bq25628e();
	failed = true;
	return false;
}

void pm_handle(void)
{
	if (!time_reached(next_handle))
		return;

	switch(task)
	{
	case 0:
		if (set_charge_current())
			task = 1;
		else 
			task = 0;
		next_handle = make_timeout_time_ms(20);
		break;
	case 1:
		pm_set_state();
		task = 2;
		next_handle = make_timeout_time_ms(20);
		break;
	case 2:
		pm_set_current();
		task = 3;
		next_handle = make_timeout_time_ms(20);
		break;
	case 3:
		if (pm_apply_current())
			task = 1;
		else
			task = 0;
		next_handle = make_timeout_time_ms(20);
	}
	return;
}

void pm_cleanup(void)
{
	reset_bq25628e();
	return;
}