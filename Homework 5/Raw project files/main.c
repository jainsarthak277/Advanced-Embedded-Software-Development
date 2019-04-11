//*****************************************************************************
//
// main.c - Code to integrate TMP102 temperature sensor to TIVA EK-TM4C1294XL
// using FreeRTOS.
//
// Copyright (c) 2013-2017 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.1.4.178 of the EK-TM4C1294XL Firmware Package.
//
//*****************************************************************************

//*****************************************************************************
// Reference is given to https://www.digikey.com/eewiki/display/microcontroller/I2C+Communication+with+the+TI+Tiva+TM4C123GXL*/
// for I2C initialization and https://www.freertos.org/Documentation/FreeRTOS_Reference_Manual_V9.0.0.pdf
// for FreeRTOS APIs
//*****************************************************************************

#include "inc/log.h"
#include "inc/temp.h"
#include "inc/led.h"
#include "inc/alert.h"

#define QUEUE_SIZE      (10)
#define TEMPERATURE_INTERVAL    (120000000)
#define LED_INTERVAL            (12000000)

//*****************************************************************************
//
// Global variable to hold the system clock speed.
//
//*****************************************************************************
uint32_t g_ui32SysClock;


//*****************************************************************************
//
// Global instance structure for the I2C master driver.
//
//*****************************************************************************
tI2CMInstance g_sI2CInst;

//*****************************************************************************
//
// Counter value used by the FreeRTOS run time stats feature.
// http://www.freertos.org/rtos-run-time-stats.html
//
//*****************************************************************************
volatile unsigned long g_vulRunTimeStatsCountValue;

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}

#endif


//*****************************************************************************
//
// This hook is called by FreeRTOS when an stack overflow error is detected.
//
//*****************************************************************************
void
vApplicationStackOverflowHook(xTaskHandle *pxTask, char *pcTaskName)
{
    //
    // This function can not return, so loop forever.  Interrupts are disabled
    // on entry to this function, so no processor interrupts will interrupt
    // this loop.
    //
    while(1)
    {
    }
}

//*****************************************************************************
//
// Initialize UART.
//
//*****************************************************************************
void
ConfigureUART(void)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    UARTStdioConfig(0, 115200, 16000000);
}

void i2c_init(void)
{
    //enable GPIO peripheral that contains I2C 2
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

    //enable I2C module 2
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C2);

    // Configure the pin muxing for I2C2 functions on port N4 and N5.
    GPIOPinConfigure(GPIO_PN5_I2C2SCL);
    GPIOPinConfigure(GPIO_PN4_I2C2SDA);

    // Select the I2C function for these pins.
    GPIOPinTypeI2CSCL(GPIO_PORTN_BASE, GPIO_PIN_5);
    GPIOPinTypeI2C(GPIO_PORTN_BASE, GPIO_PIN_4);

    // Enable and initialize the I2C0 master module.  Use the system clock for
    // the I2C2 module.  The last parameter sets the I2C data transfer rate.
    // If false the data rate is set to 100kbps and if true the data rate will
    // be set to 400kbps.
    I2CMasterInitExpClk(I2C2_BASE, g_ui32SysClock, false);
}

void timer_init(void)
{
    //
    // Enable the peripherals used by this example.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

    //
    // Enable processor interrupts.
    //
    ROM_IntMasterEnable();

    //
    // Configure the two 32-bit periodic timers.
    //
    ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    ROM_TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
    ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, TEMPERATURE_INTERVAL);
    ROM_TimerLoadSet(TIMER1_BASE, TIMER_A, LED_INTERVAL);

    //
    // Setup the interrupts for the timer timeouts.
    //
    ROM_IntEnable(INT_TIMER0A);
    ROM_IntEnable(INT_TIMER1A);
    ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    ROM_TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    //
    // Enable the timers.
    //
    ROM_TimerEnable(TIMER0_BASE, TIMER_A);
    ROM_TimerEnable(TIMER1_BASE, TIMER_A);
}

void Timer0_Temp_Handler(void)
{
    ROM_IntMasterDisable();
    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    xSemaphoreTake(g_UART_sem, portMAX_DELAY);
    UARTprintf("\nEntered Temperature sensor handler");
    xSemaphoreGive(g_UART_sem);

    xSemaphoreGive(g_temp_sem);
    ROM_IntMasterEnable();
}

void Timer1_LED_Handler(void)
{
    ROM_IntMasterDisable();
    ROM_TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    xSemaphoreTake(g_UART_sem, portMAX_DELAY);
    UARTprintf("\nEntered LED handler");
    xSemaphoreGive(g_UART_sem);

    xSemaphoreGive(g_led_sem);
    ROM_IntMasterEnable();

}

//*****************************************************************************
//
// Initialize FreeRTOS and start the initial set of tasks.
//
//*****************************************************************************
int
main(void)
{
    //
    // Configure the system frequency.
    //
    g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                             SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL |
                                             SYSCTL_CFG_VCO_480), 120000000);

    ConfigureUART();

    i2c_init();

    log_queue = xQueueCreate(QUEUE_SIZE, sizeof(struct message));
    if(log_queue == NULL)
    {
        UARTprintf("\nError creating logger message queue\n");
        exit(1);
    }

    g_logqueue_sem = xSemaphoreCreateMutex();
    if(g_logqueue_sem == NULL)
    {
        UARTprintf("\nError creating logger queue semaphore\n");
        exit(1);
    }

    g_UART_sem = xSemaphoreCreateMutex();
    if(g_UART_sem == NULL)
    {
        UARTprintf("\nError creating UART semaphore\n");
        exit(1);
    }

    g_temp_sem = xSemaphoreCreateBinary();
    if(g_temp_sem == NULL)
    {
        UARTprintf("\nError creating temperature task semaphore\n");
        exit(1);
    }

    g_led_sem = xSemaphoreCreateBinary();
    if(g_led_sem == NULL)
    {
        UARTprintf("\nError creating LED task semaphore\n");
        exit(1);
    }


    UARTprintf("Welcome to AESD EX. 5 on TM4C1294XL\n");

    if(xTaskCreate(log_task, (const portCHAR*)"LOGGER TASK", 128, NULL,
                   tskIDLE_PRIORITY, NULL) != pdTRUE)
    {
        UARTprintf("\nError creating logger task\n");
        exit(1);
    }

    if(xTaskCreate(temp_task, (const portCHAR*)"TEMPERATURE TASK", 128, NULL,
                   tskIDLE_PRIORITY, NULL) != pdTRUE)
    {
        UARTprintf("\nError creating temperature task\n");
        exit(1);
    }

    if(xTaskCreate(led_task, (const portCHAR*)"LED TASK", 128, NULL,
                   tskIDLE_PRIORITY, NULL) != pdTRUE)
    {
        UARTprintf("\nError creating led task\n");
        exit(1);
    }

    if(xTaskCreate(alert_task, (const portCHAR*)"ALERT TASK", 128, NULL,
                   tskIDLE_PRIORITY, &alerter) != pdTRUE)
    {
        UARTprintf("\nError creating alert task\n");
        exit(1);
    }

    timer_init();

    //
    // Start the scheduler.  This should not return.
    //
    vTaskStartScheduler();

    //
    // In case the scheduler returns for some reason, print an error and loop
    // forever.
    //
    UARTprintf("RTOS scheduler returned unexpectedly.\n");
    while(1)
    {
        //
        // Do Nothing.
        //
    }
}
