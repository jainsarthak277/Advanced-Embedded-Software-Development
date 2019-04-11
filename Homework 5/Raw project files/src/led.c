/*
 * led.c
 *
 *  Created on: Apr 10, 2019
 *      Author: Sarthak Jain
 *
 *      The LED task waits on a semaphore to be released by the LED task's timer.
 *      Once the semaphore is released, task stores data in the local instance of structure, reads
 *      data from the sensor via I2C2 bus, and sends it via message queue to the logger thread.
 */

#include "../inc/led.h"

void led_config()
{
    // Enable the GPIO port that is used for the on-board LED.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

    // Check if the peripheral access is enabled.
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));

    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);
}

extern void led_task(void* pvParameters)
{
    led_config();
    uint32_t toggle_cnt = 0;
    struct message led_msg;

    while(1)
    {
        xSemaphoreTake(g_led_sem, portMAX_DELAY);
        toggle_cnt++;

        if(toggle_cnt & 0x01)
        {
            GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
            GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x00);
        }
        else
        {
            GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0x00);
            GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
        }
        led_msg.tick_stamp = xTaskGetTickCount();
        led_msg.id = LED_TASK;
        led_msg.data = toggle_cnt;
        led_msg.name = "Sarthak";

        xSemaphoreTake(g_logqueue_sem, portMAX_DELAY);
        if(xQueueSend(log_queue, (void*)&led_msg, portMAX_DELAY) != pdTRUE)
        {
            UARTprintf("\nLED send to logger queue failed\n");
        }
        xSemaphoreGive(g_logqueue_sem);
    }
}


