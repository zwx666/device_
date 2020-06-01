
#include <stdio.h>
#include <string.h>
// RTOS include
#include "FreeRTOS.h"
#include "task.h"

#include "heyos_type.h"

#include "devices/heyos_drivers.h"
#include "gpio.h"

static void set_button_irq_cb(void *parameters)
{
    printf("button 1 donw\n");
}

void misc_dev_init(void)
{

}

int home_led_control(uint32_t control)
{

    return 1;
}



int motor_contorl(uint32_t cmd);
int home_led_control(uint32_t cmd);
