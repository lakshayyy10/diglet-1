#ifndef BASIC_STEPPER_H
#define BASIC_STEPPER_H

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

/* Stepper motor structure definition */
struct stepper_motor {
    struct gpio_dt_spec dir;
    struct gpio_dt_spec step;
};

/**
 * @brief Initialize stepper motor
 */
void stepper_init(void);

/**
 * @brief Move stepper motor based on command value
 * 
 * @param motor Pointer to stepper motor structure
 * @param cmd Command value (500 for CCW, 950 for stop, 1500 for CW)
 * @param pos Current position
 * @return Updated position
 */
int stepper_motor_write(const struct stepper_motor *motor, uint16_t cmd, int pos);

/**
 * @brief Set stepper motor direction and speed
 * 
 * @param direction 0: stop, 1: clockwise, -1: counterclockwise
 */
void stepper_set_direction(int direction);

#endif /* BASIC_STEPPER_H */
