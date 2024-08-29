#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "freertos/task.h"

const uart_port_t uart_num = UART_NUM_0;
TaskHandle_t xReadHandle = NULL;
TaskHandle_t xReplHandle = NULL;

QueueHandle_t uart_queue;


typedef struct forth_read_buffer_t {
	char start[128];
	char *cursor;
	uint8_t chars_left;
	uint8_t size;
} forth_read_buffer_t;

forth_read_buffer_t uart_read_buffer = { .chars_left = 0, .size = 128 };


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
	uart_event_t event;
	for (;;) {
		if (xQueueReceive(uart_queue, (void *)&event, (TickType_t)portMAX_DELAY) && event.type == UART_DATA ) {
			while (uart_read_buffer.chars_left) {
				vTaskDelay(200 / portTICK_PERIOD_MS);
			}
			data_len = uart_read_bytes(uart_num, &data, uart_read_buffer.size, 100);
			if (data_len) {
				memcpy(uart_read_buffer.start, data, data_len);
				uart_read_buffer.cursor = uart_read_buffer.start;
				uart_read_buffer.chars_left = data_len;
			}
		}
	}
}

void uart_repl_task(void * pvParameters) {
	const char *color_green = "\033[32m";
	const char *color_default = "\033[39m";

	while (1) {
		while (uart_read_buffer.chars_left) {
			uart_write_bytes(uart_num, color_green,5);
			uart_write_bytes(uart_num, uart_read_buffer.cursor,1);
			uart_write_bytes(uart_num, color_default,5);
			uart_read_buffer.cursor++;
			uart_read_buffer.chars_left--;
			vTaskDelay(500 / portTICK_PERIOD_MS);
		}
		vTaskDelay(200 / portTICK_PERIOD_MS);
	}
}



void app_main(void)

{
	ESP_ERROR_CHECK(uart_start());
	// Write data to UART.
	const char* test_str = "This is a test string.\n";
	uart_write_bytes(uart_num, (const char*)test_str, strlen(test_str));
	
	xTaskCreate( uart_read_task, "uart_read_task", 1024, (void *) 0, tskIDLE_PRIORITY, &xReadHandle );
	xTaskCreate( uart_repl_task, "uart_repl_task", 1024, (void *) 0, tskIDLE_PRIORITY, &xReplHandle );
}

