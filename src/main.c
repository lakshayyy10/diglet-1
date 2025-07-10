#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/kernel/thread_stack.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#define STACK_SIZE 4096
#define PRIORITY 2
#define STEPPER_TIMER 100

struct stepper_motor {
  struct gpio_dt_spec dir;
  struct gpio_dt_spec step;
};

enum StepperDirection {
  STOP_PULSE = 0,
  HIGH_PULSE = 1,
  LOW_PULSE = 2
};

const struct stepper_motor stepper = {
  .dir = GPIO_DT_SPEC_GET(DT_ALIAS(stepper0), dir_gpios),
  .step = GPIO_DT_SPEC_GET(DT_ALIAS(stepper0), step_gpios)
}; // Missing semicolon here

K_THREAD_STACK_DEFINE(stack_area, STACK_SIZE);
struct k_work_q work_q;
int stepper_pos = 0;
enum StepperDirection stepper_dir = STOP_PULSE;
struct k_work stepper_work_item;

int Stepper_motor_write(const struct stepper_motor *motor, enum StepperDirection dir, int pos) {
  if (dir == STOP_PULSE) {
    return pos;
  }
  if (dir == HIGH_PULSE) {
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

void stepper_work_handler(struct k_work *work_ptr) {
  stepper_pos = Stepper_motor_write(&stepper, stepper_dir, stepper_pos);
}

void stepper_timer_handler(struct k_timer *stepper_timer_ptr) {
  k_work_submit_to_queue(&work_q, &stepper_work_item);
}

K_TIMER_DEFINE(stepper_timer, stepper_timer_handler, NULL);

int main() {
  
  printk("Stepper Motor Control\n");
  k_work_queue_init(&work_q);
  k_work_init(&stepper_work_item, stepper_work_handler);
  if (!device_is_ready(stepper.dir.port)) {
    printk("Direction GPIO port not ready\n");
    return 0;
  }
  if (!device_is_ready(stepper.step.port)) {
    printk("Step GPIO port not ready\n");
    return 0;
  }
  if (gpio_pin_configure_dt(&(stepper.dir), GPIO_OUTPUT_INACTIVE)) {
    printk("Direction pin not configured\n");
    return 0;
  }
  if (gpio_pin_configure_dt(&(stepper.step), GPIO_OUTPUT_INACTIVE)) {
    printk("Step pin not configured\n");
    return 0;
  }
  k_work_queue_start(&work_q, stack_area, K_THREAD_STACK_SIZEOF(stack_area),
                     PRIORITY, NULL);
  k_timer_start(&stepper_timer, K_SECONDS(1), K_USEC(STEPPER_TIMER / 2));
  
  while (1) {
    printk("clockwise\n");
    stepper_dir = HIGH_PULSE;
    k_sleep(K_SECONDS(5));
    printk("Stopped\n");
    stepper_dir = STOP_PULSE;
    k_sleep(K_SECONDS(2));
    printk("anticlockwise\n");
    stepper_dir = LOW_PULSE;
    k_sleep(K_SECONDS(5));
    printk("Stopped\n");
    stepper_dir = STOP_PULSE;
    k_sleep(K_SECONDS(2));
  }
  
  return 0;
}
