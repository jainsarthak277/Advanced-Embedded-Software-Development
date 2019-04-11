/*
 * led.h
 *
 *  Created on: Apr 10, 2019
 *      Author: Sarthak Jain
 *
 *      Contains all headers used for led.c
 */

#ifndef INC_LED_H_
#define INC_LED_H_

#include "log.h"

void led_config(void);
extern void led_task(void* pvParameters);

#endif /* INC_LED_H_ */
