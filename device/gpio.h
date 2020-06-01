#ifndef __GPIO_H
#define __GPIO_H

#include "heyos_type.h"

#define DEV_HOME_LED_IOCTL_GET_DEV_ID                  1

#define DEV_GPIO_CMD_SET_MODE_GENERAL                  2
#define DEV_GPIO_CMD_SET_MODE_PWM                      3
#define DEV_GPIO_CMD_SET_MODE_READ                     4
#define DEV_GPIO_CMD_SET_MODE_ADC                      5
#define DEV_GPIO_CMD_SET_MODE_DAC                      6

#define DEV_HOME_LED_SET_ON                           10
#define DEV_HOME_LED_SET_OFF                          11
#define DEV_HOME_LED_BREATH                           12


#endif
