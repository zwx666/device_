#include "dev_motor.h"
#include "heyos_def.h"
#include "ryos_timer.h"
#include "devices/heyos_drivers.h"

//mcu include 
#include "tcpwm/cy_tcpwm.h"
#include "PWM_MOTOR_1.h"
#include "PWM_MOTOR_2.h"

#define DEV_MOTOR_DEBUG                         (TRUE)

#if(DEV_MOTOR_DEBUG == TRUE)
#define MOTOR_LOG_DEBUG                         LOG_DEBUG             
#else
#define MOTOR_LOG_DEBUG(...)
#endif

#define MOTOR_STATUS_IS_OFF                     (0)
#define MOTOR_STATUS_IS_ON                      (1)

static struct heyos_device dev_motor;
static heyos_mutex_t mutex_motor = NULL;
static ry_timer_t motor_cutoff_timer = NULL;
static u8_t motor_status_flag = MOTOR_STATUS_IS_OFF;    

static void motor_timeout_timer_start(uint32_t timeout_ms);

static void motor_diable_pwm(void)
{
    // ry_hal_gpio_set(GPIO_IDX_MOTOR_PWM, ry_gpio_set_disable);
    Cy_TCPWM_Disable_Multiple(PWM_MOTOR_1_HW, PWM_MOTOR_1_CNT_MASK);
    Cy_TCPWM_Disable_Multiple(PWM_MOTOR_2_HW, PWM_MOTOR_2_CNT_MASK);
    pmic_set_peripheral_power(pmic_ry_peripheral_motor, false);
}

static void motor_config_pwm(uint32_t period1, uint32_t compare1,
                                    uint32_t period2, uint32_t compared2)
{
    // ry_hal_gpio_set(GPIO_IDX_MOTOR_PWM, ry_gpio_set_output_strong_inoff);
    pmic_set_peripheral_power(pmic_ry_peripheral_motor, true);
    cy_stc_tcpwm_pwm_config_t config;

    config = PWM_MOTOR_1_config;
    config.period0 = period1;
    config.compare0 = compare1;
    (void) Cy_TCPWM_PWM_Init(PWM_MOTOR_1_HW, PWM_MOTOR_1_CNT_NUM, &config);
    Cy_TCPWM_Enable_Multiple(PWM_MOTOR_1_HW, PWM_MOTOR_1_CNT_MASK);
    config = PWM_MOTOR_2_config;
    config.period0 = period2;
    config.compare0 = compared2;
    (void) Cy_TCPWM_PWM_Init(PWM_MOTOR_2_HW, PWM_MOTOR_2_CNT_NUM, &config);
    Cy_TCPWM_Enable_Multiple(PWM_MOTOR_2_HW, PWM_MOTOR_2_CNT_MASK);

    Cy_TCPWM_TriggerStart(PWM_MOTOR_1_HW, PWM_MOTOR_1_CNT_MASK);
    Cy_TCPWM_TriggerReloadOrIndex(PWM_MOTOR_2_HW, PWM_MOTOR_2_CNT_MASK);
}

static ry_sts_t motor_init(struct heyos_device *dev)
{    
    if (mutex_motor == NULL) {
          mutex_motor = heyos_mutex_create("motor");
          if (mutex_motor == NULL) {
              return RY_ERR_INIT_FAIL;
          }
    }

    ry_timer_param init_timer = {
        .name = "motor",
        .data = NULL,
        .timeout_CB = motor_timeout_handler,
    };

    if (motor_cutoff_timer == NULL) {
        motor_cutoff_timer = ry_timer_create(&init_timer);          
        if (motor_cutoff_timer == NULL) {
              return RY_ERR_INIT_FAIL;
          }
    }

    MOTOR_LOG_DEBUG("motor init successful\r\n");

    return RY_SUCC;
}

static ry_sts_t motor_off(void) 
{
    motor_diable_pwm();
    return RY_SUCC;
}

static ry_sts_t motor_slow(void)
{
    motor_config_pwm(9999, 10000, 1000*1000, 150*1000);   
    motor_timeout_timer_start(150);
    return RY_SUCC;
}

static ry_sts_t motor_mediu(void)
{
    motor_config_pwm(9999, 10000, 1000*1000, 200*1000);   
    motor_timeout_timer_start(200);
    return RY_SUCC;
}

static ry_sts_t motor_fast(void)
{
    motor_config_pwm(9999, 10000, 1000*1000, 500*1000);   
    motor_timeout_timer_start(500);
    return RY_SUCC;
}

static ry_sts_t motor_high(void)
{
    motor_config_pwm(9999, 10000, 1000*1000, 500*1000);   
    motor_timeout_timer_start(60 * 1000);    //60s超时才停掉，如果没有主动停
    return RY_SUCC;
}

static void motor_timeout_handler(void* p_arg)
{
    //motor_off();
    MOTOR_LOG_DEBUG("motor_timeout_handler\r\n");
    motor_ioctl(&dev_motor, DEV_MOTOR_IOCTR_OFF, DEV_MOTOR_SWITCH_FORCE);     //定时器需要配置成软件定时器，中断服务程序像线程一样参与调度
}

static void motor_timeout_timer_start(uint32_t timeout_ms)
{
    uint32_t tick = (timeout_ms*RT_TICK_PER_SECOND)/1000 + 1;   //保证时长, 只能延后stop
    ry_timer_start_tick(motor_cutoff_timer, tick, motor_timeout_handler, NULL);
}

static void motor_timeout_timer_stop(void)
{
    ry_timer_stop(motor_cutoff_timer);
}

static ry_sts_t motor_ioctl(struct heyos_device *dev, u32_t cmd, void *args)
{
    ry_sts_t sta = RY_SUCC;
    u32_t motor_switch = *((u32_t *)args);

    heyos_mutex_take(mutex_motor);
    
    if (motor_cut == DEV_MOTOR_SWITCH_NULL && motor_status_flag == MOTOR_STATUS_IS_ON) {
        MOTOR_LOG_DEBUG("motor is on not switch\r\n");
        goto exit;
    } else {
        motor_off();
    }

    if (ry_timer_isRunning(motor_cutoff_timer)) {
        motor_timeout_timer_stop(); 
    }

    switch (cmd) {
        case DEV_MOTOR_IOCTR_OFF:
            motor_off();
            MOTOR_LOG_DEBUG("motor off\r\n");
            break;

        case DEV_MOTOR_IOCTR_SLOW:
            motor_slow();
            MOTOR_LOG_DEBUG("motor slow\r\n");
            break;

        case DEV_MOTOR_IOCTR_MEDIU:
            motor_mediu();
            MOTOR_LOG_DEBUG("motor mediu\r\n");
            break;

        case DEV_MOTOR_IOCTR_FAST:
            motor_fast();
            MOTOR_LOG_DEBUG("motor fast\r\n");
            break;

        case DEV_MOTOR_IOCTR_HIGH:
            motor_high();
            MOTOR_LOG_DEBUG("motor high\r\n");
            break;

        default:
            sta =  RY_ERR_INVALID_PARA;
            MOTOR_LOG_DEBUG("RY_ERR_INVALID_PARA\r\n");
            goto exit;
            break;
    }

    if (cmd != DEV_MOTOR_IOCTR_OFF) {
        motor_status_flag = MOTOR_STATUS_IS_ON;
    } else {
        motor_status_flag = MOTOR_STATUS_IS_OFF;
    }

    MOTOR_LOG_DEBUG("motor_status_flag = %d\r\n", motor_status_flag);
exit:
    heyos_mutex_release(mutex_motor);
    return sta;
}

void dev_motor_register(void)
{
    static struct device_ops motor_ops = {
        .init = motor_init,
        .close = NULL,
        .suspend = NULL,
        .resume = NULL,
        .read = NULL,
        .write = NULL,
        .ioctl = motor_ioctl,
        .restart = NULL,
    };

    strcpy(dev_motor.name, "motor");
    dev_motor.id = DEFAULT_DEV_ID_MOTOR;
    dev_motor.ops = &motor_ops;

    heyos_device_register(&dev_motor);
}

EXPORT_DEV_REGISTER(dev_motor_register);



