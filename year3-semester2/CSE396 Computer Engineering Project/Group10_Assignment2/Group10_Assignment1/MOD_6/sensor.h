#ifndef MODULE_SENSOR_H
#define MODULE_SENSOR_H

/**
 * @file    sensor.h
 * @brief   MOD-06 – Sensor Integration – public interface
 *
 * This module abstracts raw environmental data from the LiDAR forward arc
 * and supplementary proximity sensors (Ultrasonic/IR). It applies noise 
 * filtering and continuously evaluates the forward path for unexpected 
 * dynamic obstacles. It provides clean obstacle_alert_t structs to MOD-05 
 * (Navigation) to trigger reactive halting and rerouting behaviors.
 *
 * @author  Sayed Khalilullah Hashimi [200104004806]
 * @date    2026-03-29
 * @version 0.2
 *
 * Changelog:
 * v0.2 (2026-03-29) – Aligned with group's unified project_types.h
 * v0.1 (2026-03-28) – Initial draft of proximity and detour logic
 */

/* ── Includes / Dependencies ────────────────────────────────────── */
#include <stdint.h>
#include <stdbool.h>
#include "../../project_types.h"   /* shared project-wide types */

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants & Macros ─────────────────────────────────────────── */

/** @brief Max reliable LiDAR/Sensor range for reactive avoidance (m) */
#define SENSOR_MAX_RANGE_M       4.0f

/** @brief Distance threshold to trigger an active obstacle alert (m) */
#define SENSOR_OBSTACLE_THRESH_M 0.50f

/** @brief The forward viewing angle to monitor for path blockage (+/- deg) */
#define SENSOR_FORWARD_ARC_DEG   25.0f

/** @brief Sensor read and update rate in Hz. */
#define SENSOR_UPDATE_HZ         20

/** @brief Header/module version string. */
#define SENSOR_VERSION           "0.2"

/* ── Error / Return Code Enum ───────────────────────────────────── */

/**
 * @brief Return codes for Sensor Integration public functions.
 */
typedef enum {
    SENSOR_OK             =  0,   /**< Operation completed successfully     */
    SENSOR_ERR_INIT       = -1,   /**< Hardware initialisation failure      */
    SENSOR_ERR_TIMEOUT    = -2,   /**< Sensor failed to respond in time     */
    SENSOR_WARN_NOISE     =  1    /**< High variance in sensor readings     */
} sensor_status_t;

/* ── Public Function Declarations ───────────────────────────────── */

/**
 * @brief  Initialise the sensor hardware and filtering pipelines.
 *
 * Configures GPIO pins for ultrasonic/IR sensors and establishes the
 * data pipeline to receive the forward arc subset of the LiDAR scan.
 *
 * @return SENSOR_OK on success, SENSOR_ERR_INIT on failure.
 */
sensor_status_t sensor_init(void);

/**
 * @brief  Update the internal sensor state and run noise filters.
 *
 * Should be called in the main control loop at SENSOR_UPDATE_HZ.
 * Processes raw hardware inputs and prepares the consolidated alerts.
 *
 * @return SENSOR_OK on success, SENSOR_ERR_TIMEOUT if a sensor dropped.
 */
sensor_status_t sensor_update(void);

/**
 * @brief  Retrieve the latest consolidated obstacle alert for Navigation.
 *
 * @param[out] alert  Caller-owned obstacle_alert_t struct to fill.
 * @return SENSOR_OK on success, SENSOR_ERR_INIT if not initialised.
 */
sensor_status_t sensor_get_alert(obstacle_alert_t *alert);

/**
 * @brief  Read raw data from a specific supplementary sensor.
 *
 * Useful for MOD-02 (Digital Twin) or MOD-04 (SLAM) if they need
 * raw distance feeds rather than consolidated alerts.
 *
 * @param[in]  id       The ID of the sensor to read (0=front-left, etc.)
 * @param[out] reading  Caller-owned sensor_reading_t struct to fill.
 * @return SENSOR_OK on success, SENSOR_ERR_TIMEOUT on failure.
 */
sensor_status_t sensor_get_raw_reading(uint8_t id, sensor_reading_t *reading);

/**
 * @brief  Determine the safest detour bearing based on lateral free space.
 *
 * Evaluates the left and right peripheral scans to recommend a detour direction.
 *
 * @param[out] best_bearing_deg  Recommended heading offset (e.g., -45.0 or 45.0).
 * @return SENSOR_OK if a clear path is found, SENSOR_WARN_NOISE if blocked on both sides.
 */
sensor_status_t sensor_get_best_detour(float *best_bearing_deg);

#ifdef __cplusplus
}
#endif

#endif /* MODULE_SENSOR_H */
