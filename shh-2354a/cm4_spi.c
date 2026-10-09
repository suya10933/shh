// cm4_spi.c

#include <stdio.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "cm4_spi.h"

#define CM4_SPI 			spi0
#define CM4_SPI_MOSI		0
#define CM4_SPI_CSN			1
#define CM4_SPI_SCK			2
#define CM4_SPI_MISO		3

#define CMD_READ 			0x80
#define CMD_ADDR_MASK 		0x7f

#define RAM_SIZE  			128

typedef enum {
	STATE_HEADER,
	STATE_RX,
	STATE_TX
} cm4_spi_state_t;

static volatile uint8_t ram[RAM_SIZE];
static uint8_t header[2];

static uint8_t dma_rx_dummy;
static uint8_t dma_tx_dummy = 0x00;

static uint dma_rx;
static uint dma_tx;

static volatile cm4_spi_state_t state = STATE_HEADER;

static void cm4_spi_dma(volatile uint8_t *rx_buf,
	bool rx_inc,
	const volatile uint8_t *tx_buf,
	bool tx_inc,
	uint8_t len)
{
	dma_channel_config rx_cfg =
		dma_channel_get_default_config(dma_rx);
	channel_config_set_transfer_data_size(&rx_cfg, DMA_SIZE_8);
	channel_config_set_read_increment(&rx_cfg, false);
	channel_config_set_write_increment(&rx_cfg, rx_inc);
	channel_config_set_dreq(&rx_cfg, spi_get_dreq(CM4_SPI, false));
	dma_channel_configure(
		dma_rx,
		&rx_cfg,
		rx_buf,
		&spi_get_hw(CM4_SPI)->dr,
		len,
		false);

	dma_channel_config tx_cfg =
		dma_channel_get_default_config(dma_tx);
	channel_config_set_transfer_data_size(&tx_cfg, DMA_SIZE_8);
	channel_config_set_read_increment(&tx_cfg, tx_inc);
	channel_config_set_write_increment(&tx_cfg, false);
	channel_config_set_dreq(&tx_cfg, spi_get_dreq(CM4_SPI, true));
	dma_channel_configure(
		dma_tx,
		&tx_cfg,
		&spi_get_hw(CM4_SPI)->dr,
		tx_buf,
		len,
		false);

	dma_start_channel_mask((1u << dma_rx) |	(1u << dma_tx));

	return;
}

static void cm4_spi_header(void)
{
	state = STATE_HEADER;
	cm4_spi_dma(header, true, &dma_tx_dummy, false, 2);
	return;
}

static void cm4_spi_dma_abort(void)
{
	dma_channel_config rx_cfg = dma_get_channel_config(dma_rx);
	dma_channel_config tx_cfg = dma_get_channel_config(dma_tx);

	channel_config_set_enable(&rx_cfg, false);
	channel_config_set_enable(&tx_cfg, false);
	dma_channel_set_config(dma_rx, &rx_cfg, false);
	dma_channel_set_config(dma_tx, &tx_cfg, false);

	dma_channel_abort(dma_rx);
	dma_channel_abort(dma_tx);
}

static void cm4_spi_irq(uint gpio, uint32_t events)
{
	static bool read;
	static uint8_t addr;
	static uint8_t length;

	uint32_t rx_count = (dma_channel_hw_addr(dma_rx)->transfer_count &
		DMA_CH0_TRANS_COUNT_COUNT_BITS);
	uint32_t tx_count = (dma_channel_hw_addr(dma_tx)->transfer_count &
		DMA_CH0_TRANS_COUNT_COUNT_BITS);


	switch (state)
	{
	case STATE_HEADER:
		{
			read = ((header[0] & CMD_READ) != 0);
			addr = header[0] & CMD_ADDR_MASK;
			length = header[1];
			cm4_spi_dma_abort();
			if ((length > RAM_SIZE - addr) || (length == 0) || (rx_count != 0))
			{
				cm4_spi_dma_abort();
				cm4_spi_disable();
				cm4_spi_enable();
				return;
			}
			if (read)
			{
				state = STATE_RX;
				cm4_spi_dma(&dma_rx_dummy, false, &ram[addr], true, length);
			}
			else
			{
				state = STATE_TX;
				cm4_spi_dma(&ram[addr], true, &dma_tx_dummy, false, length);
			}
			break;
		}
	case STATE_RX:
		{
			cm4_spi_dma_abort();
			cm4_spi_disable();
			cm4_spi_enable();
			break;
		}
	case STATE_TX:
		{
			cm4_spi_dma_abort();
			cm4_spi_disable();
			cm4_spi_enable();
			break;
		}
	default:
		{
			cm4_spi_dma_abort();
			cm4_spi_disable();
			cm4_spi_enable();
			break;
		}
	}
	return;
}

void init_cm4_spi(void)
{
	dma_rx = dma_claim_unused_channel(true);
	dma_tx = dma_claim_unused_channel(true);

	ram[0] = 0xDE;
	ram[1] = 0xAD;
	ram[2] = 0xBE;
	ram[3] = 0xEF;
	for (uint8_t i = 0; i < 64; ++i)
	{
		ram[4 + i] = i;
	}

	return;
}

void cm4_spi_enable(void)
{
	spi_init(CM4_SPI, 1000 * 1000);

	spi_set_format(
		CM4_SPI,
		8,
		SPI_CPOL_0,
		SPI_CPHA_1,
		SPI_MSB_FIRST
	);

	spi_set_slave(CM4_SPI, true);

	gpio_disable_pulls(0);
	gpio_disable_pulls(1);
	gpio_disable_pulls(2);
	gpio_disable_pulls(3);

	gpio_set_function(0, GPIO_FUNC_SPI);
	gpio_set_function(1, GPIO_FUNC_SPI);
	gpio_set_function(2, GPIO_FUNC_SPI);
	gpio_set_function(3, GPIO_FUNC_SPI);

	gpio_set_irq_enabled_with_callback(CM4_SPI_CSN, GPIO_IRQ_EDGE_RISE, true, &cm4_spi_irq);

	cm4_spi_header();
	return;
}

void cm4_spi_disable(void)
{
	gpio_set_irq_enabled_with_callback(CM4_SPI_CSN, GPIO_IRQ_EDGE_RISE, false, &cm4_spi_irq);
	spi_deinit(CM4_SPI);
	gpio_init(3);
	gpio_set_dir(3, GPIO_IN);
	gpio_disable_pulls(3);
	gpio_init(0);
	gpio_set_dir(0, GPIO_IN);
	gpio_disable_pulls(0);
	gpio_init(1);
	gpio_set_dir(1, GPIO_IN);
	gpio_disable_pulls(1);
	gpio_init(2);
	gpio_set_dir(2, GPIO_IN);
	gpio_disable_pulls(2);

	cm4_spi_dma_abort();

	return;
}

volatile uint8_t *cm4_spi_get_ram(void)
{
	return ram;
}