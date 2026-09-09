#ifndef MODULE_SLAM_H
#define MODULE_SLAM_H

/**
 * @file    slam.h
 * @brief   MOD-04 – SLAM (Mapping & Localization) – public interface
 *
 * Provides LiDAR-based simultaneous localization and mapping using the
 * KISS-ICP odometry pipeline, fusing RPLidar A2 scans with wheel
 * odometry and IMU priors from MOD-03.  Maintains a 2-D occupancy grid
 * (5 cm/cell) and continuously publishes slam_state_t to MOD-05 (20 Hz).
 * Supports drift correction at predefined corridor-junction checkpoints
 * via scan-to-reference matching (confidence >= 0.80).
 *
 * @author  Fatih Emre OGAN      [230104004090]
 * @author  Mervan Deniz YILDIRIM [230104004173]
 * @date    2026-03-29
 * @version 0.1
 *
 * Changelog:
 *  v0.1 (2026-03-29) – Initial draft; slam_init, slam_update,
 *                       slam_get_pose, slam_get_map, slam_reset_pose,
 *                       slam_process_checkpoint, slam_shutdown defined.
 */

/* ── Includes / Dependencies ────────────────────────────────────── */
#include <stdint.h>
#include "../../project_types.h"   /* shared project-wide types */

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants & Macros ─────────────────────────────────────────── */

/** @brief Occupancy grid cell size in metres (5 cm/cell). */
#define SLAM_GRID_RESOLUTION_M   0.05f

/** @brief Minimum ICP confidence to accept a checkpoint pose snap. */
#define SLAM_CHECKPOINT_THRESH   0.80f

/** @brief RPLidar A2 usable range cap in metres. */
#define SLAM_LIDAR_MAX_RANGE_M   8.0f

/** @brief Pose publish rate to MOD-05 in Hz. */
#define SLAM_UPDATE_HZ           20

/** @brief Header/module version string. */
#define SLAM_VERSION             "0.1"

/* ── Error / Return Code Enum ───────────────────────────────────── */

/**
 * @brief Return codes for SLAM public functions.
 *
 * Negative values indicate errors; zero is success; positive values
 * are non-fatal warnings.
 */
typedef enum {
    SLAM_OK             =  0,   /**< Operation completed successfully     */
    SLAM_ERR_INIT       = -1,   /**< Initialisation failure               */
    SLAM_ERR_LIDAR      = -2,   /**< LiDAR communication / scan error     */
    SLAM_WARN_LOW_CONF  =  1    /**< ICP confidence below threshold       */
} slam_status_t;

/* ── Public Function Declarations ───────────────────────────────── */

/**
 * @brief  Initialise the SLAM engine and load a pre-built map.
 *
 * Opens the RPLidar A2 device, allocates the occupancy grid, and
 * optionally loads an existing .pgm/.yaml map for re-localisation.
 *
 * @param[in] map_path  Path to the occupancy map file (.pgm/.yaml).
 *                      Pass NULL to start with an empty grid.
 * @return SLAM_OK on success, SLAM_ERR_INIT or SLAM_ERR_LIDAR on failure.
 */
slam_status_t slam_init(const char *map_path);

/**
 * @brief  Feed one control-loop tick of odometry and IMU data into the
 *         SLAM pipeline.
 *
 * Fuses the latest encoder odometry and IMU reading with the current
 * LiDAR scan via KISS-ICP.  Updates the internal pose estimate and,
 * when new scan data triggers a grid change, marks map_updated in the
 * internal slam_state_t.
 *
 * Should be called at the rate data arrives from MOD-03 (50–100 Hz);
 * the pose is published to consumers at SLAM_UPDATE_HZ.
 *
 * @param[in] odom  Latest odometry_data_t from MOD-03.
 * @param[in] imu   Latest imu_data_t from MOD-03.
 * @return SLAM_OK on success, SLAM_WARN_LOW_CONF if ICP confidence
 *         drops below SLAM_CHECKPOINT_THRESH, SLAM_ERR_LIDAR on
 *         scan failure.
 */
slam_status_t slam_update(const odometry_data_t *odom,
                           const imu_data_t      *imu);

/**
 * @brief  Copy the current pose estimate.
 *
 * Thread-safe: acquires internal mutex before copying.
 *
 * @param[out] out  Caller-owned slam_state_t to fill.
 * @return SLAM_OK on success, SLAM_ERR_INIT if engine not initialised.
 */
slam_status_t slam_get_pose(slam_state_t *out);

/**
 * @brief  Return a read-only pointer to the current occupancy grid.
 *
 * The pointer remains valid until the next slam_update() call that
 * modifies the grid.  Callers must not free or write through it.
 *
 * @param[out] grid_out  Output pointer to internal grid array.
 * @param[out] width     Grid width in cells.
 * @param[out] height    Grid height in cells.
 * @return SLAM_OK on success, SLAM_ERR_INIT if engine not initialised.
 */
slam_status_t slam_get_map(uint8_t  **grid_out,
                            uint16_t  *width,
                            uint16_t  *height);

/**
 * @brief  Override the internal pose estimate.
 *
 * Called by MOD-05 after a successful checkpoint_ack_t (CP_ACK_OK)
 * to correct accumulated drift.
 *
 * @param[in] x_m       New X position in map frame (metres).
 * @param[in] y_m       New Y position in map frame (metres).
 * @param[in] theta_rad New heading in map frame (radians, CCW).
 * @return SLAM_OK on success.
 */
slam_status_t slam_reset_pose(float x_m, float y_m, float theta_rad);

/**
 * @brief  Process a checkpoint pose-snap request from MOD-05.
 *
 * Performs scan-to-reference matching at the given checkpoint.  If
 * the match confidence meets SLAM_CHECKPOINT_THRESH, the snapped
 * pose is written into @p ack; otherwise result is set to CP_ACK_FAIL.
 *
 * @param[in]  req  Checkpoint request from MOD-05.
 * @param[out] ack  Caller-owned checkpoint_ack_t to fill.
 * @return SLAM_OK if snap succeeded, SLAM_WARN_LOW_CONF if confidence
 *         was insufficient (ack->result == CP_ACK_FAIL).
 */
slam_status_t slam_process_checkpoint(const checkpoint_req_t *req,
                                       checkpoint_ack_t       *ack);

/**
 * @brief  Shut down the SLAM pipeline and release all resources.
 *
 * Stops the RPLidar motor, frees the occupancy grid, and joins any
 * internal threads.  Safe to call even if slam_init() was never called.
 */
void slam_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* MODULE_SLAM_H */
