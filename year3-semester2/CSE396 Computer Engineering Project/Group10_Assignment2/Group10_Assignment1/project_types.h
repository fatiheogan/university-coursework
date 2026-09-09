#ifndef PROJECT_TYPES_H
#define PROJECT_TYPES_H

/**
 * @file    project_types.h
 * @brief   LIDAR-Guided Hospital Navigation Robot -- shared type definitions
 * @author  Group 10 (CSE 396 -- Computer Engineering Project)
 * @date    2026-03-29
 * @version 0.2
 *
 * This header is the single source of truth for every data type exchanged
 * between modules. If a struct or enum crosses a module boundary, it lives
 * here. Module-local types stay in the owning module header.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ===========================================================================
 * MOD-01 <-> MOD-05   Navigation Goal + Status
 * ======================================================================== */

typedef struct {
    char     destination[32];
    uint8_t  waypoint_id;
    uint32_t timestamp_ms;
} nav_goal_t;

typedef enum {
    NAV_STATE_IDLE      = 0,
    NAV_STATE_MOVING    = 1,
    NAV_STATE_WAITING   = 2,
    NAV_STATE_REROUTING = 3,
    NAV_STATE_ARRIVED   = 4,
    NAV_STATE_RETURNING = 5
} nav_state_e;

typedef struct {
    nav_state_e state;
    char        message[64];
    uint32_t    timestamp_ms;
} nav_status_t;

/* ===========================================================================
 * MOD-03 -> MOD-04   Odometry + IMU
 * ======================================================================== */

typedef struct {
    int32_t  left_ticks;
    int32_t  right_ticks;
    float    left_speed_mps;
    float    right_speed_mps;
    uint32_t timestamp_ms;
} odometry_data_t;

typedef struct {
    float    gyro_z_dps;
    float    accel_x_mss;
    float    accel_y_mss;
    float    temp_c;
    uint32_t timestamp_ms;
} imu_data_t;

/* ===========================================================================
 * MOD-04 -> MOD-05, MOD-02   SLAM
 * ======================================================================== */

typedef struct {
    float    x_m;
    float    y_m;
    float    theta_rad;
    float    confidence;
    uint8_t  map_updated;
    uint32_t timestamp_ms;
} slam_state_t;

typedef struct {
    uint8_t  *grid;
    uint16_t  width_cells;
    uint16_t  height_cells;
    float     resolution_m;
    float     origin_x_m;
    float     origin_y_m;
    uint32_t  timestamp_ms;
} occ_map_t;

/* ===========================================================================
 * MOD-05 <-> MOD-03   Motion
 * ======================================================================== */

typedef struct {
    float    left_pwm;
    float    right_pwm;
    uint8_t  stop;
    uint32_t timestamp_ms;
} motion_cmd_t;

typedef struct {
    float    actual_left_mps;
    float    actual_right_mps;
    uint8_t  stall_flag;
    uint32_t timestamp_ms;
} motion_status_t;

/* ===========================================================================
 * MOD-06 -> MOD-04, MOD-05   Sensors
 * ======================================================================== */

typedef enum {
    SENSOR_TYPE_ULTRASONIC  = 0,
    SENSOR_TYPE_IR_OBSTACLE = 1
} sensor_type_e;

typedef struct {
    sensor_type_e type;
    uint8_t       sensor_id;
    float         distance_m;
    uint8_t       obstacle;
    uint32_t      timestamp_ms;
} sensor_reading_t;

typedef struct {
    uint8_t  active;
    float    min_distance_m;
    float    bearing_deg;
    uint32_t timestamp_ms;
} obstacle_alert_t;

/* ===========================================================================
 * MOD-05 <-> MOD-04   Checkpoint handshake
 * ======================================================================== */

typedef enum {
    CP_REQ_SNAP = 0
} checkpoint_req_e;

typedef struct {
    checkpoint_req_e type;
    uint8_t          checkpoint_id;
} checkpoint_req_t;

typedef enum {
    CP_ACK_OK   = 0,
    CP_ACK_FAIL = -1
} checkpoint_ack_e;

typedef struct {
    checkpoint_ack_e result;
    uint8_t          checkpoint_id;
    slam_state_t     snapped_pose;
} checkpoint_ack_t;

/* ===========================================================================
 * MOD-04 / MOD-05 -> MOD-02   Telemetry
 * ======================================================================== */

typedef struct {
    slam_state_t pose;
    nav_status_t nav;
    float        battery_v;
    uint32_t     uptime_ms;
} telemetry_t;

#ifdef __cplusplus
}
#endif

#endif /* PROJECT_TYPES_H */
