/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>

#include <string.h>

#define MSG_SIZE 32

/* queue to store up to 10 messages (aligned to 4-byte boundary) */
K_MSGQ_DEFINE(uart_msgq, MSG_SIZE, 10, 4);

static const struct device *const uart_in = DEVICE_DT_GET(DT_ALIAS(serial_in));
static const struct device *const uart_out = DEVICE_DT_GET(DT_ALIAS(serial_out));

/* receive buffer used in UART ISR callback */
static char rx_buf_in[MSG_SIZE] = {0};
static int rx_buf_pos_in = 1;
static char rx_buf_out[MSG_SIZE] = {1};
static int rx_buf_pos_out = 1;

/*
 * Read characters from UART until line end is detected. Afterwards push the
 * data to the message queue.
 */
void serial_cb(const struct device *dev, void *user_data)
{
	uint8_t c;
	char *buf;
	int *pos;

	if (!uart_irq_update(dev)) {
		return;
	}

	if (dev == uart_in) {
		buf = rx_buf_in;
		pos = &rx_buf_pos_in;
	} else {
		buf = rx_buf_out;
		pos = &rx_buf_pos_out;
	}


	while (uart_irq_rx_ready(dev)) {

		uart_fifo_read(dev, &c, 1);

		if ((c == '\n' || c == '\r')) {
			if (*pos > 1) {
				/* terminate string */
				buf[*pos] = '\0';

				/* if queue is full, message is silently dropped */
				k_msgq_put(&uart_msgq, buf, K_NO_WAIT);

				/* reset the buffer (it was copied to the msgq) */
				*pos = 1;
			}
		} else if (*pos < (MSG_SIZE - 2)) {
			buf[*pos] = c;
			*pos += 1;
		}
		/* else: characters beyond buffer size are dropped */
	}
}

/*
 * Print a null-terminated string character by character to the UART interface
 */
void print_uart(const struct device *dev, char *buf)
{
	int msg_len = strlen(buf);

	for (int i = 0; i < msg_len; i++) {
		uart_poll_out(dev, buf[i]);
	}
}

void main(void)
{
	char tx_buf[MSG_SIZE];

	if (!device_is_ready(uart_in)) {
		printk("UART_IN device not found!");
		return;
	}

	if (!device_is_ready(uart_out)) {
		printk("UART_OUT device not found!");
		return;
	}

	/* configure interrupt and callback to receive data */
	uart_irq_callback_user_data_set(uart_in, serial_cb, NULL);
	uart_irq_rx_enable(uart_in);
	uart_irq_callback_user_data_set(uart_out, serial_cb, NULL);
	uart_irq_rx_enable(uart_out);

	printk("Echo bot started on " CONFIG_BOARD "\r\n");

	/* indefinitely wait for input from the user */
	while (k_msgq_get(&uart_msgq, &tx_buf, K_FOREVER) == 0) {
		const struct device *dev;
		if (tx_buf[0] == 0) {
			dev = uart_out;
			printk("SEND: %s\n", &tx_buf[1]);
			print_uart(dev, &tx_buf[1]);
			print_uart(dev, "\r\n");
		} else {
			printk("RECV: %s\n", &tx_buf[1]);
		}
	}
}
