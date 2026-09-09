#ifndef MODULE_NAV_PATHPLAN_H
#define MODULE_NAV_PATHPLAN_H

/**
 * @file   nav_pathplan.h
 * @brief  NAV – Path Planning & Control – public interface (MOD-05)
 * @author  Ertuğrul Ergül  [240104004994]
 * @author  Ömer Faruk Gürsel [210104004053]
 * @date   2026-03-29
 * @version 0.1
 *
 * Changelog:
 *   v0.1 (2026-03-29) – Initial draft: core data types, path-planning
 *                        API, controller start/stop, recovery, callbacks.
 *
 * MOD-05 runs on the Raspberry Pi 4 (2 GB, Bookworm 64-bit) alongside
 * all other software modules.  It is responsible for high-level autonomous
 * movement of the LIDAR-Guided Hospital Navigation Robot:
 *   - Consumes the 2-D occupancy grid (5 cm/cell) and robot pose from
 *     MOD-04 (SLAM / KISS-ICP + RPLidar A2) to plan collision-free paths
 *     to target waypoints on the hospital floor (~300–400 m²).
 *   - Implements a proportional heading controller (Kp = 1.2) that outputs
 *     differential PWM commands to MOD-03 (L298N H-bridge + DC motors).
 *   - Reacts to obstacle alerts from MOD-06 (HC-SR04 ultrasonic + IR
 *     sensors) and triggers wait / reroute recovery behaviours.
 *   - Accepts voice-commanded navigation goals from MOD-01 (Whisper STT)
 *     and publishes navigation status to MOD-01 (Piper TTS) and MOD-02
 *     (Digital Twin via WebSocket on port 8765).
 *
 * Target hardware summary:
 *   Platform   : Raspberry Pi 4 Model B (2 GB)
 *   Drive      : 2× DC geared motors via L298N H-bridge (50 Hz PWM)
 *   Encoders   : 20 CPR quadrature on each wheel (GPIO 17/18, 22/23)
 *   LiDAR      : RPLidar A2 (360°, up to 8 m range) — owned by MOD-04
 *   IMU        : MPU-6050 on I²C @ 400 kHz (addr 0x68) — owned by MOD-03
 *   Proximity  : 2× HC-SR04 ultrasonic + 2× digital IR — owned by MOD-06
 *   Power      : LiPo 11.1 V / 2200 mAh via UBEC
 *
 * Published data:
 *   motion_cmd_t     → MOD-03  @ 50 Hz   (left/right PWM + emergency stop)
 *   nav_status_t     → MOD-01  on state change  (FSM state + TTS message)
 *   nav_status_t     → MOD-02  on state change  (Digital Twin overlay)
 *   checkpoint_req_t → MOD-04  on junction arrival (pose-snap request)
 *   nav_cmd_vel_t    → internal @ 50 Hz  (linear/angular before diff. conversion)
 *
 * Consumed data:
 *   slam_state_t     ← MOD-04  @ 20 Hz   (x, y, theta, confidence)
 *   occ_map_t        ← MOD-04  on map update  (occupancy grid pointer)
 *   motion_status_t  ← MOD-03  @ 50 Hz   (actual wheel speeds, stall flag)
 *   obstacle_alert_t ← MOD-06  @ up to 20 Hz  (closest obstacle dist + bearing)
 *   nav_goal_t       ← MOD-01  on voice command  (destination + waypoint ID)
 *   checkpoint_ack_t ← MOD-04  on checkpoint response  (snapped pose or fail)
 *
 * Consumers of this module need only include this header and project_types.h.
 * No implementation details are exposed here.
 */

#include <stdint.h>
#include <stdbool.h>

/* ── Shared / project-wide headers ────────────────────────────────── */
#include "../project_types.h"        /* slam_state_t, occ_map_t, motion_cmd_t,
                                       motion_status_t, obstacle_alert_t,
                                       nav_goal_t, nav_state_e, nav_status_t,
                                       checkpoint_req_t, checkpoint_ack_t,
                                       telemetry_t -- shared project-wide types */

#ifdef __cplusplus
extern "C" {
#endif

/* ====================================================================
 *  Constants & Configuration Macros
 * ==================================================================== */

/** Maximum number of waypoints that may be queued in a single mission. */
#define NAV_MAX_WAYPOINTS           64

/** Maximum nodes the A*/Dijkstra planner will expand per planning call. */
#define NAV_PLANNER_MAX_NODES       8192

/** Default path-following look-ahead distance (metres). */
#define NAV_LOOKAHEAD_M             0.35f

/** Goal-reached tolerance: position (metres) and heading (radians). */
#define NAV_GOAL_XY_TOL_M           0.05f
#define NAV_GOAL_YAW_TOL_RAD       0.10f

/** Time (ms) without forward progress before stuck-detection triggers. */
#define NAV_STUCK_TIMEOUT_MS        3000

/** Obstacle avoidance safety margin around the robot footprint (metres). */
#define NAV_OBSTACLE_MARGIN_M       0.15f

/** Minimum clearance (metres) for the planner to consider a cell free. */
#define NAV_MIN_CLEARANCE_M         0.20f

/** Controller loop period (ms).  Matches MOD-03 motion_cmd_t consumption rate. */
#define NAV_CONTROL_PERIOD_MS       50

/** Header / interface version string.  Bump on any public API change. */
#define NAV_VERSION                 "0.1"

/* ====================================================================
 *  Data Types
 * ==================================================================== */

/**
 * @brief Status / error codes returned by every nav_* function.
 */
typedef enum {
    NAV_OK                  =  0, /**< Operation completed successfully       */
    NAV_ERR_NOT_INIT        = -1, /**< Module not yet initialised             */
    NAV_ERR_INVALID_PARAM   = -2, /**< NULL pointer or out-of-range argument  */
    NAV_ERR_NO_PATH         = -3, /**< Planner could not find a feasible path */
    NAV_ERR_TIMEOUT         = -4, /**< Operation exceeded its time budget     */
    NAV_ERR_QUEUE_FULL      = -5, /**< Waypoint queue is at NAV_MAX_WAYPOINTS */
    NAV_ERR_BUSY            = -6, /**< A navigation task is already running   */
    NAV_ERR_OBSTACLE        = -7, /**< Immediate obstacle blocks all motions  */
    NAV_ERR_MAP_STALE       = -8  /**< Map data too old to plan reliably      */
} nav_result_t;

/**
 * @brief High-level navigation state reported by nav_get_state().
 */
typedef enum {
    NAV_RUNTIME_IDLE        = 0, /**< No active goal                          */
    NAV_RUNTIME_PLANNING    = 1, /**< Computing a new path                    */
    NAV_RUNTIME_FOLLOWING   = 2, /**< Executing path-following controller     */
    NAV_RUNTIME_AVOIDING    = 3, /**< Reactive obstacle avoidance active      */
    NAV_RUNTIME_STUCK       = 4, /**< Stuck-detection triggered               */
    NAV_RUNTIME_RECOVERING  = 5, /**< Executing a recovery behaviour          */
    NAV_RUNTIME_GOAL_REACHED= 6, /**< Final waypoint reached within tolerance */
    NAV_RUNTIME_ABORTED     = 7  /**< Navigation cancelled or un-recoverable  */
} nav_runtime_state_t;

/**
 * @brief A 2-D waypoint with optional heading constraint.
 *
 * Set @c has_yaw to false if the robot may arrive at any orientation.
 */
typedef struct {
    float   x;          /**< Target X in map frame (metres)               */
    float   y;          /**< Target Y in map frame (metres)               */
    float   yaw;        /**< Desired heading at goal (radians, -π … π)    */
    bool    has_yaw;    /**< true → controller enforces final heading     */
} nav_waypoint_t;

/**
 * @brief Velocity command output by the path-following controller.
 *
 * Computed every NAV_CONTROL_PERIOD_MS (50 Hz) while a goal is active.
 * Internally converted to differential left/right PWM values in a
 * motion_cmd_t and published to MOD-03 (L298N H-bridge) at the same rate.
 * Also available to other modules via nav_get_cmd_vel() or the
 * nav_cmd_vel_cb_t callback.
 */
typedef struct {
    float   linear_v;   /**< Forward velocity  (m/s, +ve = forward)       */
    float   angular_w;  /**< Rotational speed  (rad/s, +ve = CCW)         */
    uint32_t timestamp; /**< ms since boot when this command was computed  */
} nav_cmd_vel_t;

/**
 * @brief Run-time tuning parameters passed to nav_init().
 *
 * All fields have documented defaults; pass NULL to nav_init() to use
 * them.
 */
typedef struct {
    float   lookahead_m;        /**< Pure-pursuit look-ahead (default 0.35 m)   */
    float   max_linear_v;       /**< Speed cap (m/s, default 0.5 — matches HW_MAX_SPEED_MPS) */
    float   max_angular_w;      /**< Turn-rate cap (rad/s, default 1.5)         */
    float   goal_xy_tol;        /**< Position tolerance (m, default 0.05)       */
    float   goal_yaw_tol;       /**< Heading tolerance (rad, default 0.10)      */
    float   obstacle_margin;    /**< Safety inflation (m, default 0.15)         */
    uint32_t stuck_timeout_ms;  /**< Stuck timer (ms, default 3000)             */
} nav_config_t;

/**
 * @brief Compact progress snapshot returned by nav_get_progress().
 */
typedef struct {
    nav_runtime_state_t state;      /**< Current high-level state             */
    uint8_t      waypoints_total;   /**< Total waypoints in current mission   */
    uint8_t      waypoints_done;    /**< Waypoints already reached            */
    float        dist_to_goal;      /**< Euclidean distance to active WP (m)  */
    float        path_length;       /**< Remaining planned path length (m)    */
    uint32_t     timestamp;         /**< ms since boot                        */
} nav_progress_t;

/* ====================================================================
 *  Callback Types
 * ==================================================================== */

/**
 * @brief Called every control cycle (50 Hz) with the latest velocity command.
 *
 * MOD-03 (Hardware & Motor Control) should register this to receive
 * real-time drive commands without polling.  The callback runs in the
 * NAV control-loop thread context on the Raspberry Pi 4.
 *
 * @param cmd  Pointer to the velocity command (valid only during call).
 */
typedef void (*nav_cmd_vel_cb_t)(const nav_cmd_vel_t *cmd);

/**
 * @brief Called whenever the navigation state machine transitions.
 *
 * Used to publish nav_status_t to MOD-01 (Piper TTS announcement) and
 * MOD-02 (Digital Twin WebSocket overlay) on each FSM state change.
 *
 * @param prev  State before the transition.
 * @param curr  State after the transition.
 */
typedef void (*nav_state_cb_t)(nav_runtime_state_t prev, nav_runtime_state_t curr);

/**
 * @brief Called when a waypoint is reached (including the final one).
 *
 * @param wp_index  Zero-based index of the reached waypoint.
 * @param wp        The waypoint that was reached.
 */
typedef void (*nav_goal_cb_t)(uint8_t wp_index, const nav_waypoint_t *wp);

/* ====================================================================
 *  Public Functions
 * ==================================================================== */

/* ── Lifecycle ─────────────────────────────────────────────────────── */

/**
 * @brief  Initialise the navigation module.
 *
 * Allocates internal buffers, reads default or user-supplied parameters,
 * and registers with MOD-04 (SLAM) for slam_state_t (20 Hz) and
 * occ_map_t updates, with MOD-03 for motion_status_t (50 Hz), and
 * with MOD-06 for obstacle_alert_t (up to 20 Hz).
 *
 * @param  cfg  Pointer to tuning parameters, or NULL for defaults.
 * @return NAV_OK on success; NAV_ERR_INVALID_PARAM if cfg contains
 *         out-of-range values.
 */
nav_result_t nav_init(const nav_config_t *cfg);

/**
 * @brief  Shut down the navigation module and release resources.
 *
 * Stops any active goal, publishes a zero-PWM motion_cmd_t to MOD-03
 * (L298N), and frees internal buffers.  Safe to call even if nav_init()
 * was not called.
 */
void nav_deinit(void);

/* ── Goal Management ───────────────────────────────────────────────── */

/**
 * @brief  Command the robot to navigate to a single waypoint.
 *
 * Cancels any active mission.  Plans a path from the current pose and
 * begins path-following immediately.
 *
 * @param  wp  Target waypoint in map frame.
 * @return NAV_OK, NAV_ERR_NO_PATH, NAV_ERR_NOT_INIT,
 *         NAV_ERR_MAP_STALE.
 */
nav_result_t nav_goto(const nav_waypoint_t *wp);

/**
 * @brief  Enqueue an ordered list of waypoints as a multi-point mission.
 *
 * The robot visits each waypoint in sequence.  Cancels any active mission.
 *
 * @param  wps    Array of waypoints.
 * @param  count  Number of waypoints (1 … NAV_MAX_WAYPOINTS).
 * @return NAV_OK, NAV_ERR_QUEUE_FULL, NAV_ERR_NO_PATH,
 *         NAV_ERR_INVALID_PARAM.
 */
nav_result_t nav_goto_multi(const nav_waypoint_t *wps, uint8_t count);

/**
 * @brief  Cancel the current goal / mission and stop the robot.
 *
 * Publishes a zero-PWM motion_cmd_t to MOD-03 and transitions to the
 * idle runtime state.
 *
 * @return NAV_OK, or NAV_ERR_NOT_INIT.
 */
nav_result_t nav_cancel(void);

/* ── Runtime Control ───────────────────────────────────────────────── */

/**
 * @brief  Pause path-following; the robot decelerates to a stop.
 *
 * The planned path is retained so nav_resume() can continue from the
 * closest point.
 *
 * @return NAV_OK, NAV_ERR_NOT_INIT, NAV_ERR_BUSY (already paused).
 */
nav_result_t nav_pause(void);

/**
 * @brief  Resume a previously paused navigation task.
 *
 * @return NAV_OK, NAV_ERR_NOT_INIT, NAV_ERR_NO_PATH (path invalidated
 *         while paused).
 */
nav_result_t nav_resume(void);

/**
 * @brief  Force an immediate re-plan from the current pose.
 *
 * Useful after MOD-04 (SLAM) reports a significant occupancy-grid update
 * (occ_map_t.map_updated != 0) or after a checkpoint pose correction.
 *
 * @return NAV_OK, NAV_ERR_NOT_INIT, NAV_ERR_NO_PATH.
 */
nav_result_t nav_replan(void);

/* ── Status & Telemetry ────────────────────────────────────────────── */

/**
 * @brief  Return the current navigation state.
 *
 * @return One of the nav_runtime_state_t values.
 */
nav_runtime_state_t nav_get_state(void);

/**
 * @brief  Fill a progress snapshot for the active mission.
 *
 * @param  out  Caller-owned nav_progress_t to populate.
 * @return NAV_OK, NAV_ERR_NOT_INIT, NAV_ERR_INVALID_PARAM.
 */
nav_result_t nav_get_progress(nav_progress_t *out);

/**
 * @brief  Get the latest velocity command the controller has computed.
 *
 * @param  out  Caller-owned nav_cmd_vel_t to populate.
 * @return NAV_OK, NAV_ERR_NOT_INIT, NAV_ERR_INVALID_PARAM.
 */
nav_result_t nav_get_cmd_vel(nav_cmd_vel_t *out);

/* ── Callback Registration ─────────────────────────────────────────── */

/**
 * @brief  Register (or clear) the velocity-command callback.
 *
 * @param  cb  Function pointer, or NULL to unregister.
 * @return NAV_OK, NAV_ERR_NOT_INIT.
 */
nav_result_t nav_register_cmd_vel_cb(nav_cmd_vel_cb_t cb);

/**
 * @brief  Register (or clear) the state-transition callback.
 *
 * @param  cb  Function pointer, or NULL to unregister.
 * @return NAV_OK, NAV_ERR_NOT_INIT.
 */
nav_result_t nav_register_state_cb(nav_state_cb_t cb);

/**
 * @brief  Register (or clear) the waypoint-reached callback.
 *
 * @param  cb  Function pointer, or NULL to unregister.
 * @return NAV_OK, NAV_ERR_NOT_INIT.
 */
nav_result_t nav_register_goal_cb(nav_goal_cb_t cb);

/* ── Configuration at Runtime ──────────────────────────────────────── */

/**
 * @brief  Update tuning parameters while the module is running.
 *
 * Changes take effect on the next control cycle.  Does NOT trigger a
 * re-plan; call nav_replan() afterwards if the new parameters affect
 * path feasibility (e.g. obstacle_margin).
 *
 * @param  cfg  New parameters (all fields must be valid).
 * @return NAV_OK, NAV_ERR_NOT_INIT, NAV_ERR_INVALID_PARAM.
 */
nav_result_t nav_set_config(const nav_config_t *cfg);

/**
 * @brief  Copy the current active parameters into a caller-owned struct.
 *
 * @param  out  Destination.
 * @return NAV_OK, NAV_ERR_NOT_INIT, NAV_ERR_INVALID_PARAM.
 */
nav_result_t nav_get_config(nav_config_t *out);

#ifdef __cplusplus
}
#endif

#endif /* MODULE_NAV_PATHPLAN_H */
