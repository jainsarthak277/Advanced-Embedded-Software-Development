/*
 * temp.c
 *
 *  Created on: Apr 10, 2019
 *      Author: Sarthak Jain
 *
 *      Temperature task waits on a semaphore to be released by the temperature task's timer.
 *      Once the semaphore is released, task stores data in the local instance of structure, reads
 *      data from the sensor via I2C2 bus, and sends it via message queue to the logger thread.
 *
 *      The temperature task also notifies the alert task whenever the data read from the temperature
 *      sensor is above a certain threshold, specified as 25 degrees C in this case.
 */

#include "../inc/temp.h"

float read_temp(void)
{
    uint16_t data;
    float temp;

    I2CMasterSlaveAddrSet(I2C2_BASE, SLAVE_ADDRESS, false);
    I2CMasterDataPut(I2C2_BASE, CONFIG_REG);
    I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    while(I2CMasterBusy(I2C2_BASE));


    I2CMasterSlaveAddrSet(I2C2_BASE, SLAVE_ADDRESS, true);
    I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);

    while(!I2CMasterBusy(I2C2_BASE));
    while(I2CMasterBusy(I2C2_BASE));

    data = I2CMasterDataGet(I2C2_BASE);
    data = data << 8;

    I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);

    while(!I2CMasterBusy(I2C2_BASE));
    while(I2CMasterBusy(I2C2_BASE));

    data = data | I2CMasterDataGet(I2C2_BASE);
    data = data >> 4;

    temp = data * 0.0625;
    return temp;
}

extern void temp_task(void* pvParameters)
{
    struct message temp_msg;
    while(1)
    {
        xSemaphoreTake(g_temp_sem, portMAX_DELAY);

        temp_msg.tick_stamp = xTaskGetTickCount();
        temp_msg.id = TEMP_TASK;
        temp_msg.data = read_temp();

        if(temp_msg.data > THRESHOLD)
        {
            xTaskNotify(alerter, 0, eNoAction);
        }

        xSemaphoreTake(g_logqueue_sem, portMAX_DELAY);
        if(xQueueSend(log_queue, (void*)&temp_msg, portMAX_DELAY) != pdTRUE)
        {
            UARTprintf("\nTemperature send to logger queue failed\n");
        }
        xSemaphoreGive(g_logqueue_sem);
    }
}


