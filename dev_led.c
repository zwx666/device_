#include "dev_led.h"
#include "heyos_def.h"
#include "ryos_timer.h"
#include "devices/heyos_drivers.h"

//mcu include 
#include "tcpwm/cy_tcpwm.h"
#include "PWM_MOTOR_1.h"
#include "PWM_MOTOR_2.h"

#define DEV_LED_DEBUG                         (TRUE)

#if(DEV_LED_DEBUG == TRUE)
#define LED_LOG_DEBUG                         LOG_DEBUG             
#else
#define LED_LOG_DEBUG(...)
#endif

static struct heyos_device dev_led;
static heyos_mutex_t mutex_led = NULL;

// pmw = (pwm1^pwm2)&(pwm3)
// pwm1,pwm2-->19800Hz
// pmw3 --> 100KHz
static void led_breath_config_pwm(uint32_t pwm1_period, uint32_t pwm1_compare,
                                            uint32_t pwm2_period, uint32_t pwm2_compare,
                                                      uint32_t pwm3_period, uint32_t pwm3_compare)
{
    cy_stc_tcpwm_pwm_config_t config;
    pmic_set_peripheral_power(pmic_ry_peripheral_home_led, true);

    config = PWM_BREATH_CTRL_1_config;
    config.period0 = pwm1_period;
    config.compare0 = pwm1_compare;
    Cy_TCPWM_PWM_Init(PWM_BREATH_CTRL_1_HW, PWM_BREATH_CTRL_1_CNT_NUM, &config);
    Cy_TCPWM_Enable_Multiple(PWM_BREATH_CTRL_1_HW, PWM_BREATH_CTRL_1_CNT_MASK);

    config = PWM_BREATH_CTRL_2_config;
    config.period0 = pwm2_period;
    config.compare0 = pwm2_compare;
    Cy_TCPWM_PWM_Init(PWM_BREATH_CTRL_2_HW, PWM_BREATH_CTRL_2_CNT_NUM, &config);
    Cy_TCPWM_Enable_Multiple(PWM_BREATH_CTRL_2_HW, PWM_BREATH_CTRL_2_CNT_MASK);

    config = PWM_BREATH_CTRL_3_config;
    config.period0 = pwm3_period;
    config.compare0 = pwm3_compare;
    Cy_TCPWM_PWM_Init(PWM_BREATH_CTRL_3_HW, PWM_BREATH_CTRL_3_CNT_NUM, &config);
    Cy_TCPWM_Enable_Multiple(PWM_BREATH_CTRL_3_HW, PWM_BREATH_CTRL_3_CNT_MASK);

    Cy_TCPWM_TriggerStart(PWM_BREATH_CTRL_1_HW, PWM_BREATH_CTRL_1_CNT_MASK);
    Cy_TCPWM_TriggerStart(PWM_BREATH_CTRL_2_HW, PWM_BREATH_CTRL_2_CNT_MASK);
    Cy_TCPWM_TriggerStart(PWM_BREATH_CTRL_3_HW, PWM_BREATH_CTRL_3_CNT_MASK);
}

static ry_sts_t led_off(void)
{
    Cy_TCPWM_Disable_Multiple(PWM_BREATH_CTRL_1_HW, PWM_BREATH_CTRL_1_CNT_MASK);
    Cy_TCPWM_Disable_Multiple(PWM_BREATH_CTRL_2_HW, PWM_BREATH_CTRL_2_CNT_MASK);
    Cy_TCPWM_Disable_Multiple(PWM_BREATH_CTRL_3_HW, PWM_BREATH_CTRL_3_CNT_MASK);
    pmic_set_peripheral_power(pmic_ry_peripheral_home_led, false);
    return RY_SUCC;
}

static ry_sts_t led_on_weak(void)
{
    breath_led_config_pwm(199, 0, 199, 200, 99, component_home_led_get_default_brightless());    //50% 1K常亮
    return RY_SUCC;
}

static ry_sts_t led_on_mediu(void)
{
    breath_led_config_pwm(199, 0, 199, 200, 99, component_home_led_get_default_brightless());    //50% 1K常亮
    return RY_SUCC;
}

static ry_sts_t led_on_strong(void)
{
    breath_led_config_pwm(199, 0, 199, 200, 99, component_home_led_get_default_brightless());    //50% 1K常亮
    return RY_SUCC;
}

static ry_sts_t led_breath_off(void)
{
    Cy_TCPWM_Disable_Multiple(PWM_BREATH_CTRL_1_HW, PWM_BREATH_CTRL_1_CNT_MASK);
    Cy_TCPWM_Disable_Multiple(PWM_BREATH_CTRL_2_HW, PWM_BREATH_CTRL_2_CNT_MASK);
    Cy_TCPWM_Disable_Multiple(PWM_BREATH_CTRL_3_HW, PWM_BREATH_CTRL_3_CNT_MASK);
    pmic_set_peripheral_power(pmic_ry_peripheral_home_led, false);
    return RY_SUCC;
}

static ry_sts_t led_breath_slow(void)
{
    return RY_SUCC;
}

static ry_sts_t led_breath_mediu(void)
{
    return RY_SUCC;
}

static ry_sts_t led_breath_fast(void)
{
    return RY_SUCC;
}

static ry_sts_t led_init(struct heyos_device *dev)
{
    if (mutex_led == NULL) {
          mutex_led = heyos_mutex_create("led");
          if (mutex_led == NULL) {
              return RY_ERR_INIT_FAIL;
          }
    }

    LED_LOG_DEBUG("led init successful\r\n");
    return RY_SUCC;
}

static ry_sts_t led_ioctl(struct heyos_device *dev, u32_t cmd, void *args)
{
    ry_sts_t sta = RY_SUCC;

    heyos_mutex_take(mutex_led);

    switch (cmd) {
        case DEV_LED_IOCTR_OFF:
            led_off();
            LED_LOG_DEBUG("led off\r\n");
            break;

        case DEV_LED_IOCTR_WEAK:
            led_on_weak();
            LED_LOG_DEBUG("led on weak\r\n");
            break;

        case DEV_LED_IOCTR_MEDIU:
            led_on_mediu();
            LED_LOG_DEBUG("led on mediu\r\n");
            break;

        case DEV_LED_IOCTR_STRONG:
            led_on_strong();
            LED_LOG_DEBUG("led on strong\r\n");
            break;

        case DEV_LED_IOCTR_BREATH_OFF:
            led_breath_off();
            LED_LOG_DEBUG("led breath off\r\n");
            break;

        case DEV_LED_IOCTR_BREATH_SLOW:
            led_breath_slow();
            LED_LOG_DEBUG("led breath slow\r\n");
            break;

        case DEV_LED_IOCTR_BREATH_MEDIU:
            led_breath_mediu();
            LED_LOG_DEBUG("led breath mediu\r\n");
            break;

        case DEV_LED_IOCTR_BREATH_FAST:
            led_breath_fast();
            LED_LOG_DEBUG("led breath fast\r\n");
            break;

        default:
            sta = RY_ERR_INVALID_PARA;
            LED_LOG_DEBUG("RY_ERR_INVALID_PARA\r\n");
            break;
    }

    heyos_mutex_release(mutex_led);
    return sta;
}

void dev_led_register(void)
{
    static struct device_ops led_ops = {
        .init = led_init,
        .close = NULL,
        .suspend = NULL,
        .resume = NULL,
        .read = NULL,
        .write = NULL,
        .ioctl = led_ioctl,
        .restart = NULL,
    };

    strcpy(dev_led.name, "led");
    dev_led.id = DEFAULT_DEV_ID_LED;
    dev_led.ops = &led_ops;

    heyos_device_register(&dev_led);
}

EXPORT_DEV_REGISTER(dev_led_register);


