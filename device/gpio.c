#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
// RTOS include
#include "FreeRTOS.h"
#include "task.h"

// heos sys incle
#include "devices/heyos_drivers.h"
#include "gpio.h"

static struct heyos_device dev_gpio;

u32_t irq_pin_num = 0;
static void gpio_irq_handle(void *param)
{
    srand((uint32_t)time(0));
    irq_pin_num = (u16_t)rand()%(20 - 0) + 0;

    if(dev_gpio.cbs.IRQ_callback != NULL)
    {
        dev_gpio.cbs.IRQ_callback(&dev_gpio, &irq_pin_num);
    }
}

void TimerInit(void);
static ry_sts_t gpio_init (struct heyos_device *dev)
{
    printf("gpio init\n");
    TimerInit();

return RY_SUCC;
}

static ry_sts_t gpio_close (struct heyos_device *dev)
{
    printf("gpio close\n");
    return RY_SUCC;
}

static ry_sts_t gpio_ioctl (struct heyos_device *dev, u32_t cmd, void *args)
{
    switch (cmd)
    {
    case DEV_HOME_LED_SET_OFF:
        printf("home led turn off\n");
        break;
    case DEV_HOME_LED_SET_ON:
        printf("home led turn on\n");
        break;
    case DEV_HOME_LED_BREATH:
        printf("home led turn breathing\n");
        break;
    default:
        break;
    }
    return RY_SUCC;
}

void dev_gpio_register(void)
{
    static struct device_ops dev_ops = {
        .init    =  gpio_init,
        .close   =  gpio_close,
        .ioctl   =  gpio_ioctl,
    };

    strcpy(dev_gpio.name, "botton");
    dev_gpio.id = DEFAULT_DEV_ID_BUTTON;
    dev_gpio.ops = &dev_ops;

    heyos_device_register(&dev_gpio);

}
EXPORT_DEV_REGISTER(dev_gpio_register);

// test code timer to trigger gpip irq
#include "timers.h"
void Timer_cb( TimerHandle_t pTimer)
{
    gpio_irq_handle(NULL);
}

void TimerInit(void)
{
    static TimerHandle_t gpio_Timer;
    gpio_Timer = xTimerCreate("io_irq", 30 * 1000, pdTRUE, NULL, Timer_cb);
    if(gpio_Timer != NULL)
    {
        xTimerStart(gpio_Timer, 0);
    }
}
