/*
 * log.h
 *
 *  Created on: Apr 10, 2019
 *      Author: Sarthak Jain
 *
 *      Contains all headers used for log.c
 */

#ifndef LOG_H_
#define LOG_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "driverlib/i2c.h"
#include "sensorlib/i2cm_drv.h"
#include "utils/uartstdio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "drivers/pinout.h"
#include "drivers/buttons.h"

#define TEMP_TASK   (10)
#define LED_TASK    (11)
#define ALERT_TASK  (12)

struct message
{
    uint8_t id;
    TickType_t tick_stamp;
    float data;
    char* name;
};

QueueHandle_t log_queue;
TaskHandle_t alerter;
xSemaphoreHandle g_logqueue_sem, g_UART_sem, g_temp_sem, g_led_sem;

extern void log_task(void* pvParameters);

#endif /* LOG_H_ */
