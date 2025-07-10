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
#define DROP_TIME 100

/* Tarzan Channel values */
static uint16_t channel_range[] = {0, 950, 2047};

const struct stepper_motor stepper_dev = {
  .dir = GPIO_DT_SPEC_GET(DT_ALIAS(stepper0), dir_gpios),
  .step = GPIO_DT_SPEC_GET(DT_ALIAS(stepper0), step_gpios)
};

static uint8_t current_tube = 0;
static int32_t current_position = 0;
static int pos = 0;
static int target_position = 0;
static uint16_t step_cmd = 950; 

static void stepper_step_handler(struct k_work *work);
K_WORK_DEFINE(step_work, stepper_step_handler);

static void tube_change_handler(struct k_timer *dummy);
K_TIMER_DEFINE(tube_timer, tube_change_handler, NULL);

static void stepper_pulse_handler(struct k_timer *dummy);
K_TIMER_DEFINE(stepper_timer, stepper_pulse_handler, NULL);

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

static void stepper_step_handler(struct k_work *work) {
  pos = stepper_motor_write(&stepper_dev, step_cmd, pos);
  
  if (current_position == target_position) {
    step_cmd = 950; // Stop
    printk("Position reached: %d (tube %d)\n", current_position, current_tube);
  } else if (current_position < target_position) {
    step_cmd = 1500; // Move clockwise
    current_position++;
  } else {
    step_cmd = 500;  // Move counterclockwise
    current_position--;
  }
}

static void stepper_pulse_handler(struct k_timer *dummy) {
  k_work_submit(&step_work);
}

static void tube_change_handler(struct k_timer *dummy) {
  current_tube = (current_tube + 1) % NUM_TEST;
  printk("Moving to tube %d\n", current_tube);
  
  target_position = current_tube * STEPS_PER;
  
  k_timer_start(&stepper_timer, K_NO_WAIT, K_USEC(800)); // Adjust pulse frequency as needed
}

void stepper_init(void)
{
  if (!gpio_is_ready_dt(&stepper_dev.dir) || !gpio_is_ready_dt(&stepper_dev.step)) {
    printk("Stepper GPIO not ready\n");
    return;
  }
  
  if (gpio_pin_configure_dt(&(stepper_dev.dir), GPIO_OUTPUT_INACTIVE) ||
      gpio_pin_configure_dt(&(stepper_dev.step), GPIO_OUTPUT_INACTIVE)) {
    printk("GPIO configuration failed\n");
    return;
  }
  
  printk("Stepper ready\n");
  current_position = 0;
  target_position = 0;
  
  k_timer_start(&tube_timer, K_MSEC(DROP_TIME), K_MSEC(DROP_TIME));
}
void stepper_move_to_tube(uint8_t tube_number) {
  if (tube_number < NUM_TEST) {
    current_tube = tube_number;
    target_position = current_tube * STEPS_PER;
    k_timer_start(&stepper_timer, K_NO_WAIT, K_USEC(50));
  }
}
