/*
 * alert.c
 *
 *  Created on: Apr 10, 2019
 *      Author: Sarthak Jain
 *
 *      The alert task waits on a TaskNotifyTake to be given by the temperature task.
 *      Once Notify is given, indicating that the temperature is out of given threshold,
 *      the task stores data in the local instance of structure and sends it via message
 *      queue to the logger thread.
 */

#include "../inc/alert.h"

extern void alert_task(void* pvParameters)
{
    struct message alert_msg;
    while(1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        alert_msg.tick_stamp = xTaskGetTickCount();
        alert_msg.id = ALERT_TASK;
        alert_msg.name = "TEMPERATURE CROSSING THRESHOLD!";

        xSemaphoreTake(g_logqueue_sem, portMAX_DELAY);
        if(xQueueSend(log_queue, (void*)&alert_msg, portMAX_DELAY) != pdTRUE)
        {
            UARTprintf("\nTemperature send to logger queue failed\n");
        }
        xSemaphoreGive(g_logqueue_sem);
    }
}


