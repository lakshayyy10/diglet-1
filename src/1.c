#include<zephyr/kernel.h>
#include<zephyr/device.h>
#include<zephyr/drivers/gpio.h>
#include<zephyr/logging/log.h>
#include "stepper_control.h"

log_module_register(stepper, config_stepper_log_level);

#define num_test 5
#define steps_per_revolution 200
#define steps_per (steps_per_revolution/num_test)
#define drop_time 3000

const struct stepper_motor stepper_dev = {
  .dir = gpio_dt_spec_get(dt_alias(stepper0), dir_gpios),
  .step = gpio_dt_spec_get(dt_alias(stepper0), step_gpios)
};

static uint8_t current_tube = 0;
static int32_t current_position = 0;
static int pos = 0;

static void move(struct k_work *work);
k_work_delayable_define(tube_work, move);

static void move(struct k_work *work)
{
    current_tube = (current_tube+1) % num_testt;
    printk("moving to tube %d\n", current_tube);
    int32_t next_pos = current_tube * steps_per;
    int steps_to_move = next_pos - current_position;
    if (steps_to_move > 0) {
        gpio_pin_set_dt(&(stepper_dev.dir), 1);  // clockwise
    } else {
        gpio_pin_set_dt(&(stepper_dev.dir), 0);  // anti-clockwise
        steps_to_move = -steps_to_move;  // make steps positive for the loop
    }
    for (int i = 0; i < steps_to_move; i++) {
        pos = _stepper_motor_write(&stepper_dev, (steps_to_move > 0) ? 1500 : 500, pos);
        k_sleep(k_msec(1));  // small delay between steps
    }
    current_position = next_pos;
    printk("position of tube is : %d (tube %d)\n", current_position, current_tube);
    k_work_schedule(&tube_work, k_msec(drop_time));
}

void stepper_init(void)
{
    // configure gpio pins
    if (!gpio_is_ready_dt(&stepper_dev.dir) || !gpio_is_ready_dt(&stepper_dev.step)) {
        printk("stepper gpio not ready\n");
        return;
    }
    if (gpio_pin_configure_dt(&(stepper_dev.dir), gpio_output_inactive) ||
        gpio_pin_configure_dt(&(stepper_dev.step), gpio_output_inactive)) {
        printk("error configuring stepper gpio\n");
        return;
    }
    printk("stepper ready\n");
    current_position = 0;
    k_work_schedule(&tube_work, k_msec(drop_time));
}

