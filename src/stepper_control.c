#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <stdlib.h>
#include "stepper_control.h"

LOG_MODULE_REGISTER(stepper, CONFIG_STEPPER_LOG_LEVEL);

#define NUM_TEST 5
#define STEPS_PER_REVOLUTION 200
#define STEPS_PER (STEPS_PER_REVOLUTION/NUM_TEST)
#define DROP_TIME 3000

/* Tarzan Channel values */
static uint16_t channel_range[] = {0, 950, 2047};

const struct stepper_motor stepper_dev = {
  .dir = GPIO_DT_SPEC_GET(DT_ALIAS(stepper0), dir_gpios),
  .step = GPIO_DT_SPEC_GET(DT_ALIAS(stepper0), step_gpios)
};

static uint8_t current_tube = 0;
static int32_t current_position = 0;
static int pos = 0;

static void move(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(tube_work, move);

int stepper_motor_write(const struct stepper_motor *motor, uint16_t cmd, int pos) {
  if (abs(cmd - channel_range[1]) < 200) {
    return pos;
  }

  if (cmd > channel_range[1]) {
    gpio_pin_set_dt(&(motor->dir), 1); // clockwise
    pos += 1;
  } else {
    gpio_pin_set_dt(&(motor->dir), 0); // anti-clockwise
    pos -= 1;
  }
  switch (pos & 0x03) {
  case 0:
    gpio_pin_set_dt(&(motor->step), 0);
    break;
  case 1:
    gpio_pin_set_dt(&(motor->step), 1);
    break;
  case 2:
    gpio_pin_set_dt(&(motor->step), 1);
    break;
  case 3:
    gpio_pin_set_dt(&(motor->step), 0);
    break;
  }
  
  return pos;
}

static void move(struct k_work *work)
{
    current_tube = (current_tube + 1) % NUM_TEST;
    printk("Moving to tube %d\n", current_tube);
    
    int32_t next_pos = current_tube * STEPS_PER;
    int steps_to_move = next_pos - current_position;
    
    if (steps_to_move > 0) {
        gpio_pin_set_dt(&(stepper_dev.dir), 1);  // clockwise
    } else {
        gpio_pin_set_dt(&(stepper_dev.dir), 0);  // anti-clockwise
        steps_to_move = -steps_to_move;  // make steps positive for the loop
    }
    
    for (int i = 0; i < steps_to_move; i++) {
        pos = stepper_motor_write(&stepper_dev, (steps_to_move > 0) ? 1500 : 500, pos);
        k_sleep(K_MSEC(1)); 
    }
    
    current_position = next_pos;
    printk("Position of tube is: %d (tube %d)\n", current_position, current_tube);
    
    k_work_schedule(&tube_work, K_MSEC(DROP_TIME));
}

void stepper_init(void)
{
    if (!gpio_is_ready_dt(&stepper_dev.dir) || !gpio_is_ready_dt(&stepper_dev.step)) {
        printk("Stepper GPIO not ready\n");
        return;
    }
    
    if (gpio_pin_configure_dt(&(stepper_dev.dir), GPIO_OUTPUT_INACTIVE) ||
        gpio_pin_configure_dt(&(stepper_dev.step), GPIO_OUTPUT_INACTIVE)) {
        printk("not working\n");
        return;
    }
    
    printk("Stepper ready\n");
    current_position = 0;
    
    k_work_schedule(&tube_work, K_MSEC(DROP_TIME));
}
