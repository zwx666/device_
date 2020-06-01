#include "dev_uart.h"
#include "heyos_def.h"
#include "devices/heyos_drivers.h"

static struct heyos_device dev_uart;
static heyos_mutex_t mutex_uart = NULL;
static heyos_sem_t sem_uart = NULL;

static void uart_isr_callback(void)
{
    /* enter interrupt */
    rt_interrupt_enter();
    
    heyos_sem_post(sem_uart);

    /* leave interrupt */
    rt_interrupt_leave();
}

static ry_sts_t uart_init(struct heyos_device *dev)
{
    if (mutex_uart == NULL) {
        mutex_uart = heyos_mutex_create("uart");
        if (mutex_uart == NULL) {
            return RY_ERR_INIT_FAIL;
        }
    }

    if (sem_uart == NULL) {
        sem_uart = heyos_sem_create("uart");
        if (sem_uart == NULL) {
            return RY_ERR_INIT_FAIL;
        }
    }

    if (Cy_SCB_UART_Init(UART_DEBUG_SCB__HW, &UART_DEBUG_config, &UART_DEBUG_context) != CY_SCB_UART_SUCCESS) {
        rt_assert_handler("UART0 init", __FUNCTION__, __LINE__);
    }
    Cy_SCB_UART_Enable(UART_DEBUG_SCB__HW);

    /* Unmasking only the RX fifo not empty interrupt bit */
    UART_DEBUG_SCB__HW->INTR_RX_MASK = SCB_INTR_RX_MASK_NOT_EMPTY_Msk;

    /* Interrupt Settings for UART */
    Cy_SysInt_Init(&UART_DEBUG_SCB_IRQ_cfg, uart_isr_callback);

    return RY_SUCC;
}

static u32_t uart_read(struct heyos_device *dev, void *buffer, u32_t *data_typ, u32_t length)
{
    bool transfer_succeeded = true;
    uint32_t rx_valid_cnt;
    u32_t i;

    heyos_mutex_take(mutex_uart);

    for (i = 0; i < length; i++) {
        heyos_sem_wait(sem_uart);
        rx_valid_cnt = Cy_SCB_UART_GetNumInRxFifo(UART_DEBUG_SCB__HW);
        if(rx_valid_cnt > 0) {
            buffer[i] = Cy_SCB_UART_Get(UART_DEBUG_SCB__HW);
        } else {
            transfer_succeeded = false;
            break;
        }
    }
   
    heyos_mutex_release(mutex_uart);

    return transfer_succeeded;
}

static u32_t uart_write(struct heyos_device *dev, void *buffer, u32_t data_typ, u32_t length)
{	
    bool transfer_succeeded = true;
    u32_t i;

    heyos_mutex_take(mutex_sem);
    for (i = 0; i < length; i++) {
        Cy_SCB_UART_Put(UART_DEBUG_SCB__HW, (buffer[i] & 0x1FF)); 
    }
    heyos_mutex_release(mutex_sem);

    return transfer_succeeded;
}

void dev_uart_register(void)
{
    static struct device_ops uart_ops = {
        .init = uart_init,
        .close = NULL,
        .suspend = NULL,
        .resume = NULL,
        .read = uart_read,
        .write = uart_write,
        .ioctl = NULL,
        .restart = NULL,
    };

    strcpy(dev_uart.name, "uart");
    dev_uart.id = DEFAULT_DEV_ID_UART;
    dev_uart.ops = &uart_ops;

    heyos_device_register(&dev_uart);
}

EXPORT_DEV_REGISTER(dev_uart_register);





