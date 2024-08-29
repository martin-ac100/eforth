#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "freertos/task.h"
#include "forth_prims.h"

const uart_port_t uart_num = UART_NUM_0;
TaskHandle_t xReadHandle = NULL;
TaskHandle_t xReplHandle = NULL;

QueueHandle_t uart_queue;



forth_input_buffer_t uart_input_buffer = { .unread = 0, .write_pos = 0 };
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
	uart_event_t event;
	for (;;) {
		if (xQueueReceive(uart_queue, (void *)&event, (TickType_t)portMAX_DELAY) && event.type == UART_DATA ) {
			while (uart_input_buffer.unread ) {
				vTaskDelay( 10 / portTICK_PERIOD_MS);

			}
			data_len = uart_read_bytes(uart_num, &data, forth_buffer_size, 10 / portTICK_PERIOD_MS);
			if (data_len) {
				memcpy(uart_input_buffer.start, data, data_len);
				uart_input_buffer.write_pos = uart_input_buffer.start + data_len;
				uart_input_buffer.unread = data_len;
			}
		}
	}
}

void uart_repl_task(void * pvParameters) {
	const char *color_green = "\033[32m";
	const char *color_default = "\033[39m";

	while (1) {
		while (uart_input_buffer.unread) {
			uart_write_bytes(uart_num, color_green,5);
			uart_write_bytes(uart_num, uart_input_buffer.write_pos - uart_input_buffer.unread--, 1);
			uart_write_bytes(uart_num, color_default,5);
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

