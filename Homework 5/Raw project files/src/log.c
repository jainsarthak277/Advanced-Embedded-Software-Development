/*
 * log.c
 *
 *  Created on: Apr 10, 2019
 *      Author: jains
 *
 *      Task logs data sent via message queue from three tasks, Alert, temperature and LED tasks.
 *      Logger task uses a pre-defined structure containing the sender's ID to identify the sender,
 *      and accordingly logs data to serial terminal.
 */

#include "../inc/log.h"

extern void log_task(void* pvParameters)
{
    struct message log_msg;
    while(1)
    {
        if(xQueueReceive(log_queue, (void*)&log_msg, portMAX_DELAY) != pdTRUE)
        {
            xSemaphoreTake(g_UART_sem, portMAX_DELAY);
            UARTprintf("\nQueue Receive failed\n");
            xSemaphoreGive(g_UART_sem);
        }

        if(log_msg.id == TEMP_TASK)
        {
            xSemaphoreTake(g_UART_sem, portMAX_DELAY);
            UARTprintf("\n*****TEMPERATURE SENSOR DATA*****");
            UARTprintf("\n<TIME: %d msecs>", log_msg.tick_stamp);
            UARTprintf("\nData received: %d.%d degrees C", (int)log_msg.data, (int)((log_msg.data-(int)log_msg.data)*100));
            UARTprintf("\n***************\n");
            xSemaphoreGive(g_UART_sem);
        }

        else if(log_msg.id == LED_TASK)
        {
            xSemaphoreTake(g_UART_sem, portMAX_DELAY);
            UARTprintf("\n*****LED DATA*****");
            UARTprintf("\n<TIME: %d msecs>", log_msg.tick_stamp);
            UARTprintf("\nToggle count received: %d", (int)log_msg.data);
            UARTprintf("\nUser name: %s", log_msg.name);
            UARTprintf("\n***************\n");
            xSemaphoreGive(g_UART_sem);
        }

        else if(log_msg.id == ALERT_TASK)
        {
            xSemaphoreTake(g_UART_sem, portMAX_DELAY);
            UARTprintf("\n*****ALERT!!!*****");
            UARTprintf("\n<TIME: %d msecs>", log_msg.tick_stamp);
            UARTprintf("\n%s", log_msg.name);
            UARTprintf("\n***************\n");
            xSemaphoreGive(g_UART_sem);
        }
    }
}
