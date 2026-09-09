# MOD-05 — Path Planning & Control

> **One-sentence purpose:** MOD-05 plans collision-free paths on the SLAM-produced occupancy grid and drives the robot to target waypoints using a proportional heading controller, with obstacle avoidance and recovery behaviours for safe hospital corridor navigation.

## Authors

| Name                    | Student ID       | Role        |
|-------------------------|------------------|-------------|
| Ertuğrul Ergül          | [240104004994]   | Co-primary  |
| Ömer Faruk Gürsel       | [210104004053]   | Co-primary  |

**Course:** CSE 396 Computer Engineering Project (Group 10)

## Dependencies

### Other modules

| Module | Why it is needed | Direction |
|---|---|---|
| MOD-04 (SLAM) | Supplies `slam_state_t` (pose @ 20 Hz) and `occ_map_t` (occupancy grid on update) | MOD-04 â†’ MOD-05 |
| MOD-03 (Hardware) | Consumes `motion_cmd_t` (PWM @ 50 Hz); supplies `motion_status_t` (actual speeds + stall flag @ 50 Hz) | MOD-05 â†” MOD-03 |
| MOD-06 (Sensors) | Supplies `obstacle_alert_t` (closest obstacle distance + bearing @ up to 20 Hz) | MOD-06 â†’ MOD-05 |
| MOD-01 (Voice) | Supplies `nav_goal_t` (destination waypoint on voice command); consumes `nav_status_t` (TTS announcement on state change) | MOD-01 â†” MOD-05 |
| MOD-02 (Digital Twin) | Consumes `nav_status_t` via `telemetry_t` (WebSocket JSON @ 5â€“10 Hz on port 8765) | MOD-05 â†’ MOD-02 |

### Shared types

| Header | Purpose |
|---|---|
| `project_types.h` | Central shared header at the repository root; defines all inter-module types: `slam_state_t`, `occ_map_t`, `motion_cmd_t`, `motion_status_t`, `obstacle_alert_t`, `nav_goal_t`, `nav_state_e`, `nav_status_t`, `checkpoint_req_t`, `checkpoint_ack_t`, `telemetry_t` |

### External libraries

| Library | Purpose |
|---|---|
| Python 3.11 + `threading` | Control-loop thread and queue-based IPC (Raspberry Pi OS Bookworm 64-bit) |
| NumPy | Costmap inflation and A* grid operations |

---

## Hardware Context

MOD-05 does not own any hardware directly, but its outputs drive the following physical subsystems owned by other modules:

| Subsystem | Owner | Hardware | How MOD-05 interacts |
|---|---|---|---|
| Drive motors | MOD-03 | 2Ã— DC geared motors via L298N H-bridge (50 Hz PWM) | Publishes `motion_cmd_t` (left/right PWM duty-cycle + stop flag) at 50 Hz |
| Wheel encoders | MOD-03 | 20 CPR quadrature encoders (GPIO 17/18, 22/23) | Receives `motion_status_t` (actual speeds, stall flag) at 50 Hz |
| LiDAR | MOD-04 | RPLidar A2 (360Â°, 8 m range, USB) | Indirectly â€” consumes the occupancy grid (`occ_map_t`) built from scans |
| IMU | MOD-03 | MPU-6050 on IÂ²C @ 400 kHz (addr 0x68) | Indirectly â€” pose heading fused by MOD-04 SLAM |
| Proximity sensors | MOD-06 | 2Ã— HC-SR04 ultrasonic + 2Ã— digital IR | Receives `obstacle_alert_t` with min distance and bearing |
| Power | MOD-03 | LiPo 11.1 V / 2200 mAh via UBEC | MOD-05 should monitor `battery_v` in `telemetry_t` for low-battery shutdown |

---

## Data Types Produced

| Type | Destination | Rate | Key fields |
|---|---|---|---|
| `motion_cmd_t` | MOD-03 | 50 Hz | `left_pwm`, `right_pwm` (normalised 1.0 +1.0), `stop`, `timestamp_ms` |
| `nav_status_t` | MOD-01, MOD-02 | On FSM state change | `state` (nav_state_e), `message` (Turkish TTS string), `timestamp_ms` |
| `checkpoint_req_t` | MOD-04 | On junction arrival | `type` (CP_REQ_SNAP), `checkpoint_id` |
| `nav_cmd_vel_t` | Internal | 50 Hz | `linear_v`, `angular_w`, `timestamp` converted to `motion_cmd_t` before publishing |
| `nav_progress_t` | Any (via polling) | On request | `state`, `waypoints_total`, `waypoints_done`, `dist_to_goal`, `path_length` |

## Data Types Consumed

| Type | Source | Rate | Notes |
|---|---|---|---|
| `slam_state_t` | MOD-04 | 20 Hz | `x_m`, `y_m`, `theta_rad`, `confidence`; skip reroute if `confidence < 0.80` |
| `occ_map_t` | MOD-04 | On map update | Row-major grid pointer; 0=free, 100=occupied, 255=unknown; 5 cm/cell; copy before releasing lock |
| `motion_status_t` | MOD-03 | 50 Hz | `actual_left_mps`, `actual_right_mps`, `stall_flag`; stall triggers WAITING transition |
| `obstacle_alert_t` | MOD-06 | Up to 20 Hz | `active`, `min_distance_m`, `bearing_deg` (NaN = unknown); triggers MOVING â†’ WAITING |
| `nav_goal_t` | MOD-01 | On voice command | `destination` (UTF-8 name), `waypoint_id` (grid index), `timestamp_ms` |
| `checkpoint_ack_t` | MOD-04 | On checkpoint response | `result` (CP_ACK_OK / CP_ACK_FAIL), `snapped_pose`; call `slam_reset_pose()` on OK |

---

## Quick-Start Integration

```c
#include "nav_pathplan.h"

/* 1. (Optional) Prepare custom tuning â€” or pass NULL for defaults */
nav_config_t cfg = {
    .lookahead_m     = 0.40f,
    .max_linear_v    = 0.3f,       /* well within HW_MAX_SPEED_MPS (0.5 m/s) */
    .max_angular_w   = 1.2f,
    .goal_xy_tol     = 0.05f,
    .goal_yaw_tol    = 0.10f,
    .obstacle_margin = 0.15f,
    .stuck_timeout_ms = 3000
};

/* 2. Initialise the navigation module */
if (nav_init(&cfg) != NAV_OK) {
    /* handle init failure */
}

/* 3. Register callback so MOD-03 receives velocity commands */
nav_register_cmd_vel_cb(motor_on_cmd_vel);

/* 4. Send the robot to a waypoint (e.g. from a nav_goal_t voice command) */
nav_waypoint_t goal = { .x = 2.0f, .y = 1.5f, .yaw = 0.0f, .has_yaw = false };
nav_result_t ret = nav_goto(&goal);

if (ret == NAV_ERR_NO_PATH) {
    /* target is unreachable in the current occupancy grid */
}

/* 5. Poll progress (or use the state callback instead) */
nav_progress_t prog;
nav_get_progress(&prog);

/* 6. Checkpoint drift correction at a corridor junction */
checkpoint_req_t req = { .type = CP_REQ_SNAP, .checkpoint_id = 3 };
checkpoint_ack_t ack;
slam_process_checkpoint(&req, &ack);
if (ack.result == CP_ACK_OK) {
    slam_reset_pose(ack.snapped_pose.x_m,
                    ack.snapped_pose.y_m,
                    ack.snapped_pose.theta_rad);
    nav_replan();
}

/* 7. When finished, tear down */
nav_deinit();
```

---

## API Summary

### Lifecycle

| Function | Parameters | Return |
|---|---|---|
| `nav_init` | `const nav_config_t *cfg` (or `NULL`) | `nav_result_t` â€” `NAV_OK` or error |
| `nav_deinit` | `void` | `void` |

### Goal Management

| Function | Parameters | Return |
|---|---|---|
| `nav_goto` | `const nav_waypoint_t *wp` | `nav_result_t` |
| `nav_goto_multi` | `const nav_waypoint_t *wps, uint8_t count` | `nav_result_t` |
| `nav_cancel` | `void` | `nav_result_t` |

### Runtime Control

| Function | Parameters | Return |
|---|---|---|
| `nav_pause` | `void` | `nav_result_t` |
| `nav_resume` | `void` | `nav_result_t` |
| `nav_replan` | `void` | `nav_result_t` |

### Status & Telemetry

| Function | Parameters | Return |
|---|---|---|
| `nav_get_state` | `void` | `nav_runtime_state_t` |
| `nav_get_progress` | `nav_progress_t *out` | `nav_result_t` |
| `nav_get_cmd_vel` | `nav_cmd_vel_t *out` | `nav_result_t` |

### Callback Registration

| Function | Parameters | Return |
|---|---|---|
| `nav_register_cmd_vel_cb` | `nav_cmd_vel_cb_t cb` (or `NULL`) | `nav_result_t` |
| `nav_register_state_cb` | `nav_state_cb_t cb` (or `NULL`) | `nav_result_t` |
| `nav_register_goal_cb` | `nav_goal_cb_t cb` (or `NULL`) | `nav_result_t` |

### Configuration

| Function | Parameters | Return |
|---|---|---|
| `nav_set_config` | `const nav_config_t *cfg` | `nav_result_t` |
| `nav_get_config` | `nav_config_t *out` | `nav_result_t` |

### Constants & Macros

| Macro | Value | Meaning |
|---|---|---|
| `NAV_MAX_WAYPOINTS` | `64` | Max waypoints per mission |
| `NAV_PLANNER_MAX_NODES` | `8192` | A* expansion limit per planning call |
| `NAV_LOOKAHEAD_M` | `0.35f` | Pure-pursuit look-ahead distance (m) |
| `NAV_GOAL_XY_TOL_M` | `0.05f` | Position goal tolerance (m) |
| `NAV_GOAL_YAW_TOL_RAD` | `0.10f` | Heading goal tolerance (rad) |
| `NAV_STUCK_TIMEOUT_MS` | `3000` | No-progress timer before stuck detection (ms) |
| `NAV_OBSTACLE_MARGIN_M` | `0.15f` | Safety inflation around robot footprint (m) |
| `NAV_MIN_CLEARANCE_M` | `0.20f` | Minimum cell clearance for planner (m) |
| `NAV_CONTROL_PERIOD_MS` | `50` | Controller loop period â€” matches MOD-03 rate (ms) |
| `NAV_VERSION` | `"0.1"` | Interface version string |

---

## Known Limitations & TODOs

- **RISK-03  Proportional controller may oscillate in wide corridors:** The heading controller uses a fixed Kp = 1.2 with no derivative term.  In wide open areas the robot may weave.  A PD or PID upgrade is planned before Week 7 integration, but requires empirical tuning on the physical platform.

- **RISK-05  Waypoint-ID mapping not finalised:** The `nav_goal_t.waypoint_id` lookup depends on the completed hospital floor map from MOD-04.  Interim solution: hard-coded JSON waypoint file.  Coordinate system and ID scheme must be locked down during Week 5 initial mapping.

- **RISK-07  Stuck recovery is basic:** Stuck detection triggers a simple back-and-rotate sequence.  If the robot is boxed in (narrow dead-end), this may not be enough.  More sophisticated behaviours (wall-following, spiral search) are TODO.

- **RISK-08  HC-SR04 bearing uncertainty:** `obstacle_alert_t.bearing_deg` may be NaN due to the ultrasonic beam angle (~30Â°).  MOD-05 must check `isnan(bearing_deg)` before using bearing for avoidance direction â€” fallback is to stop and wait rather than swerve blindly.

- **RISK-09  Occupancy grid may lag behind physical world:** The planner relies on `occ_map_t` from MOD-04, which is updated only when LiDAR scans detect changes.  Fast-moving obstacles (people walking) may not be reflected in the grid in time; MOD-06 `obstacle_alert_t` is the primary safety net for reactive avoidance.

- **TODO:** Thread safety the control loop and callbacks are assumed to run from a single context.  Mutexes will be added if MOD-03 / MOD-06 push data from separate threads.

- **TODO:** `motion_cmd_t` push mechanism (shared `queue.Queue`) integration with MOD-03 is defined in the interface contract but not yet wired in this header.  Will be finalised in v0.2.

---

