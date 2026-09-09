#ifndef MODULE_HW_H
#define MODULE_HW_H

/**
 * @file    hw.h
 * @brief   Hardware & Motor Control - public interface (MOD-03)
 * @author  Hicran KOÇ
 * @date    2026-03-29
 * @version 0.1
 *
 * Changelog:
 *   v0.1 (2026-03-29) - Initial draft: hw_init, hw_set_motion, hw_emergency_stop,
 *                        hw_get_odometry, hw_get_imu, hw_get_battery_voltage
 *
 * MOD-03 owns all low-level hardware interfacing on the Raspberry Pi 4:
 *   - PWM signals for the L298N H-bridge motor driver (50 Hz carrier)
 *   - Quadrature encoder pulse counting via GPIO edge-detect interrupts
 *   - MPU-6050 IMU data acquisition over I2C at 400 kHz (address 0x68)
 *   - Power distribution monitoring (LiPo 11.1 V / 2200 mAh via UBEC)
 *
 * Published data:
 *   odometry_data_t  → MOD-04  @ 50 Hz   (left/right ticks + wheel speeds)
 *   imu_data_t       → MOD-04  @ 100 Hz  (gyro yaw-rate, accel xyz, temperature)
 *   motion_status_t  → MOD-05  @ 50 Hz   (actual wheel speeds, stall flag)
 *
 * Consumed data:
 *   motion_cmd_t     ← MOD-05  @ 50 Hz   (PWM duty-cycle targets per wheel)
 *
 * Consumers of this module need only include this header and project_types.h.
 * No implementation details are exposed here.
 */

#include <stdint.h>
#include "../../project_types.h"   /* odometry_data_t, imu_data_t, motion_cmd_t,
                                      motion_status_t -- shared project-wide types */

/* ---------------------------------------------------------------------------
 * Constants & Macros
 * ------------------------------------------------------------------------- */

/** @brief Encoder counts per full wheel revolution (both wheels). */
#define HW_ENCODER_CPR          20

/** @brief Wheel diameter in metres (65 mm nominal). */
#define HW_WHEEL_DIAMETER_M     0.065f

/** @brief Distance between the two drive-wheel centres, in metres (170 mm). */
#define HW_WHEELBASE_M          0.17f

/** @brief PWM carrier frequency driving the L298N H-bridge (Hz). */
#define HW_PWM_FREQ_HZ          50

/**
 * @brief Maximum allowable wheel speed — safety cap (m/s).
 *
 * hw_set_motion() will clamp any requested speed to ±HW_MAX_SPEED_MPS
 * before writing to the PWM peripheral.
 */
#define HW_MAX_SPEED_MPS        0.5f

/**
 * @brief I2C bus address of the MPU-6050 IMU (AD0 pin tied low).
 *
 * If AD0 is pulled high on a board revision, change this to 0x69.
 */
#define HW_IMU_I2C_ADDR         0x68

/** @brief I2C clock frequency used for MPU-6050 communication (Hz). */
#define HW_IMU_I2C_FREQ_HZ      400000

/**
 * @brief Complementary filter blending coefficient for heading fusion.
 *
 * heading = alpha * gyro_integrated + (1.0 - alpha) * accel_derived
 */
#define HW_IMU_COMP_ALPHA       0.98f

/**
 * @brief BCM GPIO pin numbers for encoder channels (A and B, left wheel).
 *
 * Change if your wiring differs.  Right-wheel pins are 22 (A) and 23 (B).
 */
#define HW_ENC_LEFT_A_PIN       17
#define HW_ENC_LEFT_B_PIN       18
#define HW_ENC_RIGHT_A_PIN      22
#define HW_ENC_RIGHT_B_PIN      23

/** @brief Header / interface version string. Bump on any public API change. */
#define HW_VERSION              "0.1"

/* ---------------------------------------------------------------------------
 * Error / Return Code Enumeration
 * ------------------------------------------------------------------------- */

/**
 * @brief Return codes for all hw_* functions.
 *
 * Functions return HW_OK (0) on success.  Negative values are errors.
 * Callers must check every return value and propagate or handle errors.
 */
typedef enum {
    HW_OK            =  0,  /**< Operation completed successfully. */
    HW_ERR_GPIO      = -1,  /**< GPIO initialisation or write failure. */
    HW_ERR_I2C       = -2,  /**< I2C bus error (MPU-6050 unreachable or NACK). */
    HW_ERR_TIMEOUT   = -3   /**< Operation did not complete within deadline. */
} hw_status_t;

/* ---------------------------------------------------------------------------
 * Public Function Declarations
 * ------------------------------------------------------------------------- */

/**
 * @brief  Initialise all MOD-03 hardware subsystems.
 *
 * Must be called once at startup before any other hw_* function.
 * Sets up:
 *   - RPi.GPIO (or pigpio) in BCM mode
 *   - PWM channels for IN1–IN4, ENA, ENB on the L298N
 *   - Rising-edge interrupt handlers for all four encoder channels
 *   - I2C bus at HW_IMU_I2C_FREQ_HZ and MPU-6050 configuration registers
 *
 * @return HW_OK on success; HW_ERR_GPIO if GPIO setup fails;
 *         HW_ERR_I2C if the IMU does not acknowledge.
 */
hw_status_t hw_init(void);

/**
 * @brief  Apply a motion command to the left and right motors.
 *
 * Translates the normalised PWM duty cycles in @p cmd to L298N PWM signals.
 * Values are clamped to [-HW_MAX_SPEED_MPS, +HW_MAX_SPEED_MPS] before
 * being written.  If @p cmd->stop is non-zero, both motors are cut
 * immediately (equivalent to calling hw_emergency_stop()).
 *
 * Intended to be called by MOD-05 at 50 Hz.
 *
 * @param  cmd  Pointer to a caller-owned, fully initialised motion_cmd_t.
 *              Must not be NULL.
 * @return HW_OK on success; HW_ERR_GPIO if the PWM write fails.
 */
hw_status_t hw_set_motion(const motion_cmd_t *cmd);

/**
 * @brief  Immediately cut power to both motors (safety override).
 *
 * Sets all L298N IN pins LOW and duty cycle to 0.  This function is
 * non-blocking and should complete within one PWM period (20 ms).
 * May be called from any thread or signal handler.
 *
 * @return HW_OK unconditionally (best-effort stop; never blocks).
 */
hw_status_t hw_emergency_stop(void);

/**
 * @brief  Read the latest odometry sample accumulated by the encoder ISR.
 *
 * The encoder interrupt service routine maintains running tick counters.
 * This function atomically snapshots the counters since the previous call,
 * resets the delta counters, and computes wheel speeds in m/s using
 * HW_ENCODER_CPR and HW_WHEEL_DIAMETER_M.
 *
 * Call at 50 Hz to match the MOD-04 consumption rate.
 *
 * @param[out] out  Caller-owned odometry_data_t to fill.  Must not be NULL.
 * @return HW_OK on success; HW_ERR_TIMEOUT if the mutex cannot be acquired
 *         within 2 ms.
 */
hw_status_t hw_get_odometry(odometry_data_t *out);

/**
 * @brief  Read the latest IMU sample from the MPU-6050 via I2C.
 *
 * Performs a synchronous I2C burst read of the gyroscope and accelerometer
 * registers (registers 0x3B–0x48), converts raw counts to physical units,
 * and stores the result in @p out.  Temperature register (0x41) is also
 * read.
 *
 * Call at 100 Hz to match the MOD-04 consumption rate.
 *
 * @param[out] out  Caller-owned imu_data_t to fill.  Must not be NULL.
 * @return HW_OK on success; HW_ERR_I2C on bus error;
 *         HW_ERR_TIMEOUT if the I2C transaction exceeds 5 ms.
 */
hw_status_t hw_get_imu(imu_data_t *out);

/**
 * @brief  Read the current battery voltage from the ADC / voltage divider.
 *
 * The LiPo pack voltage is divided down to the RPi ADC input range
 * (0–3.3 V) using a resistor network.  This function reads the ADC,
 * applies the calibration factor, and writes the result to @p v_out.
 *
 * A reading below 10.5 V indicates a nearly-depleted cell; the caller
 * should trigger a safe-shutdown sequence (see RISK-06).
 *
 * @param[out] v_out  Output pointer; receives battery voltage in volts.
 *                    Must not be NULL.
 * @return HW_OK on success; HW_ERR_GPIO if the ADC read fails.
 */
hw_status_t hw_get_battery_voltage(float *v_out);

#endif /* MODULE_HW_H */
