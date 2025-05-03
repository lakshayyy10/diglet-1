#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include "stepper_control.h"

LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);

int main(void)
{
    printk("agriprobe initialize\n"); 
    stepper_init();
    while (1) {
        k_sleep(K_SECONDS(1));
    }
    
    return 0;
}
