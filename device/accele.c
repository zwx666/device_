#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
// RTOS include
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "devices/heyos_drivers.h"
#include "accele.h"

#include "heyos_def.h"
#include "heyos_type.h"

// * data producer
static u16_t rand_max = 50;
static u16_t rand_min = 5;
static u32_t data_producer(struct accele_data * acc_data, u32_t size_datalen, u32_t rand_data_type)
{
    int i;
    u32_t rand_num = 0;

    srand((uint32_t)time(0));
    if(rand_max > 1)
        rand_num = (u16_t)rand()%(rand_max - rand_min) + rand_min;
    else
        rand_num = 1;

    for(i = 0; i < rand_num; i++)
    {
        acc_data->x = (char)rand()%255 + 1;
        acc_data->y = (char)rand()%255 + 1;
        acc_data->z = (char)rand()%255 + 1;
        acc_data->tim = xTaskGetTickCount()+(i*20);
        acc_data++;
    }
    return rand_num;
}
// end data producer

static struct heyos_device dev_accele;

static void accele_Handler_IRQ(void *param)
{
    printf("accele IRQ\n");
    if(dev_accele.cbs.IRQ_callback != NULL)
    {
        dev_accele.sta = DEV_ACITVE;
        dev_accele.cbs.IRQ_callback(&dev_accele, param);
    }
}

static void finish_init(TimerHandle_t pTimer )
{
    if(dev_accele.cbs.init_finish_cb != NULL)
    {
        dev_accele.sta = DEV_ACITVE;
        dev_accele.cbs.init_finish_cb(&dev_accele, dev_accele.cbs.finish_param);
    }
}

static void acceleTimer(void)
{
    static TimerHandle_t gpio_Timer;
    gpio_Timer = xTimerCreate("acc_init", 5 * 1000, pdFALSE, NULL, finish_init);
    if(gpio_Timer != NULL)
    {
        xTimerStart(gpio_Timer, 0);
    }
}

static ry_sts_t accele_init(struct  heyos_device *dev)
{
    printf("accele init:  ");
    printf("X=0,Y=0,Z=0\n");
    acceleTimer();

    return RY_SUCC;
};

static ry_sts_t accele_close (struct heyos_device *dev)
{
    printf("accele close\n");
    return RY_SUCC;
}

static ry_sts_t accele_suspend (struct heyos_device *dev)
{
    printf("accele suspend\n");
    return RY_SUCC;
}

static ry_sts_t accele_resume (struct heyos_device *dev)
{
    printf("accele resume\n");
    return RY_SUCC;
}

static ry_sts_t accele_restart (struct heyos_device *dev)
{
    printf("accele restart\n");
    return RY_SUCC;
}

static u32_t accele_read(struct heyos_device *dev, void * buffer, u32_t *data_typ, u32_t data_max)
{
    static int _cnt = 0;
    struct accele_data *data;
    printf("accele read\n");
    *data_typ = ACCELE_XYZ;
    data = (struct accele_data *)buffer;

    _cnt = data_producer(buffer,sizeof(struct accele_data), 1);

    return _cnt;
}

static u32_t accele_write (struct heyos_device *dev, void * buffer, u32_t data_typ, u32_t data_num)
{
    printf("accele write\n");


    return data_num;
}


static ry_sts_t accele_ioctl (struct heyos_device *dev, u32_t cmd, void *args)
{
    printf("accele ioctl\n");
    switch (cmd)
    {
    case DEV_ACCELE_IOCTL_GET_DEV_ID:

        break;
    case DEV_ACCELE_IOCTL_SET_MODE:

        break;
    case DEV_ACCELE_IOCTL_SET_SAMPLE_RATE:

        break;

    case DEV_ACCELE_IOCTL_SIMU_RAND_TYPE1:
        printf("accele mode set sample 10~1\n");
        rand_max = 10; rand_min = 1;
        break;
    case DEV_ACCELE_IOCTL_SIMU_RAND_TYPE2:
        printf("accele mode set sample 60~40\n");
        rand_max = 60; rand_min = 40;
        break;
    case DEV_ACCELE_IOCTL_SIMU_RAND_TYPE3:
        printf("accele mode set sample 1\n");
        rand_max = 1; rand_min = 1;
        break;
    default:
        break;
    }

    return RY_SUCC;
}

void dev_accele_register(void)
{
    static struct device_ops acc_ops = {
        .init = accele_init,
        .close = accele_close,
        .suspend = accele_suspend,
        .resume = accele_resume,
        .read = accele_read,
        .write = accele_write,
        .ioctl = accele_ioctl,
        .restart = accele_restart,
    };

    strcpy(dev_accele.name, "accele");
    dev_accele.id = DEFAULT_DEV_ID_ACCELEROMETER;
    dev_accele.ops = &acc_ops;

    heyos_device_register(&dev_accele);
}

EXPORT_DEV_REGISTER(dev_accele_register);

void test_file(void);
