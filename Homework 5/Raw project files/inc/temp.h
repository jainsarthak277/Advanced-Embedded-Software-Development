/*
 * temp.h
 *
 *  Created on: Apr 10, 2019
 *      Author: Sarthak Jain
 *
 *      Contains all headers used for temp.c
 */

#ifndef INC_TEMP_H_
#define INC_TEMP_H_

#include "log.h"

#define SLAVE_ADDRESS   (0x48)
#define CONFIG_REG      (0x00)
#define THRESHOLD       (25)

float read_temp(void);
extern void temp_task(void* pvParameters);

#endif /* INC_TEMP_H_ */
