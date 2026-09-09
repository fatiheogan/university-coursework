#ifndef MODULE_SLAM_H
#define MODULE_SLAM_H

/**
 * @file    slam.h
 * @brief   SLAM – Simultaneous Localization and Mapping (Public Interface)
 * @author  Fatih Emre OGAN  |  Mervan Deniz YILDIRIM
 * @date    2025-03-24
 * @version 0.1
 *
 * Changelog:
 *   v0.1 (2025-03-24) - Initial draft: slam_init, slam_update, slam_get_pose,
 *                        slam_get_map, slam_reset, slam_status_t, slam_pose_t,
 *                        slam_map_t data types
 */

#include <stdint.h>
#include <stdbool.h>

/* ---------------------------------------------------------------
 * Constants & Macros
 * --------------------------------------------------------------- */

/** Maximum number of LiDAR scan points per frame */
#define SLAM_MAX_SCAN_POINTS     360

/** Occupancy grid map dimensions (cells) */
#define SLAM_MAP_WIDTH           400
#define SLAM_MAP_HEIGHT          400

/** Map resolution: metres per cell */
#define SLAM_MAP_RESOLUTION_M    0.05f   /* 5 cm per cell */

/** ICP match-quality threshold (0.0–1.0) above which pose is snapped */
#define SLAM_ICP_MATCH_THRESHOLD 0.75f

/** Maximum allowed pose correction per update cycle (metres) */
#define SLAM_MAX_CORRECTION_M    0.20f

/** SLAM update rate target (Hz) */
#define SLAM_UPDATE_RATE_HZ      10

/* ---------------------------------------------------------------
 * Status / Error Codes
 * --------------------------------------------------------------- */

/**
 * @brief Return codes for all SLAM API calls.
 */
typedef enum {
    SLAM_OK                =  0, /**< Operation succeeded                  */
    SLAM_ERR_NOT_INIT      = -1, /**< Module not yet initialised           */
    SLAM_ERR_BAD_PARAM     = -2, /**< NULL pointer or out-of-range value   */
    SLAM_ERR_NO_SCAN       = -3, /**< No LiDAR scan data available         */
    SLAM_ERR_ICP_FAIL      = -4, /**< ICP scan matching below threshold    */
    SLAM_ERR_MAP_FULL      = -5, /**< Occupancy grid capacity exceeded     */
    SLAM_ERR_TIMEOUT       = -6  /**< Sensor read timed out                */
} slam_status_t;

/* ---------------------------------------------------------------
 * Data Types
 * --------------------------------------------------------------- */

/**
 * @brief Robot pose in 2-D space.
 */
typedef struct {
    float    x;          /**< X position in metres (world frame)  */
    float    y;          /**< Y position in metres (world frame)  */
    float    theta;      /**< Heading in radians  (-π … +π)       */
    uint32_t timestamp;  /**< Milliseconds since boot             */
} slam_pose_t;

/**
 * @brief Single LiDAR range measurement.
 */
typedef struct {
    float    angle_rad;  /**< Bearing of measurement (radians)    */
    float    range_m;    /**< Range in metres; 0 = invalid        */
} slam_scan_point_t;

/**
 * @brief One full LiDAR scan frame.
 */
typedef struct {
    slam_scan_point_t points[SLAM_MAX_SCAN_POINTS]; /**< Scan points array  */
    uint16_t          count;                        /**< Valid point count  */
    uint32_t          timestamp;                    /**< ms since boot      */
} slam_scan_t;

/**
 * @brief 2-D occupancy grid map.
 *
 * Cell values: 0 = free, 100 = occupied, 50 = unknown.
 */
typedef struct {
    uint8_t  cells[SLAM_MAP_HEIGHT][SLAM_MAP_WIDTH]; /**< Occupancy values  */
    float    origin_x;   /**< World X of cell [0][0] in metres    */
    float    origin_y;   /**< World Y of cell [0][0] in metres    */
    float    resolution; /**< Metres per cell (= SLAM_MAP_RESOLUTION_M) */
    uint32_t timestamp;  /**< Last update timestamp (ms)          */
} slam_map_t;

/**
 * @brief Snapshot of SLAM internal diagnostics (read-only).
 */
typedef struct {
    float    icp_score;      /**< Last ICP match quality (0.0–1.0)  */
    float    drift_estimate; /**< Accumulated odometry drift (m)    */
    bool     map_ready;      /**< True once initial map is built    */
    uint32_t update_count;   /**< Number of successful updates      */
} slam_diagnostics_t;

/* ---------------------------------------------------------------
 * Callback Types
 * --------------------------------------------------------------- */

/**
 * @brief Callback invoked whenever the pose estimate is updated.
 * @param pose  Pointer to the freshly computed pose (read-only).
 */
typedef void (*slam_pose_cb_t)(const slam_pose_t *pose);

/**
 * @brief Callback invoked whenever the occupancy map is updated.
 * @param map  Pointer to the updated map (read-only).
 */
typedef void (*slam_map_cb_t)(const slam_map_t *map);

/* ---------------------------------------------------------------
 * Public Functions
 * --------------------------------------------------------------- */

/**
 * @brief  Initialise the SLAM module.
 *
 * Must be called once before any other slam_* function.
 * Allocates internal state, zeros the occupancy grid, and resets the pose.
 *
 * @param  initial_pose  Starting pose; pass NULL to use (0, 0, 0).
 * @return SLAM_OK on success, SLAM_ERR_BAD_PARAM if configuration invalid.
 */
slam_status_t slam_init(const slam_pose_t *initial_pose);

/**
 * @brief  Feed a new LiDAR scan and odometry delta into the SLAM pipeline.
 *
 * Runs one ICP iteration, updates the occupancy grid, and corrects the
 * pose estimate. Should be called at SLAM_UPDATE_RATE_HZ.
 *
 * @param  scan      New scan frame from the sensor module.
 * @param  dx        Odometry delta X  (metres, from M3/M6).
 * @param  dy        Odometry delta Y  (metres, from M3/M6).
 * @param  dtheta    Odometry delta heading (radians, from M3/M6).
 * @return SLAM_OK, SLAM_ERR_ICP_FAIL (pose not corrected), or error code.
 */
slam_status_t slam_update(const slam_scan_t *scan,
                           float dx, float dy, float dtheta);

/**
 * @brief  Retrieve the latest estimated robot pose.
 *
 * @param  out  Caller-owned slam_pose_t to be filled.
 * @return SLAM_OK or SLAM_ERR_NOT_INIT.
 */
slam_status_t slam_get_pose(slam_pose_t *out);

/**
 * @brief  Retrieve a pointer to the current occupancy grid map.
 *
 * The returned pointer is valid until the next slam_update() call.
 * Consumers (M5-NAV, M2-DigitalTwin) must copy if they need persistence.
 *
 * @param  out  Set to point to the internal slam_map_t.
 * @return SLAM_OK or SLAM_ERR_NOT_INIT.
 */
slam_status_t slam_get_map(const slam_map_t **out);

/**
 * @brief  Register a callback for pose update events.
 *
 * @param  cb  Function pointer; pass NULL to unregister.
 * @return SLAM_OK or SLAM_ERR_BAD_PARAM.
 */
slam_status_t slam_register_pose_callback(slam_pose_cb_t cb);

/**
 * @brief  Register a callback for map update events.
 *
 * @param  cb  Function pointer; pass NULL to unregister.
 * @return SLAM_OK or SLAM_ERR_BAD_PARAM.
 */
slam_status_t slam_register_map_callback(slam_map_cb_t cb);

/**
 * @brief  Read SLAM internal diagnostics (non-blocking, read-only).
 *
 * @param  out  Caller-owned slam_diagnostics_t to be filled.
 * @return SLAM_OK or SLAM_ERR_NOT_INIT.
 */
slam_status_t slam_get_diagnostics(slam_diagnostics_t *out);

/**
 * @brief  Reset the SLAM module to its initial state.
 *
 * Clears the occupancy grid, resets the pose, and zeroes counters.
 * Does NOT de-initialise hardware.
 *
 * @return SLAM_OK.
 */
slam_status_t slam_reset(void);

#endif /* MODULE_SLAM_H */
