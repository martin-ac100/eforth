#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "freertos/task.h"
#include "forth_prims.h"

const uart_port_t uart_num = UART_NUM_0;
TaskHandle_t xReadHandle = NULL;
TaskHandle_t xReplHandle = NULL;

QueueHandle_t uart_queue;



forth_input_buffer_t uart_input_buffer = { .unread = 0 };
#define forth_buffer_size sizeof(uart_input_buffer.start)


esp_err_t uart_start() {
	uart_config_t uart_config = {
	    .baud_rate = 115200,
	    .data_bits = UART_DATA_8_BITS,
	    .parity = UART_PARITY_DISABLE,
	    .stop_bits = UART_STOP_BITS_1,
	    .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
	    .rx_flow_ctrl_thresh = 122,
	};
	// Configure UART parameters
	ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
	// Setup UART buffered IO with event queue
	const int uart_buffer_size = (1024 * 2);
	// Install UART driver using an event queue here
	return uart_driver_install(UART_NUM_0, uart_buffer_size, uart_buffer_size, 10, &uart_queue, 0);
}

void uart_read_task(void * pvParameters) {
	uint8_t data[256];
	int data_len = 0;
   int line_pos = 0;
   int i = 0;
	uart_event_t event;
   uart_write_bytes(uart_num,"uart_reader_started\r\n",21);
	for (;;) {
		if (xQueueReceive(uart_queue, (void *)&event, (TickType_t)portMAX_DELAY) && event.type == UART_DATA ) {
			while (uart_input_buffer.unread ) {
				vTaskDelay( 10 / portTICK_PERIOD_MS);

			}
			data_len = uart_read_bytes(uart_num, &data, 1, 10 / portTICK_PERIOD_MS);
			if (data_len) {
            uart_input_buffer.read_pos = uart_input_buffer.chars;
            for ( i = 0; i < data_len; i++) {
               if ( uart_input_buffer.unread == 0 ) {
                  uart_input_buffer.chars[line_pos++] = data[i];
                  uart_write_bytes(uart_num, &data[i], 1);
                  if ( data[i] == '\r' ) {
                     uart_write_bytes(uart_num,"\r\n",2);
                     uart_input_buffer.read_pos = uart_input_buffer.chars;
                     uart_input_buffer.unread = line_pos;
                     line_pos = 0;
                  }
               }
               vTaskDelay( 10 / portTICK_PERIOD_MS);
            }
			}
		}
	}
}

int uart_write(const void *src, int size) {
   return uart_write_bytes(uart_num, src, size);
}
