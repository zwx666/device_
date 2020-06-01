#include "dev_hr.h"
#include "heyos_def.h"
#include "ryos_timer.h"
#include "devices/heyos_drivers.h"

#define DEV_HR_DEBUG                         (TRUE)

#if(DEV_HR_DEBUG == TRUE)
#define HR_LOG_DEBUG                         LOG_DEBUG             
#else
#define HR_LOG_DEBUG(...)
#endif

static struct heyos_device dev_hr;
static heyos_mutex_t mutex_hr = NULL;

static ry_sts_t hr_init(struct heyos_device *dev)
{
    #if 0
    ryos_delay_ms(120);
    hal_reset_0();
    ryos_delay_ms(12);
    hal_reset_1();
    ryos_delay_ms(20);
    #endif
            
    if (mutex_hr == NULL) {
        mutex_hr = heyos_mutex_create("motor");
        if (mutex_hr == NULL) {
            return RY_ERR_INIT_FAIL;
        }
    }

    ry_hal_i2cm_init(I2C_IDX_HR); 

    HR_LOG_DEBUG("hr init successful\r\n");

    return RY_SUCC;
}

static ry_sts_t hr_close(struct heyos_device *dev)
{
    return RY_SUCC;
}

static ry_sts_t hr_suspend(struct heyos_device *dev)
{
    return RY_SUCC;
}

static ry_sts_t hr_resume(struct heyos_device *dev)
{
    return RY_SUCC;
}

static ry_sts_t hr_restart(struct heyos_device *dev)
{
    return RY_SUCC;
}

static u32_t hr_read(struct heyos_device *dev, void *buffer, u32_t *data_typ, u32_t length)
{
    bool transfer_succeeded = true;

    heyos_mutex_take(mutex_hr);
    ry_hal_i2cm_rx(I2C_IDX_HR, (uint8_t*)buffer, length);
    heyos_mutex_release(mutex_hr);

    return transfer_succeeded;
}

static u32_t hr_write(struct heyos_device *dev, void *buffer, u32_t data_typ, u32_t length)
{	
    bool transfer_succeeded = true;
    
    heyos_mutex_take(mutex_hr);
    ry_hal_i2cm_tx(I2C_IDX_HR, (uint8_t*)buffer, length);
    heyos_mutex_release(mutex_hr);

    return transfer_succeeded;
}

static ry_sts_t hr_ioctl(struct heyos_device *dev, u32_t cmd, void *args)
{
    ry_sts_t sta = RY_SUCC;

     return sta;
}

void dev_hr_register(void)
{
    static struct device_ops hr_ops = {
        .init = hr_init,
        .close = hr_close,
        .suspend = hr_suspend,
        .resume = hr_resume,
        .read = hr_read,
        .write = hr_write,
        .ioctl = hr_ioctl,
        .restart = hr_restart,
    };

    strcpy(dev_hr.name, "hr");
    dev_hr.id = DEFAULT_DEV_ID_HRM;
    dev_hr.ops = &hr_ops;

    heyos_device_register(&dev_hr);
}

EXPORT_DEV_REGISTER(dev_hr_register);

