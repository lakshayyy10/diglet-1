

#ifndef STEPPER_CONTROL_H
#define STEPPER_CONTROL_H

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

struct stepper_motor {
    struct gpio_dt_spec dir;  /* Direction pin */
    struct gpio_dt_spec step; /* Step pin */
};

void stepper_init(void);

int stepper_motor_write(const struct stepper_motor *motor, uint16_t cmd, int pos);
void stepper_move_to_tube(uint8_t tube_number);

#endif /* STEPPER_CONTROL_H */
