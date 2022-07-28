#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "rc522.h"
#include "driver/spi_master.h"

// UART DEFİNE DEFİNİTİONS
#define UART_TXD (GPIO_NUM_1)
#define UART_RXD (GPIO_NUM_3)
#define UART_RTS (UART_NUM_0)
#define UART_CTS (UART_PIN_NO_CHANGE)
#define UART_PORT_NUM      (UART_NUM_0)
#define UART_BAUD_RATE     (115200)      //CONFIG_EXAMPLE_UART_BAUD_RATE
#define TASK_STACK_SIZE    (2048)      //CONFIG_EXAMPLE_TASK_STACK_SIZE
#define BUF_SIZE (1024)

void uart_cnfg(void){      // Uart Configure Function
    
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TXD, UART_RXD, UART_RTS, UART_CTS));

}

uint8_t variable_adress[5];

void tag_handler(uint8_t* sn) { // serial number is always 5 bytes long
    
    int err_num = 0;
    uint8_t card_adress[5] = {0xf3,0x9c,0x92,0xa1,0x5c};
    for(int i=0;i<=4;i++){
        variable_adress[i] = sn[i];
        if(variable_adress[i] != card_adress[i]){
            err_num ++;
        }
    }
}
static void uart_task(void *arg)
{   
    int len = 12 ;
    while (1) {

        if( err_num == 0 ){
        uart_write_bytes(UART_PORT_NUM, 1, len);
        }
        else{
        uart_write_bytes(UART_PORT_NUM, 0, len);
        }
    }
}

void app_main(void)
{
    const rc522_start_args_t start_args = {     // RC522 Start Structer
        .miso_io  = 25,
        .mosi_io  = 23,
        .sck_io   = 19,
        .sda_io   = 22,
        .callback = &tag_handler,

    };

    uart_cnfg();    // Uart Configure Function
    rc522_start(start_args);    // RC522 Start Function
    xTaskCreate(uart_task, "uart_task",TASK_STACK_SIZE, NULL, 10, NULL);
}
