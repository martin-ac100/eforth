/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "forth_prims.h"
extern io_buff_t uart_fth;

/**
 * This is an example which echos any data it receives on configured UART back to the sender,
 * with hardware flow control turned off. It does not use UART driver event queue.
 *
 * - Port: configured UART
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below (See Kconfig)
 */

#define ECHO_TEST_TXD (CONFIG_EXAMPLE_UART_TXD)
#define ECHO_TEST_RXD (CONFIG_EXAMPLE_UART_RXD)
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define ECHO_UART_PORT_NUM      (CONFIG_EXAMPLE_UART_PORT_NUM)
#define ECHO_UART_BAUD_RATE     (CONFIG_EXAMPLE_UART_BAUD_RATE)

#define BUF_SIZE (1024)

void uart_read_task(void *arg)
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(ECHO_UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    io_buff_t uart_in = {.c_pos = 0, .c_top = 0, .c_size = BUF_SIZE, .c_buff = (char *)data };

    while (1) {
        // Read data from the UART
        if (uart_fth.c_pos == uart_fth.c_top) {
            uart_fth.c_pos = 0;
            uart_fth.c_top = 0;
            if (uart_in.c_pos >= uart_in.c_top) {
                uart_in.c_pos = 0;
                uart_in.c_top = uart_read_bytes(ECHO_UART_PORT_NUM, data, (BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);
                uart_write_bytes(ECHO_UART_PORT_NUM, (const char *) data, uart_in.c_top);
            }
            else {
                while (uart_in.c_pos < uart_in.c_top) {
                        uart_fth.c_buff[uart_fth.c_pos] = uart_in.c_buff[uart_in.c_pos];
                        uart_in.c_pos++;
                        if (uart_in.c_buff[uart_in.c_pos] == '\n') {
                            uart_fth.c_top = uart_fth.c_pos;
                            uart_fth.c_pos = 0;
                            break;
                        }
                        uart_fth.c_pos++;
                }
            }
        }
        vTaskDelay(5 / portTICK_PERIOD_MS);

    }
}

int uart_write(const void *src, int size) {
    return uart_write_bytes(ECHO_UART_PORT_NUM, src, (size_t)size);
}

