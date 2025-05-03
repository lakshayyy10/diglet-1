/*
 * Stepper motor control header file
 */

#ifndef STEPPER_CONTROL_H
#define STEPPER_CONTROL_H

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

/** 
 * @brief Structure for stepper motor pin configuration
 */
struct stepper_motor {
    struct gpio_dt_spec dir;  /* Direction pin */
    struct gpio_dt_spec step; /* Step pin */
};

/**
 * @brief Initialize the stepper motor
 */
void stepper_init(void);

/**
 * @brief Write to stepper motor
 *
 * @param motor Pointer to stepper motor structure
 * @param cmd Command value (1500 for clockwise, 500 for anti-clockwise)
 * @param pos Current position
 * @return New position
 */
int stepper_motor_write(const struct stepper_motor *motor, uint16_t cmd, int pos);

#endif /* STEPPER_CONTROL_H */
