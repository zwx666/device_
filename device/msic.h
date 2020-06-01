#ifndef _MISC_H
#define _MISC_H

#include "heyos_type.h"
/****** miscellaneous device *********/

typedef enum led_t
{
    LED_OFF = 0,
    LED_ON = 1,
    LED_BREATH,
};

typedef enum motor_t
{
    MOTOR_OFF = 0,
    MOTOR_SLOW,
    MOTOR_MEDIU,
    MOTOR_FAST,
    MOTOR_HIGH,
};


void set_button_irq_cb(void *(*func));

int motor_contorl(uint32_t cmd);
int home_led_control(uint32_t cmd);



#endif
